#pragma once
class CHellpFun
{
public:
	CHellpFun();
	~CHellpFun();

	//检查服务状态
	static void CheckService(void);
	//开启线程监控服务状态，如果服务没了，就重新启动
	static void ProcService(void);
	//通过进程名称获取进程PID
	static DWORD GetPIDByName(wchar_t* npszName);
	//守护线程
	static bool ProtectThread(LPVOID npData);
	//杀掉指定进程
	static BOOL KillProc(DWORD ProcessID);
	//杀掉某些进程
	static void KillProcByName(wchar_t* npszName);
};

