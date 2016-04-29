// PropertyPages.cpp : implementation file
//

#include "stdafx.h"
#include "Supplicant.h"
#include "PropertyPages.h"


///////////////////////////////////////////////////////////////////////////////
// CommonPropertyPage dialog
#include <PCAP.H>

IMPLEMENT_DYNAMIC(CommonPropertyPage, CPropertyPage)
CommonPropertyPage::CommonPropertyPage()
	: CPropertyPage(CommonPropertyPage::IDD), m_nCurrentNetworkCard(0)
{
}

CommonPropertyPage::~CommonPropertyPage()
{
}

void CommonPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NETWORK_INTERFACE, m_wndNetworkCardList);
	DDX_Control(pDX, IDC_ACCESS_SERVER_TYPE, m_wndAccessType);
	DDX_Control(pDX, IDC_AUTH_TYPE, m_wndAuthMethod);

}

BEGIN_MESSAGE_MAP(CommonPropertyPage, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_NETWORK_INTERFACE, OnNetworkInterfaceSelect)
END_MESSAGE_MAP()

BOOL CommonPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_wndAccessType.AddString("IEEE 802.1X 接入服务器");
	m_wndAccessType.SetCurSel(0);
	m_wndAuthMethod.AddString("MD5 Challenge");
	m_wndAuthMethod.SetCurSel(0);

	// Obtain network interface list
	pcap_if_t *alldevs, *d;
	char errbuf[PCAP_ERRBUF_SIZE+1];

	if (pcap_findalldevs(&alldevs, errbuf) == -1) {
		AfxMessageBox(IDS_NO_IF_FOUND);
		return FALSE;
	}
	for(d=alldevs;d;d=d->next) {
		if (d->addresses && AF_INET==d->addresses->addr->sa_family)  {
			m_wndNetworkCardList.AddString(d->description);	// Add to the combo box
			m_strNetworkCardNames.Add(d->name);
		}
	}
	pcap_freealldevs(alldevs);

	// Get last last choice from registry
	m_nCurrentNetworkCard = theApp.GetProfileInt("Common", "NetworkCardIndex", 0);
	m_wndNetworkCardList.SetCurSel(m_nCurrentNetworkCard);

	// Write current network card name into registry
	theApp.WriteProfileString("Common", "NetworkCardName", m_strNetworkCardNames[m_nCurrentNetworkCard]);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CommonPropertyPage::OnNetworkInterfaceSelect()
{
	m_nCurrentNetworkCard = m_wndNetworkCardList.GetCurSel();
	SetModified();
}

BOOL CommonPropertyPage::OnApply()
{
	theApp.WriteProfileInt("Common", "NetworkCardIndex", m_nCurrentNetworkCard);
	theApp.WriteProfileString("Common", "NetworkCardName", m_strNetworkCardNames[m_nCurrentNetworkCard]);

	return CPropertyPage::OnApply();
}



///////////////////////////////////////////////////////////////////////////////
// RunningPropertyPage

IMPLEMENT_DYNAMIC(RunningPropertyPage, CPropertyPage)
RunningPropertyPage::RunningPropertyPage()
	: CPropertyPage(RunningPropertyPage::IDD)
{
}

RunningPropertyPage::~RunningPropertyPage()
{
}

void RunningPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_KEEP_PASSWORD, m_nKeepPassword);
	DDX_Check(pDX, IDC_AUTO_RUN, m_nAutoRun);
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_nAutoConnect);
	DDX_Radio(pDX, IDC_CONNECTION_ALWAYS, m_nDisconnectOnIdle);
	DDX_Text(pDX, IDC_IDLE_DISCONNECT_TIME, m_nIdleTimeout);
}

BEGIN_MESSAGE_MAP(RunningPropertyPage, CWnd)
	ON_BN_CLICKED(IDC_KEEP_PASSWORD, OnKeepPasswordClick)
	ON_BN_CLICKED(IDC_AUTO_RUN, OnAutoRunClick)
	ON_BN_CLICKED(IDC_AUTO_CONNECT, OnAutoConnectClick)
	ON_BN_CLICKED(IDC_CONNECTION_ALWAYS, OnConnectionAlwaysClick)
	ON_BN_CLICKED(IDC_CONNECTION_TIMEOUT, OnConnectionTimeoutClick)
END_MESSAGE_MAP()

// RunningPropertyPage message handlers

BOOL RunningPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_nKeepPassword		= theApp.GetProfileInt("Running", "KeepPassword", 0);
	m_nAutoRun			= theApp.GetProfileInt("Running", "AutoRun", 0);
	m_nAutoConnect		= theApp.GetProfileInt("Running", "AutoConnect", 0);
	m_nDisconnectOnIdle	= theApp.GetProfileInt("Running", "DisconnectOnIdle", 0);
	m_nIdleTimeout		= theApp.GetProfileInt("Running", "IdleTimeout", 60);

	UpdateData(false);

	if (!m_nDisconnectOnIdle)
		GetDlgItem(IDC_IDLE_DISCONNECT_TIME)->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL RunningPropertyPage::OnApply()
{
	theApp.WriteProfileInt("Running", "KeepPassword", m_nKeepPassword);
	theApp.WriteProfileInt("Running", "AutoRun", m_nAutoRun);
	theApp.WriteProfileInt("Running", "AutoConnect", m_nAutoConnect);
	theApp.WriteProfileInt("Running", "DisconnectOnIdle", m_nDisconnectOnIdle);
	theApp.WriteProfileInt("Running", "IdleTimeout", m_nIdleTimeout);

	// Set or delete the auto-run key-value in registry.
	theApp.SetAutoRun(m_nAutoRun);

	return CPropertyPage::OnApply();
}

void RunningPropertyPage::OnConnectionAlwaysClick()
{ 
	SetModified();
	GetDlgItem(IDC_IDLE_DISCONNECT_TIME)->ShowWindow(SW_HIDE);
}

void RunningPropertyPage::OnConnectionTimeoutClick()
{
	SetModified();
	GetDlgItem(IDC_IDLE_DISCONNECT_TIME)->ShowWindow(SW_SHOW);
}


///////////////////////////////////////////////////////////////////////////////
// ExtendedPropertyPage dialog

IMPLEMENT_DYNAMIC(ExtendedPropertyPage, CPropertyPage)
ExtendedPropertyPage::ExtendedPropertyPage()
	: CPropertyPage(ExtendedPropertyPage::IDD)
{
}

ExtendedPropertyPage::~ExtendedPropertyPage()
{
}

void ExtendedPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RECONNECT_INTERVAL, m_nReconnectInterval);
	DDX_Text(pDX, IDC_RECONNECT_RETRIES, m_nReconnectRetries);
	DDX_Radio(pDX, ID_DISABLE_ONLINE_UPDATE, m_nEnableOnlineUpdate);
	DDX_Text(pDX, IDC_ONLINE_UPDATE_INTERVAL, m_nOnlineUpdateInterval);
}

BEGIN_MESSAGE_MAP(ExtendedPropertyPage, CPropertyPage)
	ON_BN_CLICKED(ID_ENABLE_ONLINE_UPDATE, OnEnableUpdateClick)
	ON_BN_CLICKED(ID_DISABLE_ONLINE_UPDATE, OnDisableUpdateClick)
END_MESSAGE_MAP()


// ExtendedPropertyPage message handlers
BOOL ExtendedPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_nReconnectInterval	= theApp.GetProfileInt("Extended", "ReconnectInterval", 10);
	m_nReconnectRetries		= theApp.GetProfileInt("Extended", "ReconnectRetries", 3);
	m_nEnableOnlineUpdate	= theApp.GetProfileInt("Extended", "EnableOnlineUpdate", 0);
	m_nOnlineUpdateInterval	= theApp.GetProfileInt("Extended", "OnlineUpdateInterval", 30);
	m_nLastUpdateTime		= theApp.GetProfileInt("Extended", "LastUpdateTime", time(0));

	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ExtendedPropertyPage::OnApply()
{
	theApp.WriteProfileInt("Extended", "ReconnectInterval", m_nReconnectInterval);
	theApp.WriteProfileInt("Extended", "ReconnectRetries", m_nReconnectRetries);
	theApp.WriteProfileInt("Extended", "EnableOnlineUpdate", m_nEnableOnlineUpdate);
	theApp.WriteProfileInt("Extended", "OnlineUpdateInterval", m_nOnlineUpdateInterval);
	theApp.WriteProfileInt("Extended", "LastUpdateTime", m_nLastUpdateTime);

	return CPropertyPage::OnApply();
}


///////////////////////////////////////////////////////////////////////////////
// NetworkPropertyPage dialog

IMPLEMENT_DYNAMIC(NetworkPropertyPage, CPropertyPage)
NetworkPropertyPage::NetworkPropertyPage()
	: CPropertyPage(NetworkPropertyPage::IDD)
{
}

NetworkPropertyPage::~NetworkPropertyPage()
{
}

void NetworkPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_OBTAIN_TYPE,	m_ctlIpObtainType);
	DDX_Control(pDX, IDC_LOCAL_ADDR,		m_ctlIpAddr);
	DDX_Control(pDX, IDC_GATEWAY_ADDR,		m_ctlGatewayAddr);
	DDX_Control(pDX, IDC_DHCP_ADDR,			m_ctlDhcpServerAddr);
}


BEGIN_MESSAGE_MAP(NetworkPropertyPage, CPropertyPage)
END_MESSAGE_MAP()


// NetworkPropertyPage message handlers

BOOL NetworkPropertyPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//
	// Append IP address obtain method.
	//
	m_ctlIpObtainType.AddString("静态IP地址");
	m_ctlIpObtainType.AddString("DHCP获取IP地址");
	
	//
	// Look up the specific adapter info structure
	//
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc ( ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) != NO_ERROR) {
		AfxMessageBox("GetAdapterInfo failed!");
		free (pAdapterInfo);
		return false;
	}

	pAdapter = pAdapterInfo;
	CString CurrentNIC = theApp.GetProfileString("Common", "NetworkCardName");
	ASSERT(CurrentNIC.GetLength()>12);

	while (pAdapter) {
		// The Iphlp method returns AdapterName like : "{D6E60C44-CD92-4377-B517-2FD2C72F87F7}"
		// while the wpcap lib returns interface name like :"\Device\NPF_{D6E60C44-CD92-4377-B517-2FD2C72F87F7}"
		// so here we have a magic number "12" to remove the difference
		if (0==strcmp(pAdapter->AdapterName, ((const char*)CurrentNIC)+12)) 
			break;
		pAdapter = pAdapter->Next;
	}

	if (!pAdapter) {
		AfxMessageBox(IDS_NIC_NAME_CHANGED);
		free (pAdapterInfo);
		return false;
	}

	//
	// Set adapter's info
	//
	m_ctlIpAddr.EnableWindow(false);
	m_ctlGatewayAddr.EnableWindow(false);
	m_ctlDhcpServerAddr.EnableWindow(false);

	if(pAdapter->DhcpEnabled) {
		m_ctlIpObtainType.SetCurSel(0);
		m_ctlDhcpServerAddr.SetAddress(ntohl(inet_addr(pAdapter->DhcpServer.IpAddress.String)));
	} else {
		m_ctlIpObtainType.SetCurSel(1);
	}

	m_ctlIpAddr.SetAddress(ntohl(inet_addr(pAdapter->IpAddressList.IpAddress.String)));
	m_ctlGatewayAddr.SetAddress(ntohl(inet_addr(pAdapter->GatewayList.IpAddress.String)));

	free (pAdapterInfo);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL NetworkPropertyPage::OnApply()
{
	// TODO: Add your specialized code here and/or call the base class

	return CPropertyPage::OnApply();
}

///////////////////////////////////////////////////////////////////////////////
// PropertiesDlg

IMPLEMENT_DYNAMIC(PropertiesDlg, CPropertySheet)

PropertiesDlg::PropertiesDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pgCommon);
	AddPage(&m_pgRunning);
	AddPage(&m_pgExtended);
	AddPage(&m_pgNetwork);
}

PropertiesDlg::PropertiesDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_pgCommon);
	AddPage(&m_pgExtended);
	AddPage(&m_pgNetwork);
	AddPage(&m_pgRunning);
}

PropertiesDlg::~PropertiesDlg()
{
}


BEGIN_MESSAGE_MAP(PropertiesDlg, CPropertySheet)
	//{{AFX_MSG_MAP(PropertiesDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PropertiesDlg message handlers

