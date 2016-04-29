// EAPOL class
//
char const _8021x_ll_eapol_rcsid[] = "$Id: ll_eapol.cc,v 1.17 2002/10/02 11:48:55 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "eap.hh"
#include "eapol.hh"
#include "ll_eapol.hh"

namespace eap
{
    //*  methods
    //
    ll_eapol::~ll_eapol()
    {}
    
    //**  I/O
    //
    void ll_eapol::receive(int ignore_errors)
    {
	eapol::packet packet;
	
	pae_changed = 0;
	
	while (1) {
	    recv_frame(ignore_errors);
	    if (!in_data) return;
	    
	    packet = input();
	    if (packet.body_len() != packet.body().len()) {
		out::info("EAPOL and EAP lengths don't match -- discarding");
		continue;
	    }

	    if (recv_hook()) break;
	}
    }
    
    void ll_eapol::send(int ignore_errors)
    {
	out_len = output().body_len() + eapol::base_header_len + ll_hdr_len;
	send_frame(out_data, out_len, ignore_errors);
    }

    void ll_eapol::retransmit_last()
    {
	send_frame(out_data, out_len, 0);
    }

    void ll_eapol::setup_output()
    {
	eapol::packet out;

	out = output();
	out.version() = 1;
	out.body().code() = eap::response;
    }

    //**  misc
    //
    void ll_eapol::make_eapol(unsigned type)
    {
	eapol::packet packet;

	packet = output();
	packet.type() = type;
	packet.body_len() =
	    type == eapol::pckt ? (unsigned)packet.body().len() : 0;
    }
}
