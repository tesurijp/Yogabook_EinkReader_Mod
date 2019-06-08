/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#ifndef _XUI_H__
#define _XUI_H__
#include "cmmBaseObj.h"
#include "EResult.h"
#include "CfgIface.h"
#include "d2d1.h"
#include "EinkIteAPI.h"

// XUI的入口函数定义在文件尾部，EinkUiStart EinkuiGetSystem


// 默认定义
#ifndef IN
#define IN
#endif//IN
#ifndef OUT
#define OUT
#endif//OUT
#define ES_ELEMENT_RES_TEXT_MAX_LEN 200		//元素配置文件名称最大长度

#define ES_BIT_SET(_Off) (1<<(_Off-1))

// 默认无效ID
#define EID_INVALID 0


//////////////////////////////////////////////////////////////////////////
// 前置声明，无需关注
__interface IEinkuiBitmap;	// 位图，提供图像的装入、访问、改变以及复制功能；并且能够直接提供给渲染过程使用；
__interface IEinkuiBrush;	// 画刷，封装了Brush和Pen的概念
__interface IEinkuiMessage;	// 消息，生成、设置、访问和释放
__interface IEinkuiPaintBoard;	// 绘图板
__interface IEinkuiSystem;	// XUI基本接口上
__interface IXelManager;	// 对象管理器
__interface IXsElement;		// 元素
__interface ILsModule;		// 模块
__interface ICfKey;			// profile的键，包含文件CfgIface.h
__interface IConfigFile;	// profile的接口类
__interface IEinkuiIterator;	// 元素迭代器
__interface IXsWidget;		// 微件接口，微件是XUI系统中的一个概念，是同一进程中区分对扩展功能部件的一种界定方式，一个扩展功能部件可能在系统中运行多个实例，每个实例都是一个微件；具体内容见定义处
__interface IElementFactory; // 所有工厂类都必须提供本接口，一个工厂类可以实例化多种个类型的对象
__interface IXelAllocator;
__interface IEinkuiPaintResouce;

__interface IWICImagingFactory;
__interface ID3D10Device1;
__interface IDWriteFactory;

// 前置声明结束
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 

typedef ERESULT (__stdcall IBaseObject::*PXUI_CALLBACK)(ULONG nuFlag,LPVOID npContext);
typedef ERESULT (__stdcall IBaseObject::*PWINMSG_CALLBACK)(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT& Result);
typedef ERESULT(__stdcall *PXUI_CUSTOM_DRAW_CALLBACK)(unsigned char* npPixels, ULONG nuWidth, ULONG nuHeight,bool nbRefresh);//nbRefresh设置，则必须提交数据
typedef ERESULT(__stdcall *PXUI_EINKUPDATING_CALLBACK)(ULONGLONG nxNumber,LPVOID npContext);	// nuNumber是更新序号，每次更新加一，达到最大值后回到零


// {EA3270AA-3D8B-4246-AE7F-0D65F3D7BD60}
DEFINE_GUID(EGUID_EMGR_DROP_TEST, 
	0xea3270aa, 0x3d8b, 0x4246, 0xae, 0x7f, 0xd, 0x65, 0xf3, 0xd7, 0xbd, 0x60);
//IsEqualGUID





#pragma pack(4)

//////////////////////////////////////////////////////////////////////////
// 下面的宏用来设定消息编码,其中TYPE是消息产生者或者是接收者(不多于65535）
// _MJ_NUM是消息的主功能号（不大于127），_MN_NUM是消息的子功能号(511）；主功能号在64以上默认用于定义元素设定请求
// 消息名称的命名约定是，以‘EMSG_’开头，接着是类型描述，后面是意义，如：EMSG_ELEMENT_MOUSE_ON，鼠标进入一个Element后，Element发出的消息
//////////////////////////////////////////////////////////////////////////
#define EMSG_DEFINE(_TYPE,_MJ_NUM,_MN_NUM) (((_TYPE&0xFFFF)<<16)|((_MJ_NUM&0x7F)<<9)|(_MN_NUM&0x1FF))
// TYPE 主要用于区分XUI系统内置的各种控件，其中1～1023供内置控件按使用，扩展控件使用1024以上的数字
#define LMSG_TP_ITERATOR	1	// 迭代器，这个类别的消息最为特殊，仅供迭代器内部使用
#define LMSG_TP_ELEMGR	2	// 对象管理器
#define LMSG_TP_SYSTEM	3	// XUI系统
#define LMSG_TP_BITMAP	4	// 位图管理器
#define LMSG_TP_RENDER	5	// 渲染器
#define LMSG_TP_WIN_INPUT	6	// Windows系统直接提供的输入消息

#define LMSG_TP_MOUSE		10	// 鼠标
#define LMSG_TP_KEYBOARD	11	// 键盘
#define LMSG_TP_TOUCH_SCREEN 12 // 触摸屏
#define LMSG_TP_TOUCH_PAD	13 // 触摸板
#define LMSG_TP_INPUT		18	// 高级输入消息，通常是由初级输入行为翻译而来

#define LMSG_TP_OTHERS	19	// 不确定发送者

#define LMSG_ELE_BASE	20	// 可视化对象开始编码
#define LMSG_TP_ELEMENT		LMSG_ELE_BASE+1

#define LMSG_TP_BUTTON		LMSG_ELE_BASE+11
#define LMSG_TP_LIST			LMSG_ELE_BASE+12
#define LMSG_TP_COMBO			LMSG_ELE_BASE+13
#define LMSG_TP_MENU			LMSG_ELE_BASE+14
#define LMSG_TP_EDIT			LMSG_ELE_BASE+15
#define LMSG_TP_SCROLLBAR		LMSG_ELE_BASE+16
#define LMSG_TP_PROGRESS		LMSG_ELE_BASE+17
#define LMSG_TP_SLIDERBTN		LMSG_ELE_BASE+18
#define LMSG_TP_ANIMATOR		LMSG_ELE_BASE+19
#define LMSG_TP_PICTUREFRAME	LMSG_ELE_BASE+20
#define LMSG_TP_STATICTEXT	LMSG_ELE_BASE+21
#define LMSG_TP_SLIDEREBAR	LMSG_ELE_BASE+22
#define LMSG_TP_LABEL			LMSG_ELE_BASE+23

#define LMSG_TP_MENUITEMN		LMSG_ELE_BASE+24

#define LMSG_TP_POPUPMENU		LMSG_ELE_BASE+25

#define LMSG_TP_MENUBUTTON	LMSG_ELE_BASE+26

#define LMSG_TP_MENUBAR		LMSG_ELE_BASE+27

#define LMSG_TP_IMAGEBUTTON	LMSG_ELE_BASE+28

#define LMSG_TP_TOOLBAR		LMSG_ELE_BASE+29

#define LMSG_TP_WHIRLANGLE	LMSG_ELE_BASE+30

#define LMSG_TP_SPINBUTTON	LMSG_ELE_BASE+31

#define LMSG_TP_SELECT		LMSG_ELE_BASE+32

#define LMSG_TP_RADIO_BUTTON_GROUP LMSG_ELE_BASE+33

#define LMSG_TP_BI_SLIDERBAR LMSG_ELE_BASE+34

#define LMSG_TP_TIMESPINBUTTON LMSG_ELE_BASE+35



#define LMSG_TP_EXTENDED	1024	// 扩展的消息类别，开发扩展类时，可以随意使用1024 - 65535之间的任意数字，实际处理扩展类消息时，要增加对消息发送者的判断
									// 为了避免出现冲突，一个简单的办法是，使用GUIID生成工具，将新生成的GUID如：// {09BF0B66-916E-4506-B41C-30D0AB2ECC8D}的最后4位用作这个扩展ID，CC8D = 52365，只需去除小于1024的值


// 通过消息ID取类型
#define LMSG_GET_TYPE(_X) ((_X>>16)&0xFFFF)
// 通过消息ID取Major Number
#define LMSG_GET_MJNUM(_X) ((_X>>9)&0x7F)
// 通过消息ID提取Minor number
#define LMSG_GET_MNNUM(_X) (_X&0x1FF)

// 无效的消息ID
#define EMSG_MSGID_INVALID MAXUINT32






//////////////////////////////////////////////////////////////////////////
// 下面定义的是系统可能发送或接收的消息
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 系统测试消息，仅需要应答，将输入数据取反输出，返回ERESULT_SUCCESS
#define EMSG_TEST EMSG_DEFINE(LMSG_TP_SYSTEM,1,1)
// input bool
// output bool

// 工作线程异常，当一个工作线程发生异常时，该线程间会被关闭，并且给线程的拥有者（Widget）的HomePage发送次消息；同时，本消息也会被发送给SystemWidget的HomePage
#define EMSG_WORK_THREAD_EXCEPTION EMSG_DEFINE(LMSG_TP_SYSTEM,2,1)
// input
//struct _STEMS_WORKTHREAD_EXCEPTION{
//	ULONG 
//};
//typedef _STEMS_WORKTHREAD_EXCEPTION STEMS_WORKTHREAD_EXCEPTION,* PSTEMS_WORKTHREAD_EXCEPTION;
// output none
// result na



//////////////////////////////////////////////////////////////////////////
// 重绘消息，系统定期自动产生，接受者是界面管理器本身
#define EMSG_GRAPHICS_UPDATE EMSG_DEFINE(LMSG_TP_RENDER,1,1)
// input none 
// output none

//////////////////////////////////////////////////////////////////////////
//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
#define EMSG_CREATE EMSG_DEFINE(LMSG_TP_ELEMENT,1,1)
// input IEinkuiIterator* : The interface pointer which assigned to current element
// output none

//////////////////////////////////////////////////////////////////////////
// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，一旦受到此消息，在退出本消息的处理过程并且所有各级子对象也都收到Destroy消息后，对象就会被销毁，相应的Iterator*也会被销毁
#define EMSG_DESTROY EMSG_DEFINE(LMSG_TP_ELEMENT,1,2)
// input none
// output none

//////////////////////////////////////////////////////////////////////////
// 绘制准备，通知元素准备绘制，并给出当前与前一次绘制的时间间隔，逻辑元素也会收到这条消息；
// 处理这条消息时，尽可能避免执行耗时太大的行为，避免产生新的元素或者装入新的资源；
// 对于逻辑元素，接收到此消息的时候，触发对其他元素的改变行为，如移动，缩放等；对于可视元素，通常不做处理；
// 被Hide的元素不会受到此消息；!!!注意!!!每个元素只需要自身准备绘制，系统将会自动给它的子元素发送准备绘制消息
#define EMSG_PREPARE_PAINT EMSG_DEFINE(LMSG_TP_RENDER,2,1)
// input IEinkuiPaintBoard*
// output none

//////////////////////////////////////////////////////////////////////////
// 绘制，通知元素绘制自身；
// 被Hide的元素不会受到此消息；!!!注意!!!每个元素只需要负责绘制自身，系统将会自动给它的子元素发送绘制消息
#define EMSG_PAINT EMSG_DEFINE(LMSG_TP_RENDER,2,2)
// input IEinkuiPaintBoard*
// output none
// result na

//////////////////////////////////////////////////////////////////////////
// 绘制结束，当一个元素的子元素都处理完绘制消息后调用
#define EMSG_POST_PAINT EMSG_DEFINE(LMSG_TP_RENDER,2,3)


//////////////////////////////////////////////////////////////////////////
// 增强器准备，通知增强器进入增强器绘制阶段，具体的使用方法，参考增强器设计的相关内容
// 增强器的作用目标或者增强器本身被隐藏时，不会收到本消息；!!!目前暂时不支持3D增强器重叠使用，一旦重叠就只让外层起作用
#define EMSG_PREPARE_ENHANCER EMSG_DEFINE(LMSG_TP_RENDER,3,1)
// intput 
struct _STEMS_ENHANCER_RENDER{
	IEinkuiPaintBoard* mpPaintBoard;
	IEinkuiIterator* mpDestElement;
};
typedef _STEMS_ENHANCER_RENDER STEMS_ENHANCER_RENDER,* PSTEMS_ENHANCER_RENDER;
// output none
// result 返回ERESULT_SKIP_RENDER_CONTENT将跳过对目标及其子元素的绘制

//////////////////////////////////////////////////////////////////////////
// 增强渲染，当作用目标元素与其子元素完成绘制后，通知增强器开始渲染；
#define EMSG_RENDER_ENHANCER EMSG_DEFINE(LMSG_TP_RENDER,3,2)
// input STEMS_ENHANCER_RENDER	如果本增强器在EMSG_PREPARE_ENHANCER消息处理中替换了Direct2d的RenderTarget，此时系统已经自动将其恢复为未改变之前的Target
// output none
// result 返回ERESULT_REDO_RENDER_CONTENT将再次绘制对象及其子对象，增效其也会再次执行；返回其他值无定义

//////////////////////////////////////////////////////////////////////////
// 图形设备相关资源丢弃；所有元素都会收到这条消息，仅需直接拥有D2d/D3d资源的元素处理；相关资源被丢弃后，将在EMSG_PREPARE_ENHANCER时重新装入
#define EMSG_DISCARD_DEVICE_RESOURCE EMSG_DEFINE(LMSG_TP_RENDER,3,5)
// input none
// output none


//////////////////////////////////////////////////////////////////////////
// 定时器消息
#define EMSG_TIMER EMSG_DEFINE(LMSG_TP_ELEMENT,2,1)
// input
struct _STEMS_TIMER{
	ULONG TimerID;		// 定时器的ID
	ULONG BaseTick;		// 设定定时器时的Tick值
	ULONG CurrentTick;	// 当前的TickCount值
	ULONG ElapsedTick;	// 对于上一条定时器消息而言，这条消息到上一条消息用去了多少Tick
	ULONG Kicked;		// 本定时器被触发的次数，从1开始，第一次触发就是1
	void* Context;		// 设定定时器时指定的上下文
};
typedef _STEMS_TIMER STEMS_TIMER,* PSTEMS_TIMER;
// output none

//////////////////////////////////////////////////////////////////////////
// 慢刷新消息，系统每秒钟产生1次慢刷新消息，发送给希望接受慢刷新消息的窗口，设置慢刷新请增加EITR_STYLE_LAZY_UPDATE
// 请将所有缓慢变化界面效果的定时操作放在这个消息的处理过程中
#define EMSG_LAZY_UPATE EMSG_DEFINE(LMSG_TP_ELEMENT,2,10)
// input 
struct _STEMG_LAZY_UPDATE{
	ULONG Updated;	// 系统已经产生过的LazeUpdate消息次数，在大范围内，这个消息是1秒5次，但每次之间的间隔不一定是200ms，只能保证大的范围精度不偏差1s
					// 可以使用这个序数来计算特定需要的刷新率
	ULONG CurrentTick;	// 当前的Windows TickCount值
};
typedef _STEMG_LAZY_UPDATE STEMG_LAZY_UPDATE,* PSTEMG_LAZY_UPDATE;
// output none
// result na

//////////////////////////////////////////////////////////////////////////
// 转发被Hook的消息，
#define EMSG_HOOKED_MESSAGE EMSG_DEFINE(LMSG_TP_ELEMENT,2,20)
// input 
struct _STEMS_HOOKED_MSG{
	IEinkuiIterator* OriginalElement;	// 原始的消息接受元素
	IEinkuiMessage* OriginalMessage;	// 被捕获的消息
};
typedef _STEMS_HOOKED_MSG STEMS_HOOKED_MSG,* PSTEMS_HOOKED_MSG;
// output none
// result 返回ERESULT_MSG_SENDTO_NEXT，消息将被传递给下一个hook或者没有下一个hook时直接发送给目标，如果返回任何其他值，豆浆终止消息的继续传递，直接返回给消息的发送者


//////////////////////////////////////////////////////////////////////////
// 反馈消息
#define EMSG_RESPONSE_MESSAGE EMSG_DEFINE(LMSG_TP_ELEMENT,2,30)
// input 
struct _STEMS_RESPONSE_MSG{
	IEinkuiIterator* OriginalElement;	// 原始消息的接受元素
	IEinkuiMessage* OriginalMessage;	// 被反馈的消息
	void* Context;	// 设置消息反馈时，指定的上下文
};
typedef _STEMS_RESPONSE_MSG STEMS_RESPONSE_MSG,*PSTEMS_RESPONSE_MSG;
// output none

//////////////////////////////////////////////////////////////////////////
//  鼠标落点检测消息
//	原理：
//		XUI系统需要定位当前鼠标处于哪一个Element之内，这项技术称为‘鼠标落点检测’；为了检测鼠标落点，需要每一个Element配合XUI系统做实时检测。
//	实时监测的实现方法是，系统按照Element的排布前后，从ZOrder最高层向前逐个元素询问是否拥有鼠标落点，如果有某个元素声明拥有这个落点，则完成检测；
//	询问的通过消息完成，该消息携带者一个鼠标点的坐标值，这个坐标是相对于元素的局部坐标系的；
//		当一个元素被渲染增效器挂接，那么，鼠标的落点检测的消息前后都会发给这个增效器做处理。
//  特别注意，这儿进行的只是鼠标落点检测，某个元素返回ERESULT_MOUSE_OWNERSHIP，并不代表已经或者将要拥有鼠标；对于元素而言，自身获得鼠标与否的
//	判断在于接受的鼠标进入，移除消息。
#define EMSG_MOUSE_OWNER_TEST EMSG_DEFINE(LMSG_TP_ELEMGR,1,1)
// input D2D1_POINT_2F
// output none
// result 返回ERESULT_MOUSE_OWNERSHIP表示鼠标处于自身的作用区，返回错误将终止对其子元素进行检测，除此情况，应该返回ERESULT_SUCCESS

// 增效器，准备鼠标检测，
#define EMSG_ENHANCER_PRE_MOUSE_TEST EMSG_DEFINE(LMSG_TP_ELEMGR,1,5)
// input D2D1_POINT_2F
// output D2D1_POINT_2F	增效器重新定位的检测参考值
// result 返回ERESULT_STOP_ENUM_CHILD停止询问子元素，返回ERESULT_SUCCESS，继续检测子元素

// 进入模态对话状态
#define EMSG_MODAL_ENTER EMSG_DEFINE(LMSG_TP_ELEMGR,1,10)
// none
// none
// result  return ERESULT_SUCCESS to enter the modal mode,otherwise refuse to change mode



//////////////////////////////////////////////////////////////////////////
// 下面定义的消息，是通用的元素通知消息
//////////////////////////////////////////////////////////////////////////

// 活跃改变，设置有EITR_STYLE_POPUP或者EITR_STYLE_ACTIVE的元素，会收到活跃改变消息；
// !!!注意!!!，当子元素获得了激活消息时，如果
#define EMSG_ELEMENT_ACTIVATED	EMSG_DEFINE(LMSG_TP_ELEMENT,10,1)
// input
typedef struct _STEMS_ELEMENT_ACTIVATION{
	LONG State;		// 对于只有2种状态的，零表示失去状态，非零表示获得状态；对于多状态的使用场合，0表示无状态或者初始化状态，其他值表示不用的状态
	IEinkuiIterator* Activated;	// 获得激活的元素，收到此消息被激活的元素有可能是因为某个子对象被激活而激活，此参数反映的就是激活的最上层元素（子元素）；这个对象句柄仅在处理本消息的过程中有效，不要长期持有
	IEinkuiIterator* InActivated;	// 失去激活的元素；这个对象句柄仅在处理本消息的过程中有效，不要长期持有
}STEMS_ELEMENT_ACTIVATION,* PSTEMS_ELEMENT_ACTIVATION;
// output none
// result na



#define EMSG_ELEMENT_MOVED EMSG_DEFINE(LMSG_TP_ELEMENT,3,1)		//元素位置发生了改变
//Input		D2D1_POINT_2F
//Output	无
//Result	

#define EMSG_ELEMENT_RESIZED EMSG_DEFINE(LMSG_TP_ELEMENT,3,3)			//元素参考尺寸发生改变
//Input		D2D1_SIZE_F
//Output	none
//Result	

#define EMSG_SHOW_HIDE EMSG_DEFINE(LMSG_TP_ELEMENT,3,5)		//显示或隐藏
//Input		bool ,true表示显示，false隐藏
//Output	None
//Result	

#define EMSG_ENALBE_DISABLE EMSG_DEFINE(LMSG_TP_ELEMENT,3,8)	// 被启用或者被禁用
//input bool, true for it's enable, the otherwise for disable
//output none
//result


#define EMSG_MOUSE_BUTTON EMSG_DEFINE(LMSG_TP_MOUSE,1,1)		//鼠标按下
//Input		STEMS_MOUSE_BUTTON
struct _STEMS_MOUSE_BUTTON{
	bool  Presssed;			//true表示按下，反之抬起
	ULONG ActKey;		// 哪个鼠标按钮发生变化，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2，任何一个时刻只有一个按钮会发生变化
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	ULONG TickCount;	// 发生动作时的TickCount
	D2D1_POINT_2F Position;		//发生动作时的鼠标指针所在位置
};
typedef _STEMS_MOUSE_BUTTON STEMS_MOUSE_BUTTON,*PSTEMS_MOUSE_BUTTON;
//Output	None
//Result

#define EMSG_MOUSE_DBCLICK EMSG_DEFINE(LMSG_TP_MOUSE,1,2)		//鼠标双击
//Input		STEMS_MOUSE_BUTTON , STEMS_MOUSE_BUTTON::Presssed is unused
//Output	None
//Result


#define EMSG_MOUSE_MOVING EMSG_DEFINE(LMSG_TP_MOUSE,1,3)		//鼠标移动
//Input		STEMS_MOUSE_MOVING
struct _STEMS_MOUSE_MOVING{
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	ULONG TickCount;	// 当前TickCount
	D2D1_POINT_2F Position;		//鼠标指针当前位置
};
typedef _STEMS_MOUSE_MOVING STEMS_MOUSE_MOVING,*PSTEMS_MOUSE_MOVING;
//Output	None
//Result

#define EMSG_MOUSE_WHEEL EMSG_DEFINE(LMSG_TP_MOUSE,1,4)		//鼠标滚轮转动
//intput
struct _STEMS_MOUSE_WHEEL{
	short Delta;
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	ULONG TickCount;	// 当前TickCount
	IEinkuiIterator* MouseFocus;	// 当前的鼠标焦点
	//D2D1_POINT_2F Position;		//鼠标指针当前位置
};
typedef _STEMS_MOUSE_WHEEL STEMS_MOUSE_WHEEL,* PSTEMS_MOUSE_WHEEL;
//output none
// result 如果不处理本消息，务必返回ERESULT_UNEXPECTED_MESSAGE


#define EMSG_MOUSE_FOCUS EMSG_DEFINE(LMSG_TP_MOUSE,1,8)		//鼠标焦点变化
//Input	STEMS_STATE_CHANGE
struct _STEMS_STATE_CHANGE{
	LONG State;		// 对于只有2种状态的，零表示失去状态，非零表示获得状态；对于多状态的使用场合，0表示无状态或者初始化状态，其他值表示不用的状态
	IEinkuiIterator* Related;	// 关联的元素，比如从此元素获得，或者被此元素夺取；可能没有关联元素，此时为NULL；这个对象句柄仅在处理本消息的过程中有效，不要长期持有
};
typedef _STEMS_STATE_CHANGE STEMS_STATE_CHANGE,* PSTEMS_STATE_CHANGE;
//Output None
//Result

#define EMSG_MOUSE_HOVER EMSG_DEFINE(LMSG_TP_MOUSE,1,20)		//鼠标悬停
//Input		none
//Output	None
//Result


#define EMSG_KEYBOARD_FOCUS EMSG_DEFINE(LMSG_TP_KEYBOARD,1,1)		//键盘焦点变化
//Input	STEMS_STATE_CHANGE
//Output None
//Result

#define EMSG_TOUCH  EMSG_DEFINE(LMSG_TP_TOUCH_SCREEN,1 ,1)	// 触摸屏消息
// input EI_TOUCHINPUT_POINT
//#define EI_TOUCHEVENTF_DOWN 0 refer to file EinkIteApi.h
//#define EI_TOUCHEVENTF_MOVE 1
//#define EI_TOUCHEVENTF_UP 2
//typedef struct _EI_TOUCHINPUT_POINT {
//	unsigned long x;
//	unsigned long y;
//	unsigned long z;		//pressure
//	unsigned long Flags;	//0:down  1:move  2:up
//}EI_TOUCHINPUT_POINT, *PEI_TOUCHINPUT_POINT;

#define EMSG_KEYBOARD_SCREEN_KEYBOARD EMSG_DEFINE(LMSG_TP_KEYBOARD,1,2)		//弹出了屏幕键盘
//Input	None
//Output None
//Result


#define EMSG_KEY_PRESSED EMSG_DEFINE(LMSG_TP_KEYBOARD,1,8)		//键盘按键
//Input		STEMS_KEY_PRESSED
struct _STEMS_KEY_PRESSED{
	bool IsPressDown;			//true表示按下，反之抬起
	ULONG VirtualKeyCode;		//同Widows的WM_KEYDOWN消息参数wParam
	ULONG ExtendedKeyFlag;		//同Widows的WM_KEYDOWN消息参数lParam
};
typedef _STEMS_KEY_PRESSED STEMS_KEY_PRESSED,*PSTEMS_KEY_PRESSED;
//Output	None
//Result

//////////////////////////////////////////////////////////////////////////
//字符输入
#define EMSG_CHAR_INPUT EMSG_DEFINE(LMSG_TP_KEYBOARD,1,12)
// input 
struct _STEMS_CHAR_INPUT{
	wchar_t CharIn;		// 输入的字符
	ULONG Flags;		// 同WM_CHAR消息的lParam一致
};
typedef _STEMS_CHAR_INPUT STEMS_CHAR_INPUT,* PSTEMS_CHAR_INPUT;
// output none
// result na


// 快捷键被按下
#define EMSG_HOTKEY_PRESSED EMSG_DEFINE(LMSG_TP_KEYBOARD,10,1)
// input 
struct _STEMS_HOTKEY{
	IEinkuiIterator* Focus;	//当前键盘焦点，并不一定是接受当前消息的元素
	ULONG HotKeyID;		//注册时提供的ID
	ULONG VkCode;	// 快捷键的普通按键
	bool Control;	// Control键是否按下
	bool Shift;		// Shift键是否按下
	bool Alt;		// Alt键是否按下
};
typedef _STEMS_HOTKEY STEMS_HOTKEY,* PSTEMS_HOTKEY;
// output none
// result na


// 元素拖拽，接收者可以选择移动元素或者自行解读拖拽的意义
#define EMSG_DRAGGING_ELEMENT EMSG_DEFINE(LMSG_TP_INPUT,1,1)
// intput
struct _STMS_DRAGGING_ELE{
	IEinkuiIterator* DragOn;	// 拖拽作用点元素
	ULONG ActKey;		// 哪个鼠标按钮按下进行拖拽，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON，任何一个时刻只有一个按钮会激活拖拽
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	D2D1_POINT_2F Offset;		// 当前鼠标位置与开始拖拽的位置的偏差，局部坐标系
	D2D1_POINT_2F CurrentPos;	// 当前鼠标位置或者手指位置，局部坐标系
};
typedef _STMS_DRAGGING_ELE STMS_DRAGGING_ELE,* PSTMS_DRAGGING_ELE;
// output none

// 拖拽开始
#define EMSG_DRAG_BEGIN EMSG_DEFINE(LMSG_TP_INPUT,1,2)
// input STMS_DRAGGING_ELE
// output none or 返回两种Dragdrop所需的不同参数
struct _STMS_EDRGDRP_REQUEST{	// Xui drag&drop的参数
	IEinkuiIterator* Host;	// DragDrop的发起者，不用设置，系统会自动将其设定为本消息的返回者
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	D2D1_POINT_2F CurrentPos;	// 当前鼠标落点，接受此消息对象的局部坐标系
	GUID Reason;		// 目的
	ULONG Flags;		// 可以携带4字节的标志，辅助接受体判断，如果希望获得更详细的信息，需要接受体向发起者发送询问消息
	UCHAR Attachment[128];	// 128个字节的参数，如果不够，请使用消息相互交流
};
typedef _STMS_EDRGDRP_REQUEST STMS_EDRGDRP_REQUEST,* PSTMS_EDRGDRP_REQUEST;
// result 表示是否启动Drag&Drap操作，返回ERESULT_EDRAGDROP_BEGIN启动XUI Drag&drop过程，返回ERESULT_WDRAGDROP_BEGIN启动Windows Drag&Drop过程，其中Windows Drag&Drop的功能覆盖Xui Drag&Drop
//		  返回RESULT_SUCCESS则执行普通的拖拽行为，返回错误值则不进行拖拽操作


// 拖拽结束
#define EMSG_DRAG_END EMSG_DEFINE(LMSG_TP_INPUT,1,3)
// inputSTMS_DRAGGING_ELE
// output none
// result na


// Drop询问，元素设置了Style EITR_STYLE_XUIDRAGDROP才会收到
#define EMSG_XUI_DROP_TEST EMSG_DEFINE(LMSG_TP_INPUT,2,1)
// input STMS_EDRGDRP_REQUEST
// output none
// result ERESULT_EDRAGDROP_ACCEPT表示接受Drop的内容，任意其他值表示不接受

// Drop询问进入，当Dragging point进入某个Element时，发送给目标，目标必须具有EITR_STYLE_XUIDRAGDROP
#define EMSG_DRAGDROP_ENTER EMSG_DEFINE(LMSG_TP_INPUT,2,5)
// input none
// output none
// na

// Drop询问结束，当Dragging point离开当前Element时，发送给目标，目标必须具有EITR_STYLE_XUIDRAGDROP
#define EMSG_DRAGDROP_LEAVE EMSG_DEFINE(LMSG_TP_INPUT,2,6)
// input none
// output none
// na

// Drag off，发送给XUI Drag&Drog的发起者，告诉它Drag&Drop准备完成
#define EMSG_XUI_DRAG_OFF EMSG_DEFINE(LMSG_TP_INPUT,2,2)
// input IEinkuiIterator* the accepter
// output optional STMS_EDRGDRP_REQUEST
// result ERESULT_SUCCESS to confirm the DROP,others to deny the DROP

// Drop down，to inform the accepter to complete the DROP
#define EMSG_XUI_DROP_DOWN EMSG_DEFINE(LMSG_TP_INPUT,2,3)
// input STMS_EDRGDRP_REQUEST
// output none
// result na

////////////////////////////////////////////////////////////////////////////
//// 命令
//// 系统只会向具有EITR_STYLE_PAGE的窗口发送本消息
#define EMSG_COMMAND EMSG_DEFINE(LMSG_TP_INPUT,4,1)
// input nes_command::ESCOMMAND
namespace nes_command{
	enum ESCOMMAND{
		eInvalid=0,
		eCopy=1,
		eCut=2,
		ePaste=3,
		eDelete=4,
		eSelAll=5,
		eEnter=6,
		eEsc=7,
		eAltF4=8
	};
}
// output none
// result ERESULT_SUCCESS 表示处理这条消息，否则，消息将会转发给上一级的接受command的元素


//////////////////////////////////////////////////////////////////////////
// 下面的消息仅仅是System Widget的HomePage 会收到

// 显示XUI系统的MessageBox
#define EMSG_SWGT_MESSAGE_BOX EMSG_DEFINE(LMSG_TP_SYSTEM,10,1)
// input STEMS_MESSAGE_BOX
namespace nse_msgbox{

	enum EBUTTON{
		eNoneBt=0,
		eOk,
		eCancel,
		eYes,
		eNo,
		eAbort,
		eRetry,
		eIgnore,
		eContinue
	};

	enum EICON {
		eNoneIc=0,
		eQuestionIc,
		eCheckIc,
		eWarningIc,
		eErrorIc
	};

};
struct _STEMS_MESSAGE_BOX{
	nse_msgbox::EBUTTON Buttons[3];	// 定义 对话框上的三个按钮的内容，如果只需要一个按钮，将后面两个单元设置为eNoneBt
	nse_msgbox::EICON  Icon;
	const wchar_t* Title;
	const wchar_t* Text;
};
typedef _STEMS_MESSAGE_BOX STEMS_MESSAGE_BOX,* PSTEMS_MESSAGE_BOX;
// output none
// result the answer from user or error code if anything is wrong

// 显示Tip
#define EMSG_SWGT_SHOW_TIP EMSG_DEFINE(LMSG_TP_SYSTEM,11,1)
// input
struct _STEMS_SWGT_SHOWTIP{
	const wchar_t* Text;	// to show as tooltip
	D2D1_POINT_2F Position;
};
typedef _STEMS_SWGT_SHOWTIP STEMS_SWGT_SHOWTIP,* PSTEMS_SWGT_SHOWTIP;
// output none
// result na

// 隐藏Tip
#define EMSG_SWGT_HIDE_TIP EMSG_DEFINE(LMSG_TP_SYSTEM,11,2)
// input none
// output none
// result na

// 窗口尺寸改变通知
#define EMSG_SWGT_MW_RESIZED EMSG_DEFINE(LMSG_TP_SYSTEM,11,10)
// input ULONG[2], first is width followed height
// output none
// result na

// 系统屏幕发生旋转，应答系统是否转动自身
#define EMSG_QUERY_SWGT_ROTATED EMSG_DEFINE(LMSG_TP_SYSTEM,11,14)
// input ULONG as screen-orientation
// output ULONG orientation to set. MAXULONG32 indicate not rotate orientation

// 画板发生旋转
#define EMSG_SWGT_ROTATED EMSG_DEFINE(LMSG_TP_SYSTEM,11,15)
// input ULONG: new Orientation of EINK SDK
// output none
// result na

// 画刷类型定义
enum XuiBrushType{
	XuiSolidBrush,
	XuiLinearGradientBrush,
	XuiRadialGradientBrush,
	XuiBitmapBrush,
	XuiStroke,
};

//通过字符串实例化一个IEinkuiBitmap对象时需要的结构体定义，用于IXelAllocator接口提供的CreateImageByText函数参数
typedef struct _STETXT_BMP_INIT
{
	enum eSIZELIMIT{
		EL_FREESIZE,
		EL_FIXEDWIDTH,
		EL_FIXEDSIZE
	};

	enum eTEXTALIGN{
		EL_TEXTALIGN_LEADING,
		EL_TEXTALIGN_TRAILING,
		EL_TEXTALIGN_CENTER
	} ;

	enum ePARAALIGN{
		EL_PARAALIGN_NEAR,
		EL_PARAALIGN_FAR,
		EL_PARAALIGN_CENTER
	} ;

	enum eFONTWEIGHT {
		EL_FW_NORMAL,
		EL_FW_BOLD
	};

	DWORD StructSize;			//结构体大小
	const wchar_t* Text;		//要生成图片的字符串
	const wchar_t* FontName;	//字体名称
	DWORD FontSize;				//字体大小
	DWORD TextColor;			//文字颜色
	DWORD Width;				//指定宽度
	DWORD Height;				//指定高度
	eSIZELIMIT Limit;
	eTEXTALIGN Talign;
	ePARAALIGN Palign;
	eFONTWEIGHT FontWeight;		//字体weight，目前只支持普通和粗体
}STETXT_BMP_INIT,*PSTETXT_BMP_INIT;

// 设定自绘的参数
typedef struct _EINKUI_CUSTOMDRAW_SETTINGS
{
	ULONG Width;	// 在Panel上显示时的宽度
	ULONG Height;
	PXUI_CUSTOM_DRAW_CALLBACK CustomDrawFun;	// 自绘函数
}EINKUI_CUSTOMDRAW_SETTINGS,* PEINKUI_CUSTOMDRAW_SETTINGS;


#pragma pack()
// 结构体定义结束
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// XUI主接口定义
__interface IEinkuiSystem:public IBaseObject
{
	// 获得当前的界面语言描述字符串，如：简体中文'CN'
	virtual const wchar_t* __stdcall GetCurrentLanguage(void)=NULL;

	// 获得系统的DPI设置
	virtual FLOAT __stdcall GetDpiX(void)=NULL;
	virtual FLOAT __stdcall GetDpiY(void)=NULL;

	// 启用画板，当主程序装载完成后，调用这个函数启用画板；在此之前画板并不会绘制出实际图像，以避免启动不完整时，Eink屏幕多次闪动
	virtual ERESULT __stdcall EnablePaintboard(void) = NULL;

	// 重置画板，当屏幕旋转发生时，调用本函数促使系统重置画板
	virtual void __stdcall ResetPaintboard(void) = NULL;

	// 获得画板的大小
	virtual void __stdcall GetPaintboardSize(
		OUT EI_SIZE* npSize	// 获取画板大小
	) = NULL;

	// 设置本程序在Eink Panel上的显示位置，只用于自绘程序
	virtual void __stdcall SetPositionInPanel(
		ULONG nuX,
		ULONG nuY
		) = NULL;

	// 获取Eink系统参数
	virtual PEI_SYSTEM_INFO __stdcall GetEinkSystemInfo(void) = NULL;

	// 获得Windows窗口
	virtual HWND __stdcall GetMainWindow(void)=NULL;

	// 显示或者隐藏Windows通讯窗口
	virtual void __stdcall ShowMainWindow(bool nbShown=true)=NULL;

	// 返回元素管理
	virtual IXelManager* __stdcall  GetElementManager(void)=NULL;	// 绝对不会失败，无需判断返回值是否为NULL

	// 运行一个新的Widget
	virtual ERESULT __stdcall LaunchWidget(
		IN const wchar_t* nswModulePathName,	// 该Widget的模块文件的路径名，即实现此Widget的DLL名称
		IN const wchar_t* nswHomeTempleteName,	// 该Widget的HomePage的Templete Key的名字，这个Key必须在ProFile 的Root下
		IN const wchar_t* nswInstanceName,	// 本次运行的实例名，实例名不能相同，如果存在相同的实例名，系统将会返回失败
		IN IEinkuiIterator* npFrame,	// 用于定位和显示待装载Widget的Frame Element
		IN ICfKey* npInstConfig,	// Widget实例专属配置
		OUT IXsWidget** nppWidget	// 可以不填写，用于返回新建立的Widget接口，返回的接口需要释放
		)=NULL;

	// 获得当前微件接口，无需释放;如果某个线程没有使用CreateWidgetWorkThread建立，那么在这样的线程中调用GetCurrentWidget将导致异常发生
	virtual IXsWidget* __stdcall GetCurrentWidget(void)=NULL;	// 绝对不会失败，无需判断返回值是否为NULL

	// 获得当前系统中的某个Widget接口，如果返回NULL表示此编号之后没有Widget了
	// 此函数只能被System Widget调用
	virtual IXsWidget* __stdcall ObtainWidget(
		IN int niNumber		// 从0开始编号得Widegt，编号没有意义，只是Widget的存储位置；0号位置存放的是System Widget
		)=NULL;

	// 获得系统Widget
	virtual IXsWidget* __stdcall GetSystemWidget(void)=NULL;

	// 启动一个新线程，所有的Widget都应该使用这个函数建立自己的额外线程；返回值即Windows线程句柄，可以用于调用SuspendThread/ResumeThread/GetExitCodeThread
	// 最终，返回的句柄必须调用CloseHandle关闭；函数的输入参数同Windows API CreateThread一致
	virtual HANDLE __stdcall CreateWidgetWorkThread(
		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		SIZE_T dwStackSize,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		DWORD dwCreationFlags,
		LPDWORD lpThreadId
		)=NULL;

	// 退出Widget工作线程；当，传入给CreateWidgetWorkThread的线程主函数退出时，系统会自动调用终止线程操作
	virtual void __stdcall ExitWidgetWorkThread(DWORD dwExitCode)=NULL;

	// 获得分配器，分配器被用于建立公共或发布的Element对象，返回的对象无需释放
	virtual IXelAllocator* __stdcall GetAllocator(void)=NULL;

	// 打开一个Config文件；用于打开一个config文件，目前应用在Factory接口实现中，用于打开一个Conponent对应的Profile
	// 返回的句柄需要释放，如果以写数据方式打开，请调用save方法保存数据到文件
	virtual IConfigFile* __stdcall OpenConfigFile(
		IN const wchar_t* nszPathName,				// 文件的完整路径名
		IN ULONG nuCreationDisposition=CF_OPEN_EXISTING	// 同CreateFile API类似，但仅包括CF_CREATE_ALWAYS、CF_CREATE_NEW、CF_OPEN_ALWAYS、CF_OPEN_EXISTING，定义见CfgIface.h
		)=NULL;

	// 申请重新绘制以更新整个视图
	// 重复调用本函数并不会导致重复的绘制；
	virtual void __stdcall UpdateView(
		IN bool nbRefresh = false	// 必须提交全屏
		)=NULL;

	// 设置Eink绘制回调，每当UI系统向Eink服务提交时调用指定的回调函数
	virtual ERESULT __stdcall SetEinkUpdatingCallBack(
		IN PXUI_EINKUPDATING_CALLBACK npFunction,
		IN PVOID npContext
	) = NULL;

	// 重置Eink缓存；本库的工作原理是通过Eink缓存中的遗留内容来比对待显示内容，只将不同的部分发给Eink；如果Eink屏幕需要全部重绘
	//    就需要重置Eink缓存，使得全部内容绘制到Eink；主要用于App重新活动Eink屏幕的情况
	virtual void __stdcall ClearEinkBuffer(void) = NULL;

	// 给某个Element及其所有children拍照
	virtual IEinkuiBitmap* __stdcall TakeSnapshot(
		IEinkuiIterator* npToShot,
		const D2D1_RECT_F& crSourceRegion,	// 采样区域，目标元素的局部坐标系
		const D2D_SIZE_F& crBriefSize,		// 缩略图尺寸，快照的结果是一副缩略图
		const FLOAT* ColorRGBA=NULL
		)=NULL;

	// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual ID2D1Factory* __stdcall GetD2dFactory(void)=NULL;

	// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual IWICImagingFactory* __stdcall GetWICFactory(void)=NULL;

	// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual IDWriteFactory* __stdcall GetDWriteFactory(void)=NULL;

	// 申请使用Windows交互线程回调，如果Widget开发中需要调用Windows的界面模块，显示额外的Windows界面或者做与Windows界面相关的必须在Windows交互线程中执行的功能，
	// 将该功能代码移植到自己Element或者其他输出IBaseObject的类的一个独立函数中，通过调用本方法，可以使得Windows界面线程主动回调设定的独立函数，而该函数的返回值也将
	// 通过本函数直接返回给本方法的调用者。完成上述功能的过程中，本处的代码将阻塞在此方法中，并且确保整个界面正常刷行，但，Xui本身的界面将不再相应。
	// !!!注意!!!，调用不允许重入，同一时刻仅允许一处调用此方法，如果发生同时调用，后一次调用会返回ERESULT_ACCESS_CONFLICT；所以，请调用本方法的使用者，务必检查返回值。
	// 如果返回ERESULT_ACCESS_CONFLICT，请放弃调用，不要立即重新尝试。这种情况基本不可能发生。
	virtual ERESULT __stdcall CallBackByWinUiThread(
		IBaseObject* npApplicant,		// 回调对象，并不一定要求是调用本方法的对象本身，也可以是别的对象指针
		PXUI_CALLBACK npProcedureToCall,	//回调过程，过程的返回值将会返回给调用者
		ULONG nuFlag,			// 传递给回调函数的参数
		LPVOID npConText,		// 传递给回调函数的参数
		bool nbStall=false		// 等待返回期间是否需要处理后续的XUI消息，如果nbStal设为ture程序将直接等待返回，而不处理XUI消息循环
								// 使用此方法可以避免调用本方法期间，再次重入调用
		)=NULL;

	// 注册Windows消息拦截，通过本功能可以在XUI系统处理Windows消息之前截取关注的Windows消息
	// 处理截取的Windows消息的过程要尽可能短暂
	// 返回ERESULT_WINMSG_SENDTO_NEXT，XUI将消息传递给下一个拦截者，或者交由XUI系统解释处理；返回其他值将终止该Windows消息的传递过程以及XUI对该消息的处理
	virtual ERESULT __stdcall CaptureWindowsMessage(
		IN UINT nuiWinMsgID,	// 希望博获的Windows消息的ID
		IN IXsElement* npApplicant,	//申请回调的对象
		IN PWINMSG_CALLBACK npProcedure	//将Windows消息发送给此函数
		)=NULL;

	// 产生一个命令，此方法通常为SystemWidget的菜单模块调用，用来模拟一次用户按下组合键行为，该命令将会被发送至当前的键盘焦点以上的具有EITR_STYLE_COMMAND的对象
	virtual ERESULT __stdcall GenerateCommand(
		nes_command::ESCOMMAND neCmd
		)=NULL;

	// 设置某页成为模态对话方式，即用户必须完成该对话，此时界面上的其他部分都无法使用。
	// XUI的模态对话方式是全局的，处于模态对话下，所有的Widget（包括System Widget)都不能响应用户输入；所有，尽可能避免使用模式对话，除非它是必须的。
	// 使用方法是，首先创建模式对话的主对象（默认隐藏)，然后调用本函数进入模态对话方式，此时该模态对话元素对象将会收到一条EMSG_MODAL_ENTER消息，它处理相关事务后将自己显示出来；
	// 当该模态对话对象判断用户完成了对话后，隐藏自己，而后ExitModal退出模态对话方式
	// 注意，模态对话方式是可以重叠进入的，在模态对话下，可以打开子模态对话，而后一层层退出
	virtual ERESULT __stdcall DoModal(
		IEinkuiIterator* npModalElement		// 此元素是模态对话的主对象
		)=NULL;


	// 退出模态对话方式
	virtual void __stdcall ExitModal(
		IEinkuiIterator* npModalElement,	// 此元素是模态对话的主对象
		ERESULT nuResult
		)=NULL;


	// 退出整个XUI系统
	virtual void __stdcall ExitXui(void)=NULL;

	// 开发用的一个测试接口，不要调用
	virtual void __stdcall SystemTest(ULONG nuValue)=NULL;

	//本次操作是否是TOUCH操作
	virtual bool __stdcall IsTouch(void) = NULL;

public:
	// 创建渐变画刷
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType,
		D2D1_COLOR_F noColor
		) = NULL;

	// 渐变画刷时，需要传入多个颜色点
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType, 
		D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
		) = NULL;

};

//////////////////////////////////////////////////////////////////////////
// 元素管理器
//#define EMSG_POST_NORMAL	0
//#define EMSG_POST_FAST		2 // 表示消息就有Fast属性，这类消息应该按先后排列在消息队列的前部，等待处理，相同目标，相同类型的消息，只会存在后发的一条
//#define EMSG_POST_REVERSE	3 // 当消息队列同时存在这类消息的时候，将会被后进先出的方式排列在消息队列中，它们的优先级高于普通消息，但低于Fast消息
//#define EMSG_POST_REDUCE	4 // 当消息队列中出现发给同一个对象的第三条消息时，去掉发送给统一对象的第二条消息
// 下面的4中Post消息，只能设置其一
#define EMSG_POSTTYPE_FAST	2			// 快速消息，适用于人工输入行为，普通用途不要使用
#define EMSG_POSTTYPE_REVERSE 3			// 倒置消息，将会被后进先出的方式排列在普通消息队列中，通常用于需要正序遍历逆序执行的用途
#define EMSG_POSTTYPE_NORMAL 4			// 普通消息，按照先进先出的方式防止在普通消息队列中，用于普通用途
#define EMSG_POSTTYPE_REDUCE 5			// 简约消息，将被放置在慢速队列中，并且，当队列中存在2条以上消息时，将删除两条中间的消息
#define EMSG_POSTTYPE_QUIT	6			// 系统使用

__interface IXelManager:public IBaseObject
{
	// 向系统管理器注册一个Element，返回迭代器对象；失败返回NULL
	// 成功返回的接口对象
	virtual IEinkuiIterator* __stdcall  RegisterElement(
		IN IEinkuiIterator* npParent,	// 父元素的迭代器
		IN IXsElement* npElement,	// 待注册的子元素
		IN ULONG nuEID=0	// 本元素的EID，在同一个父元素下，各子元素的EID必须唯一，如果不关心EID，请设置=0，系统自动分配
		)=NULL;

	// 向系统管理器注销一个Element，此功能仅应该被待注销Element自身或者XUI系统调用；这个方法已经废弃
	virtual ERESULT __stdcall UnregisterElement(
		IN IEinkuiIterator* npElementIterator	// 该元素对应的迭代器
		)=NULL;

	// 验证一个Iterator是否有效，返回ERESULT_SUCCESS有效，返回ERESULT_ITERATOR_INVALID无效
	virtual ERESULT __stdcall VerifyIterator(
		IN IEinkuiIterator* npIterator	// 迭代器
		)=NULL;

	// 在对象管理器中查找一个Element，返回该Element的迭代器对象；失败返回NULL
	// 成功返回的接口对象； 如果经常需要通过元素获得它的注册迭代器，请保存迭代器的指针，因为调用本方法使用全树遍历查找获取迭代器对象，耗时较大；
	virtual IEinkuiIterator* __stdcall FindElement(
		IN IXsElement* npElement
		)=NULL;

	// 获得根元素；如果是为了给对象管理器发送消息，也可以直接使用NULL指针表示对象管理器
	// 成功返回的接口对象
	virtual IEinkuiIterator* __stdcall GetRootElement(void)=NULL;

	// 获得菜单页，所有的菜单都建立在这个页
	virtual IEinkuiIterator* __stdcall GetMenuPage(void)=NULL;

	// 获得ToolTip平面，这个最高的页
	virtual IEinkuiIterator* __stdcall GetToolTipPage(void)=NULL;

	// 获得桌面元素；桌面元素在启动XUI引擎时指定，桌面元素实现具体应用的全局功能，如：Idealife的全局应用由"Idealife"元素类的实例提供，
	// 通过给它发Idealife的应用消息执行Idealife的系统调用
	// 成功返回的接口对象
	virtual IEinkuiIterator* __stdcall GetDesktop(void)=NULL;

	// 重新设定父元素，nbZTop==true设置于Zoder顶层，false设置在底层
	virtual ERESULT __stdcall SetParentElement(IEinkuiIterator* npParent,IEinkuiIterator* npChild,bool nbZTop=true)=NULL;

	// 获得鼠标焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
	// 因为鼠标焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
	virtual IEinkuiIterator* __stdcall GetMouseFocus(void)=NULL;

	// 获得键盘焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
	// 因为键盘焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
	virtual IEinkuiIterator* __stdcall GetKeyboardFocus(void)=NULL;

	// 重置拖拽起点，仅当系统正处于拖拽行为中时，可以将拖转转移给他人
	// 如果试图转移到的目标元素不能支持拖拽行为，当前的拖拽行为也会终止，当前拖拽的目标元素会收到Drag_end消息
	virtual ERESULT __stdcall ResetDragBegin(IEinkuiIterator* npToDrag)=NULL;

	// 清理输入消息LMSG_GET_TYPE(MsgId) == LMSG_TP_WIN_INPUT
	virtual void __stdcall CleanHumanInput(bool nbStallInput=false) = NULL;

	// 分配一个消息
	// 消息发送后，发送者仍然需要释放
	virtual IEinkuiMessage* __stdcall  AllocateMessage(void)=NULL;

	// 分配一个消息，初始化相关参数
	// 消息发送后，发送者仍然需要释放
	virtual IEinkuiMessage* __stdcall AllocateMessage(
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区
		IN int niInputSize,	// 输入数据的大小
		OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
		IN int niOutputSize,	// 输出缓冲区大小
		IN bool nbInputVolatile=true	// 输入缓冲区是否是易失的，参见IXuiMessage::SetInputData获得更多信息
		)=NULL;

	// 给指定元素发送一条消息，发送模式是Send
	// 消息发送后，发送者仍然需要释放；尽可能避免从Windows界面线程向其他对象发送Send类型的消息
	virtual ERESULT __stdcall SendMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IEinkuiMessage* npMsg
		)=NULL;

	// 给指定元素发送一条消息，发送模式是Post
	// 消息发送后，发送者仍然需要释放
	virtual ERESULT __stdcall PostMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IEinkuiMessage* npMsg,
		IN ULONG nuPostType=EMSG_POSTTYPE_NORMAL	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
		)=NULL;

	// 简单地给指定元素发送一条消息，发送模式是Send；此函数的返回成功就是消息处理的返回值，错误的原因就不一定是消息处理的返回值，可能是消息发送失败
	// 尽可能避免从Windows界面线程向其他对象发送Send类型的消息
	virtual ERESULT __stdcall SimpleSendMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区，注意，如果希望传递的参数是一个指针本身，如：lpAnybody，应该如此调用SetInputData(&lpAnybody,sizeof(lpAnybody),ture/false);
		IN int niInputSize,	// 输入数据的大小
		OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
		IN int niOutputSize	// 输出缓冲区大小
		)=NULL;

	// 简单地给指定元素发送一条消息，发送模式是Post；无法获得消息处理的返回值；此函数的返回值仅表示消息是否被成功填入消息队列
	virtual ERESULT __stdcall SimplePostMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区，注意，如果希望传递的参数是一个指针本身，如：lpAnybody，应该如此调用SetInputData(&lpAnybody,sizeof(lpAnybody),ture/false);
		IN int niInputSize,	// 输入数据的大小
		IN ULONG nuPostType=EMSG_POSTTYPE_NORMAL	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
		)=NULL;

	// 枚举全部元素，每当发现一个Element时调用枚举请求者提供的ElementEnter函数；当一个元素没有子元素时，将调用提供的ElementLeave
	// 因为根节点是XUI系统的虚拟对象，枚举不会触及根节点
	virtual ERESULT __stdcall EnumAllElement(
		bool nbReverse,				// 反向，指的是枚举子节点时，按照Zorder的顺序枚举，或者按照Zorder的逆序枚举
		IBaseObject* npApplicant,	// 发起对象
		ERESULT (__stdcall IBaseObject::*ElementEnter)(IEinkuiIterator* npRecipient),//如果返回ERESULT_ENUM_CHILD继续枚举；返回ERESULT_STOP_ENUM_CHILD或任意其他值将停止枚举此元素的此元素的子元素
		ERESULT (__stdcall IBaseObject::*ElementLeave)(IEinkuiIterator* npRecipient) //返回值ERESULT_EXIT_ENUM或者错误码，停止全部枚举行为；ERESULT_REDO_ENUM则再次对此对象及其子对象执行枚举；其他值继续执行
		)=NULL;

	// 增加Iterator的引用，由于XUI的客户程序可能会遗漏对Iterator的释放和引用操作，所以默认的Iterator->AddRef()和Iterator->Release()方法是假的，并不会产生实际的调用，但Element被Close后，对应的Iterator一定
	// 会被释放；在本接口中提供了真实的引用和释放的方法操作Iterator对象，切记要谨慎操作，过多地释放将会导致XUI崩溃；
	// 增加Iterator引用
	virtual int __stdcall AddRefIterator(IEinkuiIterator* npIterator)=NULL; 

	// 释放Iterator
	virtual int __stdcall ReleaseIterator(IEinkuiIterator* npIterator)=NULL;

	// 注册快捷键，当快捷键被触发，注册快捷键的Element将会受到；
	// 如果普通按键组合（不包含Alt键)按下的当时，存在键盘焦点，按键消息会首先发送给键盘焦点，如果焦点返回ERESULT_KEY_UNEXPECTED才会判断是否存在快捷键行为
	virtual bool __stdcall RegisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册的元素，将有它收到注册是快捷键消息
		IN ULONG nuHotKeyID,	// 事先定义好的常数，用来区分Hotkey；系统不会检测ID是否重复，可以使用同一个ID对应不同的按键组合；
								// 用户使用的ID不要超过0x7FFFFFFF，大于等于0x80000000的ID保留给系统预设的快捷使用；
		IN ULONG nuVkNumber,	// 虚拟键码
		IN bool nbControlKey,	// 是否需要Control组合
		IN bool nbShiftKey,		// 是否需要Shift组合
		IN bool nbAltKey,		// 是否需要Alt组合
		IN IEinkuiIterator* npFocus=NULL	// 指定焦点范围，仅当该元素及其子元素获得键盘焦点时，才会触发本次注册的快捷键;
										// 使用NULL作为参数而不指定焦点范围，则无论键盘焦点在何处都能够收到注册的快捷键的消息；
		)=NULL;

	// 注销快捷键
	virtual bool __stdcall UnregisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册者
		IN ULONG UnuKeyNumber
		)=NULL;

};
// 下面定义默认快捷键ID
#define EHOTKEY_MAKE_ID(_X) (_X+0x80000000)
#define EHOTKEY_COPY	EHOTKEY_MAKE_ID(1)
#define EHOTKEY_CUT		EHOTKEY_MAKE_ID(2)
#define EHOTKEY_PASTE	EHOTKEY_MAKE_ID(3)

#define EHOTKEY_SELECT_ALL	EHOTKEY_MAKE_ID(10)
#define EHOTKEY_ESC		EHOTKEY_MAKE_ID(20)
#define EHOTKEY_ALTF4	EHOTKEY_MAKE_ID(21)

#define EHOTKEY_ENTER	EHOTKEY_MAKE_ID(25)
#define EHOTKEY_DELETE	EHOTKEY_MAKE_ID(30)



//////////////////////////////////////////////////////////////////////////
// 消息接口
__interface IEinkuiMessage:public IBaseObject
{

	// 获得消息的接收目标的Iterator接口
	virtual IEinkuiIterator* __stdcall GetDestination(void)=NULL;

	// 设置消息的接受目标，供系统的消息发送模块调用，自动设置发送目标
	virtual ERESULT __stdcall SetDestination(IEinkuiIterator* npMsgDestItr)=NULL;

	// 获得消息ID
	virtual ULONG __stdcall GetMessageID(void)=NULL;

	// 设置消息ID
	virtual ERESULT __stdcall SetMessageID(ULONG nuMsgID)=NULL;

	// 获得返回值
	virtual ERESULT __stdcall GetResult(void)=NULL;

	// 设置返回值，函数返回的是本操作的成功与否
	virtual ERESULT __stdcall SetResult(ERESULT nuResult)=NULL;

	// 设置消息发送者，当消息被用于从控件向控件使用者发送事件（控件产生的消息）时，需要设置消息的发送者；Iterator接口提供的PostMessageToParent方法会自动设置发送者
	virtual void __stdcall SetMessageSender(IEinkuiIterator* npSender)=NULL;

	// 获得消息的发送者
	virtual IEinkuiIterator* __stdcall GetMessageSender(void)=NULL;

	// 设置Input数据；如果消息是被Post的，nbVolatile必须为ture；如果消息是被Send给目标的，将按照参数nbVolatile决定是否复制输入缓冲
	virtual ERESULT __stdcall SetInputData(
		const void* npBuffer,	// 注意，如果希望传递的参数是一个指针本身，如：lpAnybody，应该如此调用SetInputData(&lpAnybody,sizeof(lpAnybody),ture/false);
		int niSize,
		bool nbVolatile=true	// true:此缓冲区是易失的，需要复制数据到内部缓冲; false 此缓冲区是非易失的，在消息发送和返回的过程中有效
		)=NULL;

	// 获得Input数据指针，注意，获得的指针仅在持有消息，并且没有发生改变时有效，一旦将消息发送出去，或者调用了消息的设定值的方法，都将导致该指针失效
	// 注意，此方法获得的指针并不一定同前一次调用SetInputData设定的指针相同
	virtual const void* __stdcall GetInputData(void)=NULL;

	// 获得输入缓冲区的大小
	virtual int __stdcall GetInputDataSize(void)=NULL;

	// 设置Output缓冲区，大多数的消息无需Output Buffer，如果需要消息返回大量数据的，应该Send这条消息，而不是Post它；如果确实需要Post这条消息，那么请参考下面的设置反馈消息的方法;
	// 如果选择Post这条消息，请千万保证设定的Output缓冲区不被修改和释放，以免消息的接受方对该缓冲区访问产生错误。
	// 本方法仅能在消息被发送之前调用
	virtual ERESULT __stdcall SetOutputBuffer(
		OUT void* npBuffer,
		IN int niSize	// 缓冲区总尺寸,不是数据大小,本函数被调用时,缓冲区应该是空的
		)=NULL;

	// 获得Output缓冲区指针，注意，获得的指针仅在持有该消息时有效；
	virtual void* __stdcall GetOutputBuffer(void)=NULL;

	// 获得Output缓冲区的大小
	virtual int __stdcall GetOutputBufferSize(void)=NULL;

	// 设置填入Output缓冲区的数据大小
	// 本方法仅能在消息被发送后调用，供消息的接收者调用
	virtual void SetOutputDataSize(int niSize)=NULL;
	// 获得Output缓冲区的数据大小
	// 本方法仅能在消息被发送后调用，供消息的接收者调用
	virtual int GetOutputDataSize(void)=NULL;

	// 设置反馈消息，当消息被目标对象处理完毕后，系统将自动生成一个新的消息返回给发送者，这个新的消息叫做反馈消息
	virtual ERESULT __stdcall SetResponseMessage(
		IN IEinkuiIterator* npReceiver,	// 接受反馈消息的目标
		IN void* npContext=NULL	// 设置上下文，供调用者设置和使用，当该消息被反馈时，传递给接收者
		)=NULL;

	// 消息接收者用于确定当前消息是否被Send而来，返回true,the message was sent from source element; false, the message was posted by the sender;
	virtual bool __stdcall IsSent(void)=NULL;
};

// 微件接口，微件是同一进程中区分对扩展功能部件的一种界定方式，一个扩展功能部件可能在系统中运行多个实例，每个实例都是一个微件；
// 同理，不同的功能部件的每一个运行实例都是独立的微件；微件具有类名和实例名两个名称，微件类就是一个独立的功能部件；
// 系统本身必须拥有一个基础的功能部件，只有唯一的实例，实例名一定是‘System’,这个微件也叫作‘系统微件’
__interface IXsWidget : public IBaseObject 
{
	// 获取微件所在安装目录，不带有后缀的'\'
	virtual const wchar_t* __stdcall GetWidgetDefaultPath(void)=NULL;

	// 该Widget的模块文件名，即实现此Widget的DLL名称，如"IdeaMain.dll"
	virtual const wchar_t* __stdcall GetModuleName(void)=NULL;

	// 获得本微件实例名，系统启动时建立的第一个微件叫做‘system’
	virtual const wchar_t* __stdcall GetInstanceName(void)=NULL;

	// 获得微件所在Module的工厂接口
	virtual IElementFactory* __stdcall GetDefaultFactory(void)=NULL;

	// 获得微件的实例专属配置；一个微件Module可以在一个进程中同时运行多个实例，与此同时，一台电脑上，每一个Windows用户帐户下，都可以运行一个Idealife进程；
	//		所以需要为微件的每一个运行态实例提供一个专属配置，它可以将需要长期保存的设置类信息存放到这个专属配置中
	virtual ICfKey* __stdcall GetInstanceConfig(void)=NULL;

	//获取微件用来存放临时文件的目录,返回的地址不包含\\结尾,同一个的微件的不同实例得到的是不同的文件夹路径
	virtual bool __stdcall GetInstanceTempPath(OUT wchar_t* npswPath,IN LONG nlBufferLen) = NULL;

	// 获得微件实例的Home Page
	virtual IEinkuiIterator* __stdcall GetHomePage(void)=NULL;

	// 退出Widget，此功能主要供SystemWidget调用关闭指定的Widget；对SystemWidget调用此方法，将导致整个XUI程序退出；
	// 不能假定调用此方法后指定关闭的Widget实例会立即退出，但调用此方法后应该防止再次调用指定widget实例的任何方法，避免给其发送其他消息；
	// 调用此方法后，Widget实例会在合适的执行状态及时退出；对于该Widget实例而言，此方法被调用后，它的HomePage将在适当的时候收到Destroy消息；
	virtual void __stdcall Close(void)=NULL;
};


//////////////////////////////////////////////////////////////////////////
// 操作位图的接口
//////////////////////////////////////////////////////////////////////////

__interface IEinkuiBitmap : public IBaseObject {

public:

	// 获取指定位置像素值, ARGB
	virtual ERESULT __stdcall GetPixel(IN DWORD x, IN DWORD y, DWORD &nPixel) = NULL;  

	// 获取位图基本信息
	virtual ERESULT __stdcall GetBmpInfo(OUT BITMAP* npBmpInfo) = NULL;	

	// 获取位图宽度
	virtual UINT __stdcall GetWidth(void) = NULL; 

	// 获取位图高度
	virtual UINT __stdcall GetHeight(void) = NULL; 

	// 设置延展线
	virtual void __stdcall SetExtendLine(IN LONG nlX,IN LONG nlY) = NULL;

	// 获取横向延展线
	virtual LONG __stdcall GetExtnedLineX(void) = NULL;

	// 获取纵向延展线
	virtual LONG __stdcall GetExtnedLineY(void) = NULL;

	// 缩放位图
	virtual ERESULT __stdcall Scale(UINT npWidth, UINT npHeight) = NULL; 

	// 获取IWICBitmap接口指针，使用者需要自行释放此接口指针
	virtual ERESULT __stdcall GetWICObject(OUT IWICBitmap** ppWicBitmap) = NULL;

	// 供系统内部调用，废弃Direct2D的设备相关资源
	virtual void __stdcall DiscardsBitmapResource(void) = NULL;

	// 复制出一个新的对象,复制出来的对象，需要使用者自己释放
	virtual IEinkuiBitmap* __stdcall DuplicateBitmap(void) = NULL;

public:
	// 获取指定区域的位图数据
	virtual bool __stdcall GetBitmapBuffer(IN D2D1_RECT_F ndSrcRect, OUT void* npRawData);

	// 保存成缩略图
	virtual bool __stdcall SaveAsThumbnail(IN LONG nuWidth, IN LONG nuHeight, IN D2D1_RECT_F ndSampleRect, const wchar_t* nswOutputFilePath);

};


//////////////////////////////////////////////////////////////////////////
// 元素迭代接口，XUI系统为每一个元素对象分配对应的迭代器
//////////////////////////////////////////////////////////////////////////
__interface IEinkuiIterator:public IBaseObject
{
	// 启动Iterator，准备接受消息，调用这个方法后，Element首先会收到EMSG_CREATE消息，这个方法通常在Element的实例化函数退出前调用
	virtual void __stdcall Start(void)=NULL;

	// 关闭一个元素，调用Close之后，此Iterator对应的对象会收到Destroy消息；当程序从Close返回之后，此Iterator以及它所指向的Element对象都不能在被使用，并且，无需对本Iterator调用Release方法
	virtual void __stdcall Close(void)=NULL;

	// 获得本Element所属的Etype
	virtual const wchar_t* __stdcall  GetType(void)=NULL;

	// 获得本Element的EID
	virtual ULONG __stdcall  GetID(void)=NULL;

	// 获得本迭代器对应的Element对象
	virtual IXsElement* __stdcall GetElementObject(void)=NULL;

	// 获得父对象
	virtual IEinkuiIterator* __stdcall  GetParent(void)=NULL;

	// 获得下一层的子对象的总数
	virtual int __stdcall GetSubElementCount(void);

	// 通过ZOder的排列次序获得子节点
	virtual IEinkuiIterator* __stdcall  GetSubElementByZOder(
		int niPos	// zero base index value to indicate the position in z-order array
		)=NULL;

	// 通过ID获得子节点
	virtual IEinkuiIterator* __stdcall  GetSubElementByID(ULONG nuEid)=NULL;

	// 询问某个Iterator在层次结构上是不是当前Iterator的祖先
	virtual bool __stdcall FindAncestor(const IEinkuiIterator* npIsAncestor)=NULL;

	// 给此元素发送一条消息，发送模式是Send
	// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimpleSendMessage方法
	virtual ERESULT __stdcall SendMessage(IEinkuiMessage* npMsg)=NULL;

	// 给此元素发送一条消息，发送模式是Post
	// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimplePostMessage方法
	virtual ERESULT __stdcall PostMessage(IEinkuiMessage* npMsg)=NULL;

	// 给此元素的父元素发送一条消息，发送的模式是FastPost
	// 消息发送后，发送者仍然需要释放
	// !!!注意!!! 仅用于发送者是本迭代器对应的元素之情况
	virtual ERESULT __stdcall PostMessageToParent(IEinkuiMessage* npMsg)=NULL;

	// 申请键盘焦点，如果该元素具有popup属性，也将被调整到合适的上层
	virtual ERESULT __stdcall SetKeyBoardFocus(void)=NULL;

	// 释放键盘焦点，如果nbShiftFocus==true，这将导致Tab Order的下一个键盘接收者获得焦点
	virtual void __stdcall ReleaseKeyBoardFocus(bool nbShiftFocus=false)=NULL;

	// 设置元素为活跃，如果本元素不具有EITR_STYLE_POPUP或者EITR_STYLE_ACTIVABLE风格，那么最低的一个具有EITR_STYLE_POPUP或者EITR_STYLE_ACTIVABLE风格的上层元素将被激活
	virtual void __stdcall SetActive(void)=NULL;

	// 设置Style，这将修改全部的Style选项为nuStyles
	virtual void __stdcall SetStyles(ULONG nuStyles)=NULL;

	// 修改Style，前一个参数是希望增加的Style，后一个参数是希望移除的Style，当前后两个参数中包括相同Style时，该Style不会被移除 
	virtual void __stdcall ModifyStyles(ULONG nuSet,ULONG nuClear=0)=NULL;

	// 读取Style
	virtual ULONG __stdcall GetStyles(void)=NULL;

	// 申请定时器，对于永久触发的定时器，需要注销
	virtual ERESULT __stdcall SetTimer(
		IN ULONG nuID,	  // 定时器ID
		IN ULONG nuRepeat,// 需要重复触发的次数，MAXULONG32表示永远重复
		IN ULONG nuDuration,	// 触发周期
		IN void* npContext//上下文，将随着定时器消息发送给申请者
		)=NULL;

	// 销毁定时器
	virtual ERESULT __stdcall KillTimer(
		IN ULONG nuID	  // 定时器ID
		)=NULL;

	// Hook目标，当前仅支持单层次的Hook，即，一个元素在同一时刻仅被一个元素Hook；试图Hook一个已经被Hook的元素时，将会返回失败ERESULT_ACCESS_CONFLICT
	virtual ERESULT __stdcall SetHook(
		IN IEinkuiIterator* npHooker,	// Hook请求者，一旦设置了Hook，本对象的所有消息（EMSG_HOOKED_MESSAGE不会被转发），都会先发送给Hooker处理，Hooker可以修改任意的消息，也可以阻止消息发送给本对象
		IN bool nbSet		// true to set ,false to remove
		)=NULL;

	// 获得Hooker，获取本元素被谁Hook
	virtual IEinkuiIterator* __stdcall GetHooker(void)=NULL;

	// 设置渲染增效器，增效器用于给某个元素和它的子元素提供特定的渲染，增效器可以选择Direct2D，Direct3D技术完善XUI系统的渲染
	// 同一个元素在同一时刻只能有一个增效器在工作；并且，通常增效器都是对其父元素发生作用
	// 返回ERESULT_ACCESS_CONFLICT表示多个增效器发生冲突；增效器设置，请在接收到EMSG_PREPARE_PAINT时处理，其他地方做设置，有可能导致严重错误
	virtual ERESULT __stdcall SetEnhancer(
		IN IEinkuiIterator* npEnhancer,
		IN bool nbEnable		// true 启用，false 取消
		)=NULL;

	// 获得增效器
	virtual IEinkuiIterator* __stdcall GetEnhancer(void)=NULL;

	//////////////////////////////////////////////////////////////////////////
	// 下面是所有与显示和位置相关的方法

	// 设置显示/隐藏
	virtual void __stdcall SetVisible(bool nbVisible)=NULL;

	// 读取显示/隐藏状态
	virtual bool __stdcall IsVisible(void)=NULL;

	// 设定整体透明度
	virtual void __stdcall SetAlpha(FLOAT nfAlpha)=NULL;

	// 读取整体透明度
	virtual FLOAT __stdcall GetAlpha(void)=NULL;

	// 设置平面坐标
	virtual void __stdcall SetPosition(FLOAT nfX,FLOAT nfY)=NULL;
	virtual void __stdcall SetPosition(const D2D1_POINT_2F& rPosition)=NULL;

	// 读取平面坐标
	virtual FLOAT __stdcall GetPositionX(void)=NULL;
	virtual FLOAT __stdcall GetPositionY(void)=NULL;
	virtual D2D1_POINT_2F __stdcall GetPosition(void)=NULL;
	virtual void __stdcall GetRect(D2D1_RECT_F& rRect)=NULL;

	// 设置可视区域
	virtual void __stdcall SetVisibleRegion(
		IN const D2D1_RECT_F& rRegion		// 基于相对坐标的可视区域，此区域之外不会显示本元素及子元素的内容；如果rRegion.left > region.right 表示取消可视区设置
		)=NULL;

	// 获取可视区域，返回false表示没有设置可是区域
	virtual bool __stdcall GetVisibleRegion(
		OUT D2D1_RECT_F& rRegion	// 返回可视区域，如果没有设置可视区域，则不会修改这个对象
		)=NULL;

	// 设置平面转角
	virtual void __stdcall SetRotation(
		FLOAT nfAngle,			// 角度单位 -359 -> +359度
		D2D1_POINT_2F ndCenter
		)=NULL;
	// 设置平面转角，以元素中心为旋转中心
	virtual void __stdcall SetRotation(
		FLOAT nfAngle			// 角度单位 -359 -> +359度
		)=NULL;

	// 读取平面转角
	virtual FLOAT __stdcall GetRotationAngle(void)=NULL;
	virtual D2D1_POINT_2F __stdcall GetRotationCenter(void)=NULL;
	virtual FLOAT __stdcall GetRotation(D2D1_POINT_2F& rCenter)=NULL;

	// 设置参考尺寸
	virtual void __stdcall SetSize(FLOAT nfCx,FLOAT nfCy)=NULL;
	virtual void __stdcall SetSize(const D2D1_SIZE_F& rSize)=NULL;

	// 读取参考尺寸
	virtual FLOAT __stdcall GetSizeX(void)=NULL;
	virtual FLOAT __stdcall GetSizeY(void)=NULL;
	virtual D2D1_SIZE_F __stdcall GetSize(void)=NULL;

	// 设置是否有效
	virtual void __stdcall SetEnable(bool nbSet)=NULL;

	// 读取是否有效
	virtual bool __stdcall IsEnable(void)=NULL;

	// 将本元素调整到同级窗口最高层
	virtual void __stdcall BringToTop(void)=NULL;

	// 将本元素在父元素的Z Order序列中，向下移动一位
	virtual bool __stdcall MoveDown(void)=NULL;

	// 将本元素在父元素的Z Order序列中，向上移动一位
	virtual bool __stdcall MoveUp(void)=NULL;

	// 整理所有子元素的叠放次序为原始设置（即配置文件制定的次序）
	virtual bool __stdcall ResetChildrenByDefualtZOrder(void)=NULL;

	// 重新设置本元素的Z Order的值，如果两个兄弟Element具有相同的Z Order将无法确定它们的先后循序，但系统本身运行不会出错
	virtual void __stdcall SetDefaultZOrder(const LONG nlDefaultZOrder)=NULL;

	// 获得默认的ZOrder值
	virtual LONG __stdcall GetDefaultZOrder(void)=NULL;

	//设置ToolTip，鼠标在本对象上悬停，将会自动显示的单行简短文字提示，鼠标一旦移开显示的ToolTip，它自动消失
	virtual void __stdcall SetToolTip(const wchar_t* nswTip)=NULL;

	//设置IME输入窗口位置，ndPosition是本元素局部坐标中的位置; 当原元素具有EITR_STYLE_DISABLE_IME属性时无效
	virtual void __stdcall SetIMECompositionWindow(D2D1_POINT_2F ndPosition)=NULL;

	// 从局部地址到世界地址
	virtual bool __stdcall LocalToWorld(const D2D1_POINT_2F& crLocalPoint,D2D1_POINT_2F& rWorldPoint);

	// 从世界地址转换为局部地址
	virtual bool __stdcall WorldToLocal(const D2D1_POINT_2F& crWorldPoint,D2D1_POINT_2F& rLocalPoint);

	// 对子元素建立绘制层，绘制层是一个改变子元素绘制时叠放次序的方法，通常的子元素按照从属关系排列为树形结构，绘制时也是按照树形结构先根遍历执行；
	//		引入绘制层技术后，子元素将在不同层上逐次绘制，同一个层的子元素，仍然按照从属关系的树形结构先根遍历；
	//		可以设定任意的绘制层，但不能大于max-ulong；鼠标落点的判定也收到绘制层的影响，层次高的元素首先被判定获得鼠标
	//		如果在一个已经设定了绘制层次的子树中的某个元素再次建立绘制层次，那么它的子树中的所有子元素将不受到上一个绘制层次的影响，按照新的层次工作；
	//		为了避免错误，请尽可能避免使用嵌套的绘制层次
	virtual ERESULT __stdcall CreatePaintLevel(LONG nlLevelCount)=NULL;

	// 获得子元素绘制层数量，返回0表示没有设置绘制层次
	virtual LONG __stdcall GetPaintLevelCount(void)=NULL;

	// 删除绘制层次设定
	virtual ERESULT __stdcall DeletePaintLevel(void
		//bool nbClearAll=true		// 清除掉所有子元素的绘制层次设定
		)=NULL;

	// 设定自身的绘制层次
	virtual ERESULT __stdcall SetPaintLevel(LONG nlLevel)=NULL;

	// 获得自身的绘制层次
	virtual LONG __stdcall GetPaintLevel(void)=NULL;

};

// 元素Style的值，注意，它们是可以被组合使用的，每一个值都是二进制的一位被设置
#define EITR_STYLE_NONE			0
#define EITR_STYLE_ACTIVABLE	ES_BIT_SET(1)		// 接受激活操作，具有PopUp的窗体一定具有本属性
#define EITR_STYLE_POPUP		ES_BIT_SET(2)|ES_BIT_SET(1)		// 弹出元素，它不受父元素设定的可视区域的限制，同时，同级的Popup元素的叠放次序会因为键盘和鼠标焦点的获得而自动调整，获得焦点的Popup元素将自动调整到最上层
#define EITR_STYLE_CENTER		ES_BIT_SET(4)		// 始终放置在父对象的参考尺寸中央
#define EITR_STYLE_TOP			ES_BIT_SET(5)		// 置于同级元素的最高层，多个TOP将按建立的先后颠倒叠放（后建在上），并且不受鼠标点击的影响
//#define EITR_STYLE_TOPDRAW		ES_BIT_SET(6)		// 需要在界面顶层执行绘制操作
#define EITR_STYLE_SNAPSHOT_HIDE	ES_BIT_SET(6)		// 在拍照时，不显示
#define EITR_STYLE_LAZY_UPDATE	ES_BIT_SET(11)		// 接受每秒5次的Update消息
#define EITR_STYLE_MOUSE		ES_BIT_SET(12)	// 接受鼠标消息
#define EITR_STYLE_DRAG			(ES_BIT_SET(13)|ES_BIT_SET(12))	// 接受拖拽消息
#define EITR_STYLE_ALL_DRAG		(ES_BIT_SET(14)|ES_BIT_SET(13)|ES_BIT_SET(12))	// 完全拖拽，拖拽起始于本元素的不支持拖拽的元素时，或者该元素返回拖拽失败时，拖拽消息会自动发送给本元素
#define EITR_STYLE_ALL_MWHEEL	(ES_BIT_SET(15)|ES_BIT_SET(12))	// 如果当前鼠标焦点处于某个子对象，该子对象不处理的全部鼠标Wheel消息都将被转发给本对象，具有popup属性的子对象和它的子对象都不会向上转发这条消息

#define EITR_STYLE_KEYBOARD  ES_BIT_SET(18)		// 接受键盘消息
#define EITR_STYLE_ALL_KEY	 ES_BIT_SET(19)|ES_BIT_SET(18)		// 代表自己的所有不接受键盘焦点的子对象接受键盘焦点
#define EITR_STYLE_COMMAND	ES_BIT_SET(20)	// 接受命令，命令见消息定义EMSG_COMMAND；如果当前键盘焦点不接受命令，系统将把命令发给它的上级接收者
#define EITR_STYLE_DISABLE_IME			ES_BIT_SET(22)	// 不使用IME

#define EITR_STYLE_WINDRAGDROP		ES_BIT_SET(25)	// 接受Windows的Drag&Drop行为，通过Xui系统提供的相应机制实现对Windows的支持
#define EITR_STYLE_XUIDRAGDROP		ES_BIT_SET(26)	// 接受XUI系统的Drag&Drop行为



//////////////////////////////////////////////////////////////////////////
// 元素对象的接口，元素的实现模块提供；元素对象接口，用于元素本身和元素管理器交互，外部使用元素迭代器与该元素交互
//////////////////////////////////////////////////////////////////////////
__interface IXsElement:public IBaseObject
{
	// 获得本元素的元素类型
	virtual const wchar_t* __stdcall  GetType(void)=NULL;

	// 在全局范围内验证此对象是否是nswType指定的类型，BaseObject提供的IsKindOf方法只能验证同一个模块内的对象
	virtual bool __stdcall  GlobleVerification(const wchar_t* nswType)=NULL;

	// 默认消息入口函数，用于接收输入消息
	virtual ERESULT __stdcall MessageReceiver(IEinkuiMessage* npMsg)=NULL;

	// 获得本元素的迭代器接口
	virtual IEinkuiIterator* __stdcall GetIterator(void)=NULL;

	// 获得Tab Order, -1 未设置
	virtual LONG __stdcall GetTabOrder(void)=NULL;

	// 获得Z Order，-1 未设置
	virtual LONG __stdcall GetZOrder(void)=NULL;
};


//////////////////////////////////////////////////////////////////////////
//元素类的描述方式
//////////////////////////////////////////////////////////////////////////
//元素类类型ID，命名方法，以ES_ETYPE_前缀，后面跟类型的名称；所有的扩展类型用ES_ETYPE_开头，紧跟着是扩展包名，然后是类型名，如: ES_ETYPE_STOCK_CHART，就可以理解为某个股票扩展模块的图表控件
//元素类的实例化数据结构，名称规则，ST_ELALC_前缀，后面跟数据类型的名称;如：ST_ELALC_ELEMENT
//元素类触发的消息，命名时，用EEVT_前缀
//元素类接受的访问消息，命名时，用EACT_前缀



// 基本类型，实现在XUI.DLL中，是所有元素对象的基础；它拥有ID和类型，能够判断类型，能够向系统注册和注销，能够接受消息
#define ES_ETYPE_ELEMENT L"Element"


// 触发的消息
#define EEVT_ELEMENT_MOUSE_ON		EMSG_DEFINE(EMSG_CLASS_ELEMENT,1,1)
#define EEVT_ELEMENT_MOUSE_OFF		EMSG_DEFINE(EMSG_CLASS_ELEMENT,1,2)
#define EEVT_ELEMENT_MOUSE_HOVER	EMSG_DEFINE(EMSG_CLASS_ELEMENT,1,3)	// 表示鼠标在这个控件上悬停，即停留0.4秒


//////////////////////////////////////////////////////////////////////////


// 分配器接口
__interface IXelAllocator:public IBaseObject
{
public:
	// 从配置文件中创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN ICfKey* npTemplete,			// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) = NULL;

	// 通过类名，创建对象,所创建的对象不能携带参数
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN const wchar_t* nswClassName,		// 类名
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) = NULL;


	// 加载一幅图片文件
	virtual IEinkuiBitmap* __stdcall LoadImageFile(
		IN wchar_t* nswRelativePathName,		//该值为相对路径时，起点路径为该模块Dll所在目录
		IN bool nbIsFullPath = false			//该的真表示nswRelativePathName为全路径，否则为相对路径
		) = NULL;

	// 从指定的PE文件中，提取位图资源，生成一个位图对象
	// 如果不能取道指定宽高的图标，则会用最大的图标生成位图
	virtual IEinkuiBitmap* __stdcall LoadImageFromPE(
		IN wchar_t *npPeFileName,				// 指定PE文件全路径
		IN int niDummy,							// 资源索引
		IN int niXSize = 64,					// 指定目标图标的宽度
		IN int niYSize = 64						// 指定目标图标的高度
		) = NULL;

	//从文字生成图片，STETXT_BMP_INIT定义见该接口定义上方
	virtual IEinkuiBitmap* __stdcall CreateImageByText(STETXT_BMP_INIT& rdBmpInit) = NULL;

	//从内存建立一个位图，程序返回后npRawData即可有调用者释放
	virtual IEinkuiBitmap* __stdcall CreateBitmapFromMemory(
		LONG nuWidth,					// 位图宽度
		LONG nuHeight,					// 位图高度
		LONG PixelSize,					// 像素的位宽，3 or 4
		LONG Stride,
		void* npRawData					// 位图原始数据
		) = NULL;


};


//////////////////////////////////////////////////////////////////////////
// 定义工厂类接口
// 所有工厂类都必须提供本接口，一个工厂类可以实例化多种个类型的对象

__interface IElementFactory : public IBaseObject
{

public:
	// 从配置文件中创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN ICfKey* npTemplete,			// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) = NULL;

	// 通过类名，创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN const wchar_t*		nswClassName,	// 类名
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) = NULL;

	// 获得与此Module配套的Profile文件接口，返回的接口当不再使用时，需要Release
	virtual IConfigFile* __stdcall GetTempleteFile(void)=NULL;

};


//////////////////////////////////////////////////////////////////////////
// 画刷接口
//////////////////////////////////////////////////////////////////////////

// 该接口抽象了D2D的Stroke和Brush概念，以及GDIPlus的Pen和Brush的概念
// 不管是画线还是填充，都用抽象后的IXuiBrush接口，具体的实现依据不同平台的实现代码而定


__interface IEinkuiBrush : public IBaseObject {


	// 基于D2D和GDIPlush平台的公共行为抽象
public:
	// 设置SOLID画刷的颜色
	virtual void __stdcall SetColor(IN D2D1_COLOR_F noColor) = NULL;

	// 设置渐变画刷的颜色
	virtual void __stdcall SetLinearBrushProperties(
		const D2D1_GRADIENT_STOP* npGradientStop, 
		ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
		) = NULL;

	// 设置线型 这个参数表暂时保留
	virtual bool __stdcall SetStrokeType(const D2D1_STROKE_STYLE_PROPERTIES &strokeStyleProperties, const FLOAT *dashes, UINT dashesCount) = NULL;

	// 设置线宽
	virtual void __stdcall SetStrokeWidth(IN float nfWidth) = NULL;

	// 获取现况
	virtual float __stdcall GetStrokeWidth() = NULL;

	// 供系统内部调用，废弃Direct2D的设备相关资源
	virtual void __stdcall DiscardsBrushResource(void) = NULL;

	// 复制出一个新的对象，复制出来的对象，需要使用者自己释放
	virtual IEinkuiBrush* __stdcall DuplicateBrush(void) = NULL;

	// 基于D2D的特殊抽象行为
	//public:
	//	// 获取画刷对象
	//	virtual ERESULT GetBrushObject(OUT ID2D1Brush** npBrushObject) = NULL;
	//
	//	// 获取描述线型的Stroke对象
	//	virtual ERESULT GetStrokeObject(OUT ID2D1StrokeStyle** npStrokeObject) = NULL;


	// 基于GDIPlush的特殊抽象行为
	//public:
	//	// 获取画刷对象
	//	virtual ERESULT GetBrushObject(OUT Brush** npBrushObject) = NULL;
	//
	//	// 获取画笔对象
	//	virtual ERESULT GetStrokeObject(OUT Pen** npPenObject) = NULL;




};


//////////////////////////////////////////////////////////////////////////
// 绘画资源的创建
//////////////////////////////////////////////////////////////////////////

__interface IEinkuiPaintResouce{

// 创建各类画刷的接口
public:

	// 创建渐变画刷
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType,
		D2D1_COLOR_F noColor
		) = NULL;

	// 渐变画刷时，需要传入多个颜色点
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType, 
		D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
		) = NULL;

};


//////////////////////////////////////////////////////////////////////////
// 引擎接口
//////////////////////////////////////////////////////////////////////////

// 该接口抽象了D2D和GDIPlus的绘图接口

__interface IEinkuiPaintBoard : public IBaseObject {


	// 封装位图的绘制操作
public:

	// 绘制一个图像到绘图板
	// 需要指定目标和源区域；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_RECT_F& rDestRect,	// 目标区域
		IN const D2D1_RECT_F& rSrcRect,	// 位图的采样区域
		IN IEinkuiBitmap* npBitmap,			// 待绘制的位图
		IN ULONG nuMethod,				// 采样方法，见下文定义
		IN float nfAlpha = 1.0f
		)=NULL;

	// 绘制一个图像到绘图板
	// 仅指定目标区域，采样区域是整个位图；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_RECT_F& rDestRect,	// 目标区域
		IN IEinkuiBitmap* npBitmap,			// 待绘制的位图
		IN ULONG nuMethod,				// 采样方法，见下文定义
		IN float nfAlpha = 1.0f
		)=NULL;

	// 绘制一幅图像到绘图板
	// 完全按照图像的大小绘制到目标；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_POINT_2F& rDestLeftTop,	// 目标区域的左上角
		IN IEinkuiBitmap* npBitmap,				// 待绘制的位图
		IN float nfAlpha = 1.0f
		)=NULL;


	// 封装画点画线的操作接口
public:

	// 画点线函数
	virtual ERESULT __stdcall DrawLine(
		IN const D2D1_POINT_2F&	noStartPoint,	// 起始点
		IN const D2D1_POINT_2F& noEndPoint,		// 结束点
		IN IEinkuiBrush* npBrush
		)=NULL;

	// 画多边形
	virtual ERESULT __stdcall DrawPlogon(
		IN const D2D1_POINT_2F*	noStartPoint,	// 顶点列表
		IN INT	niCount,
		IN IEinkuiBrush* npBrush 
		)=NULL;

	// 填充多边形
	virtual ERESULT __stdcall FillPlogon(
		IN const D2D1_POINT_2F*	noStartPoint,	// 顶点列表
		IN INT	niCount,
		IN IEinkuiBrush* npBrush 
		)=NULL;

	// 画椭圆
	virtual ERESULT __stdcall DrawEllipse(
		IN const D2D1_RECT_F& noRect,			
		IN IEinkuiBrush* npBrush
		)= NULL;

	// 填充椭圆
	virtual ERESULT __stdcall FillEllipse(
		IN const D2D1_RECT_F& noRect,
		IN IEinkuiBrush* npBrush
		)= NULL;


	// 画矩形
	virtual ERESULT __stdcall DrawRect(
		IN const D2D1_RECT_F& noRect,
		IN IEinkuiBrush* npBrush
		)= NULL;

	// 填充矩形
	virtual ERESULT __stdcall FillRect(
		IN const D2D1_RECT_F& noRect,
		IN IEinkuiBrush* npBrush
		)= NULL;


	// 画圆角矩形
	virtual ERESULT __stdcall DrawRoundRect(
		IN const D2D1_RECT_F& noRect,
		FLOAT radiusX,
		FLOAT radiusY,
		IN IEinkuiBrush* npBrush
		)= NULL;


	// 填充圆角矩形
	virtual ERESULT __stdcall FillRoundRect(
		IN const D2D1_RECT_F& noRect,
		FLOAT radiusX,
		FLOAT radiusY,
		IN IEinkuiBrush* npBrush
		)= NULL;	

	// 与D2D有关的一些特殊操作接口
public:

	// 获得Direct2D的RenderTarget，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 仅应该在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间执行绘制动作，不能在Prepare消息期间绘制，以免破坏渲染引擎的稳定
	virtual ID2D1RenderTarget* __stdcall GetD2dRenderTarget(void)= NULL;

	// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual ID2D1Factory* __stdcall GetD2dFactory(void)= NULL;

	// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual IWICImagingFactory* __stdcall GetWICFactory(void)= NULL;

	// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual IDWriteFactory* __stdcall GetDWriteFactory(void)= NULL;

	// 获得当前的D2d绘制用，局部坐标到世界坐标的转换矩阵
	virtual const D2D1::Matrix3x2F& __stdcall GetCurrent2DWorldMatrix(void)= NULL;

	// 获得当前可视区设置，可视区用世界坐标描述，如果D2dTarget被切换，将给出的是切换后的Target对应的世界坐标
	// 返回ERESULT_SUCCESS当前存在可视区设置，并且成功取得，ERESULT_UNSUCCESS不存在可视区设置，其他值表示错误
	virtual ERESULT __stdcall GetVisibleRegion(
		OUT D2D1_RECT_F* npRegion
		)= NULL;

	// 对于直接访问D3d和D2d对象的元素，使用本接口向系统汇报设备错误，系统返回ERESULT_SUCCESS表示可以继续执行，返回值满足宏ERESULT_FAILED则中止继续执行
	virtual ERESULT __stdcall CheckDeviceResult(
		IN HRESULT nhrDeviceResult
		)= NULL;

	// 获得当前的TickCount
	virtual ULONG __stdcall GetCurrentTickCount()= NULL;

	// 获得当前帧率
	virtual ULONG __stdcall GetCurrentFps(void)= NULL;

	// 获得当前的渲染序列号，绘制序列号是用来协调和计算当前的渲染次数，每执行一次渲染该序号加一，某些情况下，元素可以会被要求绘制2次，计数达到最大值后会从0开始
	virtual ULONG __stdcall GetRenderNumber(void)= NULL;

};
 //////////////////////////////////////////////////////////////////////////
 // 位图绘制采样方法，属于下面的IXuiPaintBoard::DrawBitmap
#define ESPB_DRAWBMP_NEAREST D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR //邻近算法，适合压缩图像
#define ESPB_DRAWBMP_LINEAR D2D1_BITMAP_INTERPOLATION_MODE_LINEAR	// 线性插值，适合方法图像；对于希望达到较好的放大效果，请使用IEinkuiBitmap的放大算法方法
#define ESPB_DRAWBMP_EXTEND 0xF0000001	// 延展方法，通过指定延展线，重叠复制延展线的象素以扩展不足部分；这个方法适合制作2D图形界面的背景等；





































#ifdef __cplusplus
extern "C" {
#endif

// XUI启动函数，如果希望修改DPI，请务必在调用此函数前调用Win API SetProcessDPIAware
int __stdcall EinkuiStart(
	IN const wchar_t* nswModulePath,	// System Widget的实现模块的文件路径名
	IN const wchar_t* nswHomeTempleteName,	// System Widget的Home Page的templete key的名字
	IN const wchar_t* nswClassesRegPath,	// Xui注册类所在的注册表路径，如:Software\\Lenovo\\Veriface5\\PublicClasses
	IN ULONG nuAutoRotate=0,				// 非零支持自动旋转
	IN PEINKUI_CUSTOMDRAW_SETTINGS npCustomDraw=NULL,	// 自绘Eink
	IN const wchar_t* nswWndTittle = NULL		// 主窗口标题
	);

// 获得XUI系统接口，绝对不会失败，无需判断返回值是否为NULL
IEinkuiSystem* __stdcall EinkuiGetSystem(void);


#ifdef __cplusplus
}
#endif

#endif//_XUI_H_