//  password class
//
//	$Id: password.hh,v 1.2 2002/09/12 11:40:53 rw Exp $
//
#ifndef _8021x_password_hh
#define _8021x_password_hh

//*  includes
//
extern "C" {
#include <sys/types.h>
}

#include "globals.hh"
#include "buffer.hh"

namespace eap
{
    //*  classes
    //**  password
    //
    class password: public buffer
    {
    public:
	virtual int get() = 0;
	
	ssize_t len() const 
	    {
		return nr;
	    }
	
    protected:
	size_t nr, max;

	password(size_t s): buffer(s), nr(0), max(s)
	    {}
	virtual ~password();
    };

    //** pwtty
    //
    class pwtty: public password
    {
    public:
	pwtty(size_t s, char const *p, char const *d = 0);
	virtual int get();

    private:
	char const *dev;
	char const *prompt;
    };
}

#endif
