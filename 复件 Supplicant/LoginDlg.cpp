// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Supplicant.h"

#include "TrayIcon.h"
#include "ConnectingDlg.h"
#include "LoginDlg.h"
#include "ConnectionDlg.h"
#include "PropertyPages.h"
#include "DownloadProgressDlg.h"

#include "EAP/EAP-DLL.h"

void eap_evt_handler(EAP_EVENT evt, EAP_STATE state);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// LoginDlg 对话框
LoginDlg::LoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LoginDlg::IDD, pParent), 
	m_nKeepPassword(0), m_nAutoRun(0),
	m_nAutoConnect(0), m_nDhcpEnable(0),
	m_nTimer(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_USERNAME, m_strUserName);
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Check(pDX, IDC_KEEP_PASSWORD, m_nKeepPassword);
	DDX_Check(pDX, IDC_AUTO_RUN, m_nAutoRun);
	DDX_Check(pDX, IDC_AUTO_CONNECT, m_nAutoConnect);
	DDX_Check(pDX, IDC_DHCP_ENABLE, m_nDhcpEnable);
}

BEGIN_MESSAGE_MAP(LoginDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(ID_CONNECT, OnConnect)
	ON_BN_CLICKED(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_EXIT, OnCancel)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_SHOW_CONNECTION, OnDisplayConnection)
	ON_COMMAND(ID_DISCONNECT, OnDisconnect)
	ON_MESSAGE(WM_TRAYICON_CALLBACK, OnTrayIconMsg)		// Message from TrayIcon
	ON_WM_TIMER()
	ON_COMMAND(ID_ONLINE_UPDATE, OnOnlineUpdate)
END_MESSAGE_MAP()


// LoginDlg 消息处理程序
BOOL LoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将\“关于...\”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// Setup the tray icon
	m_wndTrayIcon.Init( GetSafeHwnd() );

	// Create the Connecting dialog but hide it now
	m_wndConnectingDlg.Create(IDD_CONNECTING, this);
	m_wndConnectingDlg.CenterWindow();

	// Setup dialog elements value from registry
	m_strUserName = theApp.GetProfileString("Main", "UserName");
	m_nKeepPassword = theApp.GetProfileInt("Running", "KeepPassword", 0);
	if (m_nKeepPassword) 
		m_strPassword = theApp.GetProfileEncrypt("Main", "Password", "");

	m_nAutoRun = theApp.GetProfileInt("Running", "AutoRun", 0);
	m_nAutoConnect = theApp.GetProfileInt("Running", "AutoConnect", 0);
	m_nDhcpEnable = theApp.GetProfileInt("Running", "DhcpEnable", 0);
	
	UpdateData(false);

	// If no network card name found in registry, we popup Properties Dialog 
	// to let user specify the selected network card.
	if (theApp.GetProfileString("Common", "NetworkCardName", "").IsEmpty()) 
		PostMessage(WM_COMMAND, ID_PROPERTIES);

	// If auto-connect is checked, we do connect
	if(m_nAutoConnect)
		PostMessage(WM_COMMAND, ID_CONNECT);

	// Start timer
	m_nTimer = SetTimer(2, 1000, 0);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// Clean the Tray Icon, save settings to registry
void LoginDlg::OnDestroy()
{
	// Save settings into registry.
	UpdateData(true);

	theApp.WriteProfileString("Main", "UserName", m_strUserName);
	if (m_nKeepPassword)
		theApp.WriteProfileEncrypt("Main", "Password", m_strPassword.GetBuffer());

	theApp.WriteProfileInt("Running", "KeepPassword", m_nKeepPassword);
	theApp.WriteProfileInt("Running", "AutoRun", m_nAutoRun);
	theApp.WriteProfileInt("Running", "AutoConnect", m_nAutoConnect);
	theApp.WriteProfileInt("Running", "DhcpEnable", m_nAutoConnect);

	theApp.SetAutoRun(m_nAutoRun);		// If auto-run checked, set a auto run value in registry.

	// Destroy the Tray Icon
	m_wndTrayIcon.Exit(GetSafeHwnd());

	// Kill the timer
	KillTimer(m_nTimer);

	CDialog::OnDestroy();
}

void LoginDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
		OnAppAbout();
	else
		CDialog::OnSysCommand(nID, lParam);
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void LoginDlg::OnPaint() 
{
	if (IsIconic())	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialog::OnPaint();
	}
}

void LoginDlg::OnCancel()
{
	if (CONNECTED == GetCurConState()) 
		if (IDYES != AfxMessageBox(IDS_CONNECTED_EXIT_CONFIRM,MB_YESNO))
			return;
		else
			OnDisconnect();

	CDialog::OnCancel();
	DestroyWindow();
}

// Handle messages from TrayIcon
//
LRESULT LoginDlg::OnTrayIconMsg(WPARAM wParam,LPARAM lParam)
{
	switch (lParam)
	{
	case WM_LBUTTONDOWN:
		TRACE0("WM_LBUTTONDOWN\n");
		break;
	case WM_RBUTTONDOWN:
		{
			TRACE0("WM_RBUTTONDOWN\n");

			POINT pt;
			GetCursorPos(&pt);			//得到鼠标位置

			CMenu PopMenu;
			PopMenu.LoadMenu(IDR_MAINFRAME);
			PopMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,pt.x,pt.y,this);
			PopMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,pt.x,pt.y,this);	// Strange, Need call twice here

			//资源回收
			HMENU hmenu=PopMenu.Detach();
			PopMenu.DestroyMenu();
		}
		break;
	case WM_LBUTTONDBLCLK:
		OnDisplayConnection();
		break;
	default:
		//TRACE0("UNIMPLEMENTED\n");
		break;
	}

	return 0;
}

void LoginDlg::OnProperties()
{
	// Save Check Buttons in to Registry so that the PropertieUpdsDlg have same value as LoginDlg.
	UpdateData(true);
	theApp.WriteProfileInt("Running", "KeepPassword", m_nKeepPassword);
	theApp.WriteProfileInt("Running", "AutoRun", m_nAutoRun);
	theApp.WriteProfileInt("Running", "AutoConnect", m_nAutoConnect);
	theApp.WriteProfileInt("Running", "DhcpEnable", m_nAutoConnect);

	// Show Properties Dlg.
	PropertiesDlg dlg;
	dlg.DoModal();

	// Update the Check Buttons in the LoginDlg
	m_nKeepPassword		= theApp.GetProfileInt("Running", "KeepPassword", 0);
	m_nAutoRun			= theApp.GetProfileInt("Running", "AutoRun", 0);
	m_nAutoConnect		= theApp.GetProfileInt("Running", "AutoConnect", 0);
	UpdateData(false);
}

void LoginDlg::OnHelp() 
{
	char buf[512];
	GetModuleFileName( AfxGetApp()->m_hInstance, buf, sizeof(buf) );
//	string strFileName = buf;
//	string strFilePath = strFileName.substr(0, strFileName.find_last_of('\\')+1);
//	string strHelpFile = strFilePath+"hlp\\Readme.htm";

//	ShellExecute(NULL,"open",strHelpFile.c_str(),NULL,NULL,SW_SHOWNORMAL );
}

void LoginDlg::OnAppAbout() 
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void LoginDlg::OnDisplayConnection() 
{
	ConnectionDlg dlg;
	dlg.DoModal();
}

void LoginDlg::OnDisconnect() 
{
	Logout();
	m_wndTrayIcon.Reset();			// Reset the tray icon info

	ShowWindow(SW_SHOW);
}

void LoginDlg::OnConnect()
{
	UpdateData(true);

	char token = theApp.GetProfileInt("Common", "BJFU_Token", 10);	// default token as '\n'
	bool bjfu_login = theApp.GetProfileInt("Common", "BJFU_Login", 1); // default to use BJFU login

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
		return free (pAdapterInfo);
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
		PropertiesDlg dlg;
		dlg.DoModal();
		return;
	}

	//
	// Get adapter's info
	//

	if(pAdapter->DhcpEnabled)
		bjfu_login = false;		// Do not do BJFU login for DHCP client

	CString ip = pAdapter->IpAddressList.IpAddress.String;

	// Make mac address as format of "00:00:00:00:00:00"
	CString mac;
	unsigned char buf[MAX_ADAPTER_ADDRESS_LENGTH];
	memcpy(buf, pAdapter->Address, min(pAdapter->AddressLength, MAX_ADAPTER_ADDRESS_LENGTH));
	mac.Format("%02X:%02X:%02X:%02X:%02X:%02X", buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	free (pAdapterInfo);


	//
	// Do the login now
	//
	try {
		Login(	theApp.GetProfileString("Common", "NetworkCardName"),
				bjfu_login ? (m_strUserName+token+ip+token+mac) : m_strUserName,
				m_strPassword,
				eap_evt_handler
				);
	} catch(...) {
		AfxMessageBox(IDS_CABLE_DISCONNECTED);
		return Logout();				// Necessarily cleaning ups 
	}

	ShowWindow(SW_HIDE);

	// Reset the connectiong dialog message here.
	CString msg;
	msg.LoadString(IDS_SEARCHING_SERVER);
	m_wndConnectingDlg.SetDlgItemText(IDC_LOGIN_MSG, msg);
	m_wndConnectingDlg.ShowWindow(SW_SHOW);
}

// Do idle connection disconnect in OnTimer
void LoginDlg::OnTimer(UINT nIDEvent)
{
	if (CONNECTED == GetCurConState() 
		&& theApp.GetProfileInt("Running", "DisconnectOnIdle", 0) ) 
	{
		static unsigned long recvPkts	= 0;
		static unsigned long sentPkts	= 0;
		static int idleCounter			= 0;

		int timeout = theApp.GetProfileInt("Running", "IdleTimeout", 60);

		MIB_IPSTATS stat;
		GetIpStatistics(&stat);
		if (recvPkts!=stat.dwInReceives || sentPkts!=stat.dwOutRequests) {
			recvPkts = stat.dwInReceives;
			sentPkts = stat.dwOutRequests;
			idleCounter = 0;
		} else {
			idleCounter++;
		}

		if (idleCounter>timeout) {
			OnDisconnect();
			idleCounter=0;
		}
	}

	CDialog::OnTimer(nIDEvent);
}

void LoginDlg::OnOnlineUpdate()
{
	CDownloadProgressDlg dlg;

	switch(theApp.GetVersionUpdateURL(dlg.m_strURL)) {
		case SupplicantApp::CONNECTION_FAIL:
			AfxMessageBox(IDS_UPDATE_CONNECT_FAIL);
			return;
			
		case SupplicantApp::SERVER_RESPONSE_ERR:
			AfxMessageBox(IDS_UPDATE_SERVER_RESPONSE_ERR);
			return;

		case SupplicantApp::VERSION_UPTODATE:
			AfxMessageBox(IDS_UPDATE_UPTODATE);
			return;

		case SupplicantApp::HAS_NEW_VERSION:
			if (IDYES != AfxMessageBox(IDS_HAS_NEW_VERSION, MB_YESNO))
				return;

			if (IDOK==dlg.DoModal()) {

				// Prepare to exit.
				AfxMessageBox(IDS_RESTART_TO_UPDATE);
				OnDisconnect();
				DestroyWindow();

				// get current EXE file path name with short file name
				TCHAR buf[MAX_PATH];
				TCHAR szModule[MAX_PATH];
				GetModuleFileName(0,buf,MAX_PATH);
				if (GetShortPathName(buf, szModule, MAX_PATH)<=0) {
					AfxMessageBox(IDS_GET_SHORT_FILENAME_ERR, MB_ICONERROR);
					return;
				}

				// Convert downloaded filename to short name
				if (GetShortPathName(dlg.m_strSaveFile.GetBuffer(), buf, MAX_PATH)<=0) {
					AfxMessageBox(IDS_GET_SHORT_FILENAME_ERR, MB_ICONERROR);
					return;
				}
				dlg.m_strSaveFile = buf;

				// build a bat file to del old file, copy new file, and start new file.
				// a copy of this bat file is 复件 restart.bat
				std::ofstream batfile("restart.bat");
				batfile <<":repeat\n"
					<<"del "<<szModule<<"\n"
					<<"if exist "<<szModule<<" goto repeat\n"
					<<"copy "<<(const char*)dlg.m_strSaveFile<<" "<<szModule<<"\n"
					<<szModule<<"\n"
					<<"del restart.bat\n"<<std::endl;
				batfile.close();
				WinExec("restart.bat", SW_HIDE);
				return;
			} 
	}
}
