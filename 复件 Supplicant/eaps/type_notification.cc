// EAP type: notification
//
char const _8021x_eap_notification_rcsid[] = "$Id: type_notification.cc,v 1.18 2002/09/15 10:38:06 wcvs Exp $";

//*  includes
//
#include "type_notification.hh"
#include "comm.hh"
#include "buffer.hh"

namespace eap
{
    extern "C" {
#include <string.h>
    }
    
    //*  methods
    //
    int type_notification::want_this(eap::packet p)
    {
	return
	    p.type() == eap::notification ?
	    i_will : not_me;
    }

    int type_notification::process(eap::packet input, eap::packet output)
    {
	size_t len;
	
	len = input.len() - eap::header_len;
	if (len) {
	    buffer msg(len + 1);
	    out::debug("    EAP_NOTIFICATION");
	    
	    memcpy(msg, input.eap_data(), len);
	    
		// work around for MSVC, change from : msg[len] = 0;
		(ui8 &)(msg.operator [](len)) = 0;

	    out::notice("%s", (char const *)msg); // ** broken ** could contain \000
	} else
	    out::info("    empty notification packet");

	output.len() = 0;
	return eap::ret_cont;
    }
}
