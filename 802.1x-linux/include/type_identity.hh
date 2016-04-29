// EAP handler: identity
//
//	$Id: type_identity.hh,v 1.7 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eap_identity_hh
#define _8021x_eap_identity_hh

//*  includes
//
#include "eap.hh"
#include "buffer.hh"

namespace eap
{
    //*  classes
    //
    class type_identity: public eap::handler
    {
    public:
	type_identity(ui8 const *_id, size_t _len);

	virtual int want_this(eap::packet p);
	virtual int process(eap::packet input, eap::packet output);

    private:
	buffer id;
	size_t len;
    };
}

#endif
