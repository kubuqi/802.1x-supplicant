#pragma once


// ��ʾ�������ӡ��ҵ���֤��������������֤�������֤���̵ĶԻ���

class ConnectingDlg : public CDialog
{
	DECLARE_DYNAMIC(ConnectingDlg)

public:
	ConnectingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ConnectingDlg();

// Dialog Data
	enum { IDD = IDD_CONNECTING };

	// ���"ȡ��"��ť���¼�����
	afx_msg void OnBnClickedCancel();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
