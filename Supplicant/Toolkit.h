// Toolkit.h : ���帨�����ߺ���
//

namespace Toolkit
{

// ��ȡ��ǰϵͳƽ̨���ͣ������ǣ�
//		VER_PLATFORM_WIN32_NT
//		VER_PLATFORM_WIN32_WINDOWS
//		VER_PLATFORM_WIN32s
int GetSysVersion();

// ע������Ƿ����û��ֶ��޸ĵ�MAC��ַ
// ���أ�true - ��
bool HasRegMac();

// ���ע������û��޸ĵ�MAC��ַ
// ���أ��������MAC��ַ��Ŀ
int CleanRegMAC();


// ��鵱ǰMAC��ַ�Ƿ�����Ч��MAC��ַ
//
bool InvalidMAC();

}// End of namespace Toolkit