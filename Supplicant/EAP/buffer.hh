// buffer class
//
//	$Id: buffer.hh,v 1.3 2002/09/01 11:26:57 rw Exp $
//
#ifndef _8021x_buffer_hh
#define _8021x_buffer_hh

//*  includes
//
#include "globals.hh"
#include "error.hh"

namespace eap
{
    extern "C" {
#include <stdlib.h>
#include <string.h>	
    }

    //*  classes
    //**  buffer (fixed size)
    //
    class buffer
    {
    public:
	//***  types
	//
	buffer(size_t);
	buffer(const ui8 *data, size_t len);
	
	~buffer() 
	    {
		free(buf);
	    }

	operator ui8 *()
	    {
		return buf;
	    }

	operator char *()
	    {
		return (char *)buf;
	    }

	operator void *()
	    {
		return (void *)buf;
	    }

	ui8 *operator +(int ofs)
	    {
		return buf + ofs;
	    }

	ui8 & operator [](int ofs)
	    {
		return *(buf + ofs);
	    }
	
	size_t buf_size() const
	    {
		return size;
	    }

    protected:
	size_t size;
	ui8 *buf;

	buffer(buffer const &b)
	    {}

	buffer &operator =(buffer const &b)
	    {
		return *this;
	    }
    };

    //** growable
    //
    class growable: public buffer
    {
    public:
	growable(size_t _size): buffer(_size)
	    {}

	void need(size_t what) 
	    {
		if (size < what) _need(what);
	    }
	
	void expand();

    private:
	void _need(size_t what);
    };
}

#endif
