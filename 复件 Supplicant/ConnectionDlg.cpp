// ConnectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Supplicant.h"
#include "ConnectionDlg.h"

#include "EAP/EAP-DLL.h"

// ConnectionDlg dialog

IMPLEMENT_DYNAMIC(ConnectionDlg, CDialog)
ConnectionDlg::ConnectionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ConnectionDlg::IDD, pParent)
	, m_nTimer(0)
	, m_strConnectedTime(_T(""))
	, m_strConnectionSpeed(_T(""))
	, m_nRecvPackets(0)
	, m_nSentPackets(0)
{
}

ConnectionDlg::~ConnectionDlg()
{
}

BOOL ConnectionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	m_nTimer = SetTimer(1, 1000, 0);
	PostMessage(WM_TIMER);		// Update the statics right after

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void ConnectionDlg::OnCancel()
{
	KillTimer(m_nTimer);

	CDialog::OnCancel();
}

void ConnectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, ID_CONNECTED_TIME,	m_strConnectedTime);
	DDX_Text(pDX, ID_CONNECTION_SPEED,	m_strConnectionSpeed);
	DDX_Text(pDX, ID_RECEIVED_PACKET,	m_nRecvPackets);
	DDX_Text(pDX, ID_SENT_PACKET,		m_nSentPackets);
}


BEGIN_MESSAGE_MAP(ConnectionDlg, CDialog)
	ON_COMMAND(ID_DISCONNECT, OnDisconnect)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// ConnectionDlg message handlers

void ConnectionDlg::OnDisconnect()
{
	OnCancel();

	// Post disconnect message to parent. 
	::PostMessage(GetParent()->GetSafeHwnd(), WM_COMMAND, ID_DISCONNECT, 0);
}

void ConnectionDlg::OnTimer(UINT nIDEvent)
{
	// Update connected timer
	CTimeSpan t = CTime::GetCurrentTime() - GetConnectedTime();
	m_strConnectedTime = t.Format(IDS_CONNECTED_TIME_TEMPLATE);

	// Update traffic statistics
	MIB_IPSTATS stat;
	GetIpStatistics(&stat);
	m_nRecvPackets = stat.dwInReceives;
	m_nSentPackets = stat.dwOutRequests;

	// TODO: Show connection speed.


	UpdateData(false);
	CDialog::OnTimer(nIDEvent);
}
