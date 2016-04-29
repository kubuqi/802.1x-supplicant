// ethernet (802.3 + 802.11) EAPOL base class
//
//	$Id: ll_ether.hh,v 1.19 2002/10/02 11:48:55 wcvs Exp $
//
#ifndef _8021x_ll_ether_hh
#define _8021x_ll_ether_hh

//*  includes
//
#include "error.hh"
#include "ll_eapol.hh"
#include "etherlink.hh"

namespace eap
{
    extern "C" {
#include <net/ethernet.h>
    }
    
    //*  classes
    //
    class ll_ether: public ll_eapol
    {
    public:
	//**  ctor & dtor
	//
	ll_ether(char const *_dev, size_t _mtu);

	//**  sdu access
	//
	access_range src(ui8 *frame);
	access_range dst(ui8 *frame);
	access_ui16 ll_type(ui8 *frame);

	//**  I/O
	//
	virtual void setup_output();

	//**  misc
	//
	virtual void log_in_packet();
	virtual void log_out_packet();

    protected:
	etherlink link;
	ui8 ether_src[ETHER_ADDR_LEN], ether_dst[ETHER_ADDR_LEN];
	size_t out_len;

	virtual int recv_hook();
	virtual void recv_frame(int ignore_errors);
	virtual void send_frame(ui8 *frame, size_t len, int ignore_errors);
    };
}

#endif
