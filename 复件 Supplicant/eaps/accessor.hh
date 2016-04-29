//  accessor classes
//
//	$Id: accessor.hh,v 1.14 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_accessor_hh
#define _8021x_accessor_hh

//*  includes
//
#include "globals.hh"
#include "comm.hh"

namespace eap 
{
    //*  includes
    //
    extern "C" {
#ifdef FreeBSD	
#  include <sys/param.h>
#endif

#ifdef WIN32
#	include <PCAP.H>
#else
#	include <netinet/in.h>
#endif
#include <string.h>
#include <stddef.h>	
    }
    
    //*  classes
    //**  accessor
    //
    template <class T>
    class accessor
    {
    public:
	accessor(ui8 *_where = 0): where(_where)
	    {}

	int operator ==(accessor<T> const &a) const
	    {
		return
		    *(T const *)where == *(T const *)a.where;
	    }

	int operator !=(accessor<T> const &a) const
	    {
		return !(*this == a);
	    }

	accessor<T> operator =(accessor<T> const &a)
	    {
		return
		    *(T *)where = *(T const *)a.where;
	    }
		
    protected:
	ui8 * const where;
    };
    
    //**  access_ui8
    //
    class access_ui8: public accessor<ui8>
    {
    public:
	access_ui8(ui8 *w = 0): accessor<ui8>(w)
	    {}

	operator unsigned() const 
	    {
		return *where;
	    }

	ui8 operator =(unsigned v)
	    {
		return *where = v;
	    }

	ui8 operator |=(unsigned v)
	    {
		return
		    *where |= v;
	    }
    };

    //**  access_ui16
    //
    class access_ui16: public accessor<ui16>
    {
    public:
	access_ui16(ui8 *w = 0): accessor<ui16>(w)
	    {}

	operator unsigned() const
	    {
		return ntohs(*(ui16 const *)where);
	    }

	ui16 operator =(unsigned v)
	    {
		return
		    *(ui16 * const)where = htons(v);
	    }
	
	ui16 operator +=(unsigned v) 
	    {
		*(ui16 * const)where += htons(v);
		return ntohs(*(ui16 * const)where);
	    }
    };

    //**  access_ui32
    //
    class access_ui32: public accessor<ui32>
    {
    public:
	access_ui32(ui8 *w = 0): accessor<ui32>(w)
	    {}

	operator ui32() const
	    {
		return ntohl(*(ui32 * const)where);
	    }

	ui32 operator =(ui32 v)
	    {
		return
		    *(ui32 * const)where = htonl(v);
	    }
    };

    //**  access_range
    //
    class access_range
    {
    public:
	access_range(ui8 *w = 0, ui32 r = 0): where(w), range(r)
	    {}

	int equal(ui8 const *d) const
	    {
		return memcmp(where, d, range) == 0;
	    }
	
	void copy_out(ui8 *d)
	    {
		memcpy(d, where, range);
	    }

	void copy_in(ui8 *d) 
	    {
		memcpy(where, d, range);
	    }

	ui8 &operator [](size_t ndx)
	    {
		return *(where + ndx);
	    }

    private:
	ui8 *where;
	ui32 range;
    };
}

#endif
