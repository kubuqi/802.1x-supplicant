// TLS interface object
//
//	$Id: tls.hh,v 1.10 2002/08/29 08:49:19 rw Exp $
//
#ifndef _8021x_tls_hh
#define _8021x_tls_hh

#ifdef OPENSSL

//*  includes
//
#include "error.hh"
namespace eap
{
    extern "C" {
#include <openssl/err.h>	
#include <openssl/ssl.h>
    }
    
    //*  classes
    //
    class tls
    {
    public:
	//**  constants
	enum {
	    more_data,
	    success,
	    failed
	};
	
	//**  types
	//
	class error: public eap::error
	{
	public:
	    error(): ec(0)
		{}

	    virtual void print() const;
	    void raise(char const *_msg);
	    
	protected:
	    char const *msg;
	    unsigned long ec;
	};

	//**  methods
	//
	tls(char const *cert_file,
	    char const *CA_file,
	    char const *RAND_file,
	    char const *pw);
	~tls();

	void reset_ssl();
	int process_msg(char const **pmsg, size_t *psize);
	void abort();

    private:
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *in, *out;
    };
}
#endif //#ifdef OPENSSL

#endif
