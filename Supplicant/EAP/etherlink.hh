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
#ifdef WIN32
#	include <PCAP.H>
#	include <Packet32.h>
#else
#	include <unistd.h>	
#endif
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
#ifdef WIN32
		if (fp)	pcap_close(fp);
#else
		close(ll_fd);
#endif
		delete rx_frame;
	    }

	ui8 *rx(bool ignore_errors = 0);
	void tx(ui8 const *frame, size_t len, bool ignore_errors = 0);
	void get_mac(char *buf, size_t max_size);

#ifdef WIN32
	pcap_t * fd() const
		{
		return fp;
		}
#else
	int fd() const 
	    {
		return ll_fd;
	    }
#endif

	unsigned ifindex() const
	    {
		return ifndx;
	    }

    private:

#ifdef WIN32
	pcap_t * fp;
	char errbuf[PCAP_ERRBUF_SIZE];
	char name[MAX_LINK_NAME_LENGTH]; 
#else
	int ll_fd;
#endif

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
