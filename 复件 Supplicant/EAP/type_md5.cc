// EAP handler: MD5
//
char const _8021x_eap_md_rcsid[] = "$Id: type_md5.cc,v 1.18 2002/09/15 10:38:06 wcvs Exp $";

//*  includes
//
#include "stdafx.h"

#include "comm.hh"

#include "eap.hh"
#include "type_md5.hh"

namespace eap
{
    extern "C" {
#ifdef HAS_OPENSSL
	#include <openssl/md5.h>
#else
	#include "md5.h"
#endif
    }

    //*  methods
    //
	type_md5::type_md5(ui8 const *_identity, size_t _ilen, ui8 const *_secret, size_t _len)	:// zl added identiy and ilen
	chap_buf(_secret, _len, INITIAL),id_buf(_identity, _ilen)		// zl added identity and ilen
    {
		memcpy(id_buf, _identity, _ilen);
	}

    int type_md5::want_this(eap::packet p)
    {
	return
	    p.type() == eap::md5 ?
	    i_will_auth : not_me;
    }

    int type_md5::process(eap::packet input, eap::packet output)
    {
	size_t vsize;
	
//zl changed from :	vsize = *input.eap_data();
	vsize = input.len() - eap::header_len - 1;

	out::debug("    EAP_MD5, challenge length %d", vsize);
	

	// ��������������challenge��ӵ�chap_buf�У����û���������һ��
	chap_buf.need(vsize);
	chap_buf.copy_challenge(input.id(),
				   input.eap_data() + 1, vsize);

	// ���û�����+challenge������MD5���㣬�����output��buffer��
	// MD5(��������bufָ�룬�������볤�ȣ��������bufָ��)

	MD5(chap_buf, chap_buf.hdr_len()+vsize, output.eap_data() + 1);
	
	// output������'16'��ʼ������MD5�����Ľ����
	*output.eap_data() = 16;

	// �����û�ID��MD5�����Ľ��֮�󣬲�����output�ĳ�����ʾ��
	memcpy(output.eap_data()+17, id_buf, id_buf.buf_size());	// zl added
	output.len() = 17 + id_buf.buf_size();	// zl changed from : output.len() = 17;

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
