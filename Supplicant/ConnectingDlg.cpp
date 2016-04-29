// ConnectingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Supplicant.h"

#include "TrayIcon.h"
#include "ConnectingDlg.h"
#include "LoginDlg.h"

// ConnectingDlg dialog

IMPLEMENT_DYNAMIC(ConnectingDlg, CDialog)
ConnectingDlg::ConnectingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ConnectingDlg::IDD, pParent)
{
}

ConnectingDlg::~ConnectingDlg()
{
}

void ConnectingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ConnectingDlg, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// ConnectingDlg message handlers

void ConnectingDlg::OnBnClickedCancel()
{
	((LoginDlg *)GetParent())->OnDisconnect();
	OnCancel();
}
