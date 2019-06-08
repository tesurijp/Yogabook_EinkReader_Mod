/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "einkiteapi.h"
#include "EinkInternal.h"
#include "EiAppCenter.h"
#include "BufferMgr.h"

//HANDLE glhDevice = INVALID_HANDLE_VALUE;
HWND glhAppWindow = NULL;
TRSP_SYSTEM_INFO_DATA gldSysData;
CEiAppCenter gloCenter;
CBufferMgr glAppBufferMgr;
//APP屏幕显示方向,自己记下来，省得每次找服务获取
DWORD gldwOrientation = GIR_NONE;

// Eui服务核心消息接收数据交换区
#pragma data_seg("MSGBUF")
#define EI_EXBUF_SIZE 1024*4//1024 * 1024 * 2
char gldHostExBuffer[EI_EXBUF_SIZE] = { 0 };
char gldAppExBuffer[EI_EXBUF_SIZE] = { 0 }; // 调试用，采用内存映射文件后删除,现在只能支持一个App
#pragma data_seg()

#pragma comment(linker, "/SECTION:MSGBUF,RWS")



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// 获取DLL的共享内存段
LPVOID EixGetSharedData(ULONG nuIndex)
{
	if(nuIndex == 0)
		return gldHostExBuffer;
	if (nuIndex == 1)
		return gldAppExBuffer;
	
	return NULL;
}

// 获得DLL的共享内存段大小
ULONG EixGetSharedDataSize(void)
{
	return EI_EXBUF_SIZE;
}


void EiPostMessage(
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if (glhAppWindow == NULL)
		return;
	PostMessage(glhAppWindow, Msg, wParam, lParam);
}

// 初始化
// 在应用启动后，需要调用本接口函数向Eink系统注册本应用。
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiAppStart(
	HWND hInforWindow	// 用于接收本系统消息的Windows窗口句柄
	){
	ULONG luResult = ERROR_NOT_READY;

	do 
	{
		// 启动系统连接器
		//Sleep(1000 * 20);
		luResult = gloCenter.Initialize(gldSysData);
		if (luResult != ERROR_SUCCESS || gldSysData.uiWidth == 0 || gldSysData.uiHeight == 0)
			break;
		gloCenter.SetHwnd(hInforWindow);

		//Sleep(1000 * 10);
		glhAppWindow = hInforWindow;

		//获取显示用Buffer
		ULONG lulBufferSize = 0;
		BYTE* lpBuffer = gloCenter.GetBufferBase(lulBufferSize);
		if (lpBuffer == NULL)
		{
			luResult = ERROR_OUTOFMEMORY;
			break;
		}

		glAppBufferMgr.Initialize(lpBuffer, lulBufferSize, gldSysData.uiWidth, gldSysData.uiHeight);

		luResult = ERROR_SUCCESS;

	} while (false);
	

	return luResult;
}


// 获得Eink的基本信息
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiGetSystemInfo(PEI_SYSTEM_INFO pstSystemInfo)
{
	ULONG luResult = ERROR_NOT_READY;

	do 
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_EINK_INFO;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return luResult;

		//获取到显示设备信息
		memcpy_s(pstSystemInfo, sizeof(EI_SYSTEM_INFO), loMsg.Data.Item.MsgBuf, sizeof(EI_SYSTEM_INFO));
		pstSystemInfo->ulDpiX = 227;	// 这个参数需要根据当前情况更新 ???niu
		pstSystemInfo->ulDpiY = 227;

		luResult = ERROR_SUCCESS;

	} while (false);

	return luResult;
}

// 获得APP的屏幕设定信息
// get APP context indicated current screen settings
// 返回： 返回零，表示成功；返回非零，表示错误码；
DWORD EiGetAppContext(PEI_APP_CONTEXT pstAppContext)
{
	ULONG luResult = ERROR_NOT_READY;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_APP_CONTEXT;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return luResult;

		//获取到显示设备信息
		memcpy_s(pstAppContext, sizeof(EI_APP_CONTEXT), loMsg.Data.Item.MsgBuf, sizeof(EI_APP_CONTEXT));

		luResult = ERROR_SUCCESS;

	} while (false);

	return luResult;
}


// 获得绘制缓存
// 一个应用实例可以申请多个绘制缓存，通过调用本函数获得一个绘制缓存，在绘制缓存提交给系统刷新绘制之前，修改绘制缓存不会导致Eink屏幕显示被更改
// 返回：返回空指针NULL，表示失败，多为资源耗尽；返回非NULLL，表示成功，返回值即是新建的缓存
EI_BUFFER* EiGetDrawBuffer(
	BOOL bInit,		// 是否将缓存清零
	BOOL bCopy		// 是否将当前Eink屏内容复制到缓存
	){
	EI_BUFFER* lpBuffer = NULL;

	do 
	{
		//获取buffer
		lpBuffer = glAppBufferMgr.GetBuffer();
		if(lpBuffer == NULL)
			break;

		if (bInit != FALSE)
		{
			//清空
			ZeroMemory(lpBuffer->Buf, lpBuffer->ulBufferSize);
		}

		if (gldwOrientation == GIR_90 || gldwOrientation == GIR_270)
		{
			//如果是坚屏就切换一下宽高
			ULONG lulTemp = lpBuffer->ulWidth;
			lpBuffer->ulWidth = lpBuffer->ulHeight;
			lpBuffer->ulHeight = lulTemp;
		}

	} while (false);
	
	return lpBuffer;
}

// 释放绘制缓存
// 没有提交给系统绘制的绘制缓存，当不在使用时，需要调用本函数释放它
void EiReleaseDrawBuffer(
	EI_BUFFER* pstBuffer		// 指向绘制缓存
	)
{
	glAppBufferMgr.FreeBuffer(pstBuffer);

	//HeapFree(GetProcessHeap(), 0, pstBuffer);
}


// 发送绘制请求到服务器
DWORD EiSendDrawRequest(
	EI_RECT* pstArea,	// indicates the area to draw
	EI_BUFFER* pstBuffer,		// 指向绘制缓存
	DWORD nuMsgID
)
{
	DWORD luResult = 0;
	EI_RECT ldUpdate = { 0,0,0,0 };

	do
	{
		if(pstArea != NULL)
			ldUpdate = *pstArea;

		if (ldUpdate.w == 0) ldUpdate.w = gldSysData.uiWidth;
		if (ldUpdate.h == 0) ldUpdate.h = gldSysData.uiHeight;

		EI_MSG_APP_DRAW ldRegAppDraw;
		ldRegAppDraw.Area = ldUpdate;
		ldRegAppDraw.BufferOffset = glAppBufferMgr.GetBufferOffset(pstBuffer);

		// 发送绘制消息给服务
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = nuMsgID;
		ULONG luMsgBufSize = sizeof(EI_MSG_APP_DRAW);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ldRegAppDraw, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		//gloCenter.RecallMessage(loMsg);
		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}


// 绘制内容到Eink屏
// 将缓存中的内容绘制到Eink屏幕上，调用此函数后，传入的绘制缓存会被自动释放，应用不能继续使用这块缓存
// 参数x,y,w,h表示需要更新的区域；无论是全屏绘制还是局部绘制，绘制缓存中的内容始终对应的是整个屏幕
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiDraw(
	EI_RECT* pstArea,	// indicates the area to draw
	EI_BUFFER* pstBuffer		// 指向绘制缓存
	)
{
	return EiSendDrawRequest(pstArea, pstBuffer, EMHOSTID_DRAW);
}

// 清除Eink屏幕
// Clean up E-ink screen
// return:
//		na
void EiCleanupScreen(
	unsigned char ucBackgroundColor		// the background color of E-ink screen after cleaning up
)
{
	CEiSvrMsgItem loMsg;
	loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
	loMsg.Data.Item.MsgId = EMHOSTID_CLEANUP_SCREEN;
	loMsg.Data.Item.MsgBuf[0] = ucBackgroundColor;
	loMsg.Data.Item.BufSize = 1;

	gloCenter.SendMessageToService(loMsg);
}


// Fuction Switcher使用的绘制接口
// 不同于EiDraw函数，此处的pstBuffer并不是描述完整的屏幕缓冲区，而是仅仅描述Switcher条的区域
//    并且，此处的Buffer的每一个字节的最低位表示该像素是否被显示出来
DWORD EiSwitcherDraw(
	EI_BUFFER* pstBuffer		// image buffer 
	)
{
	EI_RECT ldDummy = { 0,0,0,0 };

	return EiSendDrawRequest(&ldDummy, pstBuffer, EMHOSTID_SWITCHER_DRAW);
}

// 设置Switcher的大小
void SetSwitcherLocation(
	const EI_RECT* npLocation // 其中x存放的是相对于右侧的距离
)
{
	CEiSvrMsgItem loMsg;
	loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
	loMsg.Data.Item.MsgId = EMHOSTID_SWITCHER_LOCATION;
	ULONG luMsgBufSize = sizeof(EI_RECT);
	memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, npLocation, luMsgBufSize);
	loMsg.Data.Item.BufSize = luMsgBufSize;

	gloCenter.SendMessageToService(loMsg);

}

// 设置Switcher的可视区域
void SetSwitcherShowArea(
	const EI_RECT* npArea // 其中x存放的是相对于右侧的距离
)
{
	CEiSvrMsgItem loMsg;
	loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
	loMsg.Data.Item.MsgId = EMHOSTID_SWITCHER_SHOW_AREA;
	ULONG luMsgBufSize = sizeof(EI_RECT);
	memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, npArea, luMsgBufSize);
	loMsg.Data.Item.BufSize = luMsgBufSize;

	gloCenter.SendMessageToService(loMsg);
}

// 通知系统关机封面图片的路径
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetShutdownCover(
	const wchar_t* npszFilePath	// 目标文件名
)
{
	DWORD luResult = 0;

	do
	{
		if (npszFilePath == NULL)
			break;

		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_SHUTDOWN_COVER;

		ULONG luMsgBufSize = (ULONG)(wcslen(npszFilePath) + 1) * sizeof(wchar_t);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, npszFilePath, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 促使系统产生一条WM_EI_DRAW消息给当前APP
// 编写APP时，可以将所有的绘制都放在处理WM_EI_DRAW消息的过程中，如果APP期待主动进行一次绘制操作，只需要调用本函数，促使系统发送一条WM_EI_DRAW
// 消息给APP，从而就能够将绘制操作集中在一个地方进行（WM_EI_DRAW处理过程）
void EiInvalidPanel(
	EI_RECT* pstArea	// indicates the area to update
	)
{
	// 发送绘制消息给服务，由服务来决定是否重绘
	CEiSvrMsgItem loMsg;
	loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
	loMsg.Data.Item.MsgId = EMHOSTID_INVALID_PANEL;
	ULONG luMsgBufSize = sizeof(EI_RECT);
	if (pstArea != NULL)
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, pstArea, luMsgBufSize);
	else
		RtlZeroMemory(loMsg.Data.Item.MsgBuf, luMsgBufSize);
	loMsg.Data.Item.BufSize = luMsgBufSize;

	gloCenter.PostMessageToService(loMsg);
}


// 进入Handwriting模式
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHandWritingMode(
	DWORD eMode // 进入或退出手写模式，参考GIHW_AUTO
	){

	DWORD luResult = ERROR_SUCCESS;

	do
	{
		// 注册App
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_SCENARIO;


		ULONG luMsgBufSize = (ULONG)sizeof(DWORD);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &eMode, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 设置笔宽，尚未支持
// set width of pen in handwriting mode.
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHandwritingPen(
	DWORD dwMin,	// 最小的笔宽	// min pen width
	DWORD dwMax	// 最大的笔宽	// max pen width
)
{
	return ERROR_NOT_READY;
}

// 设置屏幕显示方向
// set the screen orientation for current app
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetScreenOrient(
	DWORD eOrientation // refer to GIR_NONE
	)
{
	DWORD luResult = ERROR_NOT_READY;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_DISPLAY_ORIENTATION;


		ULONG luMsgBufSize = (ULONG)sizeof(eOrientation);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &eOrientation, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

		//自己记下来，省得每次找服务要
		gldwOrientation = eOrientation;

	} while (false);

	return luResult;
}

// 停止服务
// 当一个应用实例需要退出时，调用这个函数
void EiEnd(void){
	do
	{
		// 注销App
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_UNREG_APP;
		gloCenter.PostMessageToService(loMsg);

		//等待一下，否则服务没有关闭文件句柄是删除不了文件的
		Sleep(50);

	} while (false);

}

// 内部函数，启动一个APP
// 通知系统启动一个指定的APP
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiRunAPP(
	const wchar_t* npszFilePath,	// 目标文件名
	int niSession				//可以指定运行于哪个session,0表示system用户
) {
	DWORD luResult = 0;

	do
	{
		if (npszFilePath == NULL)
			break;

		// 注册App
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_RUN_APP;

		EI_MSG_RUN_APP ldRunInfo;
		wcscpy_s(ldRunInfo.FilePath, MAX_PATH, npszFilePath);
		ldRunInfo.SessionID = niSession;
		ULONG luMsgBufSize = sizeof(EI_MSG_RUN_APP);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ldRunInfo, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);

		//发送命令
		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 从物理坐标系转换到显示坐标系
// Convert a point from physical-coordinate to display-coordinate
void EiPhysicalToDisplay(
	IN EI_POINT* pstPointP,
	OUT EI_POINT* pstPointD
	)
{
	// 获取当前屏幕显示方向，而后以此计算坐标转换 to ???ax			
	pstPointD->x = pstPointP->x;
	pstPointD->y = pstPointP->y;
}

// 从显示坐标系转换到物理坐标系
// Convert a point from display-coordinate to physical-coordinate
void EiDisplayToPhysical(
	IN EI_POINT* pstPointD,
	OUT EI_POINT* pstPointP
)
{
	// 获取当前屏幕显示方向，而后以此计算坐标转换 to ???ax
	pstPointD->x = pstPointP->x;
	pstPointD->y = pstPointP->y;
}

// 设置FW模式
DWORD EiSetWaveformMode(
	DWORD dwMode 
) {

	DWORD luResult = ERROR_NOT_READY;

	do
	{
		// 注册App
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_FW_MODE;


		ULONG luMsgBufSize = (ULONG)sizeof(DWORD);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &dwMode, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统键盘图片的路径
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetKeyboardImg(
	const wchar_t* npszFilePath	// 目标文件名
){
	DWORD luResult = 0;

	do
	{
		if (npszFilePath == NULL)
			break;

		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_PATH;

		ULONG luMsgBufSize = (ULONG)(wcslen(npszFilePath) + 1) * sizeof(wchar_t);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, npszFilePath, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统显示状态要发生变化
// 返回： 返回非零，表示成功；返回零，表示失败；
// 1显示封面 2显示键盘A 3显示键盘B
DWORD EiSetShowStatus(
	ULONG nulStatus	// 状态
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_SHOW_STATUS;

		ULONG luMsgBufSize = (ULONG)sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nulStatus, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// Enable / disable direct handwriting region, which means handwriting is handled by tcon.
// To disable it, set both width and height = 0.
// Direct handwriting can only be used in GIHW_HANDWRITING mode.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHandwritingRect(
	EI_RECT dRect
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_HANDWRITING_RECT;

		ULONG luMsgBufSize = (ULONG)sizeof(EI_RECT);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &dRect, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// Check DisplayArea engine status.
//
// 返回： 返回TRUE，表示空闲。返回FALSE，表示绘制被占用;
// return
//		TRUE: ready
//		FALSE: not ready
BOOL EiCheckDpyReady()
{
	DWORD luResult = 0;
	BOOL lbRet = FALSE;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_DRAW_READY;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

		memcpy_s(&lbRet, sizeof(BOOL), loMsg.Data.Item.MsgBuf, sizeof(BOOL));

	} while (false);

	return lbRet;
}

// Set PartialUpdate feature
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetPartialUpdate(
	BOOL enable
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_PARTIAL_ENABLE;

		ULONG luMsgBufSize = (ULONG)sizeof(BOOL);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &enable, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// Get scenario
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiGetScenario(DWORD& rdwCurrentMode)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_SCENARIO_MODE;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

		memcpy_s(&rdwCurrentMode, sizeof(DWORD), loMsg.Data.Item.MsgBuf, sizeof(DWORD));

	} while (false);

	return luResult;
}

// 通知系统机器形态发生变化
// 返回： 返回非零，表示成功；返回零，表示失败；
// 2笔记本形态 3tent形态
DWORD EiSetLaptopMode(
	ULONG nulMode	// 模式
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_LAPTOP_MODE_CHANGED;

		ULONG luMsgBufSize = sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nulMode, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		gloCenter.RecallMessage(loMsg);

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统键盘按键音开关状态
// 返回： 返回非零，表示成功；返回零，表示失败；
// -1不设置 0关闭 1开启
DWORD EiSetKeyboardDownSounds(
	bool nbIsSet
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_DOWN;

		ULONG luMsgBufSize = sizeof(bool);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsSet, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}


// 通知系统键盘按键音开关状态
// 返回： 返回非零，表示成功；返回零，表示失败；
// -1不设置 0关闭 1开启
DWORD EiSetKeyboardUpSounds(
	bool nbIsSet
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_UP;

		ULONG luMsgBufSize = sizeof(bool);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsSet, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);
		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// Set Homebar status
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHomebarStatus(
	ULONG lulStatus   //GI_HOMEBAR_HIDE/ GI_HOMEBAR_SHOW / GI_HOMEBAR_EXPAND / GI_HOMEBAR_COLLAPSE
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_HOMEBAR_STATUS;

		ULONG luMsgBufSize = sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &lulStatus, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// OOBE complete
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiOOBEComplete()
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_OOBE_COMPLETE;

		ULONG luMsgBufSize = 0;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// OOBE cstart
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiOOBEStart()
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_OOBE_START;

		ULONG luMsgBufSize = 0;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 获取系统键盘音音量大小
// 返回：音量大小(0-100)
DWORD EiGetKeySoundsVolume(void)
{
	ULONG luResult = ERROR_NOT_READY;
	LONG Volume = 0;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_KEYBOARD_VOLUME;
		loMsg.Data.Item.BufSize = 0;
		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);
		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return 0;

		//获取值
		Volume = *(LONG*)loMsg.Data.Item.MsgBuf;
	} while (false);

	return Volume;
}

// 设置系统键盘音音量大小
// 返回：无
void EiSetKeySoundsVolume(LONG nlVolume)//(0-100)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_VOLUME;

		ULONG luMsgBufSize = sizeof(nlVolume);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nlVolume, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);
		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);
}

// Set direct handwriting setting, e.g. line width, eraser
//
// Return value
//		zero: success
//		non-zero: error code
DWORD EiSetHandwritingSetting(BYTE lineWidth)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_HANDWRITING_LINE_WIDTH;

		ULONG luMsgBufSize = sizeof(BYTE);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &lineWidth, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 获取当前机器形态
// Get tablet mode
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwCurrentMode : GIR_MODE_LAPTOP/GIR_MODE_TENT/GIR_MODE_TABLET
DWORD EiGetTabletMode(DWORD& rdwCurrentMode)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_TABLET_MODE;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

		memcpy_s(&rdwCurrentMode, sizeof(DWORD), loMsg.Data.Item.MsgBuf, sizeof(DWORD));

	} while (false);

	return luResult;
}

// 通知系统homebar形态发生变化
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiHomebarChanged(
	ULONG nulMode	// 模式
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_HOMEBAR_CHANGE;

		ULONG luMsgBufSize = sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nulMode, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统屏幕方向
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetOrientation(
	DWORD ndwOrientation	// 模式
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_ORIENTATION_CHANGED;

		DWORD luMsgBufSize = sizeof(DWORD);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ndwOrientation, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		gloCenter.RecallMessage(loMsg);

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统
// 返回： 返回非零，表示成功；返回零，表示失败；
void EiSetBCover(
	char* npszPath	// 模式
)
{
	do 
	{
		if (npszPath == NULL)
			break;

		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_BCOVER_PATH;

		ULONG luMsgBufSize = sizeof(char) * (strlen(npszPath)+1);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, npszPath, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		//发送命令
		gloCenter.SendMessageToService(loMsg);

	} while (false);
	
}

// 通知系统B面进入或退出双击模式
// 返回： 返回非零，表示成功；返回零，表示失败；
void EiBCoverEnterDbTap(
	bool nbIsEnter
)
{
	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_B_ENTER_DBTAP;

		ULONG luMsgBufSize = sizeof(bool);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsEnter, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		//发送命令
		gloCenter.SendMessageToService(loMsg);

	} while (false);
}

// 设置键盘样式
// Set keyboard style
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetKeyboardStyle(EI_KEYBOARD_STYLE ndStyle)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_STYLE;

		ULONG luMsgBufSize = (ULONG)sizeof(EI_KEYBOARD_STYLE);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ndStyle, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 调整键盘灵敏度
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetKeyboardSensitivity(
	LONG nlLevel
) {
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_KEYBOARD_SENSITIVITY;

		ULONG luMsgBufSize = (ULONG)sizeof(LONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nlLevel, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知homebar把B面灭屏，为了解决合盖屏幕自动亮起的bug
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiCloseBCover(
	BOOL nbClose
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_CLOSE_B_COVER;

		ULONG luMsgBufSize = (ULONG)sizeof(BOOL);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbClose, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务播放键盘声音，为了试音
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiPlayKeyboardSound(
	ULONG nlType
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_PLAY_KEYBOARD_SOUND;

		ULONG luMsgBufSize = (ULONG)sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nlType, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务把机器形态切换到平板模式
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiChangeTabletMode(
	bool nbIsTablet  //为真表示切换到平板，否则切换到笔记本
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_CHANGE_TABLET_MODE;

		ULONG luMsgBufSize = (ULONG)sizeof(bool);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsTablet, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 开启/关闭knock knock功能
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiOpenKnockKnock(
	bool nbIsOpen  //为真表示打开
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_OPEN_KNOCK_KNOCK;

		ULONG luMsgBufSize = (ULONG)sizeof(bool);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsOpen, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 设置C面手或笔区域
// Set TP Area
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetTpArea(SET_TP_AREA ndTpArea)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_TP_AREA;

		ULONG luMsgBufSize = (ULONG)sizeof(SET_TP_AREA);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ndTpArea, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务检测一下当前切换的应用正常切换过来没
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiCheckAppStatus()
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_CHECK_APP_STATUS;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务通知当前应用重新设置tp area
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiResetTpArea()
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_RESET_TP_AREA;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 获取隐私协议开关状态
// Get the privacy protocol switch status.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwStatus : 0 off,1 on
DWORD EiGetPrivacyStatus(DWORD& rdwStatus)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_PRIVACY_STATUS;

		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

		memcpy_s(&rdwStatus, sizeof(DWORD), loMsg.Data.Item.MsgBuf, sizeof(DWORD));

	} while (false);

	return luResult;
}


// 设置隐私协议开关状态
// Set the privacy protocol switch status.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwStatus : 0 off,1 on
DWORD EiSetPrivacyStatus(DWORD& rdwStatus)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_PRIVACY_STATUS;

		DWORD luMsgBufSize = (DWORD)sizeof(DWORD);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &rdwStatus, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知系统启动一个当前用户下的应用，例如打开某个文件夹或某个网页
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiRunExeForCurrentUset(
	const wchar_t* npszFilePath,	// 目标文件名
	const wchar_t* npszCommandLine
)
{
	DWORD luResult = 0;

	do
	{
		if (npszFilePath == NULL)
			break;

		// 注册App
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_RUN_EXE_FOR_CURRENT_USER;

		EI_MSG_RUN_APP ldRunInfo;
		wcscpy_s(ldRunInfo.FilePath, MAX_PATH, npszFilePath);
		wcscpy_s(ldRunInfo.CommandLine, MAX_PATH, npszCommandLine);
		ldRunInfo.SessionID = -1;
		ULONG luMsgBufSize = sizeof(EI_MSG_RUN_APP);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ldRunInfo, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		//先把当前存在的切换命令都清除
		gloCenter.RecallMessage(loMsg);

		//发送命令
		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务C面物理方向变化
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetCCoverOri(
	ULONG nulOri
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_SET_CCOVER_ORI;

		DWORD luMsgBufSize = (DWORD)sizeof(ULONG);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nulOri, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 通知服务电源状态变化
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiSetPowerStatus(
	DWORD ndwStatus
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_POWER_CHANGE;

		DWORD luMsgBufSize = (DWORD)sizeof(DWORD);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ndwStatus, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;
		//先把当前存在的切换命令都清除
		//gloCenter.RecallMessage(loMsg);
		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 询问是否需要显示oobe,同一用户只显示一次
// 返回： 返回非零，表示成功；返回零，表示失败；
ULONG EiIsShowOOBE(
	bool& rbFlag
)
{
	ULONG luResult = ERROR_NOT_READY;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_IS_SHOW_OOBE;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return 0;

		//获取值
		rbFlag = *(bool*)loMsg.Data.Item.MsgBuf;

	} while (false);

	return luResult;
}

// 询问8951是否处于sleep状态
// 返回： 返回非零，表示成功；返回零，表示失败；
ULONG Ei8951IsSleep(
	bool& rbSleep
)
{
	ULONG luResult = ERROR_NOT_READY;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_8951_IS_SLEEP;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return 0;

		//获取值
		rbSleep = *(bool*)loMsg.Data.Item.MsgBuf;

	} while (false);

	return luResult;
}

// 获取用户显示语言
// 返回： 返回非零，表示成功；返回零，表示失败；
ULONG EiGetUserLagID(
	DWORD& rdwLagID
)
{
	ULONG luResult = ERROR_NOT_READY;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_USER_LAGID;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return 0;

		//获取值
		rdwLagID = *(DWORD*)loMsg.Data.Item.MsgBuf;

	} while (false);

	return luResult;
}

// 通知服务8951状态变化
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD Ei8951StatusChanged(
	bool nbIsConnect
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_8951_CONNECT_OR_REMOVE;

		DWORD luMsgBufSize = (DWORD)sizeof(nbIsConnect);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsConnect, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}


// 通知服务5182状态变化
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD Ei5182StatusChanged(
	bool nbIsConnect
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_5182_CONNECT_OR_REMOVE;

		DWORD luMsgBufSize = (DWORD)sizeof(nbIsConnect);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &nbIsConnect, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 判断自己是否是前台窗口
// Is it a foreground window
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiIsForeground(bool& rbIsForeground)
{
	ULONG luResult = ERROR_NOT_READY;
	rbIsForeground = false;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_IS_FOREGROUND_WINDOW;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return 0;

		//获取值
		rbIsForeground = *(bool*)loMsg.Data.Item.MsgBuf;

	} while (false);

	return luResult;
}

// 通知Homebar切换应用
// 返回： 返回非零，表示成功；返回零，表示失败；
ULONG EiChangeApp(
	DWORD ldwID
)
{
	DWORD luResult = 0;

	do
	{
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_CHANGE_APP;

		DWORD luMsgBufSize = (DWORD)sizeof(ldwID);
		memcpy_s(loMsg.Data.Item.MsgBuf, luMsgBufSize, &ldwID, luMsgBufSize);
		loMsg.Data.Item.BufSize = luMsgBufSize;

		luResult = gloCenter.PostMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			break;

	} while (false);

	return luResult;
}

// 获取当前用户磁盘列表
// 返回： 返回非零，表示成功；返回零，表示失败；
DWORD EiGetUserDiskList(
)
{
	ULONG luResult = ERROR_NOT_READY;

	do
	{
		//给服务发送获取消息
		CEiSvrMsgItem loMsg;
		loMsg.Data.Item.AppId = GetCurrentProcessId(); // AppID直接取自身进程ID
		loMsg.Data.Item.MsgId = EMHOSTID_GET_USER_DIST_LIST;
		loMsg.Data.Item.BufSize = 0;

		luResult = gloCenter.SendMessageToService(loMsg);
		if (luResult != ERROR_SUCCESS)
			return luResult;

		luResult = ERROR_SUCCESS;

	} while (false);

	return luResult;
}