/*
 * mppe_keys.c
 *
 * Version:     $Id: mppe_keys.c,v 1.1 2002/07/26 18:50:12 aland Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2002  Axis Communications AB
 * Authors: Henrik Eriksson <henriken@axis.com> & Lars Viklund <larsv@axis.com>
 */

#include <openssl/hmac.h>
#include "eap_tls.h"

/*
 * Add value pair to reply
 */
static void add_reply(VALUE_PAIR** vp, 
		      const char* name, const char* value, int len)
{
	VALUE_PAIR *reply_attr;
	reply_attr = pairmake(name, "", T_OP_EQ);
	if (!reply_attr) {
		DEBUG("rlm_eap_tls: "
		      "add_reply failed to create attribute %s: %s\n", 
		      name, librad_errstr);
		return;
	}

	memcpy(reply_attr->strvalue, value, len);
	reply_attr->length = len;
	pairadd(vp, reply_attr);
}

/*
 * TLS PRF from RFC 2246
 */
static void P_hash(const EVP_MD *evp_md, 
		   const unsigned char *secret, unsigned int secret_len,
		   const unsigned char *seed,   unsigned int seed_len,
		   unsigned char *out, unsigned int out_len)
{
	HMAC_CTX ctx_a, ctx_out;
	unsigned char a[HMAC_MAX_MD_CBLOCK];
	unsigned int size;

	HMAC_CTX_init(&ctx_a);
	HMAC_CTX_init(&ctx_out);
	HMAC_Init_ex(&ctx_a, secret, secret_len, evp_md, NULL);
	HMAC_Init_ex(&ctx_out, secret, secret_len, evp_md, NULL);

	size = HMAC_size(&ctx_out);

	/* Calculate A(1) */
	HMAC_Update(&ctx_a, seed, seed_len);
	HMAC_Final(&ctx_a, a, NULL);

	while (1) {
		/* Calculate next part of output */
		HMAC_Update(&ctx_out, a, size);
		HMAC_Update(&ctx_out, seed, seed_len);

		/* Check if last part */
		if (out_len < size) {
			HMAC_Final(&ctx_out, a, NULL);
			memcpy(out, a, out_len);
			break;
		}

		/* Place digest in output buffer */
		HMAC_Final(&ctx_out, out, NULL);
		HMAC_Init_ex(&ctx_out, NULL, 0, NULL, NULL);
		out += size;
		out_len -= size;

		/* Calculate next A(i) */
		HMAC_Init_ex(&ctx_a, NULL, 0, NULL, NULL);
		HMAC_Update(&ctx_a, a, size);
		HMAC_Final(&ctx_a, a, NULL);
	}

	HMAC_CTX_cleanup(&ctx_a);
	HMAC_CTX_cleanup(&ctx_out);
	memset(a, 0, sizeof(a));
}

static void PRF(const unsigned char *secret, unsigned int secret_len,
		const unsigned char *seed,   unsigned int seed_len,
		unsigned char *out, unsigned char *buf, unsigned int out_len)
{
        unsigned int i;
        unsigned int len = (secret_len + 1) / 2;
	const unsigned char *s1 = secret;
	const unsigned char *s2 = secret + (secret_len - len);

	P_hash(EVP_md5(),  s1, len, seed, seed_len, out, out_len);
	P_hash(EVP_sha1(), s2, len, seed, seed_len, buf, out_len);

	for (i=0; i < out_len; i++) {
	        out[i] ^= buf[i];
	}
}

#define EAPTLS_PRF_LABEL        "client EAP encryption"
#define EAPTLS_MPPE_KEY_LEN     32

/*
 * Generate keys according to RFC 2716 and add to reply
 */
void eaptls_gen_mppe_keys(VALUE_PAIR **reply_vps, SSL *s)
{
	unsigned char out[2*EAPTLS_MPPE_KEY_LEN], buf[2*EAPTLS_MPPE_KEY_LEN];
	unsigned char seed[sizeof(EAPTLS_PRF_LABEL)-1 + 2*SSL3_RANDOM_SIZE];
	unsigned char *p = seed;

	memcpy(p, EAPTLS_PRF_LABEL, sizeof(EAPTLS_PRF_LABEL)-1);
	p += sizeof(EAPTLS_PRF_LABEL)-1;
	memcpy(p, s->s3->client_random, SSL3_RANDOM_SIZE);
	p += SSL3_RANDOM_SIZE;
	memcpy(p, s->s3->server_random, SSL3_RANDOM_SIZE);

	PRF(s->session->master_key, s->session->master_key_length,
	    seed, sizeof(seed), out, buf, 2*EAPTLS_MPPE_KEY_LEN);
	    
	p = out;
	add_reply(reply_vps, "MS-MPPE-Recv-Key", p, EAPTLS_MPPE_KEY_LEN);
	p += EAPTLS_MPPE_KEY_LEN;
	add_reply(reply_vps, "MS-MPPE-Send-Key", p, EAPTLS_MPPE_KEY_LEN);
}
