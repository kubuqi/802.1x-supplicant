#pragma once


// ��ʾ��ǰ����״̬�ĶԻ���

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
	CString			m_strConnectedTime;		// ������ʱ��
	CString			m_strConnectionSpeed;	// �����ٶ�
	unsigned long	m_nRecvPackets;			// �յ������ݰ�����
	unsigned long	m_nSentPackets;			// ���͵����ݰ�����
};
