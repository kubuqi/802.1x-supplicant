// EAP_TLS packet class
//
//*  includes
//
#include "stdafx.h"

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
