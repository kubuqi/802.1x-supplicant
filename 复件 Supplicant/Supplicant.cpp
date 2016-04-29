// Supplicant.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Supplicant.h"

#include "TrayIcon.h"
#include "ConnectingDlg.h"
#include "LoginDlg.h"

#include "Toolkit.h"
#include <atlbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// SupplicantApp
BEGIN_MESSAGE_MAP(SupplicantApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// SupplicantApp 构造
SupplicantApp::SupplicantApp()
{
}

SupplicantApp theApp;

BOOL SupplicantApp::InitInstance()
{
	// 安装完成后安装程序携带参数"CleanReg"调用Supplicant来清除注册表中
	// 用户的MAC地址
	if (0==strcmp(m_lpCmdLine, "CleanReg")) {
        Toolkit::CleanRegMAC();
		SetAutoRun(true);
		return false;
	}

	// 检查用户非法MAC地址设定
	if (Toolkit::HasRegMac() || Toolkit::InvalidMAC()) {
		AfxMessageBox(IDS_NETWORKADDR_INVALID);
		return false;
	}

	InitCommonControls();
	CWinApp::InitInstance();

	if (!AfxSocketInit()){
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	
	CoInitialize(NULL);

	AfxEnableControlContainer();

	SetRegistryKey(IDS_MANUFACTURER_NAME);

	m_pLoginDlg = new LoginDlg();
	VERIFY( m_pLoginDlg->Create(IDD_LOGIN_DIALOG) );
	m_pMainWnd = m_pLoginDlg;

	return m_pLoginDlg->ShowWindow(SW_SHOW);
}

int SupplicantApp::ExitInstance()
{
	CoUninitialize();

	delete m_pLoginDlg;
	return CWinApp::ExitInstance();
}


// Retrieve a encrypted string value from INI file or registry.
CString SupplicantApp::GetProfileEncrypt(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	char * buf;
	UINT buflen;
	GetProfileBinary(lpszSection, lpszEntry, (LPBYTE*)&buf, &buflen);
	for (int i=0; i<buflen; i++)
		buf[i] = buf[i]-128;		// simply hide the password from naked eyes ;)

	buf[buflen] = 0;

	return buf;
}

// Sets a string value to INI file or registry with entryption
BOOL SupplicantApp::WriteProfileEncrypt(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	char buf[512];
	UINT buflen = min(512, strlen(lpszValue));
	memcpy(buf, lpszValue, buflen);
	for(int i=0; i<buflen; i++)
		buf[i] = buf[i]+128;		// simply hide the password from naked eyes ;)

	WriteProfileBinary(lpszSection, lpszEntry, (LPBYTE)buf, buflen);
	return true;
}

void SupplicantApp::SetAutoRun(bool set)
{
	char szProgramPath[MAX_PATH];
	memset(szProgramPath, 0, MAX_PATH);
	GetModuleFileName(0, szProgramPath, MAX_PATH);

	CRegKey key;
	key.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	
	if (set)
		key.SetStringValue(m_pszAppName, szProgramPath);
	else
		key.DeleteValue(m_pszAppName);	
}

// Query server for software update
#include <afxinet.h>
SupplicantApp::VersionState SupplicantApp::GetVersionUpdateURL(CString & url)
{
	CInternetSession session;
	CStdioFile* pHttpFile = NULL;

	CString url_template = GetProfileString("Extended", "UpdateURL", "http://www.bjfu.edu.cn/update.asp?AppName=%s&Version=%s");
	CString request, version;
	version.LoadString(IDS_VERSION);
	request.Format((const char*)url_template, m_pszAppName, version);

	CString result;

	try	{
		pHttpFile = session.OpenURL(request);
		pHttpFile->ReadString(result);
	} catch( CInternetException* e) {
		delete pHttpFile;
		session.Close();
		e->Delete();
		return CONNECTION_FAIL;
	}

	delete pHttpFile;
	session.Close();

	result.TrimLeft();
	result.TrimRight();

	if ( 0 == result.Find("http://") ) {
		url = result;
		return HAS_NEW_VERSION;
	}

	if (-1 != result.Find("UP TO DATE")) 
		return VERSION_UPTODATE;

	return SERVER_RESPONSE_ERR;
}

/*
namespace {
// CreateLink - uses the shell's IShellLink and IPersistFile interfaces 
//   to create and store a shortcut to the specified object. 
// Returns the result of calling the member functions of the interfaces. 
// lpszPathObj - address of a buffer containing the path of the object 
// lpszPathLink - address of a buffer containing the path where the 
//   shell link is to be stored 
// lpszDesc - address of a buffer containing the description of the 
//   shell link 
 
HRESULT CreateShortCut(LPCSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc) 
{ 
    HRESULT hres; 
    IShellLink* psl; 
 
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
    if (SUCCEEDED(hres)) { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target, and add the 
        // description. 
        psl->SetPath(lpszPathObj); 

        psl->SetDescription(lpszDesc); 
 
       // Query IShellLink for the IPersistFile interface for saving the 
       // shortcut in persistent storage. 
        hres = psl->QueryInterface( IID_IPersistFile, (void**)&ppf); 
 
        if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH]; 
 
            // Ensure that the string is ANSI. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1,(LPWSTR)wsz, MAX_PATH); 

             // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save((LPWSTR)wsz, TRUE); 
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return hres; 
} 

}// End of anonymouse namespace
//*/
