// Update.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// UpdateApp:
// �йش����ʵ�֣������ Update.cpp
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
