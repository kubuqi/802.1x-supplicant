// EAP type TLS
//
char const _8021x_eap_tls_rcsid[] = "$Id: type_tls.cc,v 1.25 2002/09/15 10:38:06 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "tls.hh"
#include "type_tls.hh"

namespace eap
{
    //*  methods
    //**  helpers
    //
    void type_tls::frame_next(tls_packet *tlsp)
	/*
	  input: EAP_TLS packet containing
	        everything except a payload
	
	  output:'complete' EAP_TLS packet that
	          can be passed up to the EAP layer
	*/
    {
	char *dst, *cend, *ccur;

	dst = (char *)tlsp->tls_data();
	if (remain() <= fragment_size) cend = end;
	else cend = cur + fragment_size;
	ccur = cur;

	while (1) {
	    *dst = *cur++;
	    if (cur == cend) break;
	    ++dst;
	}
	if (cur != end)
	    tlsp->tls_flags() |= tls_packet::more_frags;

	tlsp->len() += (cur - ccur) + 1;

	out::debug("    (out) EAP_TLS, len %d, flags 0x%02x",
		   (ui16)tlsp->len(), (ui8)tlsp->tls_flags());
	out::debug("    to send: %lu", end - cur);
    }

    void type_tls::recv_next(tls_packet *tlsp, size_t len)
    {
	memcpy(cur, tlsp->tls_data(), len);
	cur += len;
    }

    void type_tls::recv_first(tls_packet *tlsp, size_t len, size_t max)
    {
	recv_buffer.need(max);
	cur = recv_buffer;
	end = cur + max;

	recv_next(tlsp, len);
    }

    int type_tls::classify_packet(tls_packet *tlsp, size_t len)
    {
	switch (tlsp->tls_flags()) {
	case 0:
	    if (len == 0) return tls_ack;
	    return tls_data;

	case tls_packet::eaptls_start:
	    if (len == 0) return tls_start;
	    return tls_invalid;

	case tls_packet::more_frags:
	case tls_packet::more_frags | tls_packet::length_included:
	    return tls_frag;

	case tls_packet::length_included:
	    return tls_data;
	}

	return tls_invalid;
    }
        
    //**  interface
    //
    type_tls::type_tls(tls *_ptls, size_t frags): recv_buffer(1024)
    {
	fragment_size = frags;
	ptls = _ptls;
	state = start;
	failed_once = 0;
    }
    
    int type_tls::want_this(eap::packet p)
    {
	return
	    p.type() == tls_packet::eap_type ?
	    i_will_auth : not_me;
    }

    int type_tls::nak_it(eap::packet p)
    {
	out::debug("    trying to negotiate TLS authentication");

	*p.eap_data() = tls_packet::eap_type;
	p.len() = 1;

	return 1;
    }

    int type_tls::process(eap::packet eap_in, eap::packet eap_out)
    {
	tls_packet tlsp_in(eap_in), tlsp_out(eap_out);
	size_t pdu_len, tls_len;
	char *p;
	int rc, pckt_type;

	pdu_len = tlsp_in.tls_pdu_len();
	
	out::debug("    (in) EAP_TLS, len %u, flags 0x%02x",
		   pdu_len, (ui8)tlsp_in.tls_flags());

	// I/O processing
	//
	pckt_type = classify_packet(&tlsp_in, pdu_len);
	
	if (pckt_type == tls_invalid) {
	    out::info("    invalid TLS packet");
	    return eap::ret_inv;
	}

	out::debug("    packet type %d", pckt_type);
	
	switch (state) {
	case start:
	    if (pckt_type != tls_start) {
		out::info("    expecting start packet");
		return eap::ret_inv;
	    }
	    
	    failed_once = 0;
	    cur = end = recv_buffer;
	    ptls->reset_ssl();

	    break;

	case frag_send:
	    if (pckt_type != tls_ack) {
		out::info("    expecting fragment ack");
		return eap::ret_inv;
	    }
	    
	    tlsp_out.tls_flags() = 0;
	    tlsp_out.len() = 0;
	    frame_next(&tlsp_out);
	    if (!tlsp_out.more_fragments()) state = recv;
	    
	    return eap::ret_cont;

	case frag_recv:
	    if (pckt_type == tls_frag || pckt_type == tls_data) {
		if (remain() < pdu_len) {
		    out::info("    fragment too large");
		    return eap::ret_inv;
		}

		recv_next(&tlsp_in, pdu_len);
		
		if (pckt_type == tls_frag) {
		    make_ack(&tlsp_out);
		    return eap::ret_cont;
		}

		break;
	    }

	    goto invalid_data;

	case recv:
	    if (pckt_type == tls_frag || pckt_type == tls_data) {
		if (pckt_type == tls_frag) {
		    if (!tlsp_in.has_length()) {
			out::info("    first fragment without length");
			return eap::ret_inv;
		    }

		    recv_first(&tlsp_in, pdu_len, tlsp_in.tls_len());
		    make_ack(&tlsp_out);
		    state = frag_recv;
		
		    return eap::ret_cont;
		}
		
		recv_first(&tlsp_in, pdu_len, pdu_len);
		break;
	    }

	invalid_data:
	    out::info("    expecting data or fragment");
	    return eap::ret_inv;
	}

	// TLS processing
	//
	p = recv_buffer;
	tls_len = cur - p;
	if (!tls_len) *p = 0;
	rc = ptls->process_msg((char const **)&p, &tls_len);
	
	switch (rc) {
	case tls::success:
	    make_ack(&tlsp_out);
	    state = start;

	    return eap::ret_cont;

	case tls::failed:
	    if (failed_once) return eap::ret_abort;
	    failed_once = 1;

	default:
	    state = recv;
	}

	// TLS output processing
	//
	if (tls_len) {
	    cur = p;
	    end = p + tls_len;

	    tlsp_out.tls_flags() = tls_packet::length_included;
	    tlsp_out.tls_len() = tls_len;
	    tlsp_out.len() = 4;
	    frame_next(&tlsp_out);
	    if (tlsp_out.more_fragments()) state = frag_send;
	}

	return eap::ret_cont;
    }

    void type_tls::abort()
    {
	state = start;
	failed_once = 0;
	ptls->abort();
    }
}
