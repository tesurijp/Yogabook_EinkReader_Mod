/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */
#include "stdafx.h"
#include "EiAppCenter.h"
#include "EinkIteAPI.h"
#include "EinkInternal.h"
#include <Objbase.h>
#include "cmmBaseObj.h"
#include <Shlobj.h>

CEiAppCenter::CEiAppCenter()
{
	muAppID = 0;
	mhFile = NULL;
	muFileLength = 0;
	mhFileMap = NULL;
	mpMappedBase = NULL;

	mhServerFile = NULL;
	muServerFileLength = 0;
	mhServerFileMap = NULL;
	mpServerMappedBase = NULL;

	mhWnd = NULL;
}

CEiAppCenter::~CEiAppCenter()
{
	//停止消息通道
	moAppListener.Stop();

	UnmapViewOfFile(mpServerMappedBase);
	UnmapViewOfFile(mpMappedBase);

	SAFE_CLOSE_HANDLE(mhFileMap);
	SAFE_CLOSE_HANDLE(mhFile);

	SAFE_CLOSE_HANDLE(mhServerFileMap);
	SAFE_CLOSE_HANDLE(mhServerFile);

	//删除自己的文件
	DeleteFile(mdRegAppInfo.mszAppFilePath);
}

//获取GUID字符串
void CEiAppCenter::GetGUIDString(const wchar_t* npszBuffer,int niLen)
{
	GUID ldGuid;
	HRESULT lhResult = S_FALSE;
	while (lhResult != S_OK)
	{
		lhResult = CoCreateGuid(&ldGuid);
		Sleep(1);
	}

	StringFromGUID2(ldGuid, (LPOLESTR)npszBuffer,niLen);
}

//打开自己的内存映射文件
bool CEiAppCenter::OpenJasonFile(const wchar_t* nszFileName)
{
	bool lbResult = false;
	ULONG luLengthHi;


	try
	{

		// 映射文件进内存
		//需要设置权限，否则服务创建的对象，普通进程无法打开
		SECURITY_DESCRIPTOR lsd;
		InitializeSecurityDescriptor(&lsd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&lsd, TRUE, (PACL)NULL, FALSE);
		SECURITY_ATTRIBUTES	lsa;
		lsa.nLength = sizeof(SECURITY_ATTRIBUTES);
		lsa.bInheritHandle = TRUE;
		lsa.lpSecurityDescriptor = &lsd;

		mhFile = CreateFile(nszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &lsa, CREATE_ALWAYS, NULL, NULL);
		if (mhFile == INVALID_HANDLE_VALUE)
			THROW_INVALID;

		//指定文件大小
		mhFileMap = CreateFileMapping(mhFile, NULL, PAGE_READWRITE, 0, EAC_FILE_SIZE, NULL);
		if (mhFileMap == INVALID_HANDLE_VALUE)
			THROW_INVALID;

		muFileLength = GetFileSize(mhFile, &luLengthHi);

		mpMappedBase = (const char*)MapViewOfFile(mhFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		THROW_ON_NULL(mpMappedBase);

		lbResult = true;
	}
	catch (...)
	{

	}

	return lbResult;
}

//打开服务的内存映射文件
bool CEiAppCenter::OpenServerJasonFile(const wchar_t* nszFileName)
{
	bool lbResult = false;
	ULONG luLengthHi;


	try
	{
		// 映射文件进内存
		mhServerFile = CreateFile(nszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, NULL, NULL);
		if (mhServerFile == INVALID_HANDLE_VALUE)
			THROW_INVALID;

		muServerFileLength = GetFileSize(mhServerFile, &luLengthHi);

		//指定文件大小
		mhServerFileMap = CreateFileMapping(mhServerFile, NULL, PAGE_READWRITE, 0, muServerFileLength, NULL);
		if (mhServerFileMap == INVALID_HANDLE_VALUE)
			THROW_INVALID;
		
		mpServerMappedBase = (const char*)MapViewOfFile(mhServerFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		THROW_ON_NULL(mpServerMappedBase);

		lbResult = true;
	}
	catch (...)
	{

	}

	return lbResult;
}

// 初始化执行体
ULONG CEiAppCenter::Initialize(TRSP_SYSTEM_INFO_DATA& rSystemInfoData)
{
	ULONG luResult;
	

	// 初始化
	// 打开App监听
	//生成唯一字符串
	wchar_t lszTempBuffer[MAX_PATH] = { 0 };
	GetGUIDString(lszTempBuffer, MAX_PATH);
	swprintf_s(mdRegAppInfo.mszAppMutex, APP_NAME_MAX, L"Global\\%s", lszTempBuffer);

	GetGUIDString(lszTempBuffer, MAX_PATH);
	swprintf_s(mdRegAppInfo.mszAppSemaphore, APP_NAME_MAX, L"Global\\%s", lszTempBuffer);

	//生成内存映射文件名
	GetGUIDString(lszTempBuffer, MAX_PATH);
	//获取TEMP目录路径
	wchar_t lszTempPath[MAX_PATH] = { 0 };
	SHGetSpecialFolderPath(NULL, lszTempPath, CSIDL_COMMON_APPDATA, FALSE);

	swprintf_s(mdRegAppInfo.mszAppFilePath, MAX_PATH, L"%s\\EinkSrv\\%s.eink", lszTempPath,lszTempBuffer);
	OpenJasonFile(mdRegAppInfo.mszAppFilePath);

	//打开和Server通讯用的文件
	swprintf_s(lszTempPath, MAX_PATH, L"%s\\EinkSrv\\FC80C0D8-FC99-4772-BCB0-ACC862F34AAC.eink", lszTempPath);
	OpenServerJasonFile(lszTempPath);

	luResult = moAppListener.CreateListener(
		mdRegAppInfo.mszAppMutex,
		mdRegAppInfo.mszAppSemaphore,
		(LPVOID)mpMappedBase,				// 内存映射文件的第一段
		muFileLength,			// 第一段的长度
		AppCenterCallBack,
		this
	);

	if (luResult != ERROR_SUCCESS)
		return luResult;

	// 打开对Host的消息连接
	luResult = moConnectToHost.CreateConnector(
		L"Global\\LN_EI_HOST_MUTEX",
		L"Global\\LN_EI_HOST_SEMAPHORE", \
		(LPVOID)mpServerMappedBase,
		muServerFileLength
	);

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
	// 添加更多响应函数调用
	default:
		break;
	}
}

// 发送消息给Service，并等待
ULONG CEiAppCenter::PostMessageToService(CEiSvrMsgItem& nrMsg)
{
	moConnectToHost.PostMsg(nrMsg);
	return ERROR_SUCCESS;
}

// 发送消息给Service，不等待
ULONG CEiAppCenter::SendMessageToService(CEiSvrMsgItem& nrMsg)
{
	ULONG luResult = ERROR_NOT_READY;


	nrMsg.Data.Item.WaitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (nrMsg.Data.Item.WaitHandle == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	nrMsg.Data.Item.OrgItem = &nrMsg;

	PostMessageToService(nrMsg);

	DWORD ldwObject = WaitForSingleObject(nrMsg.Data.Item.WaitHandle, 1000*30);	// 等待大约30秒
	if (ldwObject == WAIT_OBJECT_0)
	{
		luResult = nrMsg.Data.Item.Result;
		CloseHandle(nrMsg.Data.Item.WaitHandle);
	}
	else
	{
		luResult = ERROR_TIMEOUT;
	}

	return luResult;
}

// 撤回一类消息，将队列中此类消息全部撤回
void CEiAppCenter::RecallMessage(const CEiSvrMsgItem& nrMsg)
{
	moConnectToHost.Recall(nrMsg);
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
		RtlCopyMemory(lpOrgMsg->Data.Item.MsgBuf, nrMsg.Data.Item.MsgBuf, nrMsg.Data.Item.BufSize);
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
		if(mpMappedBase == NULL)
			break;
		lpRetBuffer = (BYTE*)mpMappedBase + EAC_MSG_BUFFER_SIZE;
		rulBufferSize = EAC_FILE_SIZE - EAC_MSG_BUFFER_SIZE;
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
	HeapFree(GetProcessHeap(), 0,(void*)dwData);
}


void SendWinMsgWithData(
	HWND		  hWnd,
	UINT          Msg,
	WPARAM        wParam,
	const void*	  pData,
	int			  iSize
)
{
	void* lpData = HeapAlloc(GetProcessHeap(), 0,iSize);
	if (lpData == NULL)
		return;
	RtlCopyMemory(lpData, pData, iSize);

	SendMessageCallback(hWnd, Msg, wParam,(LPARAM)lpData, WinMsgMemFreeCallback, (ULONG_PTR)lpData);
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

		SendWinMsgWithData(mhWnd, WM_EI_TOUCH, 1,lpInput,sizeof(EI_TOUCHINPUT_POINT));

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
		
		SendWinMsgWithData(mhWnd,WM_EI_DRAW,0,nrMsg.Data.Item.MsgBuf,sizeof(EI_RECT));

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