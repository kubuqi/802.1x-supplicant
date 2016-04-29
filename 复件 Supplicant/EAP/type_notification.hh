// EAP type: notification
//
//	$Id: type_notification.hh,v 1.7 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eap_notification_hh
#define _8021x_eap_notification_hh

//*  includes
//
#include "eap.hh"

namespace eap
{
    //*  classes
    //
    class type_notification: public eap::handler
    {
    public:
	virtual int want_this(eap::packet p);
	virtual int process(eap::packet input, eap::packet output);
    };
}

#endif
