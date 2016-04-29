#pragma once

///////////////////////////////////////////////////////////////////////////////
// "常规属性"属性页
class CommonPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CommonPropertyPage)

public:
	CommonPropertyPage();
	virtual ~CommonPropertyPage();

	enum DlgID{ IDD = IDD_COMMON_PAGE };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnNetworkInterfaceSelect();

	// Members
	CComboBox		m_wndAccessType;
	CComboBox		m_wndAuthMethod;
	CComboBox		m_wndNetworkCardList;
	CStringArray	m_strNetworkCardNames;
	int				m_nCurrentNetworkCard;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};



///////////////////////////////////////////////////////////////////////////////
// "运行属性"属性页
class RunningPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(RunningPropertyPage)

public:
	RunningPropertyPage();
	virtual ~RunningPropertyPage();

	// Dialog Data
	enum DlgID{ IDD = IDD_RUNNING_PAGE };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	afx_msg void OnKeepPasswordClick() {	SetModified();	};
	afx_msg void OnAutoRunClick() {	SetModified();	};
	afx_msg void OnAutoConnectClick() {	SetModified();	};

	afx_msg void OnConnectionAlwaysClick(); 
	afx_msg void OnConnectionTimeoutClick();

	// Data Members
	int m_nKeepPassword;
	int m_nAutoRun;
	int	m_nAutoConnect;
	int	m_nDisconnectOnIdle;
	int m_nIdleTimeout;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};



///////////////////////////////////////////////////////////////////////////////
// "扩展属性"属性页
class ExtendedPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(ExtendedPropertyPage)

public:
	ExtendedPropertyPage();
	virtual ~ExtendedPropertyPage();

	// Dialog Data
	enum DlgID{ IDD = IDD_EXTENDED_PAGE };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

	afx_msg void OnDisableUpdateClick()
	{
		SetModified(); 
		GetDlgItem(IDC_ONLINE_UPDATE_INTERVAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UPDATE_INTERVAL_TEXT)->ShowWindow(SW_HIDE);
	};
	afx_msg void OnEnableUpdateClick()
	{
		SetModified(); 
		GetDlgItem(IDC_ONLINE_UPDATE_INTERVAL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UPDATE_INTERVAL_TEXT)->ShowWindow(SW_SHOW);
	};

	// Members
	int m_nReconnectInterval;
	int m_nReconnectRetries;

	int m_nEnableOnlineUpdate;
	int m_nOnlineUpdateInterval;
	int m_nLastUpdateTime;
};


///////////////////////////////////////////////////////////////////////////////
// "网络属性"属性页
class NetworkPropertyPage : public CPropertyPage
{
	DECLARE_DYNAMIC(NetworkPropertyPage)

public:
	NetworkPropertyPage();
	virtual ~NetworkPropertyPage();

	// Dialog Data
	enum DlgID{ IDD = IDD_NETWORK_PAGE };

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	CComboBox		m_ctlIpObtainType;
	CIPAddressCtrl	m_ctlIpAddr;	
	CIPAddressCtrl	m_ctlGatewayAddr;	
	CIPAddressCtrl	m_ctlDhcpServerAddr;	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// "属性设置"对话框，包含上面所有的属性页
class PropertiesDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(PropertiesDlg)

// Construction
public:
	PropertiesDlg(UINT nIDCaption=IDS_PROPERTY_SETTING_DLG, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	PropertiesDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

	virtual ~PropertiesDlg();

	// Property pages
	CommonPropertyPage		m_pgCommon;
	ExtendedPropertyPage	m_pgExtended;
	NetworkPropertyPage		m_pgNetwork;
	RunningPropertyPage		m_pgRunning;

	// Generated message map functions
	//{{AFX_MSG(PropertiesDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

