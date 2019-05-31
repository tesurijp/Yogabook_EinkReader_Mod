/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
//定义程序所需要的消息

// 自定义消息
#define ER_TYPE_BASE	0xEA
#define EMSG_TS_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(ER_TYPE_BASE,_MJ_NUM,_MN_NUM)

// 打开文件对话框
#define EEVT_ER_OPEN_FILE		EMSG_TS_REQUEST(1,1)			
// input none
// output none
// return none

// 显示功能区提示
#define EEVT_ER_SHOW_TIP		EMSG_TS_REQUEST(1,2)			
// input none
// output none
// return none

// 前台/后台
#define EEVT_HB_ACTIVITE		EMSG_TS_REQUEST(1,3)			
// input ULONG  wparam 0后台 1前台
// output none
// return none

// list项被点击
#define EEVT_ER_LIST_CLICK		EMSG_TS_REQUEST(1,4)			
// input wchat* 路径
// output none
// return none

// 打开页码对话框
#define EEVT_ER_OPEN_PAGE_JUMP	EMSG_TS_REQUEST(1,5)			
// input none
// output none
// return none

// 告诉主界面，跳转到指定页码
#define EEVT_ER_PAGE_JUMP	EMSG_TS_REQUEST(1,6)			
// input int
// output none
// return none

// 告诉主界面，要打开的文件路径
#define EEVT_ER_OPEN_FILE_PATH	EMSG_TS_REQUEST(1,7)			
// input none
// output wchar_t* 
// return none

// homebar状态发生变化
#define EEVT_ER_HOMEBAR_CHANGED	EMSG_TS_REQUEST(1,8)			
// input ULONG
// output none
// return none

//翻页按钮被点击
#define EEVT_ER_PRE_NEXT_CLICKED	EMSG_TS_REQUEST(1,9)			
// input ULONG 是哪个按钮，上一页/下一页/中间显示隐藏区
// output none
// return none

//缩放工具栏按钮被点击
#define EEVT_ER_ZOOM_TOOLBAR_CLICKED	EMSG_TS_REQUEST(1,10)			
// input ULONG 
// output none
// return none

// 进入缩放模式
#define EEVT_ER_ENTER_ZOOM	EMSG_TS_REQUEST(1,11)			
// input bool true进入，false退出
// output none
// return none

// 设置缩放倍数
#define EEVT_ER_SET_ZOOM	EMSG_TS_REQUEST(1,12)			
// input float
// output RECT 4个方向，0表示不能移动了，1表示还能移动 
// return none

// 双屏或单屏显示
#define EEVT_ER_TWO_SCREEN	EMSG_TS_REQUEST(1,13)			
// input bool true双屏 false单屏
// output none
// return none

// 设置页面移动
#define EEVT_ER_SET_PAGE_MOVE	EMSG_TS_REQUEST(1,14)			
// input POINT 正数表示向右下移动，负数表示向左上移动
// output RECT 4个方向，0表示不能移动了，1表示还能移动 
// return none

// 进入截屏模式
#define EEVT_ER_ENTER_SNAPSHOT	EMSG_TS_REQUEST(1,15)			
// input none
// output none
// return none

// 截屏到剪切板
#define EEVT_ER_SNAPSHOT_TO_CLIPBRD	EMSG_TS_REQUEST(1,16)			
// input D2D1_RECT_F
// output none
// return none

// 设置TXT缩放倍数
#define EEVT_ER_SET_TXT_ZOOM	EMSG_TS_REQUEST(1,17)			
// input DWORD
// output none
// return none

// 开始页面排版
#define EEVT_TXT_ARRANGED_START		EMSG_TS_REQUEST(1,20)			
// input none
// output none
// return none

// 全部页面排版中，每个页面发送一条
#define EEVT_TXT_ARRANGED_DOING		EMSG_TS_REQUEST(1,21)			
// input ULONG  current count of pages loaded
// output none
// return none

// 全部页面排版完毕，注意，不能假设“收到EEVT_TXT_ARRANGED_START，就一定工会收到EEVT_ARRANGED_COMPLETE消息”，执行过程有可能被打断
#define EEVT_ARRANGED_COMPLETE		EMSG_TS_REQUEST(1,22)			
// input ULONG  count of all pages loaded
// output none
// return none
