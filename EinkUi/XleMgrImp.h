#ifndef _ELEMGRIMP_H_
#define _ELEMGRIMP_H_
/*
	本文件定义元素管理器实现类

*/
#include "BpTree.h"
#include "XsSysElement.h"

class CXelManager;
class CXuiIterator;

#define ELMGR_MAX_FREE_MESSAGE 64
#define ELMGR_HOVER_DURATION 800
#define ELMGR_TIP_DURATION 5000
#define ELMSG_MAX_MOUSE_TRACK 36


#pragma pack(4)


// 操作线程唤醒消息，无需应答
#define EMSG_WAKE_UP EMSG_DEFINE(LMSG_TP_SYSTEM,100,1)
// input none
// output none

// 系统退出消息，发送给元素管理器，通知退出
#define EMSG_QUIT_XUI EMSG_DEFINE(LMSG_TP_SYSTEM,100,2)
// input none
// output none

// 系统屏幕旋转
#define EMSG_SCREEN_ROTATION EMSG_DEFINE(LMSG_TP_SYSTEM,100,5)
// input ULONG as screen-orientation
// output NONE

// 渲染画板
#define EMSG_SYSTEM_RENDER EMSG_DEFINE(LMSG_TP_SYSTEM,100,10)
// input ULONG : 0 render,1 stop discarding render message
// output none

// Windows 交互线程回调完成
#define EMSG_WINCALL_COMPLETED EMSG_DEFINE(LMSG_TP_SYSTEM,101,1)
// input ULONG[2]  CallBackID and ERESULT
// output none
// result na

// 模态对话完成
#define EMSG_MODAL_COMPLETED EMSG_DEFINE(LMSG_TP_SYSTEM,101,2)
// input ULONG，ERESULT produce by the MODAL
// output none
// result na

// 锁住操作线程
#define EMSG_OPTHREAD_LOCK EMSG_DEFINE(LMSG_TP_SYSTEM,101,3)
// input HEVENT[2],the first the OPThread is waiting to proceed
struct _STEMS_OPT_LOCK {
	HANDLE OptToRelease;
	HANDLE OptToWait;
};
typedef _STEMS_OPT_LOCK STEMS_OPT_LOCK, *PSTEMS_OPT_LOCK;
// output 
// result 

// 关闭Widget
#define EMSG_CLOSE_WIDGET EMSG_DEFINE(LMSG_TP_SYSTEM,101,10)
// input IXsWidget* the pointer to the widget to close
// output none 
// result na

// 申请销毁元素，Sender就是申请销毁的元素
#define EMSG_APPLY_DESTROY  EMSG_DEFINE(LMSG_TP_SYSTEM,102,1)
// input none
// output none
// result na



// 获得键盘焦点
#define EMSG_SET_KEYBOARD_FOCUS EMSG_DEFINE(LMSG_TP_SYSTEM,105,1)
// input IEinkuiIterator*, Element to get keyboard focus
// output none
// result na

// 释放键盘焦点
#define EMSG_RELEASE_KEYBOARD_FOCUS EMSG_DEFINE(LMSG_TP_SYSTEM,105,2)
// input 
struct _ST_RELEASE_KEYFOCUS {
	IEinkuiIterator* CrtFocus;	// Element to release keyboard focus
	bool ShiftTab;	// 转移到下一个接受键盘的目标，如同按下Tab键
};
typedef _ST_RELEASE_KEYFOCUS ST_RELEASE_KEYFOCUS, *PST_RELEASE_KEYFOCUS;
// output none
// result na

// 设置激活
#define EMSG_SET_ACTIVE	EMSG_DEFINE(LMSG_TP_SYSTEM,105,5)
// input IEinkuiIterator*
// output none
// result na


//////////////////////////////////////////////////////////////////////////
// 鼠标转发
#define EMSG_MOUSE_FORWARD		EMSG_DEFINE(LMSG_TP_WIN_INPUT,1,1)
//intput
struct _STELEMGR_WINMSG_FORWARD {
	ULONG WinMsgID;
	WPARAM wParam;
	LPARAM lParam;
};
typedef _STELEMGR_WINMSG_FORWARD STELEMGR_WINMSG_FORWARD, *PSTELEMGR_WINMSG_FORWARD;
//output none

//////////////////////////////////////////////////////////////////////////
// 键盘消息转发
#define EMSG_KEYBOARD_FORWARD		EMSG_DEFINE(LMSG_TP_WIN_INPUT,1,2)
// input STELEMGR_WINMSG_FORWARD
// output none
// result na

//////////////////////////////////////////////////////////////////////////
// 允许处理输入消息LMSG_GET_TYPE() == LMSG_TP_WIN_INPUT
#define EMSG_INPUT_ENABLE			EMSG_DEFINE(LMSG_TP_WIN_INPUT,2,1)
// INPUT NONE
// OUTPUT NONE
// RESULT NA

// 修改此代码，将触屏消息按照鼠标消息转发，触屏逻辑未完整，Ax.2017.08.16
////////////////////////////////////////////////////////////////////////////
//// 触摸屏消息转发
//#define EMSG_TOUCH_FORWARD			EMSG_DEFINE(LMSG_TP_WIN_INPUT,1,4)
//// input STEMS_TOUCH
//// output none
//// result na

// 获取一个当前的画板现状
#define EMSG_MAIN_GET_CURRENT_BITMAP EMSG_DEFINE(LMSG_TP_SYSTEM,103,2)
// intput LPARAM 同鼠标消息
// output HBITMAP
// result na


#pragma pack()


#define ES_ETYPE_MANAGER L"ElementManager"


typedef bplustree<IEinkuiIterator*> TEIteratorVerification;

typedef cmmStack<int, 32, 16>  TElEnumIndexStack;


typedef cmmQueue<IEinkuiMessage*, 64, 64> TElMessageQueue;
typedef cmmStack<IEinkuiMessage*, ELMGR_MAX_FREE_MESSAGE, ELMGR_MAX_FREE_MESSAGE> TElMessageStack;

class CElMouseFootPrint {
public:
	D2D1_POINT_2F Point;
	ULONG KeyFlag;		// 此时其他的相关按钮的状况，设置表示为按下，可以是它们的任意组合，MK_LBUTTON/MK_RBUTTON/MK_MBUTTON/MK_XBUTTON1/MK_XBUTTON2/MK_CONTROL/MK_SHIFT
	ULONG TickCount;
	void operator=(const CElMouseFootPrint& src) {
		Point.x = src.Point.x;
		Point.y = src.Point.y;
		KeyFlag = src.KeyFlag;
		TickCount = src.TickCount;
	}
};
typedef cmmQueue<CElMouseFootPrint, ELMSG_MAX_MOUSE_TRACK, 16> TElMouseTrace;

class CElMouseTestState {
public:
	D2D1_POINT_2F Point;
	IEinkuiIterator* mpElement;
	LONG mlCrtLevel;
	bool mbSeekInLevels;
	void operator=(const CElMouseTestState& src) {
		Point.x = src.Point.x;
		Point.y = src.Point.y;
		mpElement = src.mpElement;
		mlCrtLevel = src.mlCrtLevel;
		mbSeekInLevels = src.mbSeekInLevels;
	}
};


//////////////////////////////////////////////////////////////////////////
// 元素管理器
DECLARE_BUILTIN_NAME(CXelManager)
class CXelManager : public cmmBaseObject<CXelManager, IXelManager, GET_BUILTIN_NAME(CXelManager)>
{
	friend class CEinkuiSystem;
public:
	CXelManager();
	~CXelManager();

	DEFINE_CUMSTOMIZE_CREATE(CXelManager, (), ())


		// 向系统管理器注册一个Element，返回迭代器对象；失败返回NULL
		// 成功返回的接口对象
		virtual IEinkuiIterator* __stdcall  RegisterElement(
			IN IEinkuiIterator* npParent,	// 父元素的迭代器
			IN IXsElement* npElement,	// 待注册的子元素
			IN ULONG nuEID = 0	// 本元素的EID，在同一个父元素下，各子元素的EID必须唯一，如果不关心EID，请设置=0，系统自动分配
		);

	// 向系统管理器注销一个Element，此功能仅应该被待注销Element自身或者XUI系统调用；这个方法已经废弃
	virtual ERESULT __stdcall UnregisterElement(
		IN IEinkuiIterator* npElementIterator	// 该元素对应的迭代器
	);

	// 验证一个Iterator是否有效，返回ERESULT_SUCCESS有效，返回ERESULT_ITERATOR_INVALID无效
	virtual ERESULT __stdcall VerifyIterator(
		IN IEinkuiIterator* npIterator	// 迭代器
	);

	// 在对象管理器中查找一个Element，返回该Element的迭代器对象；失败返回NULL
	// 成功返回的接口对象，不使用时需要释放； 如果经常需要通过元素获得它的注册迭代器，请保存迭代器的指针，因为调用本方法使用全树遍历查找获取迭代器对象，耗时较大；
	virtual IEinkuiIterator* __stdcall FindElement(
		IN IXsElement* npElement
	);

	// 获得根元素；如果是为了给对象管理器发送消息，也可以直接使用NULL指针表示对象管理器
	// 成功返回的接口对象，不使用时需要释放
	virtual IEinkuiIterator* __stdcall GetRootElement(void);

	// 获得菜单页，所有的菜单都建立在这个页
	virtual IEinkuiIterator* __stdcall GetMenuPage(void);

	// 获得ToolTip平面，这个最高的页
	virtual IEinkuiIterator* __stdcall GetToolTipPage(void);

	// 获得桌面元素；桌面元素在启动XUI引擎时指定，桌面元素实现具体应用的全局功能，如：Idealife的全局应用由"Idealife"元素类的实例提供，
	// 通过给它发Idealife的应用消息执行Idealife的系统调用
	// 成功返回的接口对象，不使用时需要释放
	virtual IEinkuiIterator* __stdcall GetDesktop(void);

	// 重新设定父元素，nbZTop==true设置于Zoder顶层，false设置在底层
	virtual ERESULT __stdcall SetParentElement(IEinkuiIterator* npParent, IEinkuiIterator* npChild, bool nbZTop);

	// 分配一个消息
	// 消息发送后，发送者仍然需要释放
	virtual IEinkuiMessage* __stdcall  AllocateMessage(void);

	// 分配一个消息，初始化相关参数
	// 消息发送后，发送者仍然需要释放
	virtual IEinkuiMessage* __stdcall AllocateMessage(
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区
		IN int niInputSize,	// 输入数据的大小
		OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
		IN int niOutputSize,	// 输出缓冲区大小
		IN bool nbInputVolatile = true	// 输入缓冲区是否是易失的，参见IXuiMessage::SetInputData获得更多信息
	);

	// 给指定元素发送一条消息，发送模式是Send
	// 消息发送后，发送者仍然需要释放
	virtual ERESULT __stdcall SendMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IEinkuiMessage* npMsg
	);

	// 给指定元素发送一条消息，发送模式是Post
	// 消息发送后，发送者仍然需要释放
	virtual ERESULT __stdcall PostMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IEinkuiMessage* npMsg,
		IN ULONG nuPostType = EMSG_POSTTYPE_NORMAL	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
	);

	// 简单地给指定元素发送一条消息，发送模式是Send；此函数的返回成功就是消息处理的返回值，错误的原因就不一定是消息处理的返回值，可能是消息发送失败
	virtual ERESULT __stdcall SimpleSendMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区
		IN int niInputSize,	// 输入数据的大小
		OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
		IN int niOutputSize	// 输出缓冲区大小
	);

	// 简单地给指定元素发送一条消息，发送模式是Post；无法获得消息处理的返回值；此函数的返回值仅表示消息是否被成功填入消息队列
	virtual ERESULT __stdcall SimplePostMessage(
		IEinkuiIterator* npDestElement,	// 接收消息的目标元素
		IN ULONG nuMsgID,	// 消息编码
		IN const void* npInputBuffer,	// 输入数据的缓冲区
		IN int niInputSize,	// 输入数据的大小
		IN ULONG nuPostType = EMSG_POSTTYPE_NORMAL	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
	);

	// 枚举全部元素，每当发现一个Element时调用枚举请求者提供的ElementEnter函数；当一个元素没有子元素时，将调用提供的ElementLeave
	// 因为根节点是XUI系统的虚拟对象，枚举不会触及根节点
	virtual ERESULT __stdcall EnumAllElement(
		bool nbReverse,				// 反向，指的是枚举子节点时，按照Zorder的顺序枚举，或者按照Zorder的逆序枚举
		IBaseObject* npApplicant,	// 发起对象
		ERESULT(__stdcall IBaseObject::*ElementEnter)(IEinkuiIterator* npRecipient),//如果返回ERESULT_ENUM_CHILD继续枚举；返回ERESULT_STOP_ENUM_CHILD或任意其他值将停止枚举此元素的此元素的子元素
		ERESULT(__stdcall IBaseObject::*ElementLeave)(IEinkuiIterator* npRecipient) //返回值无意义
	);

	// 增加Iterator的引用，由于XUI的客户程序可能会遗漏对Iterator的释放和引用操作，所以默认的Iterator->AddRef()和Iterator->Release()方法是假的，并不会产生实际的调用，但Element被Close后，对应的Iterator一定
	// 会被释放；在本接口中提供了真实的引用和释放的方法操作Iterator对象，切记要谨慎操作，过多地释放将会导致XUI崩溃；
	// 增加Iterator引用
	virtual int __stdcall AddRefIterator(IEinkuiIterator* npIterator);

	// 释放Iterator
	virtual int __stdcall ReleaseIterator(IEinkuiIterator* npIterator);

	// 获得鼠标焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
	// 因为鼠标焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
	virtual IEinkuiIterator* __stdcall GetMouseFocus(void);

	// 获得键盘焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
	// 因为键盘焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
	virtual IEinkuiIterator* __stdcall GetKeyboardFocus(void);

	// 重置拖拽起点，仅当系统正处于拖拽行为中时，可以将拖转转移给他人
	// 如果试图转移到的目标元素不能支持拖拽行为，当前的拖拽行为也会终止，当前拖拽的目标元素会收到Drag_end消息
	virtual ERESULT __stdcall ResetDragBegin(IEinkuiIterator* npToDrag);

	// 清理输入消息LMSG_GET_TYPE(MsgId) == LMSG_TP_WIN_INPUT
	virtual void __stdcall CleanHumanInput(bool nbStallInput = false);

	// 注册快捷键，当快捷键被触发，注册快捷键的Element将会受到；
	// 如果普通按键组合（不包含Alt键)按下的当时，存在键盘焦点，按键消息会首先发送给键盘焦点，如果焦点返回ERESULT_KEY_UNEXPECTED才会判断是否存在快捷键行为
	virtual bool __stdcall RegisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册的元素，将有它收到注册是快捷键消息
		IN ULONG nuHotKeyID,	// 事先定义好的常数，用来区分Hotkey；不能出现相同的ID，试图注册已有的Hotkey将会失败
		IN ULONG nuVkNumber,	// 虚拟键码
		IN bool nbControlKey,	// 是否需要Control组合
		IN bool nbShiftKey,		// 是否需要Shift组合
		IN bool nbAltKey,		// 是否需要Alt组合
		IN IEinkuiIterator* npFocus = NULL	// 指定焦点范围，仅当该元素及其子元素获得键盘焦点时，才会触发本次注册的快捷键;
		// 使用NULL作为参数而不指定焦点范围，则无论键盘焦点在何处都能够收到注册的快捷键的消息；
	);

	// 注销快捷键
	virtual bool __stdcall UnregisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册者
		IN ULONG UnuKeyNumber
	);

	// 锁定XUI元素树，可以重入，但要和UnlockIterators配对
	void LockIterators(void);

	// 解除锁定XUI元素树
	void UnlockIterators(void);

	// 供消息对象本身调用，释放控制
	void ReleaseMessage(IEinkuiMessage* npMsg);

	// 启动一个Iterator的消息接收
	void StartMessageReceiver(IEinkuiIterator* npIterator);

	// 申请键盘焦点，如果该元素具有popup属性，也将被调整到合适的上层
	ERESULT ApplyKeyBoard(IEinkuiIterator* npIterator);

	// 释放键盘焦点，这将导致Tab Order的下一个键盘接收者获得焦点
	void ReleaseKeyBoard(PST_RELEASE_KEYFOCUS npRelease);

	LONG GetProbeMode(void) {
		return mlProbeMode;
	}

	// 获得键盘焦点，需要释放
	CXuiIterator* InnerGetKeyFocus(void);

	// 获得鼠标焦点，需要释放
	CXuiIterator* InnerGetMouseFocus(void);

	// 获得活跃元素
	CXuiIterator* GetActiveElement(void);

	// 设置Active Popup元素
	ERESULT AssignActivation(IEinkuiIterator* npToSet);

	D2D1_POINT_2F GetCurrentMousePosition(void) {
		D2D1_POINT_2F ldPos;
		if (moMouseTrace.Size() != 0)
		{
			ldPos = moMouseTrace.Back().Point;
		}
		else
		{
			ldPos.x = 0.0f;
			ldPos.y = 0.0f;
		}

		return ldPos;
	}

	// 销毁元素
	ERESULT DestroyElement(
		IN IEinkuiIterator* npElementIterator	// 该元素对应的迭代器
	);

	// 发送命令到合适的元素
	ERESULT SendCommand(nes_command::ESCOMMAND neCmd);

	// 执行推出XUI操作
	void EndMessageQueue(void) {
		mbExitXui = true;

		moFastMessages.Clear();
		moNormalMessages.Clear();
	}

	void SetPositionInPanel(
		ULONG x,
		ULONG y
	) {
		mdTopLeftInPanel.x = (FLOAT)x;
		mdTopLeftInPanel.y = (FLOAT)y;
	}

protected:
	// 根元素迭代器
	CXuiIterator moIteratorRoot;
	CEleMgrProxy moElementRoot;
	CExclusiveAccess moIteratorLock;

	// 保存所有迭代器，用于验证
	TEIteratorVerification moIteratorVerification;

	// 桌面元素
	CXuiIterator* mpDesktopIterator;

	// 消息队列
	//CXuiMessageQueue moMessages;
	CXuiMessageQueue moFastMessages;
	CXuiMessageQueue moNormalMessages;
	// 	CXuiMessageQueue moReduceMessages;
	HANDLE mhMessageInserted;
	LONG mlMsgAllocated;
	bool mbExitXui;
	volatile LONG mlCleanHumanInput;

	// 空闲的消息
	TElMessageStack moFreeMessagePool;
	CExclusiveAccess moFreeMessageLock;

	// 鼠标控制器
	CXuiIterator* mpMouseFocus;	// 当前鼠标焦点
	CXuiIterator* mpActivatedElement;
	CExclusiveAccess moMFocusLock;
	bool mbHoverTest;
	bool mbTopDrawTest;
	TElMouseTrace moMouseTrace;
	CXuiIterator* mpMouseMoveOn;	// 鼠标可能移动到的新目标，由落点检测程序返回，但，它不一定是鼠标焦点的获得者
	LONG mlMouseMoveOnLevel;
	D2D1_POINT_2F mdPointRelative;	// 与上值对应的鼠标位于其相对坐标系的位置
	cmmStack<CElMouseTestState, 32> moPointToTest;
	bool mbTrackMouse;
	D2D1_POINT_2F mdTopLeftInPanel;	// 在Eink Panel上的左上角

	// Tip
	CXuiIterator* mpTipOwner;
	ULONG muTickOnTipShow;
	bool mbTipHide;

	// 拖拽
	bool mbDragging;
	bool mbXuiDragDrop;
	ULONG muKeyWithDragging;	// 通过哪个按键拖拽
	D2D1_POINT_2F mdDragFrom;	// 开始拖拽时的坐标
	STMS_EDRGDRP_REQUEST mdDrgdrpRequest;
	CXuiIterator* mpDraggingElement;
	CXuiIterator* mpDragMsgReceiver;
	CXuiIterator* mpRecentAskedDrop;
	CXuiIterator* mpLastAccepter;


	// 键盘控制
	CXuiIterator* mpKbFocus;	// 当前键盘焦点
	CExclusiveAccess moKFocusLock;


	// 检查模式变量
	LONG mlProbeMode;	// 是否进入检查模式
	LONG mlProbePass;	// 检查密码通过字数，‘esi'


	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	// 从消息队列提取一条消息，并且分发处理
	ERESULT ProcessNextMessage(
		IEinkuiMessage* npMsg = NULL		//不为空，表示直接处理这条消息，而不从消息队列读取
	);

	// 等待消息旗语，内部将调用WaitForSingleObject等待旗语，返回值同WaitForSingleObject一致
	ULONG WaitMessageReach(ULONG nuMilliseconds);

	// 设置根元素的Widget属性，即System Widget
	void SetRootWidget(IXsWidget* npWidget);

	// 改变键盘焦点
	void ChangeKeyFocus(CXuiIterator* npNewFocus);

	// 改变鼠标焦点
	void ChangeMouseFocus(CXuiIterator* npNewFocus);

	// 从npSeekFrom开始查找可以激活的元素，其他类不要直接调用
	void ChangeActiveElement(CXuiIterator* npSeekFrom);

	// 进入模态对话状态
	void EnterModal(void);

	// 处理鼠标输入转发
	ERESULT OnMsgMouseForward(const PSTELEMGR_WINMSG_FORWARD npMouseInput);

	// 处理键盘输入转发
	ERESULT OnMsgKeyboardForward(const PSTELEMGR_WINMSG_FORWARD npKeyStrike);

	// 允许输入
	ERESULT OnMsgEnalbeInput(void);

	// 处理Eink Touch输入转发
	//ERESULT OnMsgEinkTouchForward(const PSTEMS_TOUCH npEinkTouchInput);

	// 鼠标落点检测预处理
	ERESULT __stdcall EnterForMouseTest(IEinkuiIterator* npRecipient);

	// 鼠标落点检测后处理
	ERESULT __stdcall LeaveForMouseTest(IEinkuiIterator* npRecipient);

	// 发送鼠标按键消息
	__inline void SendMouseButtonPressed(IEinkuiIterator* npFocus, bool nbPressed, ULONG nuActKey, ULONG nuKeyFlag, ULONG nuTickCount, const D2D1_POINT_2F& rPosition);

	// 发送鼠标按键双击消息
	void SendMouseButtonDbClick(IEinkuiIterator* npFocus, ULONG nuActKey, ULONG nuKeyFlag, ULONG nuTickCount, const D2D1_POINT_2F& rPosition);

	// 检查是否启动拖拽并且发送开始拖拽消息
	__inline void DetectMouseDragBegin(CXuiIterator* npFocus, ULONG nuActKey, ULONG nuKeyFlag, const D2D1_POINT_2F& rPosition);

	// 将目标元素从子孙到祖先全部调整到Zorder最高层
	__inline void BringFocusedPopupToTop(CXuiIterator* npFocus);

	// 检查是否会获得键盘焦点，如果目标恰好是当前键盘焦点则直接返回，否则将释放当前键盘焦点
	__inline void DetectKeyboardFocus(CXuiIterator* npToDetect);

	// 发送键盘消息给目标，如果目标反馈ERESULT_UNEXPECTED_KEY，则，向上传递不支持的键盘按键信息，到Popup元素为止
	ERESULT KeyStrike(CXuiIterator* npKeyFocus, const PSTEMS_KEY_PRESSED npStrike);

	//// 将Key消息转换为Command
	//bool KeyToCommand(const PSTEMS_KEY_PRESSED npStrike);

	// 键盘焦点转移到下一个对象
	bool ShiftKeyBoardFocus(CXuiIterator* npKeyFocus);

	// 询问是否支持Drop
	__inline void DropDetect(CXuiIterator* npToDetect);

	// 执行Drop down
	__inline void TryDropDown(CXuiIterator* npToTry);

	// 递归销毁某个元素，首先销毁父元素，后销毁子元素
	void DestroyElementSubTree(IEinkuiIterator* npToDestroy);

	// 发送Mouse wheel消息，如果当前鼠标焦点不接受这条消息，那么就判断在到达第一个Popup(包括第一个popup)之前是否有EITR_STYLE_ALL_MWHEEL的元素，需要接受Mouse Wheel消息
	void TransferMouseWheel(CXuiIterator* npMouseFocus, STEMS_MOUSE_WHEEL& rInfor);

	__inline void CalculateMouseMoving(IEinkuiIterator* npOwner, const D2D1_POINT_2F& rCrtPos, const D2D1_POINT_2F& rLastPos, D2D1_POINT_2F& rResult);

	// 在对象上检测会计按键
	bool DetectHotKey(CXuiIterator* npHost, CXuiHotkeyEntry& rToFind);

	// 快捷键，返回false表示不是快捷键
	bool HotKeyProcessor(CXuiIterator*npKeyFocus, const PSTELEMGR_WINMSG_FORWARD npKeyStrike);

};

#define EELMGR_WINCAP_MOVING	ES_BIT_SET(1)
#define EELMGR_WINCAP_LB		ES_BIT_SET(2)
#define EELMGR_WINCAP_MB		ES_BIT_SET(3)
#define EELMGR_WINCAP_RB		ES_BIT_SET(4)
#define EELMGR_WINCAP_ALL		0xFFFF























#endif//_ELEMGRIMP_H_