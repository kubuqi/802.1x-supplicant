#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxmt.h"

/////////////////////////////////////////////////////////////////////////////
// 下载进度对话框

class CDownloadProgressDlg : public CDialog
{
	/////////////////////////////////////////////////////////////////////////////
	// CBSCallbackImpl
	class CBSCallbackImpl : public IBindStatusCallback
	{
	public:
		CBSCallbackImpl(HWND hWnd, HANDLE hEventStop);

		// IUnknown methods
		STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
		STDMETHOD_(ULONG, AddRef)();
		STDMETHOD_(ULONG, Release)();

		// IBindStatusCallback methods
		STDMETHOD(OnStartBinding)(DWORD, IBinding *);
		STDMETHOD(GetPriority)(LONG *);
		STDMETHOD(OnLowResource)(DWORD);
		STDMETHOD(OnProgress)(ULONG ulProgress,
							ULONG ulProgressMax,
							ULONG ulStatusCode,
							LPCWSTR szStatusText);
		STDMETHOD(OnStopBinding)(HRESULT, LPCWSTR);
		STDMETHOD(GetBindInfo)(DWORD *, BINDINFO *);
		STDMETHOD(OnDataAvailable)(DWORD, DWORD, FORMATETC *, STGMEDIUM *);
		STDMETHOD(OnObjectAvailable)(REFIID, IUnknown *);

	protected:
		ULONG m_ulObjRefCount;

	private:
		HWND m_hWnd;
		HANDLE m_hEventStop;
	};

	struct DOWNLOADSTATUS
	{
		ULONG ulProgress;
		ULONG ulProgressMax;
		ULONG ulStatusCode;
		LPCWSTR szStatusText;
	};

	struct DOWNLOADPARAM
	{
		HWND hWnd;
		HANDLE hEventStop;
		CString strURL;
		CString strFileName;
	};

	DECLARE_DYNAMIC(CDownloadProgressDlg)

public:
	CDownloadProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDownloadProgressDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DOWNLOAD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg LRESULT OnEndDownload(WPARAM, LPARAM);
	afx_msg LRESULT OnDisplayStatus(WPARAM, LPARAM lParam);
//	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

// 成员控件
public:
	CString m_strURL;
	CString m_strSaveFile;

protected:
	CProgressCtrl m_progressBar;		// 显示的下载进度条
	CString m_strProgress;				// 显示类似"已经下载333字节中的222字节"

	CButton	m_btnExit;

	CWinThread *m_pDownloadThread;
	DOWNLOADPARAM m_downloadParam;
	CEvent m_eventStop;

	void ChangeUIDownloading(bool bDownloading = true);
	static UINT Download(LPVOID pParam);
};
