/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#pragma once

//读取数据BUFFER大小
#define USBHID_BUFFER_SIZE 1024
#define USBHID_TP_REPORT_ID 0x90
#define USBHID_EMR_REPORT_ID 0x91

class USBHIDAPI
{
public:
	USBHIDAPI(SIZE& nrPanel);
	~USBHIDAPI();

	BOOL EnumHIDDevice(WORD uVID, WORD uPID, //USB VID PID
		BOOL bPresentFlag, //设备必须存在标志 0不需要插入设备
		TCHAR szDevPath[MAX_PATH + 1] = NULL, //设备路径
		int iIndex = 0); //第N个设备 （对多个相同的设备进行区分）


private:
	// 屏显坐标转换用
	SIZE mstPanel;
	//手写板设备句柄，用于读取输入数据
	HANDLE mhReadHandle;
	//数据读取完成通知事件
	HANDLE mhEventObject;
	//等待读取完成线程
	HANDLE mhReadThread;
	//读取文件结构体
	OVERLAPPED mdHIDOverlapped;
	wchar_t mszDevPath[MAX_PATH + 1];
	//读取输入内存
	//PVOID mInputReport;
	BYTE mInputReport[USBHID_BUFFER_SIZE];

	//开始读取数据
	void BeginRead();
	//读取数据线程
	static DWORD __stdcall ReadFileThread(LPVOID npParam);
	

};

