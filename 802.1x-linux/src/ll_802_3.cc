// EAPOL over 802.3
//
char const _8021x_ll_802_3_rcsid[] = "$Id: ll_802_3.cc,v 1.17 2002/10/17 10:50:41 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "ll_802_3.hh"

namespace eap 
{
    extern "C" {
#ifdef FreeBSD
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/ioctl.h>
#  include <net/bpf.h>
#endif

#ifdef Linux
#  include <sys/socket.h>
#  include <netpacket/packet.h>
#endif	
		
#include <string.h>
    }

#ifdef Linux
    //*  functions
    //
    static int configure_pae_multicast(int fd, unsigned ifindex, ui8 const *addr, int onoff)
    {
	struct packet_mreq mrq;

	mrq.mr_ifindex = ifindex;
	mrq.mr_type = PACKET_MR_MULTICAST;
	mrq.mr_alen = ETHER_ADDR_LEN;
	memcpy(mrq.mr_address, addr, ETHER_ADDR_LEN);
	return setsockopt(fd, SOL_PACKET,
			  onoff ?
			  PACKET_ADD_MEMBERSHIP : PACKET_DROP_MEMBERSHIP,
			  &mrq, sizeof(mrq));
    }
#endif    

    //*  methods
    //
    ll_802_3::ll_802_3(char const *_dev, size_t mtu):
	ll_ether(_dev, mtu)
    {
	int rc;
	system_error X;
	    
#ifdef FreeBSD
	rc = ioctl(link.fd(), BIOCPROMISC);
#endif
#ifdef Linux
	rc = configure_pae_multicast(link.fd(), link.ifindex(), PAE_GROUP_ADDR, 1);
#endif
	
	if (rc == -1) X.raise("ll_802_3 ctor");
	memcpy(ether_dst, PAE_GROUP_ADDR, sizeof(ether_dst));
    }

#ifdef Linux    
    ll_802_3::~ll_802_3()
    {
	configure_pae_multicast(link.fd(), link.ifindex(), PAE_GROUP_ADDR, 0);
    }
#endif    
    
    int ll_802_3::recv_hook()
    {
	ll_ether::recv_hook();	// returns 0
	if (dst(in_data).equal(PAE_GROUP_ADDR)) return 1;
	if (dst(in_data).equal(ether_src)) return 1;

	return 0;
    }
}
