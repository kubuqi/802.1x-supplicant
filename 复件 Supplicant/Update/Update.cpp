// Update.cpp : ����Ӧ�ó��������Ϊ��
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


// UpdateApp ����
UpdateApp::UpdateApp()
{
}


// Ψһ��һ�� UpdateApp ����
UpdateApp theApp;


// UpdateApp ��ʼ��
BOOL UpdateApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControls()�����򣬽��޷��������ڡ�
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	// SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

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
