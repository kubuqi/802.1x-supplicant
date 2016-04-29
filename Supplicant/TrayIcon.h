#pragma once

#define WM_TRAYICON_CALLBACK	WM_USER+100		// User defined message id

/////////////////////////////////////////////////////////////////////////////
// CTrayIcon window

class CTrayIcon : public CWnd
{
	DECLARE_DYNAMIC(CTrayIcon)

public:
	CTrayIcon();
	virtual ~CTrayIcon();

	void Init(HWND hWnd);
	void Exit(HWND hWnd=NULL);
	
	void SetTrayText(const char * text)
	{
		m_strTip = text;
		Update();
	};

	void SetTrayText(int RES_ID)
	{
		m_strTip.LoadString(RES_ID);
		Update();
	};

	void SetTrayIcon(int RES_ID)
	{
		m_hIcon = theApp.LoadIcon(RES_ID);
		Update();
	};

	void SetTrayMsg(int MsgID)
	{
		m_nCallbackMsgID = MsgID;
		Update();
	};

	void Reset()
	{
		m_strTip = "";
		m_nCallbackMsgID = 0;
		m_hIcon = theApp.LoadIcon(IDR_UNCONNECTED);
//		m_nCallbackMsgID = WM_TRAYICON_CALLBACK;		// FOR TEST ONLY

		Update();
	};

protected:
	DECLARE_MESSAGE_MAP()

	void Update();

	HWND	m_hParentWnd;
	HICON	m_hIcon;
	CString	m_strTip;
	int		m_nCallbackMsgID;
};

