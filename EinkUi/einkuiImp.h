/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _XUIMAIN_H_
#define _XUIMAIN_H_
#include "ImeContext.h"
#include "XsThreadBlock.h"

class CXD2dEngine;
class CXelManager;
class CXuiIterator;
class CXsWidget;
class CXelAllocator;
class CEleMgrProxy;

struct _STES_START_UP {
	D2D1_SIZE_F Dpi;
	const wchar_t* ModulePathName;		// System Widget的实现模块的文件路径名
	const wchar_t* HomeTempleteName;	// System Widget的Home Page的templete key的名字
	const wchar_t* ClassesRegPath;
	const wchar_t* WindowTitle;
	HANDLE WaitingCaller;
	ERESULT Result;
	PEINKUI_CUSTOMDRAW_SETTINGS CustomDraw;
	unsigned long AutoRotate;
};
typedef _STES_START_UP STES_START_UP,* PSTES_START_UP;



// 定时器
struct _STES_TIMER{
	ULONG muID;
	ULONG muBaseTick;	// the tickcount when timer was set
	ULONG muRecentTick;	// the tickcount when the timer was kicked recently
	ULONG muEndTick;	// 1/1000 second same as windows' tickcount
	ULONG muDuration;	// 1/1000 second
	ULONG muRepeat;		// 需要重复触发的次数，MAXULONG32表示永远重复
	ULONG muKicked;		// 已经触发的次数
	void* mpContext;	// 用户指定的上下文
	CXuiIterator* mpIterator;	// 定时器的申请者
};
typedef _STES_TIMER STES_TIMER,* PSTES_TIMER;

class CEsTimerCriterion	// 默认的判断准则
{
public:
	bool operator () (const PSTES_TIMER Obj1,const PSTES_TIMER Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1->muEndTick <Obj2->muEndTick || Obj1->muEndTick == Obj2->muEndTick && Obj1 < Obj2);
	}
};

typedef cmmSequence<PSTES_TIMER,CEsTimerCriterion> TEsTimerSequence;

typedef cmmVector<CXsWidget*,64,64> TEsWidgetVector;

class CEsThreadNode{
public:
	ULONG muThreadID;
	IXsWidget* mpOwnerWidget;
	void operator=(const class CEsThreadNode& src){
		muThreadID = src.muThreadID;
		mpOwnerWidget = src.mpOwnerWidget;
	}
};

class CEsThreadNodeCriterion	// 默认的判断准则
{
public:
	bool operator () (const CEsThreadNode& Obj1,const CEsThreadNode& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.muThreadID < Obj2.muThreadID);
	}
};

// 按照ID排序的线程队列
typedef cmmSequence<CEsThreadNode,CEsThreadNodeCriterion> TEsThreadSequence;

// 定时器ID
#define ELMGR_TIMERID_RENDER 1	// 绘制
#define ELMGR_TIMERID_LAZY 2	// 慢刷新


// Windows User 线程回调用Windows消息名称
#define ES_WINUI_CUSTOM_MSG L"{7F139C67-035E-43D7-A18C-D4ADCEF08B2E}"

// Windows User 线程回调参数
typedef struct _STES_WINTHREAD_CALLBACK{
	ULONG Signature;			// must be 'WgCc'
	ULONG Size;					// size of this struct
	ULONG BlockID;
	//HANDLE WaitEvent;
	IBaseObject* Applicant;		// 回调对象
	PXUI_CALLBACK ProcedureToCall;	//回调过程，过程的返回值将会返回给调用者
	ULONG Flag;			// 传递给回调函数的参数
	LPVOID ConText;		// 传递给回调函数的参数
}STES_WINTHREAD_CALLBACK,* PSTES_WINTHREAD_CALLBACK;


// Windows消息捕获对象
class CEsWsgCapture{
public:
	UINT muiWsgID;
	IEinkuiIterator* mpAppItr;
	IXsElement* mpApplicant;
	PWINMSG_CALLBACK mpProcedure;
	void operator=(const class CEsWsgCapture& src){
		muiWsgID = src.muiWsgID;
		mpApplicant = src.mpApplicant;
		mpProcedure = src.mpProcedure;
		mpAppItr = src.mpAppItr;
	}
};

class CEsWsgCaptureCriterion	// 默认的判断准则
{
public:
	bool operator () (const CEsWsgCapture& Obj1,const CEsWsgCapture& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.muiWsgID < Obj2.muiWsgID);
	}
};

// 按照ID排序的线程队列
typedef cmmMultiSequence<CEsWsgCapture,CEsWsgCaptureCriterion> TEsWsgCaptor;


DECLARE_BUILTIN_NAME(CEinkuiSystem)
class CEinkuiSystem: public cmmBaseObject<CEinkuiSystem,IEinkuiSystem,GET_BUILTIN_NAME(CEinkuiSystem)>
{
	friend IEinkuiSystem* __stdcall EinkuiGetSystem(void);
	friend class CEleMgrProxy;

public:
	enum ERENDER_STEP{
		eRenderBegin=1,
		eRenderParepare=2,
		eRenderRender=3,
		eRenderEnd=4,
		eRenderStop=5
	};

	// 获得唯一对象
	static CEinkuiSystem* GetUniqueObject(void);

	// 返回元素管理
	virtual IXelManager* __stdcall  GetElementManager(void);	// 绝对不会失败，无需判断返回值是否为NULL

	// 获得当前微件接口，无需释放;如果某个线程没有使用CreateWidgetWorkThread建立，那么在这样的线程中调用GetCurrentWidget将导致异常发生
	virtual IXsWidget* __stdcall GetCurrentWidget(void);	// 绝对不会失败，无需判断返回值是否为NULL

	// 获得分配器，分配器被用于建立公共或发布的Element对象，返回的对象无需释放
	virtual IXelAllocator* __stdcall GetAllocator(void);

	// 获得当前的界面语言描述字符串，如：简体中文'chn'
	virtual const wchar_t* __stdcall GetCurrentLanguage(void);

	// 获得系统的DPI设置
	virtual FLOAT __stdcall GetDpiX(void);
	virtual FLOAT __stdcall GetDpiY(void);

	// 启用画板，当主程序装载完成后，调用这个函数启用画板；在此之前画板并不会绘制出实际图像，以避免启动不完整时，Eink屏幕多次闪动
	virtual ERESULT __stdcall EnablePaintboard(bool nbIsDisable = false);

	// 重置画板，当屏幕旋转发生时，调用本函数促使系统重置画板
	virtual void __stdcall ResetPaintboard(void);

	// 获得主窗口的大小
	virtual void __stdcall GetPaintboardSize(
		OUT EI_SIZE* npSize	// 获取画板大小
	);

	// 设置本程序在Eink Panel上的显示位置，只用于自绘程序
	virtual void __stdcall SetPositionInPanel(
		ULONG nuX,
		ULONG nuY
	);



	// 获取Eink系统参数
	virtual PEI_SYSTEM_INFO __stdcall GetEinkSystemInfo(void) {
		return &mdEinkInfo;	
	}

	// 获得Windows窗口
	virtual HWND __stdcall GetMainWindow(void){
		return mhMainWnd;
	}

	// 显示或者隐藏Windows通讯窗口
	virtual void __stdcall ShowMainWindow(bool nbShown=true);

	// 启动Xui
	int Startup(
		STES_START_UP& nrStart
	);

	// 运行一个新的Widget
	virtual ERESULT __stdcall LaunchWidget(
		IN const wchar_t* nswModulePathName,	// 该Widget的模块文件的路径名，即实现此Widget的DLL名称
		IN const wchar_t* nswHomeTempleteName,	// 该Widget的HomePage的Templete Key的名字，这个Key必须在ProFile 的Root下
		IN const wchar_t* nswInstanceName,		// 本次运行的实例名，实例名不能相同，如果存在相同的实例名，系统将会返回失败
		IN IEinkuiIterator* npFrame,	// 用于定位和显示待装载Widget的Frame Element
		IN ICfKey* npInstConfig,	// Widget实例专属配置
		OUT IXsWidget** nppWidget	// 可以不填写，用于返回新建立的Widget接口，返回的接口需要释放
		);

	// 获得当前系统中的某个Widget接口，如果返回NULL表示此编号之后没有Widget了
	// 此函数只能被System Widget调用
	virtual IXsWidget* __stdcall ObtainWidget(
		IN int niNumber		// 从0开始编号得Widegt，编号没有意义，只是Widget的存储位置
		);

	// 获得系统Widget
	virtual IXsWidget* __stdcall GetSystemWidget(void);

	// Widget压栈，供CXelManager调用，当分发一条消息到目标Element时调用，表示切换到新的Widget状态
	void PushWidget(IXsWidget* npWidget);

	// Widget退栈，供ElementManager在完成一个Element的消息是调用，恢复到之前的Widget状态
	void PopWidget(void);

	// 启动一个新线程，所有的Widget都应该使用这个函数建立自己的额外线程；返回值即Windows线程句柄，可以用于调用SuspendThread/ResumeThread/GetExitCodeThread
	// 最终，返回的句柄必须调用CloseHandle关闭；函数的输入参数同Windows API CreateThread一致
	virtual HANDLE __stdcall CreateWidgetWorkThread(
		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		SIZE_T dwStackSize,
		LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter,
		DWORD dwCreationFlags,
		LPDWORD lpThreadId
		);

	// 退出Widget工作线程；当，传入给CreateWidgetWorkThread的线程主函数退出时，系统会自动调用终止线程操作
	virtual void __stdcall ExitWidgetWorkThread(DWORD dwExitCode);

	// 打开一个Config文件；用于打开一个config文件，目前应用在Factory接口实现中，用于打开一个Conponent对应的Profile
	virtual IConfigFile* __stdcall OpenConfigFile(
		IN const wchar_t* nszPathName,				// 文件的完整路径名
		IN ULONG nuCreationDisposition=CF_OPEN_EXISTING	// 同CreateFile API类似，但仅包括CF_CREATE_ALWAYS、CF_CREATE_NEW、CF_OPEN_ALWAYS、CF_OPEN_EXISTING，定义见CfgIface.h
		);

	// 给某个Element及其所有children拍照
	virtual IEinkuiBitmap* __stdcall TakeSnapshot(
		IEinkuiIterator* npToShot,
		const D2D1_RECT_F& crSourceRegion,	// 采样区域，目标元素的局部坐标系
		const D2D_SIZE_F& crBriefSize,		// 缩略图尺寸，快照的结果是一副缩略图
		const FLOAT* ColorRGBA=NULL
		);

	// 申请重新绘制以更新整个视图，Idealife的模式是整个视图重绘，只要调用一次就会更新整个输出视图；
	// 重复调用本函数并不会导致重复的绘制；
	virtual void __stdcall UpdateView(
		IN bool nbRefresh = false	// 必须提交全屏
		);

	// 设置Eink绘制回调，每当UI系统向Eink服务提交时调用指定的回调函数
	virtual ERESULT __stdcall SetEinkUpdatingCallBack(
		IN PXUI_EINKUPDATING_CALLBACK npFunction,
		IN PVOID npContext
	);

	// 重置Eink缓存；本库的工作原理是通过Eink缓存中的遗留内容来比对待显示内容，只将不同的部分发给Eink；如果Eink屏幕需要全部重绘
	//    就需要重置Eink缓存，使得全部内容绘制到Eink；主要用于App重新活动Eink屏幕的情况
	virtual void __stdcall ClearEinkBuffer(void);

	// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual ID2D1Factory* __stdcall GetD2dFactory(void);

	// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual IWICImagingFactory* __stdcall GetWICFactory(void);

	// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
	virtual IDWriteFactory* __stdcall GetDWriteFactory(void);

	// 申请使用Windows交互线程回调，如果Widget开发中需要调用Windows的界面模块，显示额外的Windows界面或者做与Windows界面相关的必须在Windows交互线程中执行的功能，
	// 将该功能代码移植到自己Element或者其他输出IBaseObject的类的一个独立函数中，通过调用本方法，可以使得Windows界面线程主动回调设定的独立函数，而该函数的返回值也将
	// 通过本函数直接返回给本方法的调用者。完成上述功能的过程中，本处的代码将阻塞在此方法中，并且确保整个界面正常刷行，但，Xui本身的界面将不再相应。
	virtual ERESULT __stdcall CallBackByWinUiThread(
		IBaseObject* npApplicant,		// 回调对象，并不一定要求是调用本方法的对象本身，也可以是别的对象指针
		PXUI_CALLBACK npProcedureToCall,	//回调过程，过程的返回值将会返回给调用者
		ULONG nuFlag,			// 传递给回调函数的参数
		LPVOID npConText,		// 传递给回调函数的参数
		bool nbStall=false		// 等待返回期间是否需要处理后续的XUI消息，如果nbStal设为ture程序将直接等待返回，而不处理XUI消息循环
		// 使用此方法可以避免调用本方法期间，再次重入调用
		);

	// 注册Windows消息拦截，通过本功能可以在XUI系统处理Windows消息之前截取关注的Windows消息
	// 处理截取的Windows消息的过程要尽可能短暂。
	// 返回ERESULT_WINMSG_SENDTO_NEXT，XUI将消息传递给下一个拦截者，或者交由XUI系统解释处理；返回其他值将终止该Windows消息的传递过程以及XUI对该消息的处理
	virtual ERESULT __stdcall CaptureWindowsMessage(
		IN UINT nuiWinMsgID,	// 希望博获的Windows消息的ID
		IN IXsElement* npApplicant,	//申请回调的对象
		IN PWINMSG_CALLBACK npProcedure	//将Windows消息发送给此函数
		);

	// 产生一个命令，此方法通常为SystemWidget的菜单模块调用，用来模拟一次用户按下组合键行为，该命令将会被发送至当前的键盘焦点以上的具有EITR_STYLE_COMMAND的对象
	virtual ERESULT __stdcall GenerateCommand(
		nes_command::ESCOMMAND neCmd
		);

	// 设置某页成为模态对话方式，即用户必须完成该对话，此时界面上的其他部分都无法使用。
	// XUI的模态对话方式是全局的，处于模态对话下，所有的Widget（包括System Widget)都不能响应用户输入；所有，尽可能避免使用模式对话，除非它是必须的。
	// 使用方法是，首先创建模式对话的主对象（默认隐藏)，然后调用本函数进入模态对话方式，此时该模态对话元素对象将会收到一条EMSG_MODAL_ENTER消息，它处理相关事务后将自己显示出来；
	// 当该模态对话对象判断用户完成了对话后，隐藏自己，而后ExitModal退出模态对话方式
	// 注意，模态对话方式是可以重叠进入的，在模态对话下，可以打开子模态对话，而后一层层退出
	virtual ERESULT __stdcall DoModal(
		IEinkuiIterator* npModalElement		// 此元素是模态对话的主对象
		);


	// 退出模态对话方式
	virtual void __stdcall ExitModal(
		IEinkuiIterator* npModalElement,	// 此元素是模态对话的主对象
		ERESULT nuResult
		);

	// 主窗口的Windows消息过程函数
	static LRESULT CALLBACK MainWindowProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
		);

	// XUI界面交互线程
	static ULONG WINAPI OperationThread(PSTES_START_UP npStartup);

	// Widget工作线程的入口
	static ULONG WINAPI WidgetWorkThread(LPVOID Context);

	// 侦测线程
	static ULONG WINAPI DetermineThread(LPVOID Context);


	// 申请定时器，对于永久触发的定时器，需要注销
	ERESULT SetTimer(
		IN CXuiIterator* npIterator,// 申请者
		IN ULONG nuID,	  // 定时器ID
		IN ULONG nuRepeat,// 需要重复触发的次数，MAXULONG32表示永远重复
		IN ULONG nuDuration,	// 触发周期
		IN void* npContext//上下文，将随着定时器消息发送给申请者
		);

	// 销毁定时器
	ERESULT KillTimer(
		IN CXuiIterator* npIterator,// 申请者
		IN ULONG nuID	  // 定时器ID
		);

	// 确认当前线程是否是操作线程
	bool IsRunningInOperationThread(void);

	// 确认当前线程是否是Windows界面线程
	bool IsRunningInWindowsUiThread(void);

	// 检查并且执行Windows线程的Callback请求，本函数只能被执行与Windows线程中的会阻塞住Windows线程的地方调用
	// returns false if we got wm_quit message
	bool RunWindowsUICallback(void);

	// 获得元素管理器对象
	CXelManager* GetElementManagerObject(void){
		return mpElementManager;
	}

	// 获得Ime上下文对象
	CXsImeContext* GetImeContext(void){
		return mpImeContext;
	}

	// 获得渲染阶段
	CEinkuiSystem::ERENDER_STEP GetRenderStep(void);

	bool IsOptLocked(void){
		return mbLocked;
	}

	// 退出整个XUI系统
	virtual void __stdcall ExitXui(void);

	virtual void __stdcall SystemTest(ULONG nuValue);

	CXsBmpList& GetBitmapList(void){
		return moAllBitmaps;
	}

	CXsBrushList& GetBrushList(void)
	{
		return moAllBrushes;
	}

	// 获得最前面的模态窗口
	IEinkuiIterator* GetTopModalElement(void);

	//// 设置闪烁Domodal的窗口
	//void FlashModalElement(IEinkuiIterator* npDodal);
	//本次操作是否是TOUCH操作
	virtual bool __stdcall IsTouch(void);
public:
	// 创建渐变画刷
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType,
		D2D1_COLOR_F noColor
		);

	// 渐变画刷时，需要传入多个颜色点
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType, 
		D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
		);	


public:
	// 唯一实例
	static CEinkuiSystem* gpXuiSystem;


protected:
	// 内部变量
	// 图形引擎接口
	CXD2dEngine* mpXuiGraphics;
	// 绘图资源基类指针
	IEinkuiPaintResouce* mpPaintResource;
	// EINK设备信息
	EI_SYSTEM_INFO mdEinkInfo;
	ULONG muAutoRotate;

	HWND mhMainWnd;
	//bool mbWindowHide;
	D2D1_SIZE_F mdDpi;
	LONG mlNewPaintboardWidth;
	LONG mlNewPaintboardHeight;
	LONG mlNewPaintboardX;
	LONG mlNewPaintboardY;

	CXsBmpList moAllBitmaps;
	CXsBrushList moAllBrushes;

	CXelManager* mpElementManager;

	// 定时器
	TEsTimerSequence moTimers;
	CExclusiveAccess moTimerListLock;
	STEMG_LAZY_UPDATE mdLazyUpdate;

	// widget控制
	//TEsWidgetStack moWidgetContext;
	CXsWgtContext moWidgetContext;	// 操作线程的Widget执行栈
	TEsWidgetVector moAllWidgets;	// 全部Widget
	TEsThreadSequence moWidgetWorkThreads;	// 全部工作线程
	ULONG muOperationThreadID;	// 操作线程
	CExclusiveAccess moWidgetLock;

	// 操作线程句柄
	HANDLE mhOperationThread;
	bool mbThreadKilled;
	bool mbFirstRun;
	bool mbLocked;	// 现在简单地用一个变量来表示，当Windows交互线程发现该变量被设置时，即假定操作线程被交互线程锁定在先。将来修改，当其他线程判断时就可能出错了

	// 运行侦测
	HANDLE mhExitDetermine;

	// 分配器
	CXelAllocator* mpAllocator;

	// 绘制控制
	volatile LONG miDisableRender;
	volatile LONG miToRender;
	volatile LONG miDiscardRenderMessage;
	volatile LONG miRefreshEink;
	volatile ULONG muRenderTick;		// 最近一次渲染的TickCount
	volatile LONG miLostToRender;	// 因为没有绘制要求，离最近一次绘制缺失的帧数
	//ULONG muAutoRenderInterval;	// 自动绘制间隔
	volatile LONG miMissedMouseTest;	// 最近一次绘制后而没有收到鼠标消息的定时器达到次数，一定数量后就需要检测鼠标
	//ULONG muRecentRenderTick;	// 最近一次绘制的时间点
	//ULONG muNextRenderTick;		// 下一次绘制的时间点

	// Windows 线程回调控制
	ULONG muWinThreadID;	// Windows交互线程
	UINT muiCustomWinMsg;// WinUI自定义消息
	CXsWgtContext moWinWgtContext;	// Windows交互线程线程的Widget执行栈
	CExclusiveAccess moWinThreadLock;
	LONG mlWinCallBack;
	//ULONG muWinUiCallBackID;
	//ERESULT muOpLoopResult;
	CXuiThreadBlock moThreadBlock;
	CXuiModalStack moModalStack;

	// Windows 消息捕获
	TEsWsgCaptor moCaptors;
	CExclusiveAccess moCaptorLock;

	// 鼠标
	USHORT msuWinCap;
	// IME
	CXsImeContext* mpImeContext;

	//是否自动软键盘的对象
	bool mbIsTouch;

	//保存任务栏预览图像，当程序最小化或隐藏时使用
	HBITMAP mpSmallPreView;
	HBITMAP mpBigPreView;

	CEinkuiSystem();
	~CEinkuiSystem();

	DEFINE_CUMSTOMIZE_CREATE(CEinkuiSystem,(),())

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	// 注册所有公共消息
	bool RegisterCommonWindowsMessage(void);

	// 建立窗口
	ERESULT CreateMainWnd(
		IN int niX,		// 主窗口在屏幕上的左上角X坐标
		IN int niY,		// 主窗口在屏幕上的左上角Y坐标
		IN int niWidth,		// 主窗口在屏幕上的宽度
		IN int niHeight,	// 主窗口在屏幕上的高度
		IN const wchar_t* nswWndTittle		// 主窗口标题
		);

	// Windows界面线程，主循环
	int HostThreadLoop();

	// 启动绘制定时器
	void EnableRender(void);

	// 终止绘制定时器
	void DisableRender(void);


	// 处理Windows WM_CREATE消息
	LRESULT WsgOnCreate(HWND nhWindow);

	// 查找或者删除一个定时器，供定时器处理函数内部调用
	bool FindTimer(
		IN CXuiIterator* npIterator,// 申请者
		IN ULONG nuID,	  // 定时器ID，不能为0
		IN bool nbDelete=false	// 是否删除
		);

	// 从定时器队列中取最近一个可能激发的时间间隔，tick（毫秒）为单位
	ULONG GetTickCountToKickTimer(ULONG nuCrtTick);

	// 定时器激发测试
	void KickTimers(ULONG nuCrtTick);

	// 启动操作线程
	ERESULT ExecuteOperationThread(PSTES_START_UP npCreate);

	ERESULT ExecuteDetermineThread(LPVOID Context);

	// 用于接收所有发送给系统的消息
	ERESULT SystemMessageReceiver(IEinkuiMessage* npMsg);

	// 定时器消息
	ERESULT OnMsgTimer(const PSTEMS_TIMER npTimerInput);

	// 检查操作线程运行状态
	void DetermineOPRunming(void);

	// 回调申请者的函数，成功后给操作线程发送恢复消息
	void CallClientProcedure(PSTES_WINTHREAD_CALLBACK npToCall);

	// 操作线程消息处理循环
	ERESULT OpThreadMessageLoop(ULONG nuBlockID);

	// 慢刷新发送预处理
	ERESULT __stdcall EnterForLazyUpdate(IEinkuiIterator* npRecipient);

	// 慢刷新发送后处理
	ERESULT __stdcall LeaveForLazyUpdate(IEinkuiIterator* npRecipient);

	ERESULT DoCapture(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam,
		HRESULT& rWinResult
		);

	// 收到Windows Mouse消息
	void OnWinMouse(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	// 收到Eink Touch消息
	void OnEinkTouch(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	// 设置windows鼠标捕获
	void SetWinMouseCapture(USHORT nsuFlag);

	// 释放windows鼠标捕获
	void ReleaseWinMouseCapture(USHORT nsuFlag);

	void WsgOnGetMinMaxInfo(MINMAXINFO *pMinMaxInfo);

	//void WsgOnSettingChanged();	discarded by ax mar.03,2013 去掉自动尺寸改变的功能

	//ERESULT __stdcall CorrectMainWindowSize(ULONG nuFlag,LPVOID npContext);

	ERESULT CloseWidget(
		IN IXsWidget* npWidget
		);

	void EndXui(void);

	// 主窗口的Windows消息过程函数
	LRESULT MessageFromXuiToWindowUi(
		WPARAM wParam,
		LPARAM lParam
		);

	// 系统绘制过程
	void RenderProcedule(ULONG nuCrtTick);

};


#define ESYS_FLAG_INITOK 1




// 启动工作线程用上下文参数
struct _STES_CREATE_THREAD{
	IXsWidget* Widget;
	LPTHREAD_START_ROUTINE ThreadProc;
	LPVOID Context;
};
typedef _STES_CREATE_THREAD STES_CREATE_THREAD,* PSTES_CREATE_THREAD;













































#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif//_XUIMAIN_H_