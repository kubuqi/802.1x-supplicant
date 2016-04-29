//  EAP handler class
//
//	$Id: eap.hh,v 1.19 2002/09/15 10:38:05 wcvs Exp $
//
#ifndef _8021x_eap_hh
#define _8021x_eap_hh

//*  includes
//
#include "globals.hh"
#include "accessor.hh"

namespace eap
{
    //*  classes
    //
    class eap
    {
    public:
	//**  constants
	//***  codes
	//
	enum {
	    request = 1,
	    response = 2,
	    success = 3,
	    failure = 4,
	};

	//***  types
	//
	enum {
	    identity = 1,
	    notification = 2,
	    nak = 3,
	    md5 = 4
	};

	//***  header
	//
	enum {
	    header_len = 5,
	    
	    eap_code = 0,
	    eap_id = 1,
	    eap_len = 2,
	    eap_type = 4
	};

	//***  'process' return codes
	//
	enum {
	    ret_cont = 0,
	    ret_inv = 1,
	    ret_ok = 2,
	    ret_fail = 3,
	    ret_abort = 4,
	    ret_old = 5
	};

	//***  misc
	//
	static unsigned const NINVALID_MAX = 5;

	//**  classes
	//***  packet
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
		    
	    access_ui8 code() const
		{
		    return access_ui8(data + eap::eap_code);
		}

	    access_ui8 id() const
		{
		    return access_ui8(data + eap::eap_id);
		}

	    access_ui16 len() const
		{
		    return access_ui16(data + eap::eap_len);
		}

	    access_ui8 type() const
		{
		    return access_ui8(data + eap::eap_type);
		}

	    ui8 * const eap_data() const 
		{
		    return data + eap::header_len;
		}

	    ui8 * const sdu() const
		{
		    return data;
		}

	protected:

	    ui8 *data;
	};
	
	//***  handler
	//
	class handler
	{
	    friend class eap;
	    
	public:
	    enum {
		not_me = 0,
		i_will = 1,
		i_will_auth = 2
	    };

	    virtual ~handler()
		{}
	    
	    virtual int process(packet input, packet output) = 0;
	    virtual int want_this(packet p) = 0;
	    
	    virtual int nak_it(packet output) 
		{
		    return 0;
		}

	protected:
	    virtual void abort();

	private:
	    handler *next;
	};


	//**  methods
	//
	eap(): handlers(0), auth_handler(0), auth_type(0), last_request(0), ninvalid(0)
	    {}
		
	~eap();

	void add_handler(handler *h)
	    {
		h->next = handlers;
		handlers = h;
	    }
	
	int process(packet input, packet output);
	
	void abort();

    private:
	//**  data
	//
	handler *handlers, *auth_handler;
	int auth_type;
	int last_request;
	unsigned ninvalid;
    };
}

#endif
