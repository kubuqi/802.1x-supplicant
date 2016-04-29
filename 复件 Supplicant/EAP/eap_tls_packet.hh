//  EAP_TLS packet class
//
//	$Id: eap_tls_packet.hh,v 1.11 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eaptls_hh
#define _8021x_eaptls_hh

//*  includes
//
#include "eap.hh"

namespace eap
{
    //*  classes
    //
    class tls_packet: public eap::packet
    {
    public:
	//*  constants
	//
	enum {
	    length_included = 0x80,
	    more_frags = 0x40,
	    eaptls_start = 0x20,

	    eap_type = 13
	};
	
	tls_packet(eap::packet _packet): packet(_packet.sdu())
	    {}

	//** accessors
	//
	access_ui8 tls_flags() const
	    {
		return access_ui8(eap_data());
	    }
	
	access_ui32 tls_len() const
	    {
		return access_ui32(eap_data() + 1);
	    }

	ui8 * const tls_data() const
	    {
		return eap_data() + (*eap_data() & length_included ? 5 : 1);
	    }

	//**  questions
	//
	size_t tls_pdu_len() const;
	    
	int has_length() const
	    {
		return *eap_data() & length_included;
	    }

	int more_fragments() const
	    {
		return *eap_data() & more_frags;
	    }

	int starttls() const
	    {
		return *eap_data() & eaptls_start;
	    }
	
	//**  helpers
	//
	void make_ack() const
	    {
		*eap_data() = 0;
		len() = 1;
	    }
    };
}

#endif
