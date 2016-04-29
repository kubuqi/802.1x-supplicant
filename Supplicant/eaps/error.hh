// exceptions
//
//	$Id: error.hh,v 1.5 2002/09/13 16:51:30 wcvs Exp $
//
#ifndef _8021x_exception_hh
#define _8021x_exception_hh

//*  includes
//
#include "globals.hh"

namespace eap
{
    extern "C" {
#include <errno.h>	
#include <stddef.h>
    }
    
    //*  classes
    //**  error 
    //
    class error
    {
    public:
	virtual ~error()
	    {}
	
	virtual void print() const = 0;
    };

    //**  out_of_mem
    //
    class out_of_mem: public error
    {
    public:
	virtual void print() const;
    };

    //**  system_error
    //
    class system_error: public error
    {
    public:
	system_error(): ec(0)
	    {}

	int code() const 
	    {
		return ec;
	    }

	virtual void print() const;
	void raise(char const *info);
		
    protected:
	int ec;
	char const *info;
    };
}

#endif
