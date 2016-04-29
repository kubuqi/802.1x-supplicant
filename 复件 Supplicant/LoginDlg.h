// LoginDlg.h : 头文件
//

#pragma once

class CTrayIcon;
class ConnectingDlg;

//
// Supplicant主窗口，提供登陆界面，以及其他消息处理，包括TrayIcon消息处理
//
class LoginDlg : public CDialog
{
public:
	LoginDlg(CWnd* pParent = NULL);	// 标准构造函数

	enum DlgID{ IDD = IDD_LOGIN_DIALOG };

	afx_msg void OnAppAbout();				// Open about box
	afx_msg void OnHelp();					// Open help file
	afx_msg void OnDisplayConnection();		// Open connection status box
	afx_msg void OnDisconnect();			// Disconnect
	afx_msg void OnConnect();				// Connect
	afx_msg void OnProperties();			// Open properties setting dialog.
	afx_msg void OnOnlineUpdate();			// Check for new version and download
	afx_msg void OnTimer(UINT nIDEvent);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon() {	return static_cast<HCURSOR>(m_hIcon);}//当用户拖动最小化窗口时系统调用此函数取得光标显示。
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam,LPARAM lParam);	    //handle message from trayicon

	DECLARE_MESSAGE_MAP()
	
	// Members
	HICON			m_hIcon;

	// Dialog UI members
	CString			m_strUserName;
	CString			m_strPassword;
	int				m_nKeepPassword;
	int				m_nAutoRun;
	int				m_nAutoConnect;
	int				m_nDhcpEnable;

	// Timer
	UINT_PTR		m_nTimer;

public:
	CTrayIcon		m_wndTrayIcon;
	ConnectingDlg	m_wndConnectingDlg;
};
