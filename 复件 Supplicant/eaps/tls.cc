// TLS interface class
//
//	Most of this has been shamelessly copied from
//	freeradius, see http://www.freeradius.org
//
char const _8021x_tls_rcsid[] = "$Id: tls.cc,v 1.15 2002/10/02 12:48:08 wcvs Exp $";

//*  includes
//
#include "comm.hh"

#include "tls.hh"

namespace eap
{
    extern "C" {
#include <string.h>
#include <openssl/rand.h>
    }
    
    //*  functions
    //
    static int return_password(char *buf, int size, int rwflag,
			       void *userdata)
    {
	int c;
	size_t n = 0;
	char const *p = (char const *)userdata;
	
	while (1) {
	    c = *p;
	    if (!c) break;
	    *buf = c;
	    
	    ++n;
	    --size;
	    if (!size) break;
	    
	    ++buf;
	    ++p;
	}

	return n;
    }

    static void ssl_info_callback(SSL *ssl, int w, int r)
    {
	out::debug("        %s", SSL_state_string_long(ssl));
	if (w & SSL_CB_ALERT) 
	    out::debug("        ALERT: %s", SSL_alert_desc_string_long(r));
    }
    
    //*  methods
    //**  tls::error
    //
    void tls::error::print() const
    {
	char ssl_msg[120];		// from OpenSSL docs

	out::notice("%s", msg);
	ERR_error_string(ec, ssl_msg);
	out::notice("OpenSSL error: %s (%ld)", ssl_msg, ec);
    }

    void tls::error::raise(char const *_msg)
    {
	msg = _msg;
	ec = ERR_get_error();
	throw(*this);
    }

    //**  tls
    //
    tls::tls(char const *cert_file,
	     char const *CA_file,
	     char const *RAND_file,
	     char const *pw)
    {
	error X;
	
	ssl = 0;
	in = out = 0;

	// initialize
	//
	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(TLSv1_method());
	// work around for VC, changed from :	SSL_CTX_set_info_callback(ctx, (void (*)())ssl_info_callback);
	SSL_CTX_set_info_callback(ctx, (void (*)(const eap::SSL *,int,int))ssl_info_callback);

	// load CA cert
	//
	if (!(SSL_CTX_load_verify_locations(ctx, CA_file, NULL) &&
	      SSL_CTX_set_default_verify_paths(ctx))) {
	    SSL_CTX_free(ctx);
	    X.raise("could not load CA certificate");
	}

	// load our cert & key
	//
	if (pw) {
	    SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)pw);
	    SSL_CTX_set_default_passwd_cb(ctx, return_password);
	}

	if (!(SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) &&
	      SSL_CTX_use_PrivateKey_file(ctx, cert_file, SSL_FILETYPE_PEM)))
	{
	    SSL_CTX_free(ctx);
	    X.raise("could not load credentials");
	}

	if (!SSL_CTX_check_private_key(ctx)) {
	    SSL_CTX_free(ctx);
	    X.raise("private key doesn't match certificate");
	}

	// set various parameters
	//
	//	NB: This has been copied more or less verbally
	//	    from freeradius and I hope it is sensible
	//	    in this context.
	//
	SSL_CTX_set_options(ctx,
			    SSL_OP_NO_SSLv2 |
			    SSL_OP_NO_SSLv3 |
			    SSL_OP_SINGLE_DH_USE);
	
	SSL_CTX_set_verify(ctx,
			   SSL_VERIFY_PEER |
			   SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	//
	// TODO: add verify depth
	//

	// load randomness
	//
	if (!(RAND_load_file(RAND_file, 1024*1024))) {
	    SSL_CTX_free(ctx);
	    X.raise("could not load randomness");
	}
    }

    tls::~tls()
    {
	if (ssl) SSL_free(ssl);
	SSL_CTX_free(ctx);
    }

    void tls::reset_ssl()
    {
	error X;
	
	if (ssl) SSL_free(ssl);

	ssl = SSL_new(ctx);
	if (!ssl) X.raise("could not create SSL object");
	
	in = BIO_new(BIO_s_mem());
	out = BIO_new(BIO_s_mem());
	SSL_set_bio(ssl, in, out);

	SSL_set_verify(ssl,
		       SSL_VERIFY_PEER |
		       SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
    }

    int tls::process_msg(char const **pmsg, size_t *psize)
    {
	error X;
	BUF_MEM *p;
	char const *msg;
	size_t size;
	int rc;

	msg = *pmsg;
	if (msg) {
	    size = *psize;
	    out::debug("      (in) TLS, size %u", size);
	    
	    BIO_reset(in);
	    BIO_write(in, msg, size);
	}

	BIO_reset(out);
	rc = SSL_connect(ssl);
	BIO_get_mem_ptr(out, &p);
	if (p->length) {
	    out::debug("      (out) TLS, size %d", p->length);
	    
	    *pmsg = p->data;
	    *psize = p->length;
	} else {
	    *pmsg = 0;
            *psize = 0;
	}
	
	switch (rc) {
	case 1:
	    out::debug("      TLS handshake succeeded");
	    return success;

	case 0:
	    out::info("      TLS connection shutdown");
	    return failed;

	case -1:
	    switch (SSL_get_error(ssl, -1)) {
	    case SSL_ERROR_ZERO_RETURN:
		out::info("      TLS connection shutdown");
		return failed;

	    case SSL_ERROR_WANT_READ:
	    case SSL_ERROR_WANT_WRITE:
		out::debug("      TLS needs more data");
		break;
		
	    case SSL_ERROR_SSL:
		out::info("      TLS protocol error");
		return failed;

	    default:
		X.raise("Unknown TLS error");
	    }
	}
	
	return more_data;
    }

    void tls::abort()
    {
	SSL_free(ssl);
	ssl = 0;
	in = out = 0;
    }
}
