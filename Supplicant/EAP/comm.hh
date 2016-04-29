// user communication
//
//	$Id: comm.hh,v 1.6 2002/08/29 08:17:01 rw Exp $
//
#ifndef _8021x_comm_hh
#define _8021x_comm_hh

//*  includes
//
#include "globals.hh"
#include "buffer.hh"
#include "error.hh"

namespace eap
{
    extern "C" {
#include <stdarg.h>	
#include <stddef.h>
    }
    
    //*  classes
    //**  out
    //
    class out
    {
    public:
	//**  constants
	//
	enum {
	    d_notice = 0,
	    d_warn = 1,
	    d_info = 2,
	    d_debug = 3
	};

	//**  data
	//
	static int debug_level;

	//**  methods
	//
	virtual ~out()
	    {}
	
	static void notice(char const *fmt, ...);
	static void warn(char const *fmt, ...);
	static void info(char const *fmt, ...);
	static void debug(char const *fmt, ...);

	static void destroy()
	    {
		delete sink;
		sink = 0;
	    }

    protected:
	growable fmt_buf;
	size_t len;

	static out *sink;

	out(): fmt_buf(128)
	    {}

	size_t remain() const
	    {
		return fmt_buf.buf_size() - len;
	    }

	void do_print(char const *fmt, va_list va);
	virtual void flush() = 0;
    };

    //**  printer
    //
    class printer: public out
    {
    public:
	static void make();

    protected:
	printer()
	    {}

	virtual void flush();
    };

    //**  logger
    //
    class logger: public out
    {
    public:
	static void make(char const *name, int fac);

    protected:
	logger()
	    {}

	virtual void flush();
    };
}

#endif

