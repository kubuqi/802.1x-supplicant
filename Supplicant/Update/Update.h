// Update.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error 在包含用于 PCH 的此文件之前包含“stdafx.h”
#endif

#include "resource.h"		// 主符号


// UpdateApp:
// 有关此类的实现，请参阅 Update.cpp
//

class UpdateApp : public CWinApp
{
public:
	UpdateApp();
	virtual BOOL InitInstance();

	enum VersionState{
		CONNECTION_FAIL,		// indicate failed to connect with update server
		SERVER_RESPONSE_ERR,	// indicate server response unreadable
		VERSION_UPTODATE,		// indicate current version is up to date.
		HAS_NEW_VERSION,		// sever has new version, update needed.
	};

	VersionState GetUpdateURL(CString & url);

	DECLARE_MESSAGE_MAP()
};

extern UpdateApp theApp;
