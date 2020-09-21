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

// 重置隐藏工具栏定时器
#define EEVT_RESET_HIDE_TIME		EMSG_TS_REQUEST(1,23)			
// input bool true重置定时器，false关闭定时器
// output none
// return none


// 显示或隐藏工具栏
#define EEVT_SHOW_TOOLBAR		EMSG_TS_REQUEST(1,24)			
// input bool
// output none
// return none


// 清空阅读历史
#define EEVT_DELETE_READ_HISTORY		EMSG_TS_REQUEST(1,25)			
// input none
// output none
// return none

// 菜单项被点击
#define EEVT_MENU_ITEM_CLICKED		EMSG_TS_REQUEST(1,26)			
// input ULONG 按钮id
// output none


// 清除本页标注
#define EEVT_MENU_ERASER_ALL	EMSG_TS_REQUEST(1,27)			
// input none
// output none
// return none


// 设置线宽
#define EEVT_SET_PEN_WIDTH	EMSG_TS_REQUEST(1,28)			
// input ULONG 笔宽数组下标
// output none
// return none

// 设置颜色
#define EEVT_SET_PEN_COLOR	EMSG_TS_REQUEST(1,29)			
// input ULONG 0红色 1黄色 2蓝色
// output none
// return none

// 输入事件
#define EEVT_TOUCH_INPUT		EMSG_TS_REQUEST(1,29)			
// input 输入消息
// output none
// return none

// 设置笔状态
#define EEVT_PEN_MODE		EMSG_TS_REQUEST(1,30)			
// input ULONG
// output none
// return none

// 设置手画线开关
#define EEVT_HAND_WRITE		EMSG_TS_REQUEST(1,31)			
// input bool
// output none
// return none

// undo
#define EEVT_UNDO		EMSG_TS_REQUEST(1,32)			
// input none
// output none
// return none

// redo
#define EEVT_REDO		EMSG_TS_REQUEST(1,33)			
// input none
// output none
// return none

// 通知工具栏更新页面状态
#define EEVT_UPDATE_PAGE_STATUS		EMSG_TS_REQUEST(1,34)	
typedef struct _PAGE_STATUS {
	int RedoCount; //redo还有几步
	int UndoCount; //undo还有几步
	int InkCount; //有多少笔迹数据
}PAGE_STATUS, *PPAGE_STATUS;

// input PAGE_STATUS
// output none
// return none


// 通知工具栏，页码发生变化
#define EEVT_PAGE_CHANGED		EMSG_TS_REQUEST(1,35)			
// input none
// output none
// return none

// 文件被修改
#define EEVT_FILE_MODIFY		EMSG_TS_REQUEST(1,36)			
// input none
// output none
// return none

// 告诉主界面，打开历史记录中的文件
#define EEVT_ER_OPEN_HISTORY_FILE_PATH	EMSG_TS_REQUEST(1,37)			
// input ULONT
// output bool 
// return none

// 设置画线区域
#define EEVT_SET_FW_LINE_RECT	EMSG_TS_REQUEST(1,38)			
// input ED_RECT
// output none 
// return none

// 获取页面移动状态按钮
#define EEVT_GET_MOVE_BUTTON_STATUS	EMSG_TS_REQUEST(1,39)			
// input none
// output RECT 
// return none

// 显示或隐藏翻页滚动条
#define EEVT_SHOW_PAGE_PROCESS	EMSG_TS_REQUEST(1,40)			
// input bool
// output none 
// return none

// 静默方式设置手画线开关，不弹toast
#define EEVT_HAND_WRITE_MUTE		EMSG_TS_REQUEST(1,41)			
// input bool
// output none
// return none

// highlight toolbar按钮事件
#define EEVT_HIGHLIGHT_BT_EVENT		EMSG_TS_REQUEST(1,42)			
// input int
// output none
// return none
//按钮类型
#define HIGHLIGHT_BT_COPY 1
#define HIGHLIGHT_BT_HIGHLIGHT 2
#define HIGHLIGHT_BT_DELETE_LINE 3
#define HIGHLIGHT_BT_UNDER_LINE 4
#define HIGHLIGHT_BT_DELETE 5
#define HIGHLIGHT_BT_TRANSLATE 6

// 选中指定区域内的文字
#define EEVT_SELECT_HIGHLIGHT		EMSG_TS_REQUEST(1,43)			
// input D2D1_RECT_F
// output SELECT_HIGHLIGHT
// return none
typedef struct _SELECT_HIGHLIGHT {
	int QuadCount; //有几个选中块
	int AnnotCount; //有几个已有高亮的对象
	D2D1_POINT_2F PointA;
	D2D1_POINT_2F PointB;
}SELECT_HIGHLIGHT, *PSELECT_HIGHLIGHT;

// 获取选中区域信息
#define EEVT_GET_SELECT_HIGHLIGHT_INFO		EMSG_TS_REQUEST(1,44)			
// input none
// output SELECT_HIGHLIGHT
// return none


// 设置页面方向
#define EEVT_SET_SCREEN_STATUS	EMSG_TS_REQUEST(1,45)			
// input ULONG
// output none
// return none
#define SCREEN_STATUS_AUTO 10 //自动
#define SCREEN_STATUS_H 11	//横屏
#define SCREEN_STATUS_V 12	//坚屏	


// 打开或关闭partaul
#define EEVT_PARTIAL_ENABLE		EMSG_TS_REQUEST(1,46)			
// input bool
// output none
// return none


// 进入缩略图界面
#define EEVT_ENTER_THUMBNAIL_DLG	EMSG_TS_REQUEST(1,47)			
// input none
// output none
// return none

//缩略图某页被点击
#define EEVT_THUMBNAIL_CLICK	EMSG_TS_REQUEST(1,48)			
// input int
// output none
// return none

// 设置页面方向
#define EEVT_THUMBNAIL_SELECT	EMSG_TS_REQUEST(1,49)			
// input ULONG
// output none
// return none
#define THUMBNAIL_SELECT_ALL 1
#define THUMBNAIL_SELECT_ANNOT 2

// 全部缩略图加载完毕
#define EEVT_THUMBNAIL_COMPLETE		EMSG_TS_REQUEST(1,50)			
// input ULONG  count of all pages loaded
// output none
// return none

// 缩略图目录更新
#define EEVT_UPDATE_THUMBNAIL_PATH		EMSG_TS_REQUEST(1,51)			
// input wchat*
// output none
// return none

// 打开或关闭B面屏幕
#define EEVT_CLOSE_B_SCREEN	EMSG_TS_REQUEST(1,52)			
// input bool true亮屏 false熄屏
// output none
// return none

// 机器形态的变化
#define EEVT_HB_MODE_CHANGE		EMSG_TS_REQUEST(1,53)			
// input ULONG  wparam :
// GIR_MODE_LAPTOP  2
// GIR_MODE_TENT 3
// GIR_MODE_TABLET 4
// output none
// return none

// [zhuhl5@20200116:ZoomRecovery]
// 通知当前缩放倍数挡位
#define EEVT_ER_SET_ZOOMLEVEL	EMSG_TS_REQUEST(1, 54)			
// input int
// output none
// return none

// [zhuhl5@20200116:ZoomRecovery]
// 转换并打开Office文档
#define EEVT_CONVERT_OPEN_OFFICE_FILE EMSG_TS_REQUEST(1, 55)			
// input int
// output none
// return none
// 取消高亮选中
#define EEVT_HIDE_HIGHLIGHT		EMSG_TS_REQUEST(1,56)			
// input none
// output none
// return none


// 清全屏
#define EEVT_CLEAR_FULL_SCREEN	EMSG_TS_REQUEST(1,57)			
// input none
// output none
// return none

// 自动撑满屏幕
#define EEVT_ER_AUTO_ZOOM EMSG_TS_REQUEST(1,58)

//文档类型
#define DOC_TYPE_PDF 0
#define DOC_TYPE_EPUB 1
#define DOC_TYPE_MOBI 2
#define DOC_TYPE_TXT 3

//笔颜色
#define PEN_COLOR_BLACK 0
#define PEN_COLOR_RED 1
#define PEN_COLOR_BLUE 2

//笔状态
#define PEN_MODE_NONE 0
#define PEN_MODE_PEN 1
#define PEN_MODE_ERASER 2
#define PEN_MODE_SELECT 3