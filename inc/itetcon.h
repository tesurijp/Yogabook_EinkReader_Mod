/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ITETCON_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ITETCON_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
//#ifdef ITETCON_EXPORTS
//#define ITETCON_API __declspec(dllexport)
//#else
//#define ITETCON_API __declspec(dllimport)
//#endif
#pragma once
#define ITETCON_API

// scenario
#define TCON_SCENARIO_NORMAL    0
#define TCON_SCENARIO_KBD       1
#define TCON_SCENARIO_WACOM_PASS_THROUGH      3

// waveform mode
#define TCON_WAVEFORM_INIT      0
#define TCON_WAVEFORM_DU2       1
#define TCON_WAVEFORM_GC16      2
#define TCON_WAVEFORM_GL16      3

#define RW_PAGE_SIZE (32*1024)
//#define __ENABLE_DLL_MODE__ 

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

typedef struct _lenovo_pipeline_info
{
	WORD usX;
	WORD usY;
	WORD usW;
	WORD usH;
} lenovo_pipeline_info;

typedef struct
{
	DWORD ulTimeStamp;
	WORD  usCur16EngStatus;
	WORD  usReserved;
	lenovo_pipeline_info GPLineInfo[16];
} TDpyEngStatus;

#define IT8951_USB_OP_UPDATE		 0xAA
#define IT8951_USB_OP_MOD_SW         0xAB

typedef struct {
	unsigned char ucModuleIndex;
	unsigned char ucEnableSW;

}T_MOD_SW_INFO;

typedef struct {
	BYTE reportID; //0x11
	BYTE parameterID; // Sub
	BYTE parameter[4]; // Parameter
}TOSKSetFeature;

typedef struct {
	unsigned short usX;
	unsigned short usY;
	unsigned short usW;
	unsigned short usH;
	unsigned char  ucID;
	unsigned char  ucFunFlag;

}T_TP_SET_AREA;


#pragma pack()

extern "C"
{

	// Open tcon device handle, must be called before access device
	//
	// Return value
	// If the function succeeds, the return value is an open handle to the tcon device.
	// If the function fails, the return value is INVALID_HANDLE_VALUE
	ITETCON_API HANDLE TconOpenDevice();



	// Open tcon device handle, must be called before access device
	//
	// Return value
	// If the function succeeds, the return value is an open handle to the tcon device.
	// If the function fails, the return value is INVALID_HANDLE_VALUE
	ITETCON_API HANDLE TconOpenHidDevice();

	// Close tcon device handle
	//
	// Return value
	// If the function succeeds, the return value is nonzero.
	ITETCON_API BOOL TconCloseHandle(HANDLE hDevice);

	// Get device info, e.g. version, panel size
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconGetSystemInfo(HANDLE hDevice, TRSP_SYSTEM_INFO_DATA* pSystemInfo);

	// Set PartialUpdate feature
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetPartialUpdate(HANDLE hDevice, BOOL enable);

	// Load image area to image buffer in tcon.
	// There can be multiple image buffer.
	// It can be partial or full region update.
	// Data transfer will be faster when both x and width are integral multiple of 4.
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconLoadImage(HANDLE hDevice, DWORD imageBuffer, DWORD x, DWORD y, DWORD width, DWORD height, BYTE* pSrcImg);

	// Load image area to last image buffer in tcon.
	// There can be multiple image buffer.
	// It can be partial or full region update.
	// Data transfer will be faster when both x and width are integral multiple of 4.
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	//	Flag: 
	//  0: erase;
	//	1: store image to flash;
	ITETCON_API BOOL TconLoadLastImage(HANDLE hDevice, BYTE Flag, BYTE* Status = NULL);

	//set last display image to display
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconDisplayLastImage(HANDLE hDevice);

	// Display image buffer area on epd. It can be partial or full region update.
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconDisplayArea(HANDLE hDevice, DWORD imageBuffer, DWORD x, DWORD y, DWORD width, DWORD height, DWORD waitReady);

	// Wait until DisplayArea operation complete
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconWaitDpyReady(HANDLE hDevice);

	// Check DisplayArea engine status.
	// bit[0, 15] is the busy bit of engine[0, 15]
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconCheckDpyReady(HANDLE hDevice, DWORD *status);

	// Get 16 pipeline engine status
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconGetDpyEngStatus(HANDLE hDevice, TDpyEngStatus* pstDpyEngInfo);

	//ITETCON_API BOOL TconReadMemAPI(HANDLE hDevice, DWORD ulMemAddr, WORD usLength, BYTE* RecvBuf);

	// Get tcon scenario
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconGetScenario(HANDLE hDevice, BYTE *scenario, BYTE* status);

	// Set tcon scenario
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetScenario(HANDLE hDevice, BYTE scenario, BYTE* status);

	// Set waveform mode for epd display, e.g. DU2, GC16, GL16
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetWaveformMode(HANDLE hDevice, BYTE mode);

	// Enable / disable direct handwriting region, which means handwriting is handled by tcon.
	// To disable it, set both width and height = 0.
	// Direct handwriting can only be used in TCON_SCENARIO_NORMAL mode.
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconEnableDirectHandwriting(HANDLE hDevice, WORD x, WORD y, WORD width, WORD height);

	// Set direct handwriting setting, e.g. line width, eraser
	// The setting is set to default after call TconEnableDirectHandwriting()
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconDirectHandwritingSetting(HANDLE hDevice, BYTE lineWidth);

	// 
	// set keyboard style
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetKeyBoardStyle(HANDLE hDevice, BYTE* data, DWORD length);

	// 
	// set ITE status
	//  1 :when windows system enter sleep,should set 1,tell ITE8951 which can sleep completed
	//  0 : when wake up ,0 should be set
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetITE8951Status(HANDLE hDevice, BYTE status);
	// 
	//  mute indicator  controlling
	//  1 :indicator on  0: indicator off
	//  
	//
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetMuteIndicatorCtrl(HANDLE hDevice, BYTE status);

	// 
	//  FnLk-Indicator controlling
	//1 :indicator on  0: indicator off
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetFnLkIndicatorCtrl(HANDLE hDevice, BYTE status);
	// 
	//  SetTPAreaAPI
	//WORD usTPX, WORD usTPY, WORD usTPW, WORD usTPH start point and width height

	//BYTE ucID
	//Area #	Xleft	Ytop	Width	Height	Flags	Result	
	//0	0	0	7680	4320	0x0000	0x0000
	//0	0	0	7680	300	0x0001	0x0001
	//ㄠ	0	300	7680	3296	0x0001	0x0001
	//ㄡ	3308	3840	404	160	0x0001	0x0001
	//ㄢ	3208	4000	604	160	0x0001	0x0001
	//ㄣ	3108	4160	804	160	0x0001	0x0001

	//BYTE ucFlag 
	//0x00 每 no report
	//0x01 每 Touch only
	//0x02 每 Pen only
	//0x03 每 Touch + Pen
	// Return value
	// If the operation completes successfully, the return value is nonzero.
	ITETCON_API BOOL TconSetTPAreaAPI(HANDLE hDevice, WORD usTPX, WORD usTPY, WORD usTPW, WORD usTPH, BYTE ucFlag, BYTE ucID);

	//struct TconBoolValues {
	//	// To set hand draw in HW draw. zhuhl5
	//	// 0 means hand strokes OFF
	//	// 1 means hand strokes ON(default)
	//	BOOL EnableHandInHWDraw{ TRUE };
	//	// To set X axis mirror(?). zhuhl5
	//	// 0 means not do transform(default)
	//	// 1 means do transform
	//	BOOL EnableAxisTransformInPenMouse{ FALSE };
	//};

	/*
		BYTE IsSet{ 1 };					// 1:Set, 0:Get
		// To set hand draw in HW draw. zhuhl5
		// 0 means hand strokes OFF
		// 1 means hand strokes ON(default)
		BOOL EnableHandInHWDraw{ TRUE };
		// To set X axis mirror(?). zhuhl5
		// 0 means not do transform(default)
		// 1 means do transform
		BOOL EnableAxisTransformInPenMouse{ FALSE };
		// Points to SDK through CUSTOM HID, priority higher then SetTpArea
		// 0: no, 1:hand, 2:pen, 3; pen and hand.
		BYTE ucByPassFlag{ 3 };
		// Points to OS through HID(aka PEN - MOUSE), priority higher then SetTpArea
		// 0: no, 1:hand, 2:pen, 3; pen and hand. 
		BYTE ucPenMouseFlag{ 3 };
		// Hardware draw line sensative to pressure
		// 0: No pressure drawline , 1: with pressure drawline. 
		BOOL EnablePressureInHWDraw{ 1 };
		// Upper button drawline
		// 0: function disable, 1: function enable
		BOOL EnableUpperButtonDrawInHWDraw{ TRUE };
	*/
	typedef struct TconBoolValues_V15
	{
		BYTE IsSet{ 1 };					// 1:Set, 0:Get
		// To set hand draw in HW draw. zhuhl5
		// 0 means hand strokes OFF
		// 1 means hand strokes ON(default)
		BOOL EnableHandInHWDraw{ TRUE };
		// To set X axis mirror(?). zhuhl5
		// 0 means not do transform(default)
		// 1 means do transform
		BOOL EnableAxisTransformInPenMouse{ FALSE };
		// Points to SDK through CUSTOM HID, priority higher then SetTpArea
		// 0: no, 1:hand, 2:pen, 3; pen and hand.
		BYTE ucByPassFlag{ 3 };
		// Points to OS through HID(aka PEN - MOUSE), priority higher then SetTpArea
		// 0: no, 1:hand, 2:pen, 3; pen and hand. 
		BYTE ucPenMouseFlag{ 3 };
		// Hardware draw line sensative to pressure
		// 0: No pressure drawline , 1: with pressure drawline. 
		BOOL EnablePressureInHWDraw{ 1 };
		// Upper button drawline
		// 0: function disable, 1: function enable
		BOOL EnableUpperButtonDrawInHWDraw{ TRUE };
	}TconBoolValues;
	ITETCON_API DWORD TconSetBoolValues(HANDLE hDevice, TconBoolValues& boolValues);

	// TconGetFWVersion zhuhl5
	ITETCON_API BOOL TconGetMemory(HANDLE hDevice, DWORD addr, _Out_ BYTE buf[], _In_ DWORD size);
	ITETCON_API BOOL TconGetFWVersion(HANDLE hDevice, _Out_ BYTE ver[], _In_ DWORD size);
};
