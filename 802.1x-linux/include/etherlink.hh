// Ethernet datalink access class
//
//	$Id: etherlink.hh,v 1.3 2002/09/29 14:08:42 wcvs Exp $
//
#ifndef _8021x_etherlink_hh
#define _8021x_etherlink_hh

//*  includes
//
#include "globals.hh"
#include "buffer.hh"

namespace eap
{
    extern "C" {
#include <sys/types.h>
#include <unistd.h>	
    }
    
    //*  classes
    //
    class etherlink
    {
    public:
	//**  methods
	//
	etherlink(char const *_iface, ui16 proto, size_t _mtu);
	
	~etherlink()
	    {
		close(ll_fd);
		delete rx_frame;
	    }

	ui8 *rx(int ignore_errors = 0);
	void tx(ui8 const *frame, size_t len, int ignore_errors = 0);
	void get_mac(char *buf, size_t max);

	int fd() const 
	    {
		return ll_fd;
	    }

	unsigned ifindex() const
	    {
		return ifndx;
	    }

    private:
	int ll_fd;
	buffer *rx_frame;
	unsigned ifndx;
#ifdef USE_BPF
	size_t bpf_buflen;
#endif
#ifdef USE_PF_PACKET	
	size_t mtu;
#endif
#ifdef Linux
	char const *iface;
#endif	
    };
}

#endif
