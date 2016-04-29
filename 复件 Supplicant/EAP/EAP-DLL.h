// EAP.h : main header file for the EAP DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//
// Call back 事件类型
// 
enum EAP_EVENT 
{
	EAP_TIMEOUT,		// EAP认证超时
	EAP_REQUEST_ID,		// 正在请求用户ID
	EAP_CHALLANGE,		// 正在校验
	EAP_SUCCESS,		// 认证通过
	EAP_FAIL			// 认证失败，或交换机强制下线
};

//
// 当前802.1X连接状态
//
enum EAP_STATE 
{
    UNCONNECTED,		// 未连接
    CONNECTING,			// 正在连接
	RECONNECTING,		// 为保活而进行重连
    CONNECTED			// 已连接
};

//
// 获取当前802.1X连接状态
//
__declspec(dllexport) EAP_STATE GetCurConState();


//
// 获取连接建立的时刻
//
__declspec(dllexport) CTime GetConnectedTime();

//
// 启动802.1X认证流程并立刻返回。
// 
__declspec(dllexport) void Login( 
				const char *eth_if,						// 进行认证所用的网卡名称.
				const char *user,						// 用户名
				const char *password,					// 密码
				void (*p)(EAP_EVENT id, EAP_STATE stat )// 回调函数指针
				);

//
// 终止802.1X流程并断开连接
//
__declspec(dllexport) void Logout();


//
// 启动DHCP请求流程
//
__declspec(dllexport) void DhcpRequest();

