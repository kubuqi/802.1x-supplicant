// ethernet datalink access class
//
//*  includes
//
#include "stdafx.h"
#include "eapol.hh"
#include "etherlink.hh"

namespace eap
{
    extern "C" {
#ifdef WIN32
#	include <string.h>
#	include <Iphlpapi.h>	// For lookup MAC address, WinPCAP does not support this.
#else
#	include <sys/socket.h>	
#	include <sys/ioctl.h>
#	include <net/if.h>
#	include <net/ethernet.h>
#	include <unistd.h>	
#endif

#ifdef USE_BPF
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/sysctl.h>	
#  include <net/bpf.h>
#  include <net/if_dl.h>
#endif
	
#ifdef USE_PF_PACKET
#  include <netpacket/packet.h>
#endif

#include <errno.h>	
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
    }

    //*  methods
    //
    etherlink::etherlink(char const *_iface, ui16 proto, size_t _mtu)
    {
	system_error X;
	rx_frame = 0;

#ifdef WIN32
	fp = NULL;
	ifndx = 0;	//TODO: obtain index of interface

	memset(name, 0, MAX_LINK_NAME_LENGTH);
	strcpy(name, _iface);

	/* Open the adapter */
	if ( (fp= pcap_open_live(_iface,	// name of the device
							 65536,		// portion of the packet to capture. 
							 // 65536 grants that the whole packet will be captured on all the MACs.
							 0,			// promiscuous mode
							 20,			// read timeout
							 errbuf		// error buffer
							 ) ) == NULL)
	{
	    X.raise("etherlink ctor: Unable to open the adapter. %s is not supported by WinPcap\n");
		if (fp)	pcap_close(fp);
		fp = NULL;
	}
	
	rx_frame = new buffer(_mtu);
	return;
#else
	ll_fd = -1;
	ifndx = if_nametoindex(_iface);
#endif


#ifdef USE_BPF
	char buf[16];
	unsigned x = 0;
	struct ifreq ifrq;

	// filter to return only EAPOL (0x888e) frames
	//
	struct bpf_insn insns[] = 
	    {
		BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 12),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0, 0, 1), 
		BPF_STMT(BPF_RET+BPF_K, 1024),
		BPF_STMT(BPF_RET+BPF_K, 0)
	    };
	struct bpf_program prg;

	// find usable bpf device
	//
	while (1) {
	    sprintf(buf, "/dev/bpf%u", x);
	    ll_fd = open(buf, O_RDWR, 0);
	    if (ll_fd != -1) break;

	    if (errno == EBUSY) {
		++x;
		if (x == 100) X.raise("no usuable bpf device");

		continue;
	    }

	    X.raise("etherlink ctor: open");
	}

	// initialize bpf device
	//
	memset(&ifrq, 0, sizeof(ifrq));
	strlcpy(ifrq.ifr_name, _iface, IFNAMSIZ);
	if (ioctl(ll_fd, BIOCSETIF, &ifrq) == -1) goto error;

	x = 0;
	if (ioctl(ll_fd, BIOCSSEESENT, &x) == -1) goto error;

	x = 1;
	if (ioctl(ll_fd, BIOCIMMEDIATE, &x) == -1) goto error;
	
	if (ioctl(ll_fd, BIOCGBLEN, &bpf_buflen) == -1) goto error;

	prg.bf_len = 4;
	prg.bf_insns = insns;
	insns[1].k = proto;
	if (ioctl(ll_fd, BIOCSETF, &prg) == -1) goto error;

	rx_frame = new buffer(bpf_buflen);
#endif
#ifdef USE_PF_PACKET
	struct sockaddr_ll ll_addr;
	
	ll_fd = socket(PF_PACKET, SOCK_RAW, htons(proto));
	if (ll_fd == -1) goto error;

	memset(&ll_addr, 0, sizeof(ll_addr));
	ll_addr.sll_family = AF_PACKET;
	ll_addr.sll_protocol = htons(proto);
	ll_addr.sll_ifindex = ifndx;
	if (bind(ll_fd, (struct sockaddr *)&ll_addr, sizeof(ll_addr)) == -1)
	    goto error;

	mtu = _mtu;
	rx_frame = new buffer(mtu);
#endif
#ifdef Linux
	iface = _iface;
#endif
	
	return;

#ifndef WIN32
error:
	close(ll_fd);
	ll_fd = -1;
	X.raise("etherlink ctor");
#endif
    }

    ui8 *etherlink::rx(bool ignore_errors)
    {
	system_error X;
	ui8 *p;
	ssize_t nr;

#ifdef USE_BPF
	nr = bpf_buflen;
#endif
#ifdef USE_PF_PACKET	
	nr = mtu;
#endif	

	p = (ui8 *)*rx_frame;
#ifdef WIN32
	struct pcap_pkthdr *header;
	nr = pcap_next_ex( fp, &header, &p);
#else
	nr = read(ll_fd, p, nr);
#endif

	if (nr <= 0) {
	    if (ignore_errors) return 0;

	    X.raise("rx: read error");
	}

#ifdef USE_BPF	
	return p + ((struct bpf_hdr *)p)->bh_hdrlen;
#else
	return p;
#endif	
    }

    void etherlink::tx(ui8 const *frame, size_t len, bool ignore_errors)
    {
	system_error X;
	ssize_t nw;
#ifdef WIN32

	// Make every packet at least 64 byte long.
	ui8 buf[64];
	memset(buf, 0xa5, 64);
	if (len<64)  {
		memcpy(buf, frame, len);
		nw = pcap_sendpacket(fp,buf,64);
	}else
		nw = pcap_sendpacket(fp,(ui8 *)frame,len);
	// zl end
#else
	nw = write(ll_fd, frame, len);
#endif
	if (nw == -1 && !ignore_errors) X.raise("tx: write");
    }

    void etherlink::get_mac(char *addr, size_t size)
    {
#ifdef FreeBSD
	int mib[6];
	char buf[sizeof(struct if_msghdr) + sizeof(struct sockaddr_dl)];
	size_t bufsize;
	struct sockaddr_dl *sa;

	mib[0] = CTL_NET;
	mib[1] = AF_ROUTE;
	mib[2] = 0;
	mib[3] = AF_LINK;
	mib[4] = NET_RT_IFLIST;
	mib[5] = ifndx;
	bufsize = sizeof(buf);
	sysctl(mib, 6, buf, &bufsize, 0, 0);
	sa = (struct sockaddr_dl *)(buf + sizeof(struct if_msghdr));
	if (size > sa->sdl_alen) {
	    memset(addr, 0, size);
	    size = sa->sdl_alen;
	}
	
	memcpy(addr, LLADDR(sa), size);
#endif

#ifdef Linux
	struct ifreq ifr;
	system_error X;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	
	if (ioctl(ll_fd, SIOCGIFHWADDR, &ifr) == -1) X.raise("get_mac");
	memcpy(addr, ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
#endif	

#ifdef WIN32

	//
	// Get MAC, standard procedure from MSDN ;)
	// 
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}

	system_error X;
	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) != NO_ERROR)  {
		free(pAdapterInfo);
		return 	X.raise("EtherLink:Failed to lookup MAC address !");
	}

	pAdapter = pAdapterInfo;
	while (pAdapter) {
		// The Iphlp method returns AdapterName like : "{D6E60C44-CD92-4377-B517-2FD2C72F87F7}"
		// while the wpcap lib returns interface name like :"\Device\NPF_{D6E60C44-CD92-4377-B517-2FD2C72F87F7}"
		// so here we have a magic number "12" to remove the difference
		if (0==strcmp(pAdapter->AdapterName, name+12)) {
			memcpy(addr, pAdapter->Address, min(pAdapter->AddressLength, size));
			return;
		}
		pAdapter = pAdapter->Next;
	}
	free(pAdapterInfo);

#endif	// WIN32
    }
}
