#include "stdafx.h"
#include "HellpFun.h"
#include "InstallDriver.h"
#include <Wtsapi32.h>
#include "Compile.h"
#include "../inc/ludp.h"

HANDLE ghProcThread = NULL;

CHellpFun::CHellpFun()
{
}


CHellpFun::~CHellpFun()
{
}
//检查服务状态
void CHellpFun::CheckService(void)
{
	HANDLE lhEventHandle = NULL;

	do
	{
		//检查服务是否还正常
		lhEventHandle = OpenEvent(READ_CONTROL, FALSE, MUTEX_EVENT_SERVICE);
		if (lhEventHandle == NULL)
		{
			//启动服务
			CInstallDriver InstallDriver;
			InstallDriver.ManageDriver(L"EinkSvr",
				L"EinkSvr",
				SC_MANAGER_ALL_ACCESS | SERVICE_ALL_ACCESS,
				SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START,
				DRIVER_FUNC_START);
		}
		else
		{
			CloseHandle(lhEventHandle);
			lhEventHandle = NULL;
		}


	} while (false);

}


//通过进程名称获取进程PID
DWORD CHellpFun::GetPIDByName(wchar_t* npszName)
{
	DWORD ldwPID = 0;

	//获取计算机名称
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD	ldwSize = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(szComputerName, &ldwSize);
	HANDLE lHandle = WTSOpenServer(szComputerName);
	WTS_PROCESS_INFO* pWtspi = NULL;
	//Opens a handle to the specified server
	do
	{
		if (lHandle == NULL)
			break;

		//枚举计算机上的进程

		DWORD dwCount;
		if (!WTSEnumerateProcesses(lHandle,
			0,
			1,
			&pWtspi,
			&dwCount))
		{
			break;
		};

		LPWSTR pProcessName;
		DWORD ProcessId;
		for (int i = 0; i < dwCount; i++)
		{
			pProcessName = pWtspi[i].pProcessName;
			ProcessId = pWtspi[i].ProcessId;

			if (_tcsicmp(pProcessName, npszName) == 0)
			{
				ldwPID = ProcessId;
				break;
			}
		}


	} while (false);

	WTSCloseServer(lHandle);
	WTSFreeMemory(pWtspi);

	return ldwPID;
}

//守护线程
bool CHellpFun::ProtectThread(LPVOID npData)
{
	HANDLE lhEventHandle = NULL;

	do
	{
		Sleep(1000 * 3);

		//如果安装程序启动了，就退出
		/*lhEventHandle = OpenEvent(READ_CONTROL, FALSE, MUTEX_EVENT_INSTALL);
		if (lhEventHandle != NULL)
		{
			CloseHandle(lhEventHandle);
			Sleep(1000*20);
		}*/

		//检查服务是否还正常
		lhEventHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, CHellpFun::GetPIDByName(L"EinkSvr.exe"));
		if (lhEventHandle != NULL)
		{
			WaitForSingleObject(lhEventHandle, INFINITE);
			CloseHandle(lhEventHandle);
		}


		//启动服务
		CInstallDriver InstallDriver;
		InstallDriver.ManageDriver(L"EinkSvr",
			L"EinkSvr",
			SC_MANAGER_ALL_ACCESS | SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			DRIVER_FUNC_START);


	} while (true);

	return 0;
}

//开启线程监控服务状态，如果服务没了，就重新启动
void CHellpFun::ProcService(void)
{
	// 守护线程
	DWORD ldwProtectThreadID = 0;
	ghProcThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)&CHellpFun::ProtectThread,
		NULL,
		0,
		&ldwProtectThreadID
	);
}


//杀掉指定进程
BOOL CHellpFun::KillProc(DWORD ProcessID)
{
	HANDLE hProc = NULL;

	//打开由ProcessVXER()传递的进程PID
	hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);

	if (hProc != NULL)
	{
		//终止进程
		if (!(TerminateProcess(hProc, 2)))
		{
			CloseHandle(hProc);
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	if (hProc != NULL)
		CloseHandle(hProc);

	return TRUE;
}

//杀掉某些进程
void CHellpFun::KillProcByName(wchar_t* npszName)
{
	//获取计算机名称
	TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 };
	DWORD	ldwSize = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(szComputerName, &ldwSize);
	//Opens a handle to the specified server
	HANDLE lHandle = WTSOpenServer(szComputerName);
	//枚举计算机上的进程
	WTS_PROCESS_INFO* pWtspi;
	DWORD dwCount;
	if (!WTSEnumerateProcesses(lHandle,
		0,
		1,
		&pWtspi,
		&dwCount))
	{
		return;
	};

	DWORD ldwProID = GetCurrentProcessId();

	LPWSTR pProcessName;
	DWORD ProcessId;
	for (int i = 0; i < dwCount; i++)
	{
		pProcessName = pWtspi[i].pProcessName;
		ProcessId = pWtspi[i].ProcessId;

		if (_tcsicmp(pProcessName, npszName) == 0)
		{
			if (ldwProID != ProcessId)
				KillProc(ProcessId); //不会杀掉自己
		}
	}
	WTSCloseServer(lHandle);
	WTSFreeMemory(pWtspi);

}