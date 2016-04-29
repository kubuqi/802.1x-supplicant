// Supplicant.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h”
#endif

#include "resource.h"

class LoginDlg;

/////////////////////////////////////////////////////////////////////////////
// SupplicantApp:

class SupplicantApp : public CWinApp
{
public:
	SupplicantApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// Set short cut in auto-run key in registry 
	void SetAutoRun(bool);

	// Retrieve a encrypted string value from INI file or registry.
	CString GetProfileEncrypt(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);

	// Sets a string value to INI file or registry with entryption
	BOOL WriteProfileEncrypt(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

	enum VersionState{
		CONNECTION_FAIL,		// indicate failed to connect with update server
		SERVER_RESPONSE_ERR,	// indicate server response unreadable
		VERSION_UPTODATE,		// indicate current version is up to date.
		HAS_NEW_VERSION,		// sever has new version, update needed.
	};
	// Query server for software update
	VersionState GetVersionUpdateURL(CString & url);
	
	DECLARE_MESSAGE_MAP()

protected:
	LoginDlg	*m_pLoginDlg;
};

extern SupplicantApp theApp;
