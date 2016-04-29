// LoginDlg.h : ͷ�ļ�
//

#pragma once

class CTrayIcon;
class ConnectingDlg;

//
// Supplicant�����ڣ��ṩ��½���棬�Լ�������Ϣ��������TrayIcon��Ϣ����
//
class LoginDlg : public CDialog
{
public:
	LoginDlg(CWnd* pParent = NULL);	// ��׼���캯��

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
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon() {	return static_cast<HCURSOR>(m_hIcon);}//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
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
