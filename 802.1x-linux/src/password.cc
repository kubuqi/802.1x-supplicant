// password class
//
char const _8021x_password_rcsid[] = "$Id: password.cc,v 1.7 2002/08/18 14:40:24 rw Exp $";

//*  includes
//
#include "comm.hh"
#include "password.hh"

namespace eap
{
    extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>	
    }
    
    //*  methods
    //**  password
    //
    password::~password()
    {
	if (nr > 0) memset(buf, 0, nr);
    }

    //**  pwtty
    //
    pwtty::pwtty(size_t s, char const *p, char const *d):
	password(s)
    {
	prompt = p;
	dev = d ? d : "/dev/tty";
    }

    int pwtty::get()
    {
	int fd;
	ssize_t n = 0;		// silence compiler
	struct termios tattr;
	
	fd = open(dev, O_RDWR, 0);
	if (fd == -1) {
	    out::warn("pwtty::get: open: %m");
	    return -1;
	}
	if (isatty(fd) == -1) {
	    out::warn("pwtty::get: %s is no tty", dev);
	    goto out;
	}

	if (tcgetattr(fd, &tattr) == -1) {
	    out::warn("pwtty::get: tcgetattr: %m");
	    goto out;
	}
	tattr.c_lflag &= ~ECHO;
	if (tcsetattr(fd, TCSANOW, &tattr) == -1) {
	    out::warn("pwtty::get: tcsetattr: %m");
	    goto out;
	}
	
	n = write(fd, prompt, strlen(prompt));
	if (n == -1) {
	    out::warn("pwtty::get: write: %m");
	    goto reset_term;
	}

	n = read(fd, buf, max);
	switch (n) {
	case -1:
	    out::warn("pwtty::get: read: %m");

	case 0:
	    goto reset_term;
	}

	nr = n - 1;
	write(fd, buf + n - 1, 1);

    reset_term:
	tattr.c_lflag |= ECHO;
	tcsetattr(fd, TCSANOW, &tattr);

    out:
	close(fd);
	return n;
    }
}
