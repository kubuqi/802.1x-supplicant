// Toolkit.h : 定义辅助工具函数
//

#include "stdafx.h"
#include <vector>

#include "Toolkit.h"

#include <atlbase.h>


// 获取当前系统平台类型，可以是：
//		VER_PLATFORM_WIN32_NT
//		VER_PLATFORM_WIN32_WINDOWS
//		VER_PLATFORM_WIN32s
int Toolkit::GetSysVersion()
{
	static int version = 0;

	if ( 0 != version)	return version;

	OSVERSIONINFOEX osvi; 
	BOOL bOsVersionInfoEx;

	memset(&osvi,0,sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);	
	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
			return 0;
	}
	
	return osvi.dwPlatformId;
}

// 注册表中是否有用户手动修改的MAC地址
// 返回：true - 有
bool Toolkit::HasRegMac()
{
	static CString NICKeyTemplate = VER_PLATFORM_WIN32_NT==GetSysVersion()
			? "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%04d"
			: "System\\CurrentControlSet\\Services\\Class\\Net\\%4d";

	CString NICKey;
	CRegKey key;

	char buf[256];
	DWORD buflen;
	for ( int i=0; i<100; i++) {
		NICKey.Format(NICKeyTemplate, i);
		if (0==key.Open(HKEY_LOCAL_MACHINE, NICKey)) {
			if (ERROR_SUCCESS==key.QueryStringValue(buf, "NetworkAddress",&buflen)) {
				key.Close();
				return true;
			}
		}
	}

	return false;
}


// 清除注册表中用户修改的MAC地址
// 返回：被清除的MAC地址数目
//
int Toolkit::CleanRegMAC()
{
	int ret = 0;		// value to be return

	static CString NICKeyTemplate = VER_PLATFORM_WIN32_NT==GetSysVersion()
			? "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%04d"
			: "System\\CurrentControlSet\\Services\\Class\\Net\\%4d";

	CString NICKey;
	CRegKey key;

	for ( int i=0; i<100; i++) {
		NICKey.Format(NICKeyTemplate, i);
		if (0==key.Open(HKEY_LOCAL_MACHINE, NICKey)) {
			if (0==key.DeleteValue("NetworkAddress"))
				++ret;
			key.Close();
		}
	}

	return ret;
}


namespace {
	const char * SecretRegKey = "SOFTWARE\\Microsoft\\IE4\\Setup";

	typedef struct {
		unsigned char addr[MAX_ADAPTER_ADDRESS_LENGTH];
	}MacAddr;
}


// 比较当前MAC地址和保存下来的MAC地址是否一致
// 如果没有保存下来的MAC地址，则保存之；
// 返回 true 表示不一致的MAC地址
//
bool Toolkit::InvalidMAC()
{
	// 取出以前保存的MAC地址列表
	CRegKey key;
	std::vector<MacAddr> addrlist;
	if (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, SecretRegKey)) {
		CString addrName;
		for (size_t i=0; i<100; i++) {
			addrName.Format("%04d", i);
			MacAddr addr;
			DWORD addrlen;
            if (ERROR_SUCCESS==key.QueryBinaryValue(addrName, addr.addr, &addrlen))
				addrlist.push_back(addr);
		}
	} else 
		 VERIFY(ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, SecretRegKey));

	//
	// 枚举当前系统MAC地址
	// 
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) != NO_ERROR)  {
		free(pAdapterInfo);
		AfxMessageBox("EtherLink:Failed to lookup MAC address !");
		return false;
	}

	pAdapter = pAdapterInfo;

	if (0==addrlist.size()) {
		// 还没有保存自己的MAC地址，现在保存
		int counter = 0;
		CString addrName;
		CRegKey key;
		VERIFY (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, SecretRegKey));

		while (pAdapter) {
			addrName.Format("%04d", counter);
			key.SetBinaryValue(addrName, pAdapter->Address, min(pAdapter->AddressLength, MAX_ADAPTER_ADDRESS_LENGTH));
			counter++;
			pAdapter = pAdapter->Next;
		}
		free(pAdapterInfo);
		return false;

	} else {
		// 有保存下来的MAC地址，进行比较
		while (pAdapter) {
			for (size_t i=0; i<addrlist.size(); i++) {
				if (0==memcmp(addrlist[i].addr, pAdapter->Address, min(pAdapter->AddressLength, MAX_ADAPTER_ADDRESS_LENGTH))) {
					free(pAdapterInfo);
					return false;
				}
			}
			pAdapter = pAdapter->Next;
		}
		free(pAdapterInfo);
		return true;
	}
}
