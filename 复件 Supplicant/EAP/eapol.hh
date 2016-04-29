// EAPOL protocol constants
//
//	$Id: eapol.hh,v 1.8 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eapol_hh
#define _8021x_eapol_hh

//*  includes
//
#include "accessor.hh"
#include "eap.hh"

namespace eap 
{
    //*  classes
    //
    struct eapol
    {
	//**  constants
	//
	enum {
	    pckt = 0,
	    start = 1,
	    logoff = 2,
	    key = 3,

	    ether_type = 0x888e,

	    eapol_version = 0,
	    eapol_type = 1,
	    eapol_blen = 2,
	    eapol_body = 4,

	    base_header_len = 4
	};

	//**  classes
	//
	class packet
	{
	public:
	    packet(): data(0)
		{}
	    
	    packet(ui8 *_data): data(_data)
		{}

	    packet &operator =(ui8 *d)
		{
		    data = d;
		    return *this;
		}

	    access_ui8 version()
		{
		    return access_ui8(data + eapol_version);
		}

	    access_ui8 type()
		{
		    return access_ui8(data + eapol_type);
		}

	    access_ui16 body_len()
		{
		    return access_ui16(data + eapol_blen);
		}

	    eap::packet body()
		{
		    return eap::packet(data + eapol_body);
		}

	private:
	    ui8 *data;
	};
    };
}

#endif
