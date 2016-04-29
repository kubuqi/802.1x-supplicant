#pragma once


// 显示当前连接状态的对话框

class ConnectionDlg : public CDialog
{
	DECLARE_DYNAMIC(ConnectionDlg)

public:
	ConnectionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ConnectionDlg();
	virtual BOOL OnInitDialog();

	enum { IDD = IDD_CONNECTION_STATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual void OnCancel();
	afx_msg void OnDisconnect();
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()

	UINT_PTR		m_nTimer;
	CString			m_strConnectedTime;		// 已连接时长
	CString			m_strConnectionSpeed;	// 连接速度
	unsigned long	m_nRecvPackets;			// 收到的数据包数量
	unsigned long	m_nSentPackets;			// 发送的数据包数量
};
