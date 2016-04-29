// EAPOL class
//
//	$Id: ll_eapol.hh,v 1.15 2002/10/02 11:48:55 wcvs Exp $
//
#ifndef _8021x_ll_eapol_hh
#define _8021x_ll_eapol_hh

//*  includes
//
#include "buffer.hh"
#include "eapol.hh"

namespace eap
{
    //*  classes
    //
    class ll_eapol
    {
    public:
	//**  methods
	//***  ctor
	//
	ll_eapol(size_t mtu):
	    pae_changed(0), in_data(0), out_data(mtu), out_len(0),
	    ll_hdr_len(0)
	    {}

	virtual ~ll_eapol();
	
	//*** I/O
	//
	size_t receive(bool ignore_errors = 0);
	void retransmit_last();
	void send(bool ignore_errors = 0);

	virtual void setup_output();

	//***  EAPOL
	//
	eapol::packet input()
	    {
		return eapol::packet(in_data + ll_hdr_len);
	    }

	eapol::packet output()
	    {
		// work around for MSVC, change from : return eapol::packet(out_data + ll_hdr_len);
		return eapol::packet(out_data + (int)ll_hdr_len);
	    }

	//***  misc
	//
	int same_pae() const
	    {
		return !pae_changed;
	    }
	
	void make_eapol(unsigned type);
	
	virtual void log_in_packet() = 0;
	virtual void log_out_packet() = 0;

    protected:
	//**  data
	//
	int pae_changed;
	
	ui8 *in_data;
	buffer out_data;
	size_t out_len, ll_hdr_len;

	//**  methods
	//
	virtual int recv_hook() = 0;
	virtual void recv_frame(bool ignore_errors) = 0;
	virtual void send_frame(ui8 *data, size_t len, bool ignore_errors) = 0;
    };
}

#endif
