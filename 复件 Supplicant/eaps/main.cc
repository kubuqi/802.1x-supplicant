// 802.1x client implementation
//
char const _8021x_main_rcsid[] = "$Id: main.cc,v 1.45 2002/10/16 13:34:09 wcvs Exp $";

//*  includes
//
extern "C" {

#ifdef WIN32
#else
#	include <pwd.h>    
#	include <syslog.h>
#	include <unistd.h>
#include <sys/wait.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
}

#include "comm.hh"
#include "error.hh"
#include "password.hh"
#include "tls.hh"

#include "eap.hh"
#include "eapol.hh"
#include "type_identity.hh"
#include "type_notification.hh"
#include "type_md5.hh"
#include "type_tls.hh"

#include "ll_802_3.hh"

//*  constants
//**  network access
//
static char const *DEFAULT_DEV = "eth0";
static size_t DEFAULT_MTU = 1500;

//**  TLS environment variables
//
static char const *CA_ENV = "EAPD_CA";
static char const *CERT_ENV = "EAPD_CERT";
static char const *FRAGS_ENV = "EAPD_FRAGS";
static char const *RAND_ENV = "EAP_RAND";

//**  misc TLS defaults
//
static char const *RAND_DEFAULT = "/dev/urandom";
static size_t const FRAGS_DEFAULT = 200; // avoid Catalyst 3550 bug

//**  authentication states
//
enum {
    INIT,
    DO_AUTH,
    DID_AUTH
};

//**  misc
//
#ifdef WIN32
#define LOG_USER	0
#endif

static unsigned const MOLEST_TIMEOUT = 20;
static int const LOG_FAC = LOG_USER;
static int const FD_CLOSE_MAX = 64*1024;

//*  variables
//
static eap::eap auth;
static eap::ll_eapol *io;
static char const *name;
static int send_eapol_start, single_shot, state, do_logoff = 1;

#ifdef WIN32
typedef int pid_t;
#endif

static pid_t molestor = -1;

extern char **environ;

//*  functions
//**  generic
//
#ifdef WIN32
	static void daemonize()	{}
	static void rip_child(int signo){}
	static void install_ripper(){}
	static void terminate(int signo){}
	static void block_fatal(int on){}
	static void install_terminate(){}
	static void do_abort() {}

	typedef int uid_t;
	typedef int gid_t;
	static uid_t getuid() { return 0;}
	static void setuid(uid_t t) {}
	static gid_t getgid() { return 0; }
	static void setgid(gid_t t) {}
	static uid_t geteuid() { return 0; }
	static void get_credentials(char const *user, uid_t *puid, gid_t *pgid) {}
#else
static void daemonize()
{
    int fd, rc;

    switch (fork()) {
    case 0:
	break;

    case -1:
	eap::out::notice("daemonize: fork: %m");
	exit(1);

    default:
	_exit(0);
    }
    
    eap::logger::make(name, LOG_FAC);
    rc = setsid();
    if (rc == -1) {
	eap::out::notice("daemonize: setsid: %m");
	exit(1);
    }
    
    switch (fork()) {
    case -1:
    case 0:
	break;

    default:
	_exit(0);
    }

    chdir("/");

    fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1) {
	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);

	close(fd);

	fd = STDERR_FILENO + 1;
    } else fd = 0;

    while (1) {
	close(fd);
	if (fd == FD_CLOSE_MAX) break;
	++fd;
    }

    environ = 0;
    eap::logger::make(name, LOG_FAC);
}

//**  signal handling
//
static void rip_child(int signo)
{
    (void)signo;

    while (waitpid(-1, NULL, WNOHANG) != -1);
}

static void install_ripper()
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = rip_child;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	eap::out::notice("install_ripper: sigaction: %m");
	exit(1);
    }
}

static void terminate(int signo)
{
    if (molestor != -1) kill(molestor, SIGTERM);
    
    if (state == DID_AUTH && do_logoff) {
	eap::out::debug_level = -1;	// prevent any output
	                                // to avoid malloc calls 
	
	io->make_eapol(eap::eapol::logoff);
	io->send();
    }

    signal(signo, SIG_DFL);
    raise(signo);
}

static void block_fatal(int on)
{
    sigset_t ss;

    sigemptyset(&ss);
    sigaddset(&ss, SIGHUP);
    sigaddset(&ss, SIGTERM);
    sigaddset(&ss, SIGINT);
    sigaddset(&ss, SIGQUIT);

    if (sigprocmask(on ?
		    SIG_BLOCK : SIG_UNBLOCK,
		    &ss, NULL) == -1) {
	eap::out::notice("block_fatal: sigprocmask: %m");
	exit(1);
    }
}

static void install_terminate()
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGHUP);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGQUIT);
    sa.sa_flags = 0;
    sa.sa_handler = terminate;

    if (sigaction(SIGHUP, &sa, NULL) == -1 ||
	sigaction(SIGTERM, &sa, NULL) == -1 ||
	sigaction(SIGINT, &sa, NULL) == -1 ||
	sigaction(SIGQUIT, &sa, NULL) == -1) {
	eap::out::notice("install_terminate: sigaction: %m");
	exit(1);
    }
}

static void do_abort()
{
    auth.abort();

    if (send_eapol_start && molestor == -1) {
	eap::out::debug("restarting molestor");
	molestor = start_molestor(0);
    }
}

static void get_credentials(char const *user, uid_t *puid, gid_t *pgid)
{
    struct passwd *pw;

    pw = getpwnam(user);
    if (!pw) pw = getpwuid(strtoul(user, 0, 10));
    if (!pw) {
	eap::out::notice("unknown user '%s'", user);
	exit(1);
    }

    *puid = pw->pw_uid;
    *pgid = pw->pw_gid;
}


#endif	// #ifdef WIN32 #ELSE

static pid_t start_molestor(int immediate_send)
{
    pid_t rc;

#ifdef WIN32
	rc = -1;
#else
	rc = fork();
#endif
    switch(rc) {
    case 0:
	break;

    case -1:
	eap::out::warn("cannot start molestor: %m");
	eap::out::info("transmitting single EAPOL_START frame");
	
	io->make_eapol(eap::eapol::start);
	io->log_out_packet();
	io->send();
	
	return -1;

    default:
	return rc;
    }

    block_fatal(0);

    io->make_eapol(eap::eapol::start);
    if (immediate_send) goto send_one;

    while (1) {
#ifdef WIN32
//		Sleep(MOLEST_TIMEOUT);
#else
		sleep(MOLEST_TIMEOUT);
#endif

		send_one:
		eap::out::debug("[molestor]: transmitting EAPOL_START");
		io->log_out_packet();
		io->send();
    }

    return 0;
}

static void stop_molestor()
{
#ifdef WIN32
#else
	kill(molestor, SIGTERM);
#endif
    molestor = -1;
    eap::out::debug("molestor stopped");
}



//** 'functional parts'
//
static void usage()
{
    eap::out::notice("Usage: %s [-dev <device>] [-mtu <mtu>] "
	"[-user <user>] [-debug <n>] [-exit] [-start] [-daemon] [-nologoff] "
	"[-md5] "
	"[-tls] [-cert <client cert>] [-ca <ca cert>] [-rand <rand file>] "
	"[-nopw] "
	"[-frags <frag size>] <username>", name);
    exit(1);
}

//**  argument processing
//
static void do_enable_md5(const char *id)	// zl changed from : static void do_enable_md5()
{
    eap::pwtty pw(4096, "Enter MD5 secret: ");

    if (pw.get() != -1 && pw.len()) auth.add_handler(new eap::type_md5(pw, pw.len(), (ui8*)id, strlen(id))); //zl changed from : (new eap::type_md5(pw, pw.len()));
    else {
	eap::out::notice("could not read MD5 secret");
	exit(1);
    }
}

static void do_enable_tls(char const *cert,
			  char const *ca,
			  char const *rand,
			  size_t frags,
			  int use_pw)
{
    eap::pwtty pw(4096, "Enter private key passphrase: ");
    eap::tls *ptls;
    char const *p;
    
    try {
	if (!cert) {
	    cert = getenv(CERT_ENV);
	    if (!cert) throw("TLS: need client certificate");
	}

	if (!ca) {
	    ca = getenv(CA_ENV);
	    if (!ca) throw("TLS: need CA certificate");
	}

	if (!rand) {
	    rand = getenv(RAND_ENV);
	    if (!rand) rand = RAND_DEFAULT;
	}

	if (!frags) {
	    p = getenv(FRAGS_ENV);
	    if (p) frags = strtoul(p, 0, 10);
	    else frags = FRAGS_DEFAULT;
	}

	if (use_pw)
	    if (pw.get() == -1) throw("TLS: could not read passphrase");

	ptls = new eap::tls(cert, ca, rand,
			    use_pw ? (char const *)pw : 0);
	auth.add_handler(new eap::type_tls(ptls, frags));
	
    } catch (char const *s) {
	eap::out::notice("%s", s);
	exit(1);
    }
}

static void parse_arguments(char const **argv)
{
#define is_opt(s) strcmp(*argv + 1, s) == 0
#define assert_arg() ++argv; if (!*argv) goto usage    
    char const *n;
    char const *dev = DEFAULT_DEV, *id = 0;
    size_t mtu = DEFAULT_MTU;
    int become_daemon = 0, enable_md5 = 0, set_uid = 0;
    uid_t uid = 0;
    gid_t gid;

    int enable_tls = 0, use_pw = 1;
    char const *cert = 0, *ca = 0, *rand = 0;
    size_t frags = 0;

    uid = getuid();
    set_uid = uid != geteuid();	// increase paranoia level

    // find last component of argv[0]
    //
    if (!set_uid) {
	n = name = *argv;
	while (1) {
	    ++n;

	    switch (*n) {
	    case 0:
		goto out;

	    case '/':
		name = n + 1;
	    }
	}
    } else name = "eapd";
 out:
    
    //  parse arguments
    //
    assert_arg();

    while (1) {
	if (**argv != '-') break;
	
	if (is_opt("dev")) {
	    assert_arg();
	    dev = *argv;
	    
	    goto loop;
	}

	if (is_opt("mtu")) {
	    assert_arg();
	    mtu = strtoul(*argv, NULL, 10);
	    
	    goto loop;
	}

	if (is_opt("debug")) {
	    assert_arg();
	    eap::out::debug_level = strtoul(*argv, NULL, 10);
	    
	    goto loop;
	}

	if (is_opt("daemon")) {
	    become_daemon = 1;
	    goto loop;
	}

	if (is_opt("exit")) {
	    single_shot = 1;
	    goto loop;
	}

	if (is_opt("start")) {
	    send_eapol_start = 1;
	    goto loop;
	}

	if (is_opt("nologoff")){
	    do_logoff = 0;
	    goto loop;
	}

	if (is_opt("md5")) {
	    enable_md5 = 1;
	    goto loop;
	}

	if (is_opt("tls")) {
	    enable_tls = 1;
	    goto loop;
	}

	if (is_opt("cert")) {
	    assert_arg();
	    cert = *argv;

	    goto loop;
	}

	if (is_opt("ca")) {
	    assert_arg();
	    ca = *argv;

	    goto loop;
	}

	if (is_opt("rand")) {
	    assert_arg();
	    rand = *argv;

	    goto loop;
	}

	if (is_opt("frags")) {
	    assert_arg();
	    frags = strtoul(*argv, 0, 10);

	    goto loop;
	}

	if (is_opt("nopw")) {
	    use_pw = 0;
	    goto loop;
	}

	if (is_opt("user")) {
	    assert_arg();
	    
	    if (set_uid) {
		eap::out::notice("ignoring -user while running setuid");
		goto loop;
	    }
	    
	    get_credentials(*argv, &uid, &gid);
	    goto loop;
	}
	
	goto usage;

    loop:
	assert_arg();
    }

    id = *argv++;
    if (*argv) goto usage;

    // initialize
    //
    auth.add_handler(new eap::type_notification);
    auth.add_handler(new eap::type_identity((ui8 *)id, strlen(id)));
	if (enable_md5) do_enable_md5(id);    // zl changed from : if (enable_md5) do_enable_md5();

    if (enable_tls) do_enable_tls(cert, ca, rand, frags, use_pw);

    if (become_daemon) daemonize();
    
    io = new eap::ll_802_3(dev, mtu);
    io->setup_output();

    if (uid) {
	setgid(gid);
	setuid(uid);
    } else {
	if (set_uid) {
	    setgid(getgid());
	    setuid(uid);
	}
    }
    
    return;

    // error handling
    //
 usage:
    usage();
#undef is_opt
#undef assert_arg    
}

//*  main
//
int main(int argc, char const **argv)
{
    int rc;
    eap::eapol::packet payload;

    eap::printer::make();

    try {
	parse_arguments(argv);
	block_fatal(1);
	
	if (send_eapol_start) {
	    eap::out::debug("starting molestor");
	    install_ripper();
	    molestor = start_molestor(1);
	}

	install_terminate();

	state = INIT;
	while (1) {
	    // input
	    //
	    block_fatal(0);
	    io->receive();
	    block_fatal(1);
	    io->log_in_packet();
	    
	    //  EAPOL
	    //
	    payload = io->input();
	    switch(payload.type()) {
	    case eap::eapol::pckt:
		break;

	    case eap::eapol::start:
	    case eap::eapol::logoff:
		eap::out::info("invalid EAPOL frame of type %d", (ui8)payload.type());
		continue;
		
	    default:
		eap::out::info("ignoring unsupported EAPOL type %d", (ui8)payload.type());
		continue;
	    }
	    
	    if (!io->same_pae()) {
		switch (state) {
		case INIT:
		    break;

		case DO_AUTH:
		    do_abort();

		case DID_AUTH:
		    state = INIT;
		    break;
		}
	    }

	    //  EAP
	    //
	    rc = auth.process(payload.body(), io->output().body());
	    
	    if (rc == eap::eap::ret_old) {
		eap::out::debug("retransmitting last frame");
		io->retransmit_last();
		
		continue;
	    }
	    
	    switch (state) {
	    case INIT:
			switch (rc) {
			case eap::eap::ret_cont:
				break;

			/*
			Catalyst 3550 pecularities
				*/    
			case eap::eap::ret_ok:
				if (molestor != -1) stop_molestor();
				continue;

			case eap::eap::ret_fail:
				if (send_eapol_start && molestor == -1) {
				eap::out::debug("starting molestor");
				molestor = start_molestor(1);
				}

				continue;

			case eap::eap::ret_abort:
				auth.abort();

			default:
				continue;
			}
			
			state = DO_AUTH;
			break;

	    case DO_AUTH:
			if (molestor != -1) stop_molestor();
			
			switch (rc) {
			case eap::eap::ret_ok:
				state = DID_AUTH;
				if (single_shot) goto out;
			    
				continue;

			case eap::eap::ret_fail:
				if (single_shot) goto out;

			case eap::eap::ret_abort:
				state = INIT;
				do_abort();

			case eap::eap::ret_inv:
				continue;
			}

			break;

	    case DID_AUTH:
			switch (rc) {
			case eap::eap::ret_cont:
				state = DO_AUTH;
				break;

			case eap::eap::ret_abort:
			case eap::eap::ret_fail:
				state = INIT;
				do_abort();

			default:
				continue;
			}
	    }

	    // output
	    //
	    io->make_eapol(eap::eapol::pckt);
	    io->log_out_packet();
	    io->send();
	}
    out:
	
	if (state == DID_AUTH && do_logoff) {
	    eap::out::debug("transmitting EAPOL_LOGOFF");
	    io->make_eapol(eap::eapol::logoff);
	    io->send();
	}
	
    } catch (eap::error &X) {
	X.print();
    }

    if (molestor != -1) stop_molestor();
    
    delete io;
    return 0;
}
