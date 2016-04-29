// EAP handler: MD5
//
char const _8021x_eap_md_rcsid[] = "$Id: type_md5.cc,v 1.18 2002/09/15 10:38:06 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "eap.hh"
#include "type_md5.hh"

namespace eap
{
    extern "C" {
#include <openssl/md5.h>
    }

    //*  methods
    //
    type_md5::type_md5(ui8 const *_secret, size_t _len):
	buffer(_secret, _len, INITIAL)
    {}

    int type_md5::want_this(eap::packet p)
    {
	return
	    p.type() == eap::md5 ?
	    i_will_auth : not_me;
    }

    int type_md5::process(eap::packet input, eap::packet output)
    {
	size_t vsize;
	
	vsize = *input.eap_data();

	out::debug("    EAP_MD5, challenge length %d", vsize);
	
	buffer.need(vsize);
	buffer.copy_challenge(input.id(),
				   input.eap_data() + 1, vsize);
	MD5(buffer, buffer.hdr_len() + vsize,
	    output.eap_data() + 1);
	*output.eap_data() = 16;
	output.len() = 17;

	return eap::ret_cont;
    }

    int type_md5::nak_it(eap::packet p)
    {
	out::debug("    trying to negotiate MD5 authentication");
	
	*p.eap_data() = eap::md5;
	p.len() = 1;
	
	return 1;
    }
}
