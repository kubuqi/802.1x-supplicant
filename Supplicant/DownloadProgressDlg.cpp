// DownloadProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DownloadProgressDlg.h"


//-----------------------------------------------------------------------------
#define    WM_USER_ENDDOWNLOAD           (WM_USER + 1)
#define    WM_USER_DISPLAYSTATUS         (WM_USER + 2)
//-----------------------------------------------------------------------------


//*****************************************************************************
//-----------------------------------------------------------------------------
CDownloadProgressDlg::CBSCallbackImpl::CBSCallbackImpl(HWND hWnd, HANDLE hEventStop)
{
	m_hWnd = hWnd;  // the window handle to display status

	m_hEventStop = hEventStop;  // the event object to signal to stop

	m_ulObjRefCount = 1;
}
//-----------------------------------------------------------------------------
// IUnknown
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::QueryInterface(REFIID riid, void **ppvObject)
{
	TRACE(_T("IUnknown::QueryInterface\n"));

	*ppvObject = NULL;
	
	// IUnknown
	if (::IsEqualIID(riid, __uuidof(IUnknown)))
	{
		TRACE(_T("IUnknown::QueryInterface(IUnknown)\n"));

		*ppvObject = this;
	}
	// IBindStatusCallback
	else if (::IsEqualIID(riid, __uuidof(IBindStatusCallback)))
	{
		TRACE(_T("IUnknown::QueryInterface(IBindStatusCallback)\n"));

		*ppvObject = static_cast<IBindStatusCallback *>(this);
	}

	if (*ppvObject)
	{
		(*reinterpret_cast<LPUNKNOWN *>(ppvObject))->AddRef();

		return S_OK;
	}
	
	return E_NOINTERFACE;
}                                             
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDownloadProgressDlg::CBSCallbackImpl::AddRef()
{
	TRACE(_T("IUnknown::AddRef\n"));

	return ++m_ulObjRefCount;
}
//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) CDownloadProgressDlg::CBSCallbackImpl::Release()
{
	TRACE(_T("IUnknown::Release\n"));

	return --m_ulObjRefCount;
}
//-----------------------------------------------------------------------------
// IBindStatusCallback
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnStartBinding(DWORD, IBinding *)
{
	TRACE(_T("IBindStatusCallback::OnStartBinding\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::GetPriority(LONG *)
{
	TRACE(_T("IBindStatusCallback::GetPriority\n"));

	return E_NOTIMPL;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnLowResource(DWORD)
{
	TRACE(_T("IBindStatusCallback::OnLowResource\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnProgress(ULONG ulProgress,
															 ULONG ulProgressMax,
															 ULONG ulStatusCode,
															 LPCWSTR szStatusText)
{
	static const LPCTSTR plpszStatus[] = 
	{
		_T("BINDSTATUS_FINDINGRESOURCE"),  // 1
		_T("BINDSTATUS_CONNECTING"),
		_T("BINDSTATUS_REDIRECTING"),
		_T("BINDSTATUS_BEGINDOWNLOADDATA"),
		_T("BINDSTATUS_DOWNLOADINGDATA"),
		_T("BINDSTATUS_ENDDOWNLOADDATA"),
		_T("BINDSTATUS_BEGINDOWNLOADCOMPONENTS"),
		_T("BINDSTATUS_INSTALLINGCOMPONENTS"),
		_T("BINDSTATUS_ENDDOWNLOADCOMPONENTS"),
		_T("BINDSTATUS_USINGCACHEDCOPY"),
		_T("BINDSTATUS_SENDINGREQUEST"),
		_T("BINDSTATUS_CLASSIDAVAILABLE"),
		_T("BINDSTATUS_MIMETYPEAVAILABLE"),
		_T("BINDSTATUS_CACHEFILENAMEAVAILABLE"),
		_T("BINDSTATUS_BEGINSYNCOPERATION"),
		_T("BINDSTATUS_ENDSYNCOPERATION"),
		_T("BINDSTATUS_BEGINUPLOADDATA"),
		_T("BINDSTATUS_UPLOADINGDATA"),
		_T("BINDSTATUS_ENDUPLOADINGDATA"),
		_T("BINDSTATUS_PROTOCOLCLASSID"),
		_T("BINDSTATUS_ENCODING"),
		_T("BINDSTATUS_VERFIEDMIMETYPEAVAILABLE"),
		_T("BINDSTATUS_CLASSINSTALLLOCATION"),
		_T("BINDSTATUS_DECODING"),
		_T("???")  // unexpected
	};

	TRACE(_T("IBindStatusCallback::OnProgress\n"));

	TRACE(_T("ulProgress: %lu, ulProgressMax: %lu\n"),
		  ulProgress, ulProgressMax);
	
	TRACE(_T("ulStatusCode: %lu "), ulStatusCode);

	if (ulStatusCode < BINDSTATUS_FINDINGRESOURCE ||
		ulStatusCode > BINDSTATUS_DECODING)
		ulStatusCode = BINDSTATUS_DECODING + 1;
	
	TRACE(_T("(%s), szStatusText: %ls\n"),
		  plpszStatus[ulStatusCode - BINDSTATUS_FINDINGRESOURCE],
		  szStatusText);

	if (m_hWnd != NULL)
	{
		// inform the dialog box to display current status,
		// don't use PostMessage
		DOWNLOADSTATUS downloadStatus =
			{ ulProgress, ulProgressMax, ulStatusCode, szStatusText };
		::SendMessage(m_hWnd, WM_USER_DISPLAYSTATUS,
					  0, reinterpret_cast<LPARAM>(&downloadStatus));
	}

	if (m_hEventStop != NULL)
	{
		if (::WaitForSingleObject(m_hEventStop, 0) == WAIT_OBJECT_0)
		{
			TRACE(_T("Canceled.\n"));
			::SendMessage(m_hWnd, WM_USER_DISPLAYSTATUS, 0, NULL);
			return E_ABORT;  // canceled by the user
		}
	}

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnStopBinding(HRESULT, LPCWSTR)
{
	TRACE(_T("IBindStatusCallback::OnStopBinding\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::GetBindInfo(DWORD *, BINDINFO *)
{
	TRACE(_T("IBindStatusCallback::GetBindInfo\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnDataAvailable(DWORD, DWORD,
											  FORMATETC *, STGMEDIUM *)
{
	TRACE(_T("IBindStatusCallback::OnDataAvailable\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
STDMETHODIMP CDownloadProgressDlg::CBSCallbackImpl::OnObjectAvailable(REFIID, IUnknown *)
{
	TRACE(_T("IBindStatusCallback::OnObjectAvailable\n"));

	return S_OK;
}
//-----------------------------------------------------------------------------
//*****************************************************************************



/////////////////////////////////////////////////////////////////////////////
// CDownloadProgressDlg dialog

IMPLEMENT_DYNAMIC(CDownloadProgressDlg, CDialog)
CDownloadProgressDlg::CDownloadProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDownloadProgressDlg::IDD, pParent)
	, m_strProgress(_T(""))
	, m_strURL(_T(""))
	, m_pDownloadThread(NULL)
{
}
CDownloadProgressDlg::~CDownloadProgressDlg()
{
}

void CDownloadProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_progressBar);
	DDX_Text(pDX, IDC_PROGRESS_TEXT, m_strProgress);
//	DDX_Control(pDX, IDOK, m_btnDownloadStop);
	DDX_Control(pDX, IDCANCEL, m_btnExit);
	DDX_Text(pDX, IDC_URL, m_strURL);
}


BEGIN_MESSAGE_MAP(CDownloadProgressDlg, CDialog)
	ON_MESSAGE(WM_USER_ENDDOWNLOAD, OnEndDownload)
	ON_MESSAGE(WM_USER_DISPLAYSTATUS, OnDisplayStatus)
END_MESSAGE_MAP()


// CDownloadProgressDlg message handlers
BOOL CDownloadProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_progressBar.SetRange(0, 100);
//	m_progressBar.SetPos(0);
	m_strProgress = "";

	USES_CONVERSION;
	if (m_strURL.IsEmpty() ||
		::IsValidURL(NULL, T2CW(m_strURL), 0) != S_OK)
	{
		::AfxMessageBox(IDS_INVALID_URL, MB_OK | MB_ICONSTOP);

		// change the UI
		this->ChangeUIDownloading(false);
		OnCancel();
		return FALSE;
	}

	// parameters to be passed to the thread
	m_downloadParam.hWnd = this->GetSafeHwnd();

	m_eventStop.ResetEvent();  // nonsignaled

	m_downloadParam.hEventStop = m_eventStop;
	m_downloadParam.strURL = m_strURL;
	m_downloadParam.strFileName.Empty();

	// create a thread to download, but don't start yet
	m_pDownloadThread = ::AfxBeginThread(CDownloadProgressDlg::Download,
											&m_downloadParam,
											THREAD_PRIORITY_NORMAL,
											0,
											CREATE_SUSPENDED);
	ASSERT(m_pDownloadThread != NULL);

	TRACE(_T("AfxBeginThread: 0x%08lX\n"), m_pDownloadThread->m_nThreadID);

	/* 
		Set the m_bAutoDelete data member to FALSE. This allows
		the CWinThread object to survive after the thread has been
		terminated. You can then access the m_hThread data member
		after the thread has been terminated. If you use this
		technique, however, you are responsible for destroying the
		CWinThread object as the framework will not automatically
		delete it for you.
	*/
	m_pDownloadThread->m_bAutoDelete = FALSE;

	// start
	m_pDownloadThread->ResumeThread();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


//-----------------------------------------------------------------------------
// sent when the downloading thread ends
// wParam and lParam: not used
// return value: not used
LRESULT CDownloadProgressDlg::OnEndDownload(WPARAM, LPARAM)
{
	ASSERT(m_pDownloadThread != NULL);
	ASSERT_KINDOF(CWinThread, m_pDownloadThread);

	TRACE(_T("OnEndDownload()\n"));

	// wait until the thread terminates
	DWORD dwExitCode;
	if (::GetExitCodeThread(m_pDownloadThread->m_hThread, &dwExitCode) &&
		dwExitCode == STILL_ACTIVE)
		::WaitForSingleObject(m_pDownloadThread->m_hThread, INFINITE);

	delete m_pDownloadThread;
	m_pDownloadThread = NULL;

	m_strSaveFile = m_downloadParam.strFileName;

	if ( m_strSaveFile=="")
		CDialog::OnCancel();
	else
		CDialog::OnOK();
	return 0;
}
//-----------------------------------------------------------------------------
// sent from the downloading thread to the dialog box to display the progress
// wParam: not used
// lParam: DOWNLOADSTATUS *, NULL if canceled
//		   {
//			   ULONG ulProgress - IN
//			   ULONG ulProgressMax - IN
//			   ULONG ulStatusCode - IN
//			   LPCWSTR szStatusText - IN
//		   }
// return value: not used
LRESULT CDownloadProgressDlg::OnDisplayStatus(WPARAM, LPARAM lParam)
{
	const DOWNLOADSTATUS *const pDownloadStatus =
		reinterpret_cast<DOWNLOADSTATUS *>(lParam);

	// form the status text
	CString strStatus;

	if (pDownloadStatus != NULL)
	{
		ASSERT(::AfxIsValidAddress(pDownloadStatus, sizeof(DOWNLOADSTATUS)));

		strStatus.LoadString(pDownloadStatus->ulStatusCode -
							 BINDSTATUS_FINDINGRESOURCE +
							 IDS_BINDSTATUS01);
		strStatus += _T("  ");
		strStatus += pDownloadStatus->szStatusText;

		CString strProgress;
		strProgress.Format(IDS_PROGRESS,
						   pDownloadStatus->ulProgress,
						   pDownloadStatus->ulProgressMax);

		strStatus += strProgress + _T("\r\n");
		m_progressBar.SetRange(0, pDownloadStatus->ulProgressMax);
		m_progressBar.SetPos(pDownloadStatus->ulProgress);
	}
	else
	{
		strStatus.LoadString(IDS_CANCELED);
		strStatus += _T("\r\n");
	}

	return 0;
}

//-----------------------------------------------------------------------------
// change the user interface
// bDownloading: true - downloading, false - not downloading
void CDownloadProgressDlg::ChangeUIDownloading(bool bDownloading /* = true */)
{
	// Exit button
	m_btnExit.EnableWindow(!bDownloading);
}

//-----------------------------------------------------------------------------
// static member function - the controlling function for the worker thread
// pParam: DOWNLOADPARAM *
//		   {
//			   HWND hWnd - IN				the window handle to display status
//			   HANDLE hEventStop - IN		the event object to signal to stop
//			   CString strURL - IN			the URL of the file to be downloaded
//			   CString strFileName - OUT	the filename of the downloaded file
//		   }
// return value: not used
UINT CDownloadProgressDlg::Download(LPVOID pParam)
{
	DOWNLOADPARAM *const pDownloadParam = static_cast<DOWNLOADPARAM *>(pParam);
	ASSERT(pDownloadParam != NULL);
	ASSERT(::AfxIsValidAddress(pDownloadParam, sizeof(DOWNLOADPARAM)));

	/*
		URLDownloadToCacheFile is a blocking function. Even though the data is
		downloaded asynchronously the function does not return until all the
		data is downloaded. If complete asynchronous downloading is desired,
		one of the other UOS functions, such as URLOpenStream, or perhaps
		general URL monikers would be more appropriate.
	*/

	CBSCallbackImpl bsc(pDownloadParam->hWnd, pDownloadParam->hEventStop);

	const HRESULT hr = ::URLDownloadToCacheFile(NULL,
												pDownloadParam->strURL,
												pDownloadParam->strFileName.
													GetBuffer(MAX_PATH),
												URLOSTRM_GETNEWESTVERSION,
												0,
												&bsc);

	/*
		The resource from the cache is used for the second and subsequent
		calls to URLDownloadToCacheFile during the session of the program
		unless the following setting is selected, in which case, every call
		to URLDownloadToCacheFile downloads the resource from the Internet.

		Control Panel/Internet/General/Temporary Internet files/Settings/
		Check for newer versions of stored pages -> Every visit to the page
	*/
	
	// empty the filename string if failed or canceled
	pDownloadParam->strFileName.ReleaseBuffer(SUCCEEDED(hr) ? -1 : 0);

	TRACE(_T("URLDownloadToCacheFile ends: 0x%08lX\nCache file name: %s\n"),
		  hr, pDownloadParam->strFileName);

	// let the dialog box know it is done
	::PostMessage(pDownloadParam->hWnd, WM_USER_ENDDOWNLOAD, 0, 0);

	return 0;
}
//-----------------------------------------------------------------------------
//*****************************************************************************
