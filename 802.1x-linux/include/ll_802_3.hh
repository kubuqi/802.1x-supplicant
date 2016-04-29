// EAPOL over 802.3
//
//	$Id: ll_802_3.hh,v 1.12 2002/10/16 13:34:09 wcvs Exp $
//
#ifndef _8021x_ll_802_3_hh
#define _8021x_ll_802_3_hh

//*  includes
//
#include "ll_ether.hh"
#include "error.hh"

namespace eap
{
    extern "C" {
#include <string.h>
    }
    
    //*  classes
    //
    class ll_802_3: public ll_ether
    {
    public:
	//**  constants
	//
	static ui8 const * const PAE_GROUP_ADDR = (ui8 const *)"\x1\x80\xc2\x0\x0\x3";
	
	//**  methods
	//
	ll_802_3(char const *_dev, size_t _mtu);
#ifdef Linux	
	virtual ~ll_802_3();
#endif	
	
    private:
	//**  methods
	//
	virtual int recv_hook();
    };
}

#endif
