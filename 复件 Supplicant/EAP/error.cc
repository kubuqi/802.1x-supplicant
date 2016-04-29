// exceptions
//

//*  includes
//
#include "stdafx.h"

#include "comm.hh"
#include "error.hh"

namespace eap
{
    extern "C" {
#include <string.h>
    }

    //*  methods
    //**  out_of_mem
    //
    void out_of_mem::print() const
    {
	out::notice("Out of memory");
    }
    
    //**  system_error
    //
    void system_error::print() const
    {
	if (ec) out::notice("%s: %s", info, strerror(ec));
	else out::notice("%s", info);
    }

    void system_error::raise(char const *_info)
    {
	ec = errno;
	info = _info;
	
	throw(*this);
    }
}
