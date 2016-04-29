// Update.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Update.h"

#include "DownloadProgressDlg.h"

#include <afxinet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// UpdateApp
BEGIN_MESSAGE_MAP(UpdateApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// UpdateApp 构造
UpdateApp::UpdateApp()
{
}


// 唯一的一个 UpdateApp 对象
UpdateApp theApp;


// UpdateApp 初始化
BOOL UpdateApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControls()。否则，将无法创建窗口。
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	// SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CDownloadProgressDlg dlg;
	m_pMainWnd = &dlg;

	switch(GetUpdateURL(dlg.m_strURL)) {
		case CONNECTION_FAIL:

		case SERVER_RESPONSE_ERR:

		case VERSION_UPTODATE:

		case HAS_NEW_VERSION:
			if (IDOK==dlg.DoModal()) {

			} else {

			}

			return FALSE;
	}
}


UpdateApp::VersionState UpdateApp::GetUpdateURL(CString & url)
{
	CInternetSession session;
	CStdioFile* pHttpFile = NULL;

	CString request = theApp.m_lpCmdLine;
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
