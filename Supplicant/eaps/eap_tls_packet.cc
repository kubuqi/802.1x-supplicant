// EAP_TLS packet class
//
char const _8021x_eaptls_rcsid[] = "$Id: eap_tls_packet.cc,v 1.6 2002/09/01 11:26:58 rw Exp $";

//*  includes
//
#include "eap_tls_packet.hh"

namespace eap
{
    //*  methods
    //
    size_t tls_packet::tls_pdu_len() const
    {
	size_t total = len() - eap::header_len - 1;

	if (has_length()) total -= 4;
	
	return total;
    }
}
