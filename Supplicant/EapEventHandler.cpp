///////////////////////////////////////////////////////////////////////////////
// Call back from EAP_ENGINE

#include "stdafx.h"
#include "Supplicant.h"

#include "TrayIcon.h"
#include "ConnectingDlg.h"
#include "LoginDlg.h"

#include "EAP/EAP-DLL.h"

void eap_evt_handler(EAP_EVENT evt, EAP_STATE state)
{
	static LoginDlg	*loginDlg = (LoginDlg*)theApp.m_pMainWnd;
	static ConnectingDlg *connDlg = &loginDlg->m_wndConnectingDlg;

	CString msg;

	ASSERT(loginDlg && connDlg);

	switch(evt) {
	case EAP_TIMEOUT:
		connDlg->ShowWindow(SW_HIDE);
		AfxMessageBox(IDS_TIME_OUT);
		loginDlg->ShowWindow(SW_SHOW);
		break;
	
	case EAP_REQUEST_ID:
		msg.LoadString(IDS_FOUND_SERVER);
		connDlg->SetDlgItemText(IDC_LOGIN_MSG, msg);
		break;

	case EAP_CHALLANGE:
		msg.LoadString(IDS_CHECKING_PWD);
		connDlg->SetDlgItemText(IDC_LOGIN_MSG, msg);
		break;

	case EAP_SUCCESS:
		if (CONNECTING==state) {
			msg.LoadString(IDS_LOGIN_OK);
			connDlg->SetDlgItemText(IDC_LOGIN_MSG, msg);
			
			if (theApp.GetProfileInt("Network", "DHCP", 0))
				DhcpRequest();

			// Check for new version update.
			if (theApp.GetProfileInt("Extended", "EnableOnlineUpdate", 0)) {
				if ( time(0) >= theApp.GetProfileInt("Extended", "LastUpdateTime", 0) 
								+ theApp.GetProfileInt("Extended", "OnlineUpdateInterval", 30)*86400 ) 
				{
					CString url;
					if (SupplicantApp::HAS_NEW_VERSION == theApp.GetVersionUpdateURL(url)) {
							PostMessage(theApp.GetMainWnd()->GetSafeHwnd(), WM_COMMAND, ID_ONLINE_UPDATE, 0);
							theApp.WriteProfileInt("Extended", "LastUpdateTime", time(0));
					}
				} 
			}

			Sleep(500);

			// Set tray icon info to connected state
			loginDlg->m_wndTrayIcon.SetTrayMsg(WM_TRAYICON_CALLBACK);
			loginDlg->m_wndTrayIcon.SetTrayIcon(IDR_CONNECTED);
			loginDlg->m_wndTrayIcon.SetTrayText(IDS_CONNECTED);
			
			// Hide the connecting dialog
			connDlg->ShowWindow(SW_HIDE);
		}
		break;

	case EAP_FAIL:
		if (CONNECTING==state) 
			AfxMessageBox(IDS_CHECK_FAIL);
		else if (CONNECTED==state) 
			AfxMessageBox(IDS_SERVER_DISCONNECTED);

		connDlg->ShowWindow(SW_HIDE);
		loginDlg->ShowWindow(SW_SHOW);
		loginDlg->m_wndTrayIcon.Reset();			// Reset the tray icon info
		break;

	default:
		break;
	}
}

