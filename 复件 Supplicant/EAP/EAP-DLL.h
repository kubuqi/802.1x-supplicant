// EAP.h : main header file for the EAP DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//
// Call back �¼�����
// 
enum EAP_EVENT 
{
	EAP_TIMEOUT,		// EAP��֤��ʱ
	EAP_REQUEST_ID,		// ���������û�ID
	EAP_CHALLANGE,		// ����У��
	EAP_SUCCESS,		// ��֤ͨ��
	EAP_FAIL			// ��֤ʧ�ܣ��򽻻���ǿ������
};

//
// ��ǰ802.1X����״̬
//
enum EAP_STATE 
{
    UNCONNECTED,		// δ����
    CONNECTING,			// ��������
	RECONNECTING,		// Ϊ�������������
    CONNECTED			// ������
};

//
// ��ȡ��ǰ802.1X����״̬
//
__declspec(dllexport) EAP_STATE GetCurConState();


//
// ��ȡ���ӽ�����ʱ��
//
__declspec(dllexport) CTime GetConnectedTime();

//
// ����802.1X��֤���̲����̷��ء�
// 
__declspec(dllexport) void Login( 
				const char *eth_if,						// ������֤���õ���������.
				const char *user,						// �û���
				const char *password,					// ����
				void (*p)(EAP_EVENT id, EAP_STATE stat )// �ص�����ָ��
				);

//
// ��ֹ802.1X���̲��Ͽ�����
//
__declspec(dllexport) void Logout();


//
// ����DHCP��������
//
__declspec(dllexport) void DhcpRequest();

