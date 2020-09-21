#pragma once

class CProcessStarter
{
public:
	CProcessStarter();
	~CProcessStarter();

	// 如果没有找到"已经激活的SessionId", 说明还没有进入桌面
	static BOOL FindActiveSessionId(OUT DWORD& dwSessionId, BOOL bNeedLog);
	//如果nbIsSystem=true则直接使用system用户启动
	//npszCommndLine参数最前面一定要是一个空格，否则无法成功传入
	static DWORD Run(const wchar_t* pcProcessPathName, HANDLE& rProHandle, int niSessionID = -1, wchar_t* npszCommndLine = NULL);
	//获取当前用户ID，0表示没有激活用户
	static DWORD GetSessionID(void);
	//获取当前用户SDK,返回的字符串在不用的时候调用LocalFree释放
	static wchar_t* GetUserSID(int niSessionID = -1);
	//获取当前用户Token
	static HANDLE GetCurrentUserToken();
private:



};
