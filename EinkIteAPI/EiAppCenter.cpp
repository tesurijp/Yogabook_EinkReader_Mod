/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#include "stdafx.h"
#include "EiAppCenter.h"
#include "EinkIteAPI.h"
#include "EinkInternal.h"
#include <Objbase.h>
#include "cmmBaseObj.h"
#include <Shlobj.h>
#include "InstallDriver.h"

CEiAppCenter::CEiAppCenter()
{
	muAppID = 0;

	muSendBufferLength = 0;
	mSendBuffer = NULL;

	muReciveBufferLength = 0;
	mpReciveBuffer = NULL;

	mhWnd = NULL;
}

CEiAppCenter::~CEiAppCenter()
{
	//停止消息通道
	moAppListener.Stop();

	ReleaseAppBuffer();
}

//获取GUID字符串
void CEiAppCenter::GetGUIDString(wchar_t* npszBuffer, int niLen)
{
	GUID ldGuid;
	HRESULT lhResult = S_FALSE;
	while (lhResult != S_OK)
	{
		lhResult = CoCreateGuid(&ldGuid);
		Sleep(1);
	}

	StringFromGUID2(ldGuid, (LPOLESTR)npszBuffer, niLen);
}

// 初始化执行体
ULONG CEiAppCenter::Initialize(TRSP_SYSTEM_INFO_DATA& rSystemInfoData)
{
	ULONG luResult;

	//生成管道名称
	wchar_t lszTempBuffer[MAX_PATH] = { 0 };
	GetGUIDString(lszTempBuffer, MAX_PATH);
	swprintf_s(mdRegAppInfo.mszAppFilePath, MAX_PATH, L"\\\\.\\pipe\\%s.eink", lszTempBuffer);

	//创建缓存
	CreateAppBuffer();

	luResult = moAppListener.CreateListener(
		mdRegAppInfo.mszAppFilePath, //管道名称
		(LPVOID)mpReciveBuffer,
		muReciveBufferLength,
		AppReciveCallBack,
		AppCenterCallBack,
		this
	);

	if (luResult != ERROR_SUCCESS)
		return luResult;

	// 打开对Host的消息连接
	wchar_t lszPipeName[MAX_PATH] = { 0 };
	luResult = GetRegSZ(L"PipeName", lszPipeName);
	if (luResult != ERROR_SUCCESS)
		return luResult;

	luResult = moConnectToHost.CreateConnector(lszPipeName);
	if (luResult != ERROR_SUCCESS)
		return luResult;

	// 注册App
	CEiSvrMsgItem loMsg;
	loMsg.Data.Item.AppId = muAppID = GetCurrentProcessId(); // AppID直接取自身进程ID
	loMsg.Data.Item.MsgId = EMHOSTID_REG_APP;
	ULONG luMsgBufSize = (ULONG)sizeof(REG_APP_INFO);
	memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &mdRegAppInfo, luMsgBufSize);
	loMsg.Data.Item.BufSize = luMsgBufSize;

	luResult = SendMessageToService(loMsg);
	if (luResult != ERROR_SUCCESS)
		return luResult;

	//获取到显示设备信息
	memcpy_s(&rSystemInfoData, sizeof(TRSP_SYSTEM_INFO_DATA), loMsg.Data.Item.MsgBuf, sizeof(TRSP_SYSTEM_INFO_DATA));

	return luResult;
}

// 释放
void CEiAppCenter::Release(void)
{
	// 关闭所有对App的连接

	// 关闭Service的监听
	moAppListener.Stop();
}

// 接收数据回调入口函数
void __stdcall CEiAppCenter::AppReciveCallBack(const char* npData, ULONG nSize, void* npContext)
{
	CEiAppCenter* lpThis = (CEiAppCenter*)npContext;
	lpThis->AppPushReciveDataToQueue(npData, nSize);
}

// 接收数据预处理函数
void CEiAppCenter::AppPushReciveDataToQueue(const char* npData, ULONG nSize)
{
	DWORD dwMsgBufferSize = sizeof(CEiSvrMsgItem);
	CEiSvrMsgItem* pMsg = (CEiSvrMsgItem*)npData;
	moAppListener.PostMsgToListener(*pMsg);
}

// 回调入口函数
void __stdcall CEiAppCenter::AppCenterCallBack(CEiSvrMsgItem& nrMsg, void* npContext)
{
	CEiAppCenter* lpThis = (CEiAppCenter*)npContext;

	lpThis->AppDispatch(nrMsg);
}

// 主分发函数
void CEiAppCenter::AppDispatch(CEiSvrMsgItem& nrMsg)
{
	switch (nrMsg.Data.Item.MsgId)
	{
	case EMAPPID_RESULT:
		MsgBack(nrMsg);
		break;
	case EMAPPID_FINGER_MOVE:
		//手指输入消息
		InputMsg(nrMsg);
		break;
	case EMAPPID_RE_DRAW:
		//服务要求APP全屏重绘
		ReDraw(nrMsg);
		break;
	case EMAPPID_ACTIVATE:
		//说明Z轴发生变化
		ZOrderChange(nrMsg);
		break;
	case EMAPPID_ORIENTATION_CHANGED:
		//服务通知EINK屏幕发生变化
		EinkScreenOrientationChange(nrMsg);
		break;
	case EMAPPID_LATTOP_CHANGED:
		//机器形态发生变化
		LaptopModeChange(nrMsg);
		break;
	case EMAPPID_EVENT:
		//服务事件
		ServiceMsg(nrMsg);
		break;
	case EMAPPID_HOMEBAR_CHANGED:
		//homebar状态发生变化
		HomebarChanged(nrMsg);
		break;
	case EMAPPID_KEYBOARD_CHANGED:
		KeyboardStyleChangeComplete(nrMsg);
		break;
	case EMAPPID_RESET_TP_AREA:
		//需要应用重新设置tp area
		ResetTPArea(nrMsg);
		break;
	case EMAPPID_PRIVACY_STATUS_CHANGED:
		//隐私开关状态发生变化
		PrivacyStatusChanged(nrMsg);
		break;
	case EMAPPID_CHECK_APP_ALIVE:
		CheckAppAlive(nrMsg);
		break;
	case EMAPPID_SMARTINFO_SETTINGS_CHANGE:
		SmartInfoSettingChange(nrMsg);
		break;
	default:
		break;
	}
}

// 发送消息给Service，不等待
ULONG CEiAppCenter::PostMessageToService(CEiSvrMsgItem& nrMsg)
{
	ULONG uMsgLen = sizeof(nrMsg.Data.Item) + nrMsg.Data.Item.BufSize - 1;
	return moConnectToHost.PostMsg(nrMsg, uMsgLen);
}

// 发送消息给Service，不等待
ULONG CEiAppCenter::PostMessageToService(CEiSvrMsgItem& nrMsg, EI_BUFFER* npBuffer)
{
	ULONG uMsgLen = sizeof(nrMsg.Data.Item) + nrMsg.Data.Item.BufSize - 1;
	memcpy_s(mSendBuffer, uMsgLen, &nrMsg, uMsgLen);

	ULONG uBufferLen = sizeof(EI_BUFFER) + npBuffer->ulBufferSize - 1;
	memcpy_s(mSendBuffer + uMsgLen, uBufferLen, npBuffer, uBufferLen);

	moConnectToHost.PostMsg(mSendBuffer, uMsgLen + uBufferLen);

	return ERROR_SUCCESS;
}

//检查服务状态
bool CEiAppCenter::ProtectService(void)
{
	HANDLE lhEventHandle = NULL;

	do
	{
		//如果安装程序启动了，就退出
		lhEventHandle = OpenEvent(READ_CONTROL, FALSE, MUTEX_EVENT_INSTALL);
		if (lhEventHandle != NULL)
		{
			CloseHandle(lhEventHandle);
			break;
		}

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

			Sleep(1000 * 10);
		}
		else
		{
			CloseHandle(lhEventHandle);
			lhEventHandle = NULL;
		}

	} while (false);

	return 0;
}

// 发送消息给Service，并等待
ULONG CEiAppCenter::SendMessageToService(CEiSvrMsgItem& nrMsg)
{
	ULONG luResult = ERROR_NOT_READY;


	nrMsg.Data.Item.WaitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (nrMsg.Data.Item.WaitHandle == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	nrMsg.Data.Item.OrgItem = &nrMsg;

	PostMessageToService(nrMsg);

	DWORD ldwObject = WaitForSingleObject(nrMsg.Data.Item.WaitHandle, 1000 * 30);	// 等待大约30秒
	if (ldwObject == WAIT_OBJECT_0)
	{
		luResult = nrMsg.Data.Item.Result;
		CloseHandle(nrMsg.Data.Item.WaitHandle);
	}
	else
	{
		luResult = ERROR_TIMEOUT;

		//如果请求超时了，就检查一下服务状态
		ProtectService();
	}

	return luResult;
}

// 发送消息和绘制内容给Service，并等待
ULONG CEiAppCenter::SendMessageToService(CEiSvrMsgItem& nrMsg, EI_BUFFER* npBuffer)
{
	ULONG luResult = ERROR_NOT_READY;

	nrMsg.Data.Item.WaitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (nrMsg.Data.Item.WaitHandle == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	nrMsg.Data.Item.OrgItem = &nrMsg;

	PostMessageToService(nrMsg, npBuffer);

	DWORD ldwObject = WaitForSingleObject(nrMsg.Data.Item.WaitHandle, 1000 * 30);	// 等待大约30秒
	if (ldwObject == WAIT_OBJECT_0)
	{
		luResult = nrMsg.Data.Item.Result;
		CloseHandle(nrMsg.Data.Item.WaitHandle);
	}
	else
	{
		luResult = ERROR_TIMEOUT;

		//如果请求超时了，就检查一下服务状态
		ProtectService();
	}

	return luResult;
}

// 撤回一类消息，将队列中此类消息全部撤回
void CEiAppCenter::RecallMessage(const CEiSvrMsgItem& nrMsg)
{
	//moConnectToHost.Recall(nrMsg);

	ULONG rcMsgId;
	memcpy_s(&rcMsgId, sizeof(ULONG), nrMsg.Data.Item.MsgBuf, sizeof(ULONG));
	
	ULONG uMsgLen = sizeof(nrMsg.Data.Item) + nrMsg.Data.Item.BufSize - 1;
	moConnectToHost.PostMsg(nrMsg, uMsgLen);
}


//////////////////////////////////////////////////////////////////////////
// 以下为具体请求的响应函数

// 注册一个App
void CEiAppCenter::MsgBack(CEiSvrMsgItem& nrMsg)
{
	// 只有回到本进程才能访问对应的内存
	CEiSvrMsgItem* lpOrgMsg = nrMsg.Data.Item.OrgItem;

	if (lpOrgMsg == NULL || lpOrgMsg->Data.Item.WaitHandle == NULL)
		return;

	// 复制返回值
	lpOrgMsg->Data.Item.Result = nrMsg.Data.Item.Result;

	// 复制返回数据
	if (nrMsg.Data.Item.BufSize > 0)
	{
		memcpy_s(lpOrgMsg->Data.Item.MsgBuf, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);
		lpOrgMsg->Data.Item.BufSize = nrMsg.Data.Item.BufSize;
	}

	// 释放发送端的等待
	SetEvent(lpOrgMsg->Data.Item.WaitHandle);
}

//返回值为地址起始地址，rulBufferSize为Buffer大小
BYTE* CEiAppCenter::GetBufferBase(ULONG& rulBufferSize)
{
	BYTE* lpRetBuffer = NULL;

	do
	{
		if (mpReciveBuffer == NULL)
			break;
		lpRetBuffer = (BYTE*)mpReciveBuffer + EAC_MSG_BUFFER_SIZE;
		rulBufferSize = EAC_APP_BUFFER_SIZE - EAC_MSG_BUFFER_SIZE;
	} while (false);

	return lpRetBuffer;
}

VOID CALLBACK WinMsgMemFreeCallback(
	_In_ HWND      hwnd,
	_In_ UINT      uMsg,
	_In_ ULONG_PTR dwData,
	_In_ LRESULT   lResult
)
{
	HeapFree(GetProcessHeap(), 0, (void*)dwData);
}


void SendWinMsgWithData(
	HWND		  hWnd,
	UINT          Msg,
	WPARAM        wParam,
	const void*	  pData,
	int			  iSize
)
{
	void* lpData = HeapAlloc(GetProcessHeap(), 0, iSize);
	if (lpData == NULL)
		return;
	memcpy_s(lpData, iSize, pData, iSize);

	SendMessageCallback(hWnd, Msg, wParam, (LPARAM)lpData, WinMsgMemFreeCallback, (ULONG_PTR)lpData);
}

// 手指输入消息
void CEiAppCenter::InputMsg(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);

		PEI_TOUCHINPUT_POINT lpInput = (PEI_TOUCHINPUT_POINT)nrMsg.Data.Item.MsgBuf;
		BREAK_ON_NULL(lpInput);

		//重置系统sleep计时
		SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);

		SendWinMsgWithData(mhWnd, WM_EI_TOUCH, 1, lpInput, sizeof(EI_TOUCHINPUT_POINT));

	} while (false);
}

//设置接收windows消息的窗口句柄
void CEiAppCenter::SetHwnd(HWND nHwnd)
{
	mhWnd = nHwnd;
}

// 全屏重绘
void CEiAppCenter::ReDraw(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);

		SendWinMsgWithData(mhWnd, WM_EI_DRAW, 0, nrMsg.Data.Item.MsgBuf, sizeof(EI_RECT));

	} while (false);
}

// Z轴发生变化
void CEiAppCenter::ZOrderChange(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		ULONG lulType = sizeof(ULONG);

		memcpy_s(&lulType, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_ACTIVATE, (WPARAM)lulType, 0);

	} while (false);
}

// Eink屏幕方向发生变化
void CEiAppCenter::EinkScreenOrientationChange(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		ULONG lulOrientation = sizeof(ULONG);

		memcpy_s(&lulOrientation, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_ROTATION, (WPARAM)lulOrientation, 0);

	} while (false);
}

// 机器形态发生变化
void CEiAppCenter::LaptopModeChange(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		ULONG lulMode = sizeof(ULONG);

		memcpy_s(&lulMode, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_LAPTOP_MODE_CHANGE, (WPARAM)lulMode, 0);

	} while (false);
}

// 服务通知事件
void CEiAppCenter::ServiceMsg(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		ULONG lulEventID = sizeof(ULONG);

		memcpy_s(&lulEventID, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_SERVICE_EVENT, (WPARAM)lulEventID, 0);

	} while (false);
}

// homebar状态发生变化
void CEiAppCenter::HomebarChanged(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		ULONG lulModeID = sizeof(ULONG);

		memcpy_s(&lulModeID, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_HOMEBAR_MODE_CHANGE, (WPARAM)lulModeID, 0);

	} while (false);
}

// 键盘样式切换完成
void CEiAppCenter::KeyboardStyleChangeComplete(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		BOOL lbRet = false;

		memcpy_s(&lbRet, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);

		PostMessage(mhWnd, WM_EI_CHANGE_KEYBOARD_STYLE_COMPLETE, (WPARAM)lbRet, 0);

	} while (false);
}

// 重新设置tp area
void CEiAppCenter::ResetTPArea(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);

		PostMessage(mhWnd, WM_EI_RESET_TP_AREA, 0, 0);

	} while (false);
}

//隐私开关状态发生变化
void CEiAppCenter::PrivacyStatusChanged(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		DWORD ldwStatus = 0;
		memcpy_s(&ldwStatus, nrMsg.Data.Item.BufSize, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);
		PostMessage(mhWnd, WM_EI_PRIVACY_CHANGE, (WPARAM)ldwStatus, 0);

	} while (false);
}

// 创建缓存
void CEiAppCenter::CreateAppBuffer()
{
	if (muSendBufferLength == 0)
	{
		muSendBufferLength = 1024 * 1024 * 3;
		mSendBuffer = new char[muSendBufferLength];
	}

	if (muReciveBufferLength == 0)
	{
		muReciveBufferLength = EAC_APP_BUFFER_SIZE;
		mpReciveBuffer = new char[muReciveBufferLength];
	}
}

// 释放缓存
void CEiAppCenter::ReleaseAppBuffer()
{
	if (muSendBufferLength > 0)
	{
		muSendBufferLength = 0;
		delete[] mSendBuffer;
	}

	if (muReciveBufferLength > 0)
	{
		muReciveBufferLength = 0;
		delete[] mpReciveBuffer;
	}
}

// 回应Service端的检测
void CEiAppCenter::CheckAppAlive(CEiSvrMsgItem& nrMsg)
{
	if (nrMsg.Data.Item.WaitHandle == NULL || nrMsg.Data.Item.OrgItem == NULL)
		return;
	if (nrMsg.Data.Item.AppId == 0)
		return;

	// 返回结果给App
	CEiSvrMsgItem loMsgBack;
	loMsgBack.Data.Item.AppId = nrMsg.Data.Item.AppId;
	loMsgBack.Data.Item.OrgItem = nrMsg.Data.Item.OrgItem;
	loMsgBack.Data.Item.MsgId = EMHOSTID_RESULT;
	loMsgBack.Data.Item.Result = ERROR_SUCCESS;
	loMsgBack.Data.Item.BufSize = 0;

	PostMessageToService(loMsgBack);
}

// 检查发消息间隔时间是否足够长，超过20秒发送运行正常的消息
void CEiAppCenter::CheckAndSendNormalRunMsg()
{
	if (moConnectToHost.GetMaxElapsedTimeOfMsg() > 1000 * 15)
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = muAppID;
		loMsg.Data.Item.MsgId = EMHOSTID_NORMAL_RUN_NOTICE;
		loMsg.Data.Item.BufSize = 0;

		PostMessageToService(loMsg);
	}
}

LSTATUS CEiAppCenter::GetRegSZ(LPCWSTR lpValueName, wchar_t* value)
{
	HKEY h_reg = 0;
	LSTATUS status = ERROR_SUCCESS;
	do
	{
		status = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Lenovo\\EinkSDK", &h_reg);
		if (status != ERROR_SUCCESS)
			break;

		DWORD data_type = REG_DWORD;
		DWORD data_size = 0;
		status = RegQueryValueExW(h_reg, lpValueName, 0, &data_type, nullptr, &data_size);
		if (status == ERROR_SUCCESS && data_size > 0)
		{
			LPBYTE buf = new BYTE[data_size];
			status = RegQueryValueExW(h_reg, lpValueName, 0, &data_type, buf, &data_size);
			memcpy_s((char*)value, data_size, buf, data_size);
			delete[] buf;
		}

	} while (false);

	if (h_reg != 0)
		RegCloseKey(h_reg);

	return status;
}

// smartinfo设置发生了变化
void CEiAppCenter::SmartInfoSettingChange(CEiSvrMsgItem& nrMsg)
{
	do
	{
		BREAK_ON_NULL(mhWnd);
		SendWinMsgWithData(mhWnd, WM_EI_SMARTINFO_SETTING_CHANGE, 0, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);
	} while (false);
}
