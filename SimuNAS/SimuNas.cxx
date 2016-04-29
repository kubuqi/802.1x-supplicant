//
// SimuNas.cxx
//
// When used behind NAT, this finds the IP address of the NAT box.
//
// Roger Hardiman
//

#include <ptlib.h>

#include <PCAP.H>
#include "eap/comm.hh"
#include "eap/error.hh"
#include "eap/tls.hh"
#include "eap/eap.hh"
#include "eap/eapol.hh"
#include "eap/type_identity.hh"
#include "eap/type_notification.hh"
#include "eap/type_md5.hh"
#include "eap/type_tls.hh"
#include "eap/ll_802_3.hh"

#include "radproto.h"

class SimuNas : public PProcess
{
  PCLASSINFO(SimuNas, PProcess)
public:
	SimuNas() : PProcess("XXX Production", "SimuNAS", 0, 1, PProcess::AlphaCode, 1)
	{
		io = NULL;
		radclient = NULL;
	};

	~SimuNas()
	{
		delete io;
		delete radclient;
	};

    void Main();
	bool Initialise();

	// EAP packet send operations 
	void SendEapRequestID();
	void SendEapRequestMD5(RadiusPDU * response);
	void SendEapSuccess();
	void SendEapFail();

	// Send request to radius server and return response
	RadiusPDU * SendAccessRequestID(eap::eap::packet & pkt);	// 
	RadiusPDU * SendAccessRequestMD5(eap::eap::packet & pkt);	//
	RadiusPDU * SendAccountStart();
	RadiusPDU * SendAccountStop();
	

private:
	eap::ll_eapol * io;
	RadiusClient  * radclient;

	RadiusAttr		tmpState;		// Used to store State returned from Radius Server
	RadiusAttr		tmpUserName;	// Used to store UserName for MD5Challenge response radius PDU.

	PString			supplicantMAC;	// Store a MAC address to represent the supplicant MAC address in Radius requests. 
};

PCREATE_PROCESS(SimuNas)


// Send requestID on receiving a EAP-START
void SimuNas::SendEapRequestID()
{
	eap::eap::packet out = io->output().body();
	out.code() = eap::eap::request;
	out.type() = (unsigned)eap::eap::identity;
	out.len() = 5;
	io->make_eapol(eap::eapol::pckt);
	io->send();

	PTRACE(2, "EAP\tRequest ID sent");
}

// Send MD5-Challenge to supplicant
void SimuNas::SendEapRequestMD5(RadiusPDU * response)
{
	PAssert(response && RadiusPDU::AccessChallenge==response->GetCode(), PInvalidParameter);

	const RadiusAttr * attr_eap = response->FindAttr(RadiusAttr::EapMessage);
	PAssert(attr_eap, PInvalidParameter);

	// Find and store State attribute for SendAccessRequestMD5 
	PAssert(response->FindAttr(RadiusAttr::State), PInvalidParameter);
	tmpState = *response->FindAttr(RadiusAttr::State);

	PBYTEArray buf(attr_eap->GetValueLength());
	attr_eap->GetValue(buf);

	memcpy(io->output().body().sdu(), buf, attr_eap->GetLength());
	io->make_eapol(eap::eapol::pckt);
	io->send();

	PTRACE(2, "EAP\tRequest MD5 sent");
}
void SimuNas::SendEapSuccess()
{
	eap::eap::packet out = io->output().body();
	out.code() = eap::eap::success;
	out.len() = 4;
	io->make_eapol(eap::eapol::pckt);
	io->send();

	PTRACE(2, "EAP\tSuccess sent");
}
void SimuNas::SendEapFail()
{
	eap::eap::packet out = io->output().body();
	out.code() = eap::eap::failure;
	out.len() = 4;
	io->make_eapol(eap::eapol::pckt);
	io->send();

	PTRACE(2, "EAP\tFail sent");
}

// Send request to radius server and return response
RadiusPDU * SimuNas::SendAccessRequestID(eap::eap::packet & pkt)
{
	PAssert(eap::eap::response==pkt.code() && eap::eap::identity==pkt.type(), PInvalidParameter);

	RadiusPDU * request = new RadiusPDU(RadiusPDU::AccessRequest);
	request->AppendAttr(RadiusAttr::UserName, pkt.eap_data(), pkt.len()-5);
	tmpUserName = *request->FindAttr(RadiusAttr::UserName);				// Save username for SendAccessRequestMD5

	request->AppendAttr(RadiusAttr::NasPort,5);
	request->AppendAttr(RadiusAttr::ServiceType, RadiusAttr::ST_Framed);
	request->AppendAttr(RadiusAttr::NasPortType, RadiusAttr::NasPort_Ehternet);
	request->AppendAttr(RadiusAttr::FramedProtocol, RadiusAttr::FP_Ppp);

	request->AppendAttr(RadiusAttr::EapMessage, pkt.sdu(), pkt.len());	//append EAP

	request->AppendAttr(RadiusAttr::FramedIpAddress, PIPSocket::Address("10.1.10.44"));
	request->AppendAttr(RadiusAttr::CallingStationId, supplicantMAC);	// fill in supplicant MAC address
	request->AppendAttr(RadiusAttr::NasIdentifier, "SimuNas");
	request->AppendAttr(RadiusAttr::NasIpAddress, PIPSocket::Address("10.1.10.44"));

	RadiusPDU * response = NULL;
	radclient->MakeRequest(*request, response);

	PTRACE(2,"Requesting radius packet sent :"<<*request);
	if (response)
		PTRACE(2,"Radius server response :"<<*request);
	else
		PTRACE(1,"ERROR\tRadius server no reply!");

	delete request;
	return response;
}

RadiusPDU * SimuNas::SendAccessRequestMD5(eap::eap::packet & pkt)
{
	// TODO: build up md5 challenge radius packet.
	PAssert(eap::eap::response==pkt.code() && eap::eap::md5==pkt.type(), PInvalidParameter);

	RadiusPDU * request = new RadiusPDU(RadiusPDU::AccessRequest);

	request->AppendAttr(tmpState);		// Insert stored State
	request->AppendAttr(tmpUserName);	// Insert stored UserName

//	request->AppendAttr(RadiusAttr::NasPort,5);
//	request->AppendAttr(RadiusAttr::ServiceType, RadiusAttr::ST_Framed);
//	request->AppendAttr(RadiusAttr::NasPortType, RadiusAttr::NasPort_Ehternet);
//	request->AppendAttr(RadiusAttr::FramedProtocol, RadiusAttr::FP_Ppp);

	request->AppendAttr(RadiusAttr::EapMessage, pkt.sdu(), pkt.len());	//append EAP

//	request->AppendAttr(RadiusAttr::FramedIpAddress, PIPSocket::Address("10.1.10.44"));
//	request->AppendAttr(RadiusAttr::CallingStationId, "00ffx03");	// fill with MAC address ?
//	request->AppendAttr(RadiusAttr::NasIdentifier, "SimuNas");
//	request->AppendAttr(RadiusAttr::NasIpAddress, PIPSocket::Address("10.1.10.44"));

	RadiusPDU * response = NULL;
	radclient->MakeRequest(*request, response);

	PTRACE(2,"Requesting radius packet sent :"<<*request);
	if (response)
		PTRACE(2,"Radius server response :"<<*request);
	else
		PTRACE(1,"ERROR\tRadius server no reply!");

	delete request;
	return response;
}

RadiusPDU * SimuNas::SendAccountStart()
{
	RadiusPDU * request = new RadiusPDU(RadiusPDU::AccountingRequest);

	request->AppendAttr(tmpUserName);	// Insert stored UserName
	request->AppendAttr(RadiusAttr::AcctStatusType, RadiusAttr::AcctStatus_Start);
	request->AppendAttr(RadiusAttr::AcctDelayTime, 0);

	request->AppendAttr(RadiusAttr::NasPort,5);
	request->AppendAttr(RadiusAttr::ServiceType, RadiusAttr::ST_Framed);
	request->AppendAttr(RadiusAttr::NasPortType, RadiusAttr::NasPort_Ehternet);
	request->AppendAttr(RadiusAttr::FramedProtocol, RadiusAttr::FP_Ppp);

	request->AppendAttr(RadiusAttr::FramedIpAddress, PIPSocket::Address("10.1.10.44"));
//	request->AppendAttr(RadiusAttr::CallingStationId, "00ffx03");	// fill with MAC address ?
	request->AppendAttr(RadiusAttr::NasIdentifier, "SimuNas");
	request->AppendAttr(RadiusAttr::NasIpAddress, PIPSocket::Address("10.1.10.44"));

	RadiusPDU * response = NULL;
	radclient->MakeRequest(*request, response);

	PTRACE(2,"Requesting radius packet sent :"<<*request);
	if (response)
		PTRACE(2,"Radius server response :"<<*request);
	else
		PTRACE(1,"ERROR\tRadius server no reply!");

	delete request;
	return response;
}

RadiusPDU * SimuNas::SendAccountStop()
{
	RadiusPDU * request = new RadiusPDU(RadiusPDU::AccountingRequest);

	request->AppendAttr(tmpUserName);	// Insert stored UserName
	request->AppendAttr(RadiusAttr::AcctStatusType, RadiusAttr::AcctStatus_Stop);
	request->AppendAttr(RadiusAttr::AcctDelayTime, 0);
	request->AppendAttr(RadiusAttr::AcctSessionTime, 24);

	request->AppendAttr(RadiusAttr::NasPort,5);
	request->AppendAttr(RadiusAttr::ServiceType, RadiusAttr::ST_Framed);
	request->AppendAttr(RadiusAttr::NasPortType, RadiusAttr::NasPort_Ehternet);
	request->AppendAttr(RadiusAttr::FramedProtocol, RadiusAttr::FP_Ppp);

	request->AppendAttr(RadiusAttr::FramedIpAddress, PIPSocket::Address("10.1.10.44"));
//	request->AppendAttr(RadiusAttr::CallingStationId, "00ffx03");	// fill with MAC address ?
	request->AppendAttr(RadiusAttr::NasIdentifier, "SimuNas");
	request->AppendAttr(RadiusAttr::NasIpAddress, PIPSocket::Address("10.1.10.44"));

	RadiusPDU * response = NULL;
	radclient->MakeRequest(*request, response);

	PTRACE(2,"Requesting radius packet sent :"<<*request);
	if (response)
		PTRACE(2,"Radius server response :"<<*request);
	else
		PTRACE(1,"ERROR\tRadius server no reply!");

	delete request;
	return response;
}

bool SimuNas::Initialise()
{
	// Get and parse all of the command line arguments.
	PArgList & args = GetArguments();
	args.Parse(
				"h-help."
				"i-interface:"
				"m-mac:"
				"r-radiuserver:"
				"s-secret:"
				"t-trace:"
				, FALSE);


	if (args.HasOption('h') || !args.HasOption('r') || !args.HasOption('s')) {
		cout << "Usage : " << GetName() << " [options]\n"
				"Options:\n"
				"  -i --interface index    : Select interface to bind with.\n"
				"  -m --supplicant mac     : Set MAC address of supplicant, which would be filled"
				"                            in Calling-Station-ID in radius request.\n"
				"  -r --radius server      : Set radius server address\n"
				"  -s --shared secret      : Set shared secret of radius server.\n"
				"  -t --trace level        : Set trace message level, like -t 3.\n"
				"  -h --help               : This help message.\n"
				<< endl;
		return false;
	}

	supplicantMAC = args.HasOption('m') ? args.GetOptionString('m') : PString("00:00:00:00:00:00");

	//
	// Set trace 
	//
	PTrace::SetOptions(PTrace::DateAndTime | PTrace::TraceLevel);
	PTrace::SetLevel(args.GetOptionString('t').AsInteger());

	//
	// Obtain NIC list.
	//
	pcap_if_t *alldevs, *d;

	char errbuf[PCAP_ERRBUF_SIZE+1];
	if (pcap_findalldevs(&alldevs, errbuf) == -1){
		PTRACE(0, "Error in pcap_findalldevs: "<<errbuf);
		return false;
	}

	std::vector<pcap_if_t *> devs;
	for(d=alldevs;d;d=d->next) {
		devs.push_back(d);
		PTRACE(0, "Interface list:"<<d->description);
	}

	//
	// Select a interface create eapol listener on it.
	//
	int index = 0;
	if (args.HasOption('i'))
		index = args.GetOptionString("i").AsInteger();
	else {
		cout<<"Please select a interface : [0]"<<endl;
		cin>>index;
	}

	if (index>=devs.size()) {
		cout<<"Interface index:"<<index<<" has exceed maximun interfaces of your system:"<<devs.size()<<endl;
		return false;
	}
	PTRACE(0, "Listenning on : "<<devs[index]->description);

	io = new eap::ll_802_3(devs[index]->name);
	pcap_freealldevs(alldevs);
	io->setup_output();

	//
	// create radius client
	//
	radclient = new RadiusClient(
		args.GetOptionString('r'),			// Radius server address
		"10.1.10.44",						// Radius client address
		args.GetOptionString('s')			// Shared secret.
		);
}

void SimuNas::Main()
{
	cout << GetName() << " Version " << GetVersion(TRUE)
		 << " by "	<< GetManufacturer()
		 << " on "	<< GetOSClass() << ' ' << GetOSName()
		 << " (" << GetOSVersion() << '-' << GetOSHardware() << ")\n\n";

	if (!Initialise())
		return;

	//
	// Loop for EAP messages.
	//
	while (true) 
	if (io->receive(true)) {
		
		//  EAPOL
		eap::eapol::packet payload = io->input();
		switch(payload.type()) {
			case eap::eapol::start: 
				PTRACE(2, "EAP-START received, sending RequestID..");
				SendEapRequestID();
				continue;

			case eap::eapol::logoff:
				PTRACE(2, "EAP-LOGOFF received, sending Accounting-Stop");
				delete SendAccountStop();	// we do not care radius response here.
				continue;
			
			case eap::eapol::pckt:	// go to EAP handlers
				break;

			default:
				PTRACE(1, "Unsupported EAPOL type "<<(ui8)payload.type()<<" received, ignored");
				continue;
		}

		//  EAP
		eap::eap::packet eap = io->input().body();
		if (eap::eap::response!=eap.code()) {
			PTRACE(2, "NAS does not handle EAP type "<<eap.code());
			continue;
		}

		RadiusPDU * response = NULL;
		switch (eap.type()) {
		case eap::eap::identity:
			// Pack EAP into radius, send it to radius server, and wait response, and send response to supplicant 
			PTRACE(2, "EAP Response Identity received..");
			response = SendAccessRequestID(eap);
			if (response&&RadiusPDU::AccessChallenge==response->GetCode())
				SendEapRequestMD5(response);
			else
				SendEapFail();
			break;

		case eap::eap::md5:
			// Pack EAP into radius, send it to radius server, and wait response, and send response to supplicant 
			PTRACE(2, "EAP Response MD5 received..");
			response = SendAccessRequestMD5(eap);
			if (response && RadiusPDU::AccessAccept==response->GetCode()) {
				SendEapSuccess();
				delete SendAccountStart();	// we don't care radius response
			} else
				SendEapFail();
				
			break;
		default:
			PTRACE(1, "NAS does not handle EAP type "<<eap.code()<<", ignored");
			break;
		}
		delete response;
	}
}


// End of SimuNas.cxx
