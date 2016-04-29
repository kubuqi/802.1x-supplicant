// buffer class
//
char const _8021x_buffer_rcsid[] = "$Id: buffer.cc,v 1.5 2002/09/04 07:16:16 rw Exp $";

//*  includes
//
#include "error.hh"
#include "buffer.hh"

namespace eap
{
    //*  methods
    //**  buffer
    //
    buffer::buffer(size_t size)
    {
	out_of_mem X;
	
	buf = (ui8 *)malloc(size);
	if (!buf) throw(X);
    }

    //**  growable
    //
    void growable::_need(size_t what)
    {
	out_of_mem X;
	ui8 *p = 0;		// silence compiler
	
	p = (ui8 *)realloc(p, what);
	if (!p) throw(X);
	buf = p;

	size = what;
    }

    void growable::expand()
    {
	out_of_mem X;
	size_t max;

	max = size * 2;
	if (max < size) throw(X);
	need(max);
    }
}
