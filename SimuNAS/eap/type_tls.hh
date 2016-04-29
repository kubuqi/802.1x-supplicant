// EAP type TLS
//
//	$Id: type_tls.hh,v 1.17 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eap_tls_hh
#define _8021x_eap_tls_hh

#ifdef OPENSSL

//*  includes
//
#include "buffer.hh"

#include "eap.hh"
#include "eap_tls_packet.hh"

#include "tls.hh"

namespace eap
{
    //*  classes
    //
    class type_tls: public eap::handler
    {
    public:
	//*  constants
	//
	enum {
	    start,
	    frag_send,
	    frag_recv,
	    recv
	};

	enum {
	    tls_start,
	    tls_ack,
	    tls_frag,
	    tls_data,
	    tls_invalid
	};

	//*  methods
	//
	type_tls(tls *_ptls, size_t frags);

	virtual int want_this(eap::packet p);
	virtual int process(eap::packet input, eap::packet output);
	virtual int nak_it(eap::packet p);

    protected:
	virtual void abort();
	
    private:
	growable recv_buffer;
	tls *ptls;
	char *cur, *end;
	size_t fragment_size;
	int state, failed_once;

	//*  methods
	//
	size_t remain() const
	    {
		return end - cur;
	    }
	
	void frame_next(tls_packet *tlsp);
	void recv_next(tls_packet *tlsp, size_t len);
	void recv_first(tls_packet *tlsp, size_t len, size_t max_size);
	
	void make_ack(tls_packet *tlsp)
	    {
		tlsp->tls_flags() = 0;
		tlsp->len() = 1;
	    }

	int classify_packet(tls_packet *tlsp, size_t len);
    };
}
#endif //#ifdef OPENSSL

#endif
