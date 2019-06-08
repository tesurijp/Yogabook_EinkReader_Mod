/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */
#pragma once
#include "EiMsgQueue.h"
#include "SvrMsg.h"


// 这是SDK的App程序与Service程序的消息交换中心
// 调用Initialize初始化后，执行体的内建线程就会启动起来，持续响应Service发送来的消息，其中一部分效益转为Windows消息发送给App主窗口
// 退出时，调用Relaese函数释放

class CEiAppCenter
{
public:
	CEiMsgQueueListener<CEiSvrMsgItem> moAppListener;
	CEiMsgQueueConnector<CEiSvrMsgItem> moConnectToHost;
	ULONG muAppID;

	CEiAppCenter();
	~CEiAppCenter();

	// 初始化
	ULONG Initialize(TRSP_SYSTEM_INFO_DATA& rSystemInfoData);

	// 释放
	void Release(void);

	// 回调入口函数
	static void __stdcall AppCenterCallBack(CEiSvrMsgItem& nrMsg, void* npContext);

	// 主分发函数
	void AppDispatch(CEiSvrMsgItem& nrMsg);

	// 发送消息给Service，并等待
	ULONG SendMessageToService(CEiSvrMsgItem& nrMsg);


	// 发送消息给Service，不等待
	ULONG PostMessageToService(CEiSvrMsgItem& nrMsg);

	// 撤回一类消息，将队列中此类消息全部撤回
	void RecallMessage(const CEiSvrMsgItem& nrMsg);

	//设置接收windows消息的窗口句柄
	void SetHwnd(HWND nHwnd);

	//////////////////////////////////////////////////////////////////////////
	// 以下为具体消息的响应函数
	void MsgBack(CEiSvrMsgItem& nrMsg);

	//获取给绘制用的Buffer地址
	//返回值为地址起始地址，rulBufferSize为Buffer大小
	BYTE* GetBufferBase(ULONG& rulBufferSize);

	// 手指输入消息
	void InputMsg(CEiSvrMsgItem& nrMsg);
	// 全屏重绘
	void ReDraw(CEiSvrMsgItem& nrMsg);
	// Z轴发生变化
	void ZOrderChange(CEiSvrMsgItem& nrMsg);
	// Eink屏幕方向发生变化
	void EinkScreenOrientationChange(CEiSvrMsgItem& nrMsg);
	// 机器形态发生变化
	void LaptopModeChange(CEiSvrMsgItem& nrMsg);
	// 服务通知事件
	void ServiceMsg(CEiSvrMsgItem& nrMsg);
	// homebar状态发生变化
	void HomebarChanged(CEiSvrMsgItem& nrMsg);
	// 键盘样式切换完成
	void KeyboardStyleChangeComplete(CEiSvrMsgItem& nrMsg);
	// 重新设置tp area
	void ResetTPArea(CEiSvrMsgItem& nrMsg);
	//隐私开关状态发生变化
	void PrivacyStatusChanged(CEiSvrMsgItem& nrMsg);
private:
	//获取GUID字符串
	void GetGUIDString(OUT const wchar_t* npszBuffer, int niLen);
	//打开自己的内存映射文件
	bool OpenJasonFile(const wchar_t* nszFileName);
	//打开自己的内存映射文件
	bool OpenServerJasonFile(const wchar_t* nszFileName);


	//自己当监听者用
	HANDLE mhFile;
	DWORD muFileLength;
	HANDLE mhFileMap;
	const char* mpMappedBase;

	//服务器当监听者用
	HANDLE mhServerFile;
	DWORD muServerFileLength;
	HANDLE mhServerFileMap;
	const char* mpServerMappedBase;

	REG_APP_INFO mdRegAppInfo;

	//接收windows消息的窗口句柄
	HWND mhWnd;
};

