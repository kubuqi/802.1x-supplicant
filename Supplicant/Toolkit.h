// Toolkit.h : 定义辅助工具函数
//

namespace Toolkit
{

// 获取当前系统平台类型，可以是：
//		VER_PLATFORM_WIN32_NT
//		VER_PLATFORM_WIN32_WINDOWS
//		VER_PLATFORM_WIN32s
int GetSysVersion();

// 注册表中是否有用户手动修改的MAC地址
// 返回：true - 有
bool HasRegMac();

// 清除注册表中用户修改的MAC地址
// 返回：被清除的MAC地址数目
int CleanRegMAC();


// 检查当前MAC地址是否是有效的MAC地址
//
bool InvalidMAC();

}// End of namespace Toolkit