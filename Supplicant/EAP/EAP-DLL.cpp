// EAP.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "resource.h"		// main symbols
#include "EAP-DLL.h"

#include <PCAP.H>


#include "comm.hh"
#include "error.hh"
#include "tls.hh"

#include "eap.hh"
#include "eapol.hh"
#include "type_identity.hh"
#include "type_notification.hh"
#include "type_md5.hh"
#include "type_tls.hh"

#include "ll_802_3.hh"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CEAPApp
/*
class CEAPApp : public CWinApp
{
public:
	CEAPApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
public: 

};

BEGIN_MESSAGE_MAP(CEAPApp, CWinApp)
END_MESSAGE_MAP()


// CEAPApp construction
CEAPApp::CEAPApp()
{
}

// The one and only CEAPApp object
CEAPApp theApp;


// CEAPApp initialization
BOOL CEAPApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	return TRUE;
}//*/


///////////////////////////////////////////////////////////////////////////////

namespace {		// Anonymouse namespace
eap::eap		*auth	= NULL;
eap::ll_eapol	*io		= NULL;
}

//
// Molestor is a thread who send EAPOL_START periodically.
//
/*namespace Molestor {

volatile LONG nExitFlag	= 0;	
HANDLE	hThread	= NULL;

// Send EAPOL_START packet every nInterval second.
unsigned long __stdcall MolestorLoop(LPVOID param)
{
	if (!io)	io = new eap::ll_802_3(theApp.strEth_IF);
    io->make_eapol(eap::eapol::start);

	int nSleepTime = (*(int*)param) * (1000/50);
	unsigned int counter = 0;
    while (0==nExitFlag) {
		counter++;
		if ( counter% nSleepTime) {
			eap::out::debug("[molestor]: transmitting EAPOL_START");
			io->log_out_packet();
			io->send();
		}
		Sleep(50);
    }

    return 0;
}

bool StartMolestor(int nInterval)
{
	ASSERT(!hThread);

	nExitFlag = 0;

	DWORD ID;
	hThread = CreateThread( NULL, 0, MolestorLoop, &nInterval, 0, &ID);
	return hThread!=NULL;
}

void StopMolestor()
{
	InterlockedIncrement(&nExitFlag);
	WaitForSingleObject(hThread, INFINITE);
	hThread = NULL;
}

}	// End of namespace Molestor//*/


//
// EAP Listener - Listen and process EAPOL packets
// 
namespace EapProcess {

// Thread managements
volatile LONG nExitFlag	= 0;
HANDLE	hThread	= NULL;

void (*fpEventhandler)(EAP_EVENT id, EAP_STATE state);			// Callback function pointer


EAP_STATE	state = UNCONNECTED;								// Keep current connection state
CTime		connectedTime = CTime::GetCurrentTime();			// Keep connected moment.

// Thread to read EAP and response.
unsigned long __stdcall ListenerLoop(LPVOID param)
{
	while (0==nExitFlag) {

		if (!io->receive(true)) {		// Read timeout.
//			if (CONNECTED!=state) {
//				fpEventhandler(EAP_TIMEOUT, state);
//				InterlockedIncrement(&nExitFlag);		// Exit loop 
//			}
			continue;
		}
	    
	    //  EAPOL
	    //
		eap::eapol::packet payload = io->input();
	    switch(payload.type()) {
			case eap::eapol::pckt:
				break;

			case eap::eapol::start:
			case eap::eapol::logoff:
				eap::out::info("invalid EAPOL frame of type %d", (ui8)payload.type());
				continue;
			
			default:
				eap::out::info("ignoring unsupported EAPOL type %d", (ui8)payload.type());
				continue;
	    }

	    //  EAP
	    //
		switch(auth->process(payload.body(), io->output().body())) {
			case eap::eap::ret_cont:
				if (CONNECTED==state ||RECONNECTING==state)
					state = RECONNECTING;
				else {
					fpEventhandler(EAP_REQUEST_ID, state);
					state = CONNECTING;
				}
				break;

			case eap::eap::ret_ok:
				if ( CONNECTING == state ) { // Do not handle reconnecting here.
					fpEventhandler(EAP_SUCCESS, state);
					state = CONNECTED;

					// Save the connected time
					connectedTime = CTime::GetCurrentTime();
				}
				continue;

			case eap::eap::ret_inv:
			case eap::eap::ret_abort:
				continue;

			case eap::eap::ret_fail:
				fpEventhandler(EAP_FAIL, state);
				state = UNCONNECTED;
				goto Exit;

			case eap::eap::ret_old:
				io->retransmit_last();
				continue;

			default:
				continue;
		}


	    // output
	    //
	    io->make_eapol(eap::eapol::pckt);
	    io->send();
	}

	io->make_eapol(eap::eapol::logoff);
	io->send();

Exit:
	delete auth;
	delete io;
	auth = NULL;
	io = NULL;
	hThread = NULL;
	
	return 0;
}

bool StartListener(int nTimeout)
{
	ASSERT(!hThread);

	nExitFlag = 0;

	DWORD ID;
	hThread = CreateThread( NULL, 0, ListenerLoop, &nTimeout, 0, &ID);
	return hThread!=NULL;
}

void StopListener()
{
	if (hThread ) {
		if(!TerminateThread(hThread,0)){
			AfxMessageBox("Fatal Error: cannot terminate the listener thread"); 
			exit(0);
		}
		//wait the end of the capture thread
		WaitForSingleObject(hThread,INFINITE);
	}

	if (io) {
		try {
			io->make_eapol(eap::eapol::logoff);
			io->send();
		}catch(...){}
	}
	
	EapProcess::state = UNCONNECTED;
}

}	// End of Namespace EapProcess

//
// Get current connection state
//
__declspec(dllexport) EAP_STATE GetCurConState()
{
	return EapProcess::state;
}

//
// Get connection time
//
__declspec(dllexport) CTime GetConnectedTime() 
{
	return EapProcess::connectedTime;
}

//
// Start the login procedure and return immediatly
// 
__declspec(dllexport) void Login( 
			const char *eth_if,		// name of the ethernet interface to be used.
			const char *user,			// user name
			const char *password,
			void (*p)(EAP_EVENT evt, EAP_STATE state)	// call back event handler.
		 )
{
	EapProcess::fpEventhandler	= p;

	ASSERT( !auth && !io);		// Make sure they both NULL.

	io = new eap::ll_802_3(eth_if);
	io->setup_output();

	auth = new eap::eap();
	auth->add_handler(new eap::type_notification());
	auth->add_handler(new eap::type_identity((ui8 *)user, strlen(user)));
	auth->add_handler( new eap::type_md5( 
						(ui8 *)(user), 
						strlen(user), 
						(ui8 *)password, 
						strlen(password))
					  );

	// Send EAPOL_START 
	io->make_eapol(eap::eapol::start);
	io->send();


	// Start Molestor thread to repeat EAPOL_START over LAN.
	// VERIFY(Molestor::StartMolestor(2));

	// Start EAP process routine.
	VERIFY(EapProcess::StartListener(2));
}

//
// Disconnect
//
__declspec(dllexport) void Logout()
{
	EapProcess::StopListener();

	delete auth;
	auth = NULL;
	delete io;
	io = NULL;
	EapProcess::hThread = NULL;
}

//
// Do DHCP request
//
#include <Iphlpapi.h>
__declspec(dllexport) void DhcpRequest()
{
	// Before calling IpReleaseAddress and IpRenewAddress we use
	// GetInterfaceInfo to retrieve a handle to the adapter
	PIP_INTERFACE_INFO pInfo;
	pInfo = (IP_INTERFACE_INFO *) malloc( sizeof(IP_INTERFACE_INFO) );
	ULONG ulOutBufLen = 0;
	DWORD dwRetVal = 0;

	// Make an initial call to GetInterfaceInfo to get
	// the necessary size into the ulOutBufLen variable
	if ( GetInterfaceInfo(pInfo, &ulOutBufLen) == ERROR_INSUFFICIENT_BUFFER) {
		free(pInfo);
		pInfo = (IP_INTERFACE_INFO *) malloc (ulOutBufLen);
	}

	// Make a second call to GetInterfaceInfo to get the
	// actual data we want
	if ((dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen)) == NO_ERROR ) {
		printf("\tAdapter Name: %ws\n", pInfo->Adapter[0].Name);
		printf("\tAdapter Index: %ld\n", pInfo->Adapter[0].Index);
		printf("\tNum Adapters: %ld\n", pInfo->NumAdapters);
	}
	else {
		printf("GetInterfaceInfo failed.\n");
		LPVOID lpMsgBuf;
				
		if (FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwRetVal,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL )) {
			printf("\tError: %s", lpMsgBuf);
		}
		LocalFree( lpMsgBuf );
	}

	// Call IpReleaseAddress and IpRenewAddress to release and renew
	// the IP address on the specified adapter.
	if ((dwRetVal = IpReleaseAddress(&pInfo->Adapter[0])) == NO_ERROR) {
		printf("IP release succeeded.\n");
	}
	else {
		printf("IP release failed.\n");
	}

	if ((dwRetVal = IpRenewAddress(&pInfo->Adapter[0])) == NO_ERROR) {
		printf("IP renew succeeded.\n");
	}
	else {
		printf("IP renew failed.\n");
	}
}
