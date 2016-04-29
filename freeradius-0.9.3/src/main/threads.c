/*
 * threads.c	request threading support
 *
 * Version:	$Id: threads.c,v 1.63.2.2 2003/07/30 15:46:22 aland Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2000  The FreeRADIUS server project
 * Copyright 2000  Alan DeKok <aland@ox.org>
 */

#include "libradius.h"

#if HAVE_PTHREAD_H

#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include "radiusd.h"
#include "rad_assert.h"
#include "conffile.h"

static const char rcsid[] =
"$Id: threads.c,v 1.63.2.2 2003/07/30 15:46:22 aland Exp $";

#define SEMAPHORE_LOCKED	(0)
#define SEMAPHORE_UNLOCKED	(1)

#define THREAD_RUNNING		(1)
#define THREAD_CANCELLED	(2)
#define THREAD_EXITED		(3)

/*
 *	Prototype to shut the compiler up.
 */
int rad_spawn_child(REQUEST *request, RAD_REQUEST_FUNP fun);

/*
 *  A data structure which contains the information about
 *  the current thread.
 *
 *  pthread_id     pthread id
 *  thread_num     server thread number, 1...number of threads
 *  semaphore     used to block the thread until a request comes in
 *  status        is the thread running or exited?
 *  request_count the number of requests that this thread has handled
 *  timestamp     when the thread started executing.
 *  request       the current request that the thread is processing.
 *  fun           the function which is handling the request.
 */
typedef struct THREAD_HANDLE {
	struct THREAD_HANDLE *prev;
	struct THREAD_HANDLE *next;
	pthread_t            pthread_id;
	int                  thread_num;
	sem_t                semaphore;
	int                  status;
	unsigned int         request_count;
	time_t               timestamp;
	REQUEST              *request;
	RAD_REQUEST_FUNP     fun;
} THREAD_HANDLE;

/*
 *	A data structure to manage the thread pool.  There's no real
 *	need for a data structure, but it makes things conceptually
 *	easier.
 */
typedef struct THREAD_POOL {
	THREAD_HANDLE *head;
	THREAD_HANDLE *tail;

	THREAD_HANDLE *next_pick;
	
	int total_threads;
	int max_thread_num;
	int start_threads;
	int max_threads;
	int min_spare_threads;
	int max_spare_threads;
	int max_requests_per_thread;
	unsigned long request_count;
	time_t time_last_spawned;
	int cleanup_delay;
} THREAD_POOL;

static THREAD_POOL thread_pool;
static int pool_initialized = FALSE;

/*
 *	Data structure to keep track of which child forked which
 *	request.  If we cared, we'd keep a list of "free" and "active"
 *	entries.
 *
 *	FIXME: Have a time out, so we clean up entries which haven't
 *	been picked up!
 */
typedef struct rad_fork_t {
	pthread_t	thread_id;
	pid_t		child_pid;
	sem_t	 	child_done;
	int		status;	/* exit status of the child */
	time_t		time_forked;
} rad_fork_t;

/*
 *  This MUST be a power of 2 for it to work properly!
 */
#define NUM_FORKERS (8192)
static rad_fork_t forkers[NUM_FORKERS];

/*
 *	This mutex ensures that only one thread is doing certain
 *	kinds of magic to the previous array.
 */
static pthread_mutex_t fork_mutex;


/*
 *	A mapping of configuration file names to internal integers
 */
static const CONF_PARSER thread_config[] = {
	{ "start_servers",           PW_TYPE_INTEGER, 0, &thread_pool.start_threads,           "5" }, 
	{ "max_servers",             PW_TYPE_INTEGER, 0, &thread_pool.max_threads,             "32" }, 
	{ "min_spare_servers",       PW_TYPE_INTEGER, 0, &thread_pool.min_spare_threads,       "3" }, 
	{ "max_spare_servers",       PW_TYPE_INTEGER, 0, &thread_pool.max_spare_threads,       "10" }, 
	{ "max_requests_per_server", PW_TYPE_INTEGER, 0, &thread_pool.max_requests_per_thread, "0" }, 
	{ "cleanup_delay",           PW_TYPE_INTEGER, 0, &thread_pool.cleanup_delay,           "5" }, 
	{ NULL, -1, 0, NULL, NULL }
};

/*
 *	The main thread handler for requests.
 *
 *	Wait on the semaphore until we have it, and process the request.
 */
static void *request_handler_thread(void *arg)
{
	THREAD_HANDLE	*self = (THREAD_HANDLE *) arg;
#if HAVE_PTHREAD_SIGMASK
	sigset_t set;

	/*
	 *	Block SIGHUP handling for the child threads.
	 *
	 *	This ensures that only the main server thread will
	 *	process HUP signals.
	 *
	 *	If we don't have sigprocmask, then it shouldn't be
	 *	a problem, either, as the sig_hup handler should check
	 *	for this condition.
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &set, NULL);
#endif
	
	/*
	 *	Loop forever, until told to exit.
	 */
	for (;;) {
		/*
		 *	Wait to be signalled.
		 */
		DEBUG2("Thread %d waiting to be assigned a request",
		       self->thread_num);
	re_wait:
		if (sem_wait(&self->semaphore) != 0) {
			/*
			 *	Interrupted system call.  Go back to
			 *	waiting, but DON'T print out any more
			 *	text.
			 */
			if (errno == EINTR) {
				goto re_wait;
			}
			radlog(L_ERR, "Thread %d failed waiting for semaphore: %s: Exiting\n",
			       self->thread_num, strerror(errno));
			break;
		}

		/*
		 *	If we've been told to kill ourselves,
		 *	then exit politely.
		 */
		if (self->status == THREAD_CANCELLED) {
			DEBUG2("Thread %d exiting on request from parent.",
					self->thread_num);
			break;
		}

		/*
		 *	Stupid implementations of sem_wait return on
		 *	signals, but don't return -1.
		 */
		if (!self->request) {
			continue;
		}

		DEBUG2("Thread %d handling request %d, (%d handled so far)",
				self->thread_num, self->request->number,
				self->request_count);
		
		/*
		 *  This responds, and resets request->child_pid
		 */
		rad_respond(self->request, self->fun);

		self->fun = NULL;
		self->request = NULL;
	}

	DEBUG2("Thread %d exiting...", self->thread_num);

	/*
	 *  Do this as the LAST thing before exiting.
	 */
	self->status = THREAD_EXITED;

	return NULL;
}

/*
 *	Take a THREAD_HANDLE, and delete it from the thread pool.
 *
 *	This function is called ONLY from the main server thread.
 */
static void delete_thread(THREAD_HANDLE *handle)
{
	THREAD_HANDLE *prev;
	THREAD_HANDLE *next;

	rad_assert(handle->request == NULL);

	prev = handle->prev;
	next = handle->next;
	rad_assert(thread_pool.total_threads > 0);
	thread_pool.total_threads--;

	/*
	 *	Remove the handle from the list.
	 */
	if (prev == NULL) {
		rad_assert(thread_pool.head == handle);
		thread_pool.head = next;
	} else {
		prev->next = next;
	}
  
	if (next == NULL) {
		rad_assert(thread_pool.tail == handle);
		thread_pool.tail = prev;
	} else {
		next->prev = prev;
	}

	/*
	 *	Ensure that deleted threads aren't picked.
	 */
	if (thread_pool.next_pick == handle) {
		thread_pool.next_pick = next;
	}

	DEBUG2("Deleting thread %d", handle->thread_num);

	/*
	 *	This thread has exited.  Delete any additional
	 *	resources associated with it (semaphore, etc).
	 */
	sem_destroy(&handle->semaphore);

	/*
	 *	Free the memory, now that we're sure the thread
	 *	exited.
	 */
	free(handle);
}


/*
 *	Spawn a new thread, and place it in the thread pool.
 *
 *	The thread is started initially in the blocked state, waiting
 *	for the semaphore.
 */
static THREAD_HANDLE *spawn_thread(time_t now)
{
	int rcode;
	THREAD_HANDLE *handle;
	pthread_attr_t attr;

	/*
	 *	Ensure that we don't spawn too many threads.
	 */
	if (thread_pool.total_threads >= thread_pool.max_threads) {
		DEBUG2("Thread spawn failed.  Maximum number of threads (%d) already running.", thread_pool.max_threads);
		return NULL;
	}

	/*
	 *	Allocate a new thread handle.
	 */
	handle = (THREAD_HANDLE *) rad_malloc(sizeof(THREAD_HANDLE));
	memset(handle, 0, sizeof(THREAD_HANDLE));
	handle->prev = NULL;
	handle->next = NULL;
	handle->pthread_id = NO_SUCH_CHILD_PID;
	handle->thread_num = thread_pool.max_thread_num++;
	handle->request_count = 0;
	handle->status = THREAD_RUNNING;
	handle->timestamp = time(NULL);

	/*
	 *	Initialize the semaphore to be for this process only,
	 *	and locked, so that the thread block until the server
	 *	kicks it.
	 */
	rcode = sem_init(&handle->semaphore, 0, SEMAPHORE_LOCKED);
	if (rcode != 0) {
		radlog(L_ERR|L_CONS, "FATAL: Failed to initialize semaphore: %s",
				strerror(errno));
		exit(1);
	}

	/*
	 *	The thread isn't currently handling a request.
	 */
	handle->request = NULL;
	handle->fun = NULL;

	/*
	 *	Initialize the thread's attributes to detached.
	 *
	 *	We could call pthread_detach() later, but if the thread
	 *	exits between the create & detach calls, it will need to
	 *	be joined, which will never happen.
	 */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	/*
	 *	Create the thread detached, so that it cleans up it's
	 *	own memory when it exits.
	 *
	 *	Note that the function returns non-zero on error, NOT
	 *	-1.  The return code is the error, and errno isn't set.
	 */
	rcode = pthread_create(&handle->pthread_id, &attr,
			request_handler_thread, handle);
	if (rcode != 0) {
		radlog(L_ERR|L_CONS, "FATAL: Thread create failed: %s",
		       strerror(rcode));
		exit(1);
	}
	pthread_attr_destroy(&attr);

	/*
	 *	One more thread to go into the list.
	 */
	thread_pool.total_threads++;
	DEBUG2("Thread spawned new child %d. Total threads in pool: %d",
			handle->thread_num, thread_pool.total_threads);

	/*
	 *	Add the thread handle to the tail of the thread pool list.
	 */
	if (thread_pool.tail) {
		thread_pool.tail->next = handle;
		handle->prev = thread_pool.tail;
		thread_pool.tail = handle;
	} else {
		rad_assert(thread_pool.head == NULL);
		thread_pool.head = thread_pool.tail = handle;
	}

	/*
	 *	Update the time we last spawned a thread.
	 */
	thread_pool.time_last_spawned = now;

	/*
	 *	And return the new handle to the caller.
	 */
	return handle;
}

/*
 *      Temporary function to prevent server from executing a SIGHUP
 *      until all threads are finished handling requests.  This returns
 *      the number of active threads to 'radiusd.c'.
 */
int total_active_threads(void) 
{
        int rcode = 0;
	THREAD_HANDLE *handle;
	
	for (handle = thread_pool.head; handle != NULL; handle = handle->next){
		if (handle->request != NULL) {
			rcode ++;
		}
	}
	return (rcode);
}

/*
 *	Allocate the thread pool, and seed it with an initial number
 *	of threads.
 *
 *	FIXME: What to do on a SIGHUP???
 */
int thread_pool_init(void)
{
	int i;
	THREAD_HANDLE	*handle;
	CONF_SECTION	*pool_cf;
	time_t		now;

	DEBUG("Initializing the thread pool...");
	now = time(NULL);

	/*
	 *	After a SIGHUP, we don't over-write the previous values.
	 */
	if (!pool_initialized) {
		/*
		 *	Initialize the thread pool to some reasonable values.
		 */
		memset(&thread_pool, 0, sizeof(THREAD_POOL));
		thread_pool.head = NULL;
		thread_pool.tail = NULL;
		thread_pool.next_pick = NULL;
		thread_pool.total_threads = 0;
		thread_pool.max_thread_num = 1;
		thread_pool.cleanup_delay = 5;
	}

	pool_cf = cf_section_find("thread");
	if (pool_cf != NULL) {
		cf_section_parse(pool_cf, NULL, thread_config);
	}

	/*
	 *	Limit the maximum number of threads to the maximum
	 *	number of forks we can do.
	 *
	 *	FIXME: Make this code better...
	 */
	if (thread_pool.max_threads >= NUM_FORKERS) {
		thread_pool.max_threads = NUM_FORKERS;
	}


	/*
	 *	The pool has already been initialized.  Don't spawn
	 *	new threads, and don't forget about forked children,
	 */
	if (pool_initialized) {
		return 0;
	}

	/*
	 *	Create a number of waiting threads.
	 *
	 *	If we fail while creating them, do something intelligent.
	 */
	for (i = 0; i < thread_pool.start_threads; i++) {
		handle = spawn_thread(now);
		if (handle == NULL) {
			return -1;
		}
	}

	pool_initialized = TRUE;
	return 0;
}

/*
 *	Assign a new request to a free thread.
 *
 *	If there isn't a free thread, then try to create a new one,
 *	up to the configured limits.
 */
int rad_spawn_child(REQUEST *request, RAD_REQUEST_FUNP fun)
{
	THREAD_HANDLE *handle;
	THREAD_HANDLE *found;
	THREAD_HANDLE *start;

	/*
	 *	Loop over the active thread pool, looking for a
	 *	waiting thread.
	 */
	found = NULL;

	/*
	 *	Minimize the time spent searching for an empty thread,
	 *	by starting off from the last one we found.  This also
	 *	helps to span requests across multiple threads.
	 */
	start = thread_pool.next_pick;
	if (!start) start = thread_pool.head;
	
	handle = start;
	while (handle) {
		/*
		 *	Ignore threads which aren't running.
		 */
		if (handle->status != THREAD_RUNNING) {
			goto next_handle;
		}

		/*
		 *	If we haven't found a free thread yet, then
		 *	check it's semaphore lock.  We don't lock it,
		 *	so if it's locked, then the thread MUST be the
		 *	one locking it, waiting for us to unlock it.
		 */
		if (handle->request == NULL) {
			found = handle;
			break;
		}

	next_handle:
		/*
		 *	There is a 'next', use it.
		 *
		 *	Otherwise, go back to the head of the list.
		 */
		if (handle->next) {
			handle = handle->next;
		} else {
			handle = thread_pool.head;
		}

		/*
		 *	Stop if we've looped back to the beginning.
		 */
		if (handle == start) break;
	} /* loop over all of the threads */

	/*
	 *	If we haven't found an active thread, then spawn a new one.
	 *
	 *	If we can't spawn a new one, complain, and exit.
	 */
	if (!found) {
		found = spawn_thread(request->timestamp);
		if (found == NULL) {
			radlog(L_INFO, 
					"The maximum number of threads (%d) are active, cannot spawn new thread to handle request", 
					thread_pool.max_threads);
			return -1;
		}
	}

	/*
	 *	Remember where we left off, so that we can start the
	 *	search from here the next time.
	 */
	thread_pool.next_pick = found->next;

	/*
	 *	OK, now 'handle' points to a waiting thread.  We move
	 *	it to the tail of the thread pool, so that we can
	 *	cycle among the threads.
	 *
	 *	We then give it the request, signal its semaphore, and
	 *	return.  The thread eventually wakes up, and handles
	 *	the request.
	 */
	DEBUG2("Thread %d assigned request %d", found->thread_num, request->number);
	request->child_pid = found->pthread_id;

	/*
	 *	Tell the child thread what to do.
	 */
	found->request = request;
	found->fun = fun;
	found->request_count++;
	thread_pool.request_count++;

	/*
	 * 	Now that the child has things to do, kick it.
	 */
	sem_post(&found->semaphore);

	return 0;
}

/*
 *	Check the min_spare_threads and max_spare_threads.
 *
 *	If there are too many or too few threads waiting, then we
 *	either create some more, or delete some.
 */
int thread_pool_clean(time_t now)
{
	int spare;
	int i, total;
	THREAD_HANDLE *handle, *next;
	int active_threads;

	/*
	 *	Loop over the thread pool, incrementing the 'active'
	 *	counter if the thread is currently processing a request.
	 *
	 *	At this point, we also delete 'exited' threads.
	 */
	active_threads = 0;
	for (handle = thread_pool.head; handle; handle = next) {
		next = handle->next;

		if (handle->request != NULL) {
			active_threads++;
			continue;
		}

		/*
		 *	Handle is NULL, so it's not processing anything.
		 *
		 *	Maybe we've asked the thread to exit, and it
		 *	has agreed.
		 */
		if (handle->status == THREAD_EXITED) {
			delete_thread(handle);
		}
	}

	spare = thread_pool.total_threads - active_threads;
	if (debug_flag) {
		static int old_total = -1;
		static int old_active = -1;

		if ((old_total != thread_pool.total_threads) ||
				(old_active != active_threads)) {
			DEBUG2("Threads: total/active/spare threads = %d/%d/%d",
					thread_pool.total_threads, active_threads, spare);
			old_total = thread_pool.total_threads;
			old_active = active_threads;
		}
	}

	/*
	 *	If there are too few spare threads, create some more.
	 */
	if (spare < thread_pool.min_spare_threads) {
		total = thread_pool.min_spare_threads - spare;

		DEBUG2("Threads: Spawning %d spares", total);
		/*
		 *	Create a number of spare threads.
		 */
		for (i = 0; i < total; i++) {
			handle = spawn_thread(now);
			if (handle == NULL) {
				return -1;
			}
		}

		/*
		 *	And exit, as there can't be too many spare threads.
		 */
		return 0;
	}

	/*
	 *	Only delete the spare threads if sufficient time has
	 *	passed since we last created one.  This helps to minimize
	 *	the amount of create/delete cycles.
	 */
	if ((now - thread_pool.time_last_spawned) < thread_pool.cleanup_delay) {
		return 0;
	}

	/*
	 *	If there are too many spare threads, delete one.
	 *
	 *	Note that we only delete ONE at a time, instead of
	 *	wiping out many.  This allows the excess servers to
	 *	be slowly reaped, just in case the load spike comes again.
	 */
	if (spare > thread_pool.max_spare_threads) {

		spare -= thread_pool.max_spare_threads;

		DEBUG2("Threads: deleting 1 spare out of %d spares", spare);

		/*
		 *	Walk through the thread pool, deleting the
		 *	first N idle threads we come across.
		 */
		for (handle = thread_pool.head; (handle != NULL) && (spare > 0) ; handle = next) {
			next = handle->next;

			/*
			 *	If the thread is not handling a
			 *	request, then tell it to exit.
			 *
			 *	Note that we delete it from the thread
			 *	pool BEFORE telling it to kill itself,
			 *	as the child thread can free the 'handle'
			 *	structure, without anyone else using it.
			 */
			if (handle->request == NULL) {
				handle->status = THREAD_CANCELLED;

				sem_post(&handle->semaphore);
				spare--;
				break;
			}
		}
	}

	/*
	 *	If the thread has handled too many requests, then make it
	 *	exit.
	 */
	if (thread_pool.max_requests_per_thread > 0) {
		for (handle = thread_pool.head; handle; handle = next) {
			next = handle->next;

			/*
			 *	Not handling a request, we can kill it.
			 */
			if ((handle->request == NULL) &&
			    (handle->request_count > thread_pool.max_requests_per_thread)) {
				handle->status = THREAD_CANCELLED;
				sem_post(&handle->semaphore);
			}
		}
	}
  
	/*
	 *	Otherwise everything's kosher.  There are not too few,
	 *	or too many spare threads.  Exit happily.
	 */
	return 0;
}

static int exec_initialized = FALSE;

/*
 *	Initialize the stuff for keeping track of child processes.
 */
void rad_exec_init(void)
{
	int i;
	
	/*
	 *	Initialize the mutex used to remember calls to fork.
	 */
	pthread_mutex_init(&fork_mutex, NULL);
	
	/*
	 *	Initialize the data structure where we remember the
	 *	mappings of thread ID && child PID to exit status.
	 */
	for (i = 0; i < NUM_FORKERS; i++) {
		forkers[i].thread_id = NO_SUCH_CHILD_PID;
		forkers[i].child_pid = -1;
		forkers[i].status = 0;
	}

	exec_initialized = TRUE;
}

/*
 *	We use the PID number as a base for the array index, so that
 *	we can quickly turn the PID into a free array entry, instead
 *	of rooting blindly through the entire array.
 */
#define PID_2_ARRAY(pid) (((int) pid ) & (NUM_FORKERS - 1))

/*
 *	Thread wrapper for fork().
 */
pid_t rad_fork(int exec_wait)
{
	sigset_t set;
	pid_t child_pid;	

	/*
	 *	The thread is NOT interested in waiting for the exit
	 *	status of the child process, so we don't bother
	 *	updating our kludgy array.
	 *
	 *	Or, there no NO threads, so we can just do the fork
	 *	thing.
	 */
	if (!exec_wait || !exec_initialized) {
		return fork();
	}

	/*
	 *	Block SIGCLHD until such time as we've saved the PID.
	 *
	 *	Note that we block SIGCHLD for ALL threads associated
	 *	with this process!  This is to prevent race conditions!
	 */
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigprocmask(SIG_BLOCK, &set, NULL);

	/*
	 *	Do the fork.
	 */
	child_pid = fork();

	/*
	 *	We managed to fork.  Let's see if we have a free
	 *	array entry.
	 */
	if (child_pid > 0) { /* parent */
		int i;
		int found;
		time_t now = time(NULL);

		/*
		 *	We store the information in the array
		 *	indexed by PID.  This means that we have
		 *	on average an O(1) lookup to find the element,
		 *	instead of rooting through the entire array.
		 */
		i = PID_2_ARRAY(child_pid);
		found = -1;
		
		/*
		 *	We may have multiple threads trying to find an
		 *	empty position, so we lock the array until
		 *	we've found an entry.
		 */
		pthread_mutex_lock(&fork_mutex);
		do {
			if (forkers[i].thread_id == NO_SUCH_CHILD_PID) {
				found = i;
				break;
			}

			/*
			 *	Clean up any stale forked sessions.
			 *
			 *	This sometimes happens, for crazy reasons.
			 */
			if ((now - forkers[i].time_forked) > 30) {
				forkers[i].thread_id = NO_SUCH_CHILD_PID;

				/*
				 *	Grab the child's exit condition,
				 *	just in case...
				 */
				waitpid(forkers[i].child_pid,
					&forkers[i].status,
					WNOHANG);
				sem_destroy(&forkers[i].child_done);
				found = i;
				break;
			}

			/*
			 *  Increment it, within the array.
			 */
			i++;
			i &= (NUM_FORKERS - 1);
		} while (i != PID_2_ARRAY(child_pid));
		pthread_mutex_unlock(&fork_mutex);

		/*
		 *	Arg.  We did a fork, and there was nowhere to
		 *	put the answer.
		 */
		if (found < 0) {
			return (pid_t) -1;
		}

		/*
		 *	In the parent, set the status, and create the
		 *	semaphore.
		 */
		forkers[found].status = -1;
		forkers[found].child_pid = child_pid;
		forkers[i].thread_id = pthread_self();
		forkers[i].time_forked = now;
		sem_init(&forkers[found].child_done, 0, SEMAPHORE_LOCKED);
	}

	/*
	 *	Unblock SIGCHLD, now that there's no chance of bad entries
	 *	in the array.
	 */
	sigprocmask(SIG_UNBLOCK, &set, NULL);

	/*
	 *	Return whatever we were told.
	 */
	return child_pid;
}

/*
 *	Thread wrapper for waitpid(), so threads can wait for
 *	the PID they forked.
 */
pid_t rad_waitpid(pid_t pid, int *status, int options)
{
	int i, rcode;
	int found;
	pthread_t self = pthread_self();

	/*
	 *	We're only allowed to wait for a SPECIFIC pid.
	 */
	if (pid <= 0) {
		return -1;
	}

	/*
	 *	Find the PID to wait for, starting at an index within
	 *	the array.  This makes the lookups O(1) on average,
	 *	instead of O(n), when the array is filling up.
	 */
	found = -1;
	i = PID_2_ARRAY(pid);
	do {
		/*
		 *	We were the ones who forked this specific
		 *	child.
		 */
		if ((forkers[i].thread_id == self) &&
		    (forkers[i].child_pid == pid)) {
			found = i;
			break;
		}

		i++;
		i &= (NUM_FORKERS - 1);
	} while (i != PID_2_ARRAY(pid));

	/*
	 *	No thread ID found: we're trying to wait for a child
	 *	we've never forked!
	 */
	if (found < 0) {
		return -1;
	}

	/*
	 *	Wait for the signal that the child's status has been
	 *	returned.
	 */
	if (options == WNOHANG) {
		rcode = sem_trywait(&forkers[found].child_done);
		if (rcode != 0) {
			return 0; /* no child available */
		}
	} else {		/* wait forever */
	re_wait:
		rcode = sem_wait(&forkers[found].child_done);
		if ((rcode != 0) && (errno == EINTR)) {
			goto re_wait;
		}
	}

	/*
	 *	We've got the semaphore.  Now destroy it.
	 *
	 *	FIXME: Maybe we want to set up the semaphores in advance,
	 *	to prevent the creation && deletion of lots of them,
	 *	if creating and deleting them is expensive.
	 */
	sem_destroy(&forkers[found].child_done);

	/*
	 *	Save the status BEFORE we re-set the thread ID.
	 */
	*status = forkers[found].status;

	/*
	 *	This next line taints the other array entries,
	 *	due to other threads re-using the data structure.
	 */
	forkers[found].thread_id = NO_SUCH_CHILD_PID;

	return pid;
}

/*
 *	Called by the main signal handler, to save the status of the child
 */
int rad_savepid(pid_t pid, int status)
{
	int i;

	/*
	 *	Find the PID to wait for, starting at an index within
	 *	the array.  This makes the lookups O(1) on average,
	 *	instead of O(n), when the array is filling up.
	 */
	i = PID_2_ARRAY(pid);

	/*
	 *	Do NOT lock the array, as nothing else sets the
	 *	status and posts the semaphore.
	 */
	do {
		/*
		 *	Any thread can get the sigchild...
		 */
		if ((forkers[i].thread_id != NO_SUCH_CHILD_PID) &&
		    (forkers[i].child_pid == pid)) {
			/*
			 *	Save the status, THEN post the
			 *	semaphore.
			 */
			forkers[i].status = status;
			sem_post(&forkers[i].child_done);

			/*
			 *	FIXME: If the child is more than 60
			 *	seconds out of date, then delete it.
			 *
			 *	That is, we've forked, and the forker
			 *	is waiting nearly forever
			 */
			return 0;		
		}

		i++;
		i &= (NUM_FORKERS - 1);
	} while (i != PID_2_ARRAY(pid));

	return -1;
}
#endif
