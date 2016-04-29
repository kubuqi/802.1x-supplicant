// EAP handler: identity
//
char const _8021x_eap_identity_rcsid[] = "$Id: type_identity.cc,v 1.16 2002/09/15 10:38:06 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "eap.hh"
#include "type_identity.hh"

namespace eap
{
    extern "C" {
#include <string.h>
    }
    
    //*  methods
    //
    type_identity::type_identity(ui8 const *_id, size_t _len): id(_len + 1)
    {
	len = _len;
	memcpy(id, _id, _len);
	// work around for MSVC, change from : id[_len] = 0;
	(ui8 &)(id.operator [](len)) = 0;
    }

    int type_identity::want_this(eap::packet p)
    {
	return
	    p.type() == (unsigned)eap::identity ?
	    i_will_auth : not_me;
    }

    int type_identity::process(eap::packet input, eap::packet output)
    {
	out::debug("    EAP_IDENTITY. Returning '%s'", (char *)id);
	
	memcpy(output.eap_data(), id, len);
	output.len() = len;

	return eap::ret_cont;
    }
}
