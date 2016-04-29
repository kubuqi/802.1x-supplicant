#pragma once


// 显示正在连接、找到认证服务器、正在验证密码等认证过程的对话框

class ConnectingDlg : public CDialog
{
	DECLARE_DYNAMIC(ConnectingDlg)

public:
	ConnectingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~ConnectingDlg();

// Dialog Data
	enum { IDD = IDD_CONNECTING };

	// 点击"取消"按钮的事件处理
	afx_msg void OnBnClickedCancel();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
