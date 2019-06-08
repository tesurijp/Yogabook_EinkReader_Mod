/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once
#ifndef _EINKINTERNAL_H_
#define _EINKINTERNAL_H_
/*
	EinkIteApi dll 的内部函数
*/



#define GIHW_KEYBOARD	1
#define GIHW_DB_CLICK	4



//服务通知应用事件
#define WM_EI_SERVICE_EVENT WM_USER + 0x501
//WParam: ULONG 
#define EI_POWER_ENTER_CONNECTED_STANDBY 0	//进入sleep
#define EI_POWER_RETURN_CONNECTED_STANDBY 1	//唤醒
#define EI_POWER_ENTER_SHUTDOWN 2			//关机
#define EI_B_COVER_DB_CLICK 3				//B面双击
#define EI_C_COVER_DB_CLICK 4				//C面双击
#define EI_C_ENTER_KB 5						//进入键盘模式
#define EI_C_SHOW_KB_DISABLE 6				//显示键盘不可用
#define EI_C_SHOW_DB_WAKEUP 7				//显示双击可以唤醒
#define EI_SET_HOMEBAR_CREATE_PAD 8			//Homebar切换到createpad
#define EI_SET_HOMEBAR_PEN_MOUSE 9			//Homebar切换到Pen mouse
#define EI_SET_HOMEBAR_EXPAND 10			//Homebar进入展开状态
#define EI_SET_HOMEBAR_COLLAPSE 11			//Homebar进入收缩状态
#define EI_SET_HOMEBAR_UP_SHOW 12			//Homebar在clone下显示出来
#define EI_SET_HOMEBAR_KEYBOARD 13			//Homebar切换到键盘
#define EI_SET_HOMEBAR_CLOSE_B_COVER 14		//Homebar关闭B面
#define EI_SET_HOMEBAR_SHOW 15				//Homebar显示，重新刷新一下
#define EI_POWER_ENTER_CONNECTED_STANDBY_NONE 16
#define EI_POWER_RETURN_CONNECTED_STANDBY_NONE 17
#define EI_OOBE_COMPLETE 18	//C面OOBE程序退出

//LParam: NA

#define EI_PLAY_KEYBOARD_SOUND_DOWN 1	//播放按下音
#define EI_PLAY_KEYBOARD_SOUND_UP 2		//播放释放音

#ifdef __cplusplus
extern "C" {
#endif

	// 内部函数，启动一个APP
	// 通知系统启动一个指定的APP
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiRunAPP(
		const wchar_t* npszFilePath,	// 目标文件名
		int niSession=-1					//可以指定运行于哪个session,0表示system用户
	);

	// 设置Switcher的大小
	void SetSwitcherLocation(
		const EI_RECT* npLocation // 其中x存放的是相对于右侧的距离
	);

	// 设置Switcher的可视区域
	void SetSwitcherShowArea(
		const EI_RECT* npArea // 其中x存放的是相对于右侧的距离
	);

	// Fuction Switcher使用的绘制接口
	// 不同于EiDraw函数，此处的pstBuffer并不是描述完整的屏幕缓冲区，而是仅仅描述Switcher条的区域
	//    并且，此处的Buffer的每一个字节的最低位表示该像素是否被显示出来
	DWORD EiSwitcherDraw(
		EI_BUFFER* pstBuffer		// image buffer 
	);

	// 通知系统关机封面图片的路径
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetShutdownCover(
		const wchar_t* npszFilePath	// 目标文件名
	);

	// 通知系统键盘图片的路径
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetKeyboardImg(
		const wchar_t* npszFilePath	// 目标文件名
	);

	// 通知系统显示状态要发生变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	// 0正常显示APP 1显示封面 2显示键盘A 3显示键盘B 
	DWORD EiSetShowStatus(
		ULONG nulStatus	// 状态
	);

	// 通知系统机器形态发生变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	// 2笔记本形态 3tent形态
	DWORD EiSetLaptopMode(
		ULONG nulMode	// 模式
	);

	// 通知系统键盘按键音开关状态
	// 返回： 返回非零，表示成功；返回零，表示失败；
	// flase关闭 true开启
	DWORD EiSetKeyboardDownSounds(
		bool nbIsSet
	);

	// 通知系统键盘按键音开关状态
	// 返回： 返回非零，表示成功；返回零，表示失败；
	// flase关闭 true开启
	DWORD EiSetKeyboardUpSounds(
		bool nbIsSet
	);

	// OOBE cstart
	//
	// 返回： 返回零，表示成功；返回非零，表示错误码；
	// return
	//		zero: success
	//		non-zero: error code
	DWORD EiOOBEStart();
	
	// 获取系统键盘音音量大小
	// 返回：音量大小(0-100)
	DWORD EiGetKeySoundsVolume(void);

	// 设置系统键盘音音量大小
	// 返回：无
	void EiSetKeySoundsVolume(LONG nlVolume);//(0-100)

	// 通知系统homebar形态发生变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiHomebarChanged(
		ULONG nulMode	// 模式
	);

	// 通知系统屏幕方向
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetOrientation(
		DWORD ndwOrientation	// 模式
	);

	// 通知系统
	// 返回： 返回非零，表示成功；返回零，表示失败；
	void EiSetBCover(
		char* npszPath	// 模式
	);

	// 通知系统B面进入或退出双击模式
	// 返回： 返回非零，表示成功；返回零，表示失败；
	void EiBCoverEnterDbTap(
		bool nbIsEnter
	);

	// 调整键盘灵敏度
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetKeyboardSensitivity(
		LONG nlLevel
	);

	// 通知homebar把B面灭屏，为了解决合盖屏幕自动亮起的bug
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiCloseBCover(
		BOOL nbClose
	);

	// 通知服务播放键盘声音，为了试音
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiPlayKeyboardSound(
		ULONG nlType
	);

	// 通知服务把机器形态切换到平板模式
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiChangeTabletMode(
		bool nbIsTablet  //为真表示切换到平板，否则切换到笔记本
	);

	// 通知服务检测一下当前切换的应用正常切换过来没
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiCheckAppStatus();


	// 开启/关闭knock knock功能
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiOpenKnockKnock(
		bool nbIsOpen  //为真表示打开
	);


	// 通知服务通知当前应用重新设置tp area
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiResetTpArea();
	
	// 通知系统启动一个当前用户下的应用，例如打开某个文件夹或某个网页
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiRunExeForCurrentUset(
		const wchar_t* npszFilePath,	// 目标文件名
		const wchar_t* npszCommandLine
	);

	// 通知服务C面物理方向变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetCCoverOri(
		ULONG nulOri
	);

	// 通知服务电源状态变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiSetPowerStatus(
		DWORD ndwStatus
	);

	// 询问是否需要显示oobe,同一用户只显示一次
	// 返回： 返回非零，表示成功；返回零，表示失败；
	ULONG EiIsShowOOBE(
		bool& rbFlag
	);


	// 通知服务8951状态变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD Ei8951StatusChanged(
		bool nbIsConnect
	);

	// 询问8951是否处于sleep状态
	// 返回： 返回非零，表示成功；返回零，表示失败；
	ULONG Ei8951IsSleep(
		bool& rbSleep
	);

	// 通知Homebar切换应用
	// 返回： 返回非零，表示成功；返回零，表示失败；
	ULONG EiChangeApp(
		DWORD ldwID
	);
	// 通知服务5182状态变化
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD Ei5182StatusChanged(
		bool nbIsConnect
	);

	// 获取当前用户磁盘列表
	// 返回： 返回非零，表示成功；返回零，表示失败；
	DWORD EiGetUserDiskList(
		void
	);

#define EI_LAPTOP_MODE 2
#define EI_TENT_MODE 3


#ifdef __cplusplus
}
#endif



#endif//_EINKINTERNAL_H_
