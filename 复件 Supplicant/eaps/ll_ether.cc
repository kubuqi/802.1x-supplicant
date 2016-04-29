// ethernet (802.3 + 802.11) EAPOL base class
//
char const _8021x_ll_ether_rcsid[] = "$Id: ll_ether.cc,v 1.31 2002/10/17 10:50:59 wcvs Exp $";

//*  includes
//
#include "eap.hh"

#include "eapol.hh"
#include "ll_ether.hh"
#include "comm.hh"
#include "error.hh"

namespace eap
{
    extern "C" {
#ifdef WIN32
#	include <stdlib.h>
#else
#	include <unistd.h>	
#	include <stdlib.h>
#endif
    }
    
    //*  methods
    //**  ll_ether
    //
    ll_ether::ll_ether(char const *_dev, size_t _mtu):
	ll_eapol(_mtu), link(_dev, eapol::ether_type, _mtu)
    {
	memset(ether_dst, 0, ETHER_ADDR_LEN);
	link.get_mac((char *)ether_src, sizeof(ether_src));
	ll_hdr_len = 2 * ETHER_ADDR_LEN + 2;
    }
    
    access_range ll_ether::dst(ui8 *frame)
    {
	return access_range(frame, ETHER_ADDR_LEN);
    }

    access_range ll_ether::src(ui8 *frame)
    {
	return access_range(frame + ETHER_ADDR_LEN, ETHER_ADDR_LEN);
    }

    access_ui16 ll_ether::ll_type(ui8 *frame)
    {
	return access_ui16(frame + 2 * ETHER_ADDR_LEN);
    }

    void ll_ether::setup_output()
    {
	ll_eapol::setup_output();
	ll_type(out_data) = eapol::ether_type;
	src(out_data).copy_in((ui8 *)ether_src);
	dst(out_data).copy_in((ui8 *)ether_dst);
    }

    void ll_ether::log_in_packet()
    {
	access_range s, d;

	s = src(in_data);
	d = dst(in_data);
	
	out::debug("(in) packet from "
		   "%02x:%02x:%02x:%02x:%02x:%02x to "
		   "%02x:%02x:%02x:%02x:%02x:%02x",
		   s[0], s[1], s[2], s[3], s[4], s[5],
		   d[0], d[1], d[2], d[3], d[4], d[5]);
    }
    
    void ll_ether::log_out_packet()
    {
	access_range s, d;

	s = src(out_data);
	d = dst(out_data);
	
	out::debug("(out) packet from "
		   "%02x:%02x:%02x:%02x:%02x:%02x to "
		   "%02x:%02x:%02x:%02x:%02x:%02x",
		   s[0], s[1], s[2], s[3], s[4], s[5],
		   d[0], d[1], d[2], d[3], d[4], d[5]);
    }

    int ll_ether::recv_hook()
    {
	if (!src(in_data).equal(ether_dst)) {
	    pae_changed = 1;
	    src(in_data).copy_out(ether_dst);
	}
	
	return 0;
    }

   void ll_ether::recv_frame(int ignore_errors)
   {
	in_data = link.rx(ignore_errors);
    }

    void ll_ether::send_frame(ui8 *data, size_t len, int ignore_errors)
    {
	link.tx(data, len, ignore_errors);
    }
}
