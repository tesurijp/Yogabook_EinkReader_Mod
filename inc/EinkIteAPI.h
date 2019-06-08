/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _EINKITEAPI_H_
#define _EINKITEAPI_H_
/*
	本DLL为应用程序，提供了访问Eink屏的全部接口。
	//This Dll will provide all interface to access eink .
	注意：
		请在同一个线程中访问此SDK提供的API接口。
	Caution:
		please call these apis in single thread .

*/





#pragma pack(1)

typedef struct _TRSP_SYSTEM_INFO_DATA
{
	unsigned int uiStandardCmdNo; // Standard command number2T-con Communication Protocol
	unsigned int uiExtendCmdNo; // Extend command number
	unsigned int uiSignature; // 31 35 39 38h (8951)
	unsigned int uiVersion; // command table version
	unsigned int uiWidth; // Panel Width
	unsigned int uiHeight; // Panel Height
	unsigned int uiUpdateBufBase; // Update Buffer Address
	unsigned int uiImageBufBase; // Image Buffer Address
	unsigned int uiTemperatureNo; // Temperature segment number
	unsigned int uiModeNo; // Display mode number
	unsigned int uiFrameCount[8]; // Frame count for each mode(8).
	unsigned int uiNumImgBuf;
	unsigned int uiWbfSFIAddr;
	unsigned int uiwaveforminfo;//low byte:A2 mode index
	unsigned int uiMultiPanelIndex;//High two byte for Y-axis, low two byte for X-axis
	unsigned int uiTpXMax;  // Tp resolution
	unsigned int uiTpYMax;
	unsigned char TPVersion[4]; //e.g. v.1.0.9  TPVersion[] = {0x00,0x01,0x00,x09}
	unsigned char ucEPDType;    //0-old (needs 180 rotation), 1 - New(no need to 180 rotation)
	unsigned char ucReserved[3];
	unsigned int uiReserved[2];

	//	void* lpCmdInfoDatas[1]; // Command table pointer
} TRSP_SYSTEM_INFO_DATA;

#pragma pack()

#pragma pack(4)

#define GIHW_OWNER_DRAW	0		// Owner-draw mode. If you need handwriting , call EiSetHandwritingRect
#define GIHW_WACOM_PASS_THROUGH	3		// WACOM Pass through mode


#define	GIR_NONE 0
#define GIR_90  90
#define GIR_180 180
#define GIR_270 270

//Laptop Tent Tablet
#define GIR_MODE_LAPTOP  2
#define GIR_MODE_TENT 3
#define GIR_MODE_TABLET 4

// waveform mode
#define GI_WAVEFORM_INIT      0
#define GI_WAVEFORM_DU2       1
#define GI_WAVEFORM_GC16      2
#define GI_WAVEFORM_GL16      3

//Homebar status
#define GI_HOMEBAR_HIDE      0
#define GI_HOMEBAR_SHOW      1
#define GI_HOMEBAR_EXPAND    2
#define GI_HOMEBAR_COLLAPSE  3
#define GI_HOMEBAR_UP_SHOW   4		//向下滑动显示homebar

typedef struct _EI_SYSTEM_INFO{
	unsigned long ulWidth; // physical Screen width
	unsigned long ulHeight; // physical Screen height
	unsigned long ulModeNo; // display mode number
	unsigned long ulOrient;	// current system upward orientation (GIR_NONE\GIR_90\GIR_180\GIR_270)
	unsigned long ulDpiX;	// DPI of X-axis
	unsigned long ulDpiY;	// DPI of Y-axis
	unsigned char ucEPDType;    //0-old (needs 180 rotation), 1 - New(no need to 180 rotation)
	unsigned int uiReserved[10];
}EI_SYSTEM_INFO, *PEI_SYSTEM_INFO;

typedef struct _EI_APP_CONTEXT {
	unsigned long ulOrient;	// current upward orientation setting by APP
	unsigned long ulWidth;	// width of screen in display-coordinate(APP-coordinate)
	unsigned long ulHeight;	// height of screen in display-coordinate(APP-coordinate)
	unsigned long ulScreenOrient;	// current B cover orientation
}EI_APP_CONTEXT,* PEI_APP_CONTEXT;

typedef struct  _EI_BUFFER{
	unsigned long ulWidth; // horizontal size of display area
	unsigned long ulHeight; // vertical size of display area
	unsigned long ulBufferSize;// buffer size
	BYTE Buf[1];
}EI_BUFFER,* PEI_BUFFER;

//Touch事件
#define EI_TOUCHEVENTF_DOWN 0
#define EI_TOUCHEVENTF_MOVE 1
#define EI_TOUCHEVENTF_UP 2
#define EI_TOUCHEVENTF_HOVERING 3
#define EI_TOUCHEVENTF_HOVERING_LEAVE 4

//Touch时笔按键
#define EI_TOUCH_PEN_BUTTON_NONE 0
#define EI_TOUCH_PEN_BUTTON_ABOVE 1
#define EI_TOUCH_PEN_BUTTON_BELOW 2
#define EI_TOUCH_EVENT_FINGER 1
#define EI_TOUCH_EVENT_PEN 2
typedef struct _EI_TOUCHINPUT_POINT {
	unsigned long x;
	unsigned long y;
	unsigned long z;		//pressure
	unsigned long Flags;	//0:down  1:move  2:up 3:hovering
	unsigned long FingerOrPen;	//1:Finger 2:Pen
	unsigned long PenButton;	//0:no 1:above 2:below
}EI_TOUCHINPUT_POINT,* PEI_TOUCHINPUT_POINT;

typedef struct _EI_POINT {
	unsigned long x;
	unsigned long y;
}EI_POINT,* PEI_POINT;

typedef struct _EI_SIZE {
	unsigned long w;
	unsigned long h;
}EI_SIZE, *PEI_SIZE;

typedef struct _EI_RECT {
	unsigned long x;		// x-coordinate of upper-left corner
	unsigned long y;		// y-coordinate of upper-left corner
	unsigned long w;	// widht
	unsigned long h;	// height
}EI_RECT,* PEI_RECT;

// 
//Gesture Message
#define WM_EI_GESTURE	WM_USER + 0x101
//WParam The identifier of the gesture command
#define		EI_GID_BEGIN		1	//A gesture is starting.
#define		EI_GID_END			2	//A gesture is ending.
#define 	EI_GID_ZOOM			3	//The zoom gesture.
#define 	EI_GID_ROTATE		4	//The rotation gesture.
#define		EI_GID_PAN			5	//The pan gesture 
#define 	EI_GID_TWOFINGERTAP 6	//The two-finger tap gesture.
#define		EI_GID_PRESSANDTAP	7	//Indicates the press and tap gesture 
//LParam a pointer to a structure defined below. This pointer will be invalid when this message return.
typedef struct _EI_GESTUREINFO {	// as same as WM_GESTURE defined within the normal windows message WM_GESTURE
	UINT      cbSize;
	DWORD     dwFlags;
	DWORD     dwID;
	HWND      hwndTarget;
	POINTS    ptsLocation;
	DWORD     dwInstanceID;
	DWORD     dwSequenceID;
	ULONGLONG ullArguments;
	UINT      cbExtraArgs;
} EI_GESTUREINFO, *PEI_GESTUREINFO;



#define KEYBOARD_STYLE_CLASSIC_BLACK 1
#define KEYBOARD_STYLE_CLASSIC_WHITE 2
#define KEYBOARD_STYLE_MODERN_BLACK 3
#define KEYBOARD_STYLE_MODERN_WHITE 4
typedef struct _EI_KEYBOARD_STYLE {
	unsigned long Langid;	// Language id
	unsigned long Style;	// 
}EI_KEYBOARD_STYLE, *PEI_KEYBOARD_STYLE;

//设置C面手写区域
typedef struct  _EI_SET_TP_AREA {
	EI_RECT Rect; //要设置的区域
	BYTE Index; //0 ~ 7   可以设置8组不同区域的rect,rect不能重叠
	BYTE Flag; //BYTE ID; //SET_SP_AREA_NO_REPORT、SET_SP_AREA_TOUCH_ONLY、SET_SP_AREA_PEN_ONLY、SET_SP_AREA_TOUCH_PEN
}SET_TP_AREA, *PSET_TP_AREA;
#define SET_SP_AREA_NO_REPORT 0x00
#define SET_SP_AREA_TOUCH_ONLY 0x01
#define SET_SP_AREA_PEN_ONLY 0x02
#define SET_SP_AREA_TOUCH_PEN 0x03


// Touch
#define WM_EI_TOUCH		WM_USER + 0x103
// WParam: NA	
// LParam: EI_TOUCHINPUT_POINT，指向触摸点结构对象，当消息处理返回后，这个指针将失效
// LParam: EI_TOUCHINPUT_POINT, point to a EI_TOUCHINPUT_POINT , when message return , this pointer will be invalid .

// panel-Rotate
// 通知App，平板发现旋转，收到转屏消息后，调用EiSetScreenOrient设置当前APP的屏幕显示方向
// Inform app that panel have been rotated. When an app receive a panel-rotation message , it can call EiSetScreenOrient to set the screen-orientation for itself.
#define WM_EI_ROTATION WM_USER + 0x104
// WParam: GIR_NONE\GIR_90\GIR_180\GIR_270
// LParam: NA

// 请求应用绘制Eink屏，当系统判断需要重新绘制时发送此消息给APP，通过调用EiInvalidPanel API促使系统发送此消息给本APP
// Ask app to re-draw Eink screen , when system need application to re-draw , system will send message to App . 
// by calling EiInvalidPanel API will urge system to send this message to app .
#define WM_EI_DRAW		WM_USER + 0x110
//WParam: NA
//LParam: a pointer of type PEI_RECT to indicate the area to be update

// 活跃应用切换 // active app switch
#define WM_EI_ACTIVATE WM_USER + 0x115
//WParam: 非零表示本应用被显示在前台。为零表示本应用被转到后台	
//WParam: none zero indicate this app is show at front , otherwise this app is switch to background .
//LParam: NA


// 机器形态切换 // Lattop mode switch
#define WM_EI_LAPTOP_MODE_CHANGE WM_USER + 0x116
//WParam: ULONG GIR_MODE_LAPTOP/GIR_MODE_TENT/GIR_MODE_TABLET
//LParam: NA

// Homebar形态切换 // Homebar mode switch
#define WM_EI_HOMEBAR_MODE_CHANGE WM_USER + 0x117
//WParam: ULONG GI_HOMEBAR_EXPAND/GI_HOMEBAR_COLLAPSE
//LParam: NA

// 键盘样式切换完成 // Change Keyboard style to complete.
#define WM_EI_CHANGE_KEYBOARD_STYLE_COMPLETE WM_USER + 0x118
//WParam: BOOL true for success
//LParam: NA

// 重新设置Tp area // Reset TP area.
#define WM_EI_RESET_TP_AREA WM_USER + 0x119
//WParam: NA
//LParam: NA

//隐私开关状态变化 //Privacy Status switch.
#define WM_EI_PRIVACY_CHANGE WM_USER + 0x120
//WParam: 0:off 1:on
//LParam: NA


#ifdef __cplusplus
extern "C" {
#endif


// 初始化
// Initialize
// 在应用启动后，需要调用本接口函数向Eink系统注册本应用。
// After app startup , need to call this function to register itself to Eink system .
// 返回： 返回零，表示成功；返回非零，表示错误码；ERROR_ALREADY_EXISTS 表示应用已经启动了
// return
//		zero: success
//		non-zero: error code
DWORD EiAppStart(
	HWND hInforWindow	// 用于接收本系统消息的Windows窗口句柄 
						// windows handle to receive eink message .
	);


// 获得Eink的基本信息
// get eink basic information .
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiGetSystemInfo(PEI_SYSTEM_INFO pstSystemInfo);


// 获得APP的屏幕设定信息
// get APP context indicated current screen settings
// 返回： 返回零，表示成功；返回非零，表示错误码；
DWORD EiGetAppContext(PEI_APP_CONTEXT pstAppContext);

// 获得绘制缓存
// Get Drawing buffer .
// 通过调用本函数获得一个绘制缓存,一个应用实例可以申请两块绘制缓存
// to get a draw-buffer that it represent pixels on the display area of Eink cover. There are two draw-buffers for each APP 
// 当APP调用EiSetScreenOrient后，需要释放之前申请的绘制缓冲，然后通过调用本函数重新获取缓存
// If an APP call EiSetScreenOrient to change current display orientation, these buffers must be release and call this function to retrieve them.
// 返回：返回空指针NULL，表示失败，多为资源耗尽；返回非NULLL，表示成功，返回值即是新建的缓存
// return
//		NULL : fail , mostly is run out of resource .
//		none NULL: success.
EI_BUFFER* EiGetDrawBuffer(
	BOOL bInit,		// 是否将缓存清零
					// whether clear buffer content to zero .
	BOOL bCopy		// 是否将当前Eink屏内容复制到缓存
					// will copy current content to buffer .
	);

// 释放绘制缓存
// release drawing content .
// 当不在使用时，需要调用本函数释放它
// please call this function to release it .
void EiReleaseDrawBuffer( 
	EI_BUFFER* pstBuffer		// 指向绘制缓存 // point to drawing buffer .
	);

// 绘制内容到Eink屏
// Draw an image to display-area in Eink-panel.
// 将缓存中的内容绘制到Eink屏幕上，调用此函数后，传入的绘制缓存会被自动释放，应用不能继续使用这块缓存
// draw content in DRAWING buffer to eink display , after call this function ,DRAWING buffer will be release , and app should not use this buffer any more .
// 参数x,y,w,h表示需要更新的区域；无论是全屏绘制还是局部绘制，绘制缓存中的内容始终对应的是整个屏幕
// x,y,w,h indicate region need to be redraw , no matter full screen or partial screen redraw , DRAWING content will be for whole screen .
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiDraw(
	EI_RECT* pstArea,	// indicates the area to draw
	EI_BUFFER* pstBuffer		// image buffer
	);

// 清除Eink屏幕
// Clean up E-ink screen
// return:
//		na
void EiCleanupScreen(
	unsigned char ucBackgroundColor		// the background color of E-ink screen after cleaning up
	);

// 促使系统产生一条WM_EI_DRAW消息给当前APP
// indicate system to generate an WM_EI_DRAW message to current app .
// 编写APP时，可以将所有的绘制都放在处理WM_EI_DRAW消息的过程中，如果APP期待主动进行一次绘制操作，只需要调用本函数，促使系统发送一条WM_EI_DRAW
// when you write an app , please place all drawing action in WM_EI_DRAW process function , if App want an redraw , can call this function , and system will generate an WM_EI_DRAW
// 消息给APP，从而就能够将绘制操作集中在一个地方进行（WM_EI_DRAW处理过程）
// message to app , so we should centralize all drawing action in one place.
void EiInvalidPanel(
	EI_RECT* pstArea	// indicates the area to update; set NULL for full panel
	);

// 设置Handwriting模式
// Set handwriting mode .
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHandWritingMode(
	DWORD eMode // Set the handwriting-mode, please refer to GIHW_HANDWRITING
	);

// 设置屏幕显示方向
// set the screen orientation for current app
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetScreenOrient(
	DWORD eOrientation // refer to GIR_NONE
	);

// 从物理坐标系转换到显示坐标系
// Convert a point from physical-coordinate to display-coordinate
void EiPhysicalToDisplay(
	IN EI_POINT* pstPointP,
	OUT EI_POINT* pstPointD
	);

// 从显示坐标系转换到物理坐标系
// Convert a point from display-coordinate to physical-coordinate
void EiDisplayToPhysical(
	IN EI_POINT* pstPointD,
	OUT EI_POINT* pstPointP
);

// Set waveform mode for epd display, e.g. GI_WAVEFORM_DU2, GI_WAVEFORM_GC16, GI_WAVEFORM_GL16
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetWaveformMode(
	DWORD dwMode
	);

// Enable / disable direct handwriting region, which means handwriting is handled by tcon.
// To disable it, set both width and height = 0.
// Direct handwriting can only be used in GIHW_OWNER_DRAW mode.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHandwritingRect(
	EI_RECT dRect
);

// Check DisplayArea engine status.
//
// 返回： 返回TRUE，表示空闲。返回FALSE，表示绘制被占用;
// return
//		TRUE: ready
//		FALSE: not ready
BOOL EiCheckDpyReady();

// Set PartialUpdate feature
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetPartialUpdate(
	BOOL enable
);


// Get scenario
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiGetScenario(DWORD& rdwCurrentMode);

// OOBE complete
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiOOBEComplete();

// Set Homebar status
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetHomebarStatus(
	ULONG lulStatus  //GI_HOMEBAR_HIDE/ GI_HOMEBAR_SHOW / GI_HOMEBAR_EXPAND / GI_HOMEBAR_COLLAPSE
);

// Set direct handwriting setting, e.g. line width, eraser
// Value 1~5
// Return value
//		zero: success
//		non-zero: error code
DWORD EiSetHandwritingSetting(BYTE lineWidth);

// 获取当前机器形态
// Get tablet mode
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwCurrentMode : GIR_MODE_LAPTOP/GIR_MODE_TENT/GIR_MODE_TABLET
DWORD EiGetTabletMode(DWORD& rdwCurrentMode);


// 设置键盘样式
// Set keyboard style
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// 该调用直接返回不等待，当收到 WM_EI_CHANGE_KEYBOARD_STYLE_COMPLETE 消息时，表示切换完成
// return
//		zero: success
//		non-zero: error code
DWORD EiSetKeyboardStyle(EI_KEYBOARD_STYLE ndStyle);

// 设置C面手或笔区域
// Set TP Area
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiSetTpArea(SET_TP_AREA ndTpArea);

// 设置隐私协议开关状态
// Set the privacy protocol switch status.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwStatus : 0 off,1 on
DWORD EiSetPrivacyStatus(DWORD& rdwStatus);

// 获取隐私协议开关状态
// Get the privacy protocol switch status.
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
//rdwStatus : 0 off,1 on
DWORD EiGetPrivacyStatus(DWORD& rdwStatus);

// 判断自己是否是前台窗口
// Is it a foreground window
//
// 返回： 返回零，表示成功；返回非零，表示错误码；
// return
//		zero: success
//		non-zero: error code
DWORD EiIsForeground(bool& rbIsForeground);

// 停止服务		// stop service .
// 当一个应用实例需要退出时，调用这个函数
// when an app exit , please call this function.
void EiEnd(void);

// 获取用户显示语言
// 返回： 返回非零，表示成功；返回零，表示失败；
ULONG EiGetUserLagID(
	DWORD& rdwLagID
);



#ifdef __cplusplus
}
#endif




#pragma pack()



#endif//_EINKITEAPI_H_
