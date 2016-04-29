// buffer class
//
#include "stdafx.h"

#include "error.hh"
#include "buffer.hh"

namespace eap
{
    //*  methods
    //**  buffer
    //
    buffer::buffer(size_t _size)
    {
	out_of_mem X;
	
	buf = (ui8 *)malloc(_size);
	if (!buf) throw(X);
	size = _size;
    }

    buffer::buffer(const ui8 *data, size_t len)
    {
	out_of_mem X;
	
	buf = (ui8 *)malloc(len);
	if (!buf) throw(X);
	size = len;

	memcpy(buf, data, len);
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
	size_t max_size;

	max_size = size * 2;
	if (max_size < size) throw(X);
	need(max_size);
    }
}
