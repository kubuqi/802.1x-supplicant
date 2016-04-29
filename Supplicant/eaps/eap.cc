// EAP handler class
//
char const _8021x_eap_rcsid[] = "$Id: eap.cc,v 1.35 2002/10/02 11:48:55 wcvs Exp $";

//*  includes
//
#include "eap.hh"
#include "ll_eapol.hh"
#include "comm.hh"

namespace eap
{
    //*  methods
    //**  eap::handler
    //
    void eap::handler::abort()
    {}
    
    //**  eap
    //
    eap::~eap()
    {
	handler *h0, *h1;

	if ((h0 = handlers)) {
	    while ((h1 = h0->next)) {
		delete h0;
		h0 = h1;
	    }

	    delete h0;
	}
    }

    int eap::process(packet input, packet output)
    {
	handler *h;
	int rc;

	out::debug("  (in) EAP code %d, type %d, id %d, len %d",
		   (ui8)input.code(),
		   (ui8)input.type(),
		   (ui8)input.id(),
		   (ui16)input.len());

	// handle EAP code
	//
	switch (input.code()) {
	case request:
	    break;

	case response:
	    out::info("  EAP_RESPONSE invalid");
	    return ret_inv;

	case success:
	    auth_type = 0;
	    auth_handler = 0;
	    last_request = -1;
	    
	    out::notice("  authenticated successfully");
	    return ret_ok;

	case failure:
	    out::notice("  authentication failed");
	    return ret_fail;

	default:
	    out::info("  ignoring unknown EAP code");
	    return ret_inv;
	}

	// check for stray retransmissions
	//
//zl	if (input.id() == (unsigned)last_request) {
//zl	    out::info("  old request.");
//zl	    return ret_old;
//zl	}

	// search type handler
	//
	h = handlers;
	while (h) {
	    switch (h->want_this(input)) {
	    case handler::not_me:
		break;
		
	    case handler::i_will_auth:
		if (auth_handler) {
		    /*
		      This is not necessary an indication of
		      an error state, but of a 'fresh' EAP
		      authentication session happening
		    */
		    if (input.type() != (unsigned)auth_type) {
			out::info("  auth type changed from "
				  "%d to %d -- EAP abort",
				  auth_type, (ui8)input.type());
			abort();
		    }
		}
		    
		auth_type = input.type();
		auth_handler = h;
		//WIN32 : NO BREAK?
		
	    case handler::i_will:
		goto handler_found; // *
	    }

	    h = h->next;
	}

	// otherwise, search NAK handler
	//
	h = handlers;
	while (h) {
	    if (h->nak_it(output)) {
		output.type() = nak;
		
		goto straight_return; // *
	    }

	    h = h->next;
	}

	out::info("  unsupported EAP type");
	return ret_inv;

	// run type handler
	//
    handler_found:
	rc = h->process(input, output);
	switch (rc) {
	case ret_cont:
	    output.type() = (unsigned)input.type();
	    goto straight_return;

	case ret_inv:
	    /*
	      The handler returns 'invalid' if it receives
	      a packet it cannot make sense of. This can either be
	      an indication of link layer transmission errors or
	      happen because the PAEs got out of sync. So return
	      'abort' after fixed number of consecutives invalids to
	      allow for possibe re-synchronization.
	    */
	    ++ninvalid;
	    
	    if (ninvalid > NINVALID_MAX) {
		out::info("  NINVALID_MAX exceeded, aborting");
		rc = ret_abort;
		ninvalid = 0;
	    }
	}

	out::warn("  handler for EAP type %d failed: %d", (ui8)input.type(), rc);
	return rc;

	// output
	//
    straight_return:
	ninvalid = 0;
	last_request = input.id();
	
	output.id() = last_request;
	output.len() += header_len;

	out::debug("  (out) EAP code %d, type %d, id %d, len %d",
		   (ui8)output.code(),
		   (ui8)output.type(),
		   (ui8)output.id(),
		   (ui16)output.len());
	
	return ret_cont;
    }

    void eap::abort()
    {
	if (auth_handler) {
	    auth_handler->abort();
	    auth_handler = 0;
	    auth_type = 0;
	}
	
	last_request = -1;
    }
} 
