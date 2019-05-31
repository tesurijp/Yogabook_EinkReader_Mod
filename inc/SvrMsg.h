#pragma once
#include "EiMsgQueue.h"
#include "EinkIteAPI.h"

// 这是SDK的Sevice程序的主要执行体与App的连接DLL之间的通讯数据和定义

//////////////////////////////////////////////////////////////////////////
// APP发送给Service的消息定义
#define EMHOSTID_REG_APP 1
// MsgBuf:struct REG_APP_INFO

//Homepage运行指定APP
#define EMHOSTID_RUN_APP 2
// MsgBuf:EI_MSG_RUN_APP
typedef struct  _EI_MSG_RUN_APP {
	wchar_t FilePath[MAX_PATH];	//可执行文件路径
	wchar_t CommandLine[MAX_PATH];	//需要带的参数
	int SessionID;		//用户ID，0表示system用户

}EI_MSG_RUN_APP, *PEI_MSG_RUN_APP;

//绘制
#define EMHOSTID_DRAW 3
// MsgBuf: REG_APP_DRAW
typedef struct  _EI_MSG_APP_DRAW {
	EI_RECT Area;
	ULONG BufferOffset;		// 绘制缓存的偏移量

}EI_MSG_APP_DRAW, *PEI_MSG_APP_DRAW;

//APP设置自身屏幕显示方向
#define EMHOSTID_SET_DISPLAY_ORIENTATION 4
// MsgBuf:DWORD

//APP设置Eink模式，0,1,2
#define EMHOSTID_SET_SCENARIO 5
// MsgBuf:DWORD

// APP请求重绘
#define EMHOSTID_INVALID_PANEL 6
// MsgBuf:EI_RECT

// APP注销自己
#define EMHOSTID_UNREG_APP 7
// MsgBuf:EI_RECT

// 设置FW模式
#define EMHOSTID_SET_FW_MODE 8
// MsgBuf:DWORD

//APP获取当前EINK设备信息
#define EMHOSTID_GET_EINK_INFO 10
// MsgBuf: none

//APP获取当前显示设定
#define EMHOSTID_GET_APP_CONTEXT 11
// MsgBuf: none

//Switcher绘制
#define EMHOSTID_SWITCHER_DRAW 15
// MsgBuf: REG_APP_DRAW

//设置Switcher的位置
#define EMHOSTID_SWITCHER_LOCATION 16
// MsgBuf: EI_RECT

//屏幕方向发生变化通知
#define EMHOSTID_ORIENTATION_CHANGED 17
// MsgBuf: DWORD

//机器形态发生变化通知
#define EMHOSTID_LAPTOP_MODE_CHANGED 18
// MsgBuf: ULONG


//系统电源变化状态变化通知
#define EMHOSTID_POWER_CHANGE	19
// MsgBuf: DWORD 0表示进入Connected standby 1表示Connected standby返回 2表示系统关机


//清理屏幕
#define EMHOSTID_CLEANUP_SCREEN	20
// MsgBuf:  unsigned char，background color

//设置关机封面图片的路径
#define EMHOSTID_SET_SHUTDOWN_COVER 21
// MsgBuf: wchat_t*

//设置键盘图片的路径
#define EMHOSTID_SET_KEYBOARD_PATH 22
// MsgBuf: wchat_t*

//设置显示状态要发生变化
#define EMHOSTID_SET_SHOW_STATUS 23
// MsgBuf: ULONG 0正常显示APP 1显示封面 2显示键盘A 3显示键盘B 4不响应APP绘制请求
#define SHOW_STATUS_NORMAL 0
#define SHOW_STATUS_COVER 1
#define SHOW_STATUS_KEYBOARD_A 2	//双击可唤醒
#define SHOW_STATUS_KEYBOARD_B 3	//键盘不可用
#define SHOW_STATUS_NOAPP 4

//守护线程通知服务启动指定应用
#define EMHOSTID_PROTECT_RUN_APP 24
// MsgBuf: ULONG 0Homebar 1Settings
#define PROTECT_RUN_APP_HOMEBAR 0
#define PROTECT_RUN_APP_SETTINGS 1
#define PROTECT_RUN_APP_KEYBOARD 2
#define PROTECT_RUN_APP_PROTECT 3

//获取ITE绘制是否ready
#define EMHOSTID_GET_DRAW_READY 25
// MsgBuf: none

//获取ITE绘制是否ready
#define EMHOSTID_SET_HANDWRITING_RECT 26
// MsgBuf: EI_RECT

//设置FW Partial开关
#define EMHOSTID_SET_PARTIAL_ENABLE 27
// MsgBuf: BOOL

//设置FW Partial开关
#define EMHOSTID_GET_SCENARIO_MODE 28
// MsgBuf: none

//设置键盘按下音开关
#define EMHOSTID_SET_KEYBOARD_DOWN 29
// MsgBuf: bool

//设置键盘抬起音开关
#define EMHOSTID_SET_KEYBOARD_UP 30
// MsgBuf: bool

//应用设置homebar状态
#define EMHOSTID_SET_HOMEBAR_STATUS 31
// MsgBuf: ULONG

//OOBE结束
#define EMHOSTID_OOBE_COMPLETE 32
// MsgBuf: none

//OOBE开始
#define EMHOSTID_OOBE_START 33
// MsgBuf: none

// 获取系统键盘音音量大小
#define EMHOSTID_SET_KEYBOARD_VOLUME 34
// MsgBuf: none

// 设置系统键盘音音量大小
#define EMHOSTID_GET_KEYBOARD_VOLUME 35
// MsgBuf: LONG (0-100)


// 设置FW笔写线宽
#define EMHOSTID_SET_HANDWRITING_LINE_WIDTH 36
// MsgBuf: BYTE

// B/C双击事件
#define EMHOSTID_BC_DBOULE_CLICK 37
// MsgBuf: bool true for C cover

// 获取当前机器形态
#define EMHOSTID_GET_TABLET_MODE 38
// MsgBuf: none

// homebar通知服务自己状态发生变化
#define EMHOSTID_HOMEBAR_CHANGE 39
// MsgBuf: ULONG

// 通知服务BCover
#define EMHOSTID_SET_BCOVER_PATH 40
// MsgBuf: wchat_t*

// 通知服务B面进入或退出双击模式
#define EMHOSTID_B_ENTER_DBTAP 41
// MsgBuf: bool true Enter

//设置Switcher的可视区域
#define EMHOSTID_SWITCHER_SHOW_AREA 42
// MsgBuf: EI_RECT

//设置键盘样式文件
#define EMHOSTID_SET_KEYBOARD_STYLE 43
// MsgBuf: EI_KEYBOARD_STYLE

//设置键盘灵敏度
#define EMHOSTID_SET_KEYBOARD_SENSITIVITY 44
// MsgBuf: LONG

//B面灭屏
#define EMHOSTID_SET_CLOSE_B_COVER 45
// MsgBuf: BOOL

//播放键盘声音，为了试音
#define EMHOSTID_PLAY_KEYBOARD_SOUND 46
// MsgBuf: none

//把系统切换到笔记本或平板模式
#define EMHOSTID_CHANGE_TABLET_MODE 47
// MsgBuf: bool //为真表示切换到平板，否则切换到笔记本

//初始化8951
#define EMHOSTID_INIT_8951 48
// MsgBuf: none

//重新加载音频设备
#define EMHOSTID_INIT_VIDEO 49
// MsgBuf: none


//Set TP Area
#define EMHOSTID_SET_TP_AREA 50
// MsgBuf: SET_TP_AREA

//Check App Status
#define EMHOSTID_CHECK_APP_STATUS 51
// MsgBuf: none

//开启/关闭knock knock功能
#define EMHOSTID_OPEN_KNOCK_KNOCK 52
// MsgBuf: bool //为真表示打开

//通知当前应用，重新设置 tp area
#define EMHOSTID_RESET_TP_AREA 53
// MsgBuf: none

// 设置隐私协议开关状态
#define EMHOSTID_SET_PRIVACY_STATUS 54
// MsgBuf: DWORD

// 获取隐私协议开关状态
#define EMHOSTID_GET_PRIVACY_STATUS 55
// MsgBuf: DWORD

//8951掉了又重连上了，我们也重新open一下设备
#define EMHOSTID_REOPEN_8951 56
// MsgBuf: none

//运行指定指定到当前用户下
#define EMHOSTID_RUN_EXE_FOR_CURRENT_USER 57
// MsgBuf:EI_MSG_RUN_APP

//设置C面计算出来的物理方向
//规则如下，在笔记本模式，C面方向和B面保持一致。在tent/table使用C面自己物理方向
#define EMHOSTID_SET_CCOVER_ORI 58
// MsgBuf:ULONG

// Touch 从HID获取转发到SeviceCenter
#define EMHOSTID_TOUCH_INPUT 59
// MsgBuf:EI_TOUCHINPUT_POINT

// 询问是否需要显示oobe,同一用户只显示一次
#define EMHOSTID_GET_IS_SHOW_OOBE 60
// MsgBuf:

// 获取用户显示语言ID
#define EMHOSTID_GET_USER_LAGID 61

// 8951 connect or remove
#define EMHOSTID_8951_CONNECT_OR_REMOVE 62
// MsgBuf:bool true for connect

// 获取自己是否是前台窗口
#define EMHOSTID_GET_IS_FOREGROUND_WINDOW 63

// 询问8951是否处于sleep状态
#define EMHOSTID_GET_8951_IS_SLEEP 64
// MsgBuf:

// 通知homebar切换应用
#define EMHOSTID_CHANGE_APP 65
// MsgBuf: DWORD

// 关闭B面设备句柄
#define EMHOSTID_CLOSE_B_COVER_HANDLE 66
// MsgBuf: none

// 5182 connect or remove
#define EMHOSTID_5182_CONNECT_OR_REMOVE 67
// MsgBuf:bool true for connect

// 获取当前用户磁盘列表
#define EMHOSTID_GET_USER_DIST_LIST 68
// MsgBuf:none

// MsgBuf:
//////////////////////////////////////////////////////////////////////////
// Sevice发送给App的消息定义
#define EMAPPID_RESULT  1000
// MsgBuf: none

// 输入消息
// touch input message
#define EMAPPID_FINGER_MOVE  1001
// MsgBuf: EI_TOUCHINPUT

// 通知APP重绘
#define EMAPPID_RE_DRAW  1002
// MsgBuf: EI_RECT

// 活跃应用切换
#define EMAPPID_ACTIVATE  1003
// MsgBuf: ULONG 非零表示本应用被显示在前台。为零表示本应用被转到后台
// MsgBuf: ULONG zero indicate this app is show at front , otherwise this app is switch to background .

// 通知APP屏幕显示方向变化
#define EMAPPID_ORIENTATION_CHANGED 1004
// MsgBuf: ULONG DMDO_DEFAULT/DMDO_90/DMDO_180/DMDO_270

// 通知APP机器形态方向变化
#define EMAPPID_LATTOP_CHANGED 1005
// MsgBuf: ULONG GIR_MODE_LAPTOP/GIR_MODE_TENT/GIR_MODE_TABLET

//服务通知应用事件
#define EMAPPID_EVENT 1006
// MsgBuf: ULONG 

// 通知APP homebar状态发生变化
#define EMAPPID_HOMEBAR_CHANGED 1007
// MsgBuf: ULONG

// 通知APP 键盘切换完成
#define EMAPPID_KEYBOARD_CHANGED 1008
// MsgBuf: BOOL

// 通知APP 重新设置TP area
#define EMAPPID_RESET_TP_AREA 1009
// MsgBuf: NA

// 隐私开关状态发生变化
#define EMAPPID_PRIVACY_STATUS_CHANGED 1010
// MsgBuf: NA


#pragma pack(4)

//内存映射文件大小
#define EAC_FILE_SIZE 1024*1024*10
//给消息分配3M空间
#define EAC_MSG_BUFFER_SIZE 1024*1024*3

//注册APP时需要传递的数据
#define APP_NAME_MAX 100	//事件名称最大长度
#define APP_FILE_SIZE_MAX	1024*1024*10 //内存映射文件大小
typedef struct _REG_APP_INFO
{
	wchar_t mszAppMutex[APP_NAME_MAX];
	wchar_t mszAppSemaphore[APP_NAME_MAX];
	wchar_t mszAppFilePath[MAX_PATH];
}REG_APP_INFO,*PREG_APP_INFO;

// 服务调用项
class CEiSvrMsgItem {
public:
	union {
		struct {
			ULONG MsgId;
			ULONG AppId;
			HANDLE WaitHandle;
			CEiSvrMsgItem* OrgItem;
			ULONG Result;	// 只有调用SendMessage时才有意义
			ULONGLONG TickSent;	// 消息发送时的TickCount
			ULONG BufSize;
			char  MsgBuf[1];
		}Item;
		char All[2048];
	}Data;
	CEiSvrMsgItem() {
		Data.Item.WaitHandle = NULL;
		Data.Item.OrgItem = NULL;
		Data.Item.BufSize = 0;
	}
	~CEiSvrMsgItem() {}

	CEiSvrMsgItem& operator =(const CEiSvrMsgItem& src) {
		RtlCopyMemory(Data.All, src.Data.All, sizeof(Data.Item) + src.Data.Item.BufSize - 1);
		return *this;
	}

	bool IsTypeOf(const CEiSvrMsgItem& nrRefTo) {
		return Data.Item.MsgId == nrRefTo.Data.Item.MsgId;
	}

	void Recall() {
		Data.Item.MsgId = 0;
	}

	void SaveTickCount(void) {
		Data.Item.TickSent = GetTickCount64();
	}

	ULONGLONG GetElapsedTick(void)	{
		return GetTickCount64() - Data.Item.TickSent;
	}

};

#define SVR_MSGITEM_BUF_MAX (2048-sizeof(CEiSvrMsgItem::Data.Item))

#pragma pack()




#pragma once
