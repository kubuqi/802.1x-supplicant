// EAP handler: MD5
//
//	$Id: type_md5.hh,v 1.9 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eap_md5_hh
#define _8021x_eap_md5_hh

//*  includes
//
#include "accessor.hh"
#include "buffer.hh"
#include "eap.hh"

namespace eap
{
    extern "C" {
#include <string.h>
    }
    
    //*  classes
    //
    class type_md5: public eap::handler
    {
    public:
	static size_t const INITIAL = 128;
	
	type_md5(ui8 const *_identity, size_t _ilen, ui8 const *_secret, size_t _len);	// zl changed from : type_md5(ui8 const *_secret, size_t _len);

	virtual int want_this(eap::packet p);
	virtual int process(eap::packet input, eap::packet output);
	virtual int nak_it(eap::packet p);

    private:
	class chap_buffer: public growable
	{
	public:
		chap_buffer(ui8 const *_secret, size_t _len, size_t _size) : 
		growable(_size + _len + 1)
		{
		    slen = _len;
		    memcpy(buf + 1, _secret, _len);
		}

	    void need(size_t what)
		{
		    growable::need(what + slen + 1);
		}

	    void copy_challenge(unsigned id, ui8 const *v, size_t len)
		{
		    *buf = id;
		    memcpy(buf + 1 + slen, v, len);
		}

	    size_t hdr_len() const
		{
		    return 1 + slen;
		}

	private:
	    size_t slen;
	};

	chap_buffer	chap_buf;
	buffer		id_buf;		// zl added identity buffer
    };
}

#endif
