// TranIcon.cpp : implementation file
//

#include "stdafx.h"
#include "Supplicant.h"
#include "TrayIcon.h"

/////////////////////////////////////////////////////////////////////////////
// CTrayIcon window


IMPLEMENT_DYNAMIC(CTrayIcon, CWnd)

CTrayIcon::CTrayIcon() :
	m_hParentWnd(0),
	m_strTip(""),
	m_nCallbackMsgID(0)
{
	m_hIcon = theApp.LoadIcon(IDR_MAINFRAME);
}

CTrayIcon::~CTrayIcon()
{
}


BEGIN_MESSAGE_MAP(CTrayIcon, CWnd)
END_MESSAGE_MAP()

void CTrayIcon::Init(HWND hWnd)
{
	m_hParentWnd = hWnd;
	Reset();
}

void CTrayIcon::Update()
{
	Exit(m_hParentWnd);
	DWORD dwMessage;
	dwMessage = NIM_ADD;

	NOTIFYICONDATA notifyIconData;
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = m_hParentWnd;
	notifyIconData.uID = 0;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = m_nCallbackMsgID;
	notifyIconData.hIcon = m_hIcon;
	strcpy( &notifyIconData.szTip[0], m_strTip );

	Shell_NotifyIcon(dwMessage, &notifyIconData);
}

void CTrayIcon::Exit(HWND hWnd)
{
	DWORD dwMessage;
	dwMessage = NIM_DELETE;

	NOTIFYICONDATA notifyIconData;
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = hWnd;
	notifyIconData.uID = 0;
	notifyIconData.uFlags = NULL;
	notifyIconData.uCallbackMessage = NULL;
 	notifyIconData.hIcon = NULL;
 	strcpy( &notifyIconData.szTip[0], "" );

	Shell_NotifyIcon(dwMessage, &notifyIconData);
}
