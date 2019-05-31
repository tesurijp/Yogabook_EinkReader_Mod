/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// Xui.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "CommonHeader.h"

#include "./Graphy/XgD2DEngine4Eink.h"

#include "einkuiimp.h"

#include "resource.h"
#include <Dwmapi.h >
#include "EinkIteAPI.h"

#define EINK_RENDER_INTERVAL 600
#define EINK_RENDER_TIMER	 100

DEFINE_BUILTIN_NAME(CEinkuiSystem)
CEinkuiSystem* CEinkuiSystem::gpXuiSystem = NULL;

CEinkuiSystem::CEinkuiSystem()
{
	mpXuiGraphics = NULL;
	mpElementManager = NULL;
	muOperationThreadID = 0;
	mpAllocator = NULL;
	mhOperationThread = NULL;
	miToRender = 1;
	miDisableRender = 0;
	//muAutoRenderInterval = 0;	// 0表示不自动绘制
	miRefreshEink = 0;
	miLostToRender = 0;
	muRenderTick = 0;
	miMissedMouseTest =0;
	mdLazyUpdate.Updated = 0;
	msuWinCap = 0;
	mpImeContext = NULL;
	miDiscardRenderMessage = 0;

	mhExitDetermine = CreateEvent(NULL,TRUE,FALSE,NULL);

	mbFirstRun = true;
	mbLocked = false;
	mlWinCallBack = 0;
	//mbWindowHide = false;
	mbIsTouch = false;
	mpSmallPreView = NULL;
	mpBigPreView = NULL;

	mdEinkInfo.ulWidth = mdEinkInfo.ulHeight = 0;
	muAutoRotate = 0;
}

CEinkuiSystem::~CEinkuiSystem()
{
	CMM_SAFE_RELEASE(mpXuiGraphics);
	CMM_SAFE_RELEASE(mpElementManager);
	CMM_SAFE_RELEASE(mpAllocator);
	CMM_SAFE_RELEASE(mpImeContext);
	if(mhOperationThread != NULL)
		CloseHandle(mhOperationThread);
	if(mhExitDetermine != NULL)
		CloseHandle(mhExitDetermine);

	while(moTimers.Size() > 0)
	{
		STES_TIMER* lpTimer = moTimers[0];
		CMM_SAFE_DELETE(lpTimer);
		moTimers.RemoveByIndex(0);
	}

	if(mpSmallPreView != NULL)
		DeleteObject(mpSmallPreView);

	if(mpBigPreView != NULL)
		DeleteObject(mpBigPreView);

	//退出EINK
	EiEnd();
}


// 创建渐变画刷
IEinkuiBrush* __stdcall CEinkuiSystem::CreateBrush(
	XuiBrushType niBrushType,
	D2D1_COLOR_F noColor
	)
{
	if (mpPaintResource == NULL)
		return NULL;

	return mpPaintResource->CreateBrush(niBrushType, noColor);

}

// 渐变画刷时，需要传入多个颜色点
IEinkuiBrush* __stdcall CEinkuiSystem::CreateBrush(
	XuiBrushType niBrushType, 
	D2D1_GRADIENT_STOP* npGradientStop, 
	ULONG nuCount, 
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
	)
{
	if (mpPaintResource == NULL)
		return NULL;

	return 	mpPaintResource->CreateBrush(niBrushType, npGradientStop, nuCount, ndLinearGradientBrushProperties);
	

}



// 获得唯一对象
CEinkuiSystem* CEinkuiSystem::GetUniqueObject(void)
{
	if(gpXuiSystem ==NULL)
		CEinkuiSystem::CreateInstance();

	CMMASSERT(gpXuiSystem !=NULL);

	return gpXuiSystem;
}


// 返回元素管理
IXelManager* __stdcall CEinkuiSystem::GetElementManager(void)	// 绝对不会失败，无需判断返回值是否为NULL
{
	return mpElementManager;
}

// 获得当前微件接口，无需释放;如果某个线程没有使用CreateWidgetWorkThread建立，那么在这样的线程中调用GetCurrentWidget将导致异常发生
IXsWidget* __stdcall CEinkuiSystem::GetCurrentWidget(void)	// 绝对不会失败，无需判断返回值是否为NULL
{
	IXsWidget* lpWidget = NULL;
	int liPos;
	CEsThreadNode loFind;

	loFind.muThreadID = GetCurrentThreadId();

	moWidgetLock.Enter();

	if(loFind.muThreadID == muOperationThreadID)
	{
		// 从Widget上下文堆栈中取栈顶
		lpWidget = moWidgetContext.GetTopWidget();
	}
	else
	if(loFind.muThreadID == muWinThreadID)
	{
		lpWidget = moWinWgtContext.GetTopWidget();
	}
	else
	{
		// 检查当前是否工作与某个Widget工作线程中
		liPos = moWidgetWorkThreads.Find(loFind);
		if(liPos >= 0)
		{	// 确实是一个工作线程
			lpWidget = moWidgetWorkThreads[liPos].mpOwnerWidget;
		}
	}

	moWidgetLock.Leave();

	if(lpWidget == NULL)
		THROW_NULL;	// 不能允许其他线程调用本方法

	return dynamic_cast<IXsWidget*>(lpWidget);
}

// 获得分配器，分配器被用于建立公共或发布的Element对象，返回的对象无需释放
IXelAllocator* __stdcall CEinkuiSystem::GetAllocator(void)
{
	return mpAllocator;
}


// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CEinkuiSystem::InitOnCreate(void){
	if(gpXuiSystem != NULL)
		return ERESULT_OBJECT_EXISTED;

	gpXuiSystem = this;

	return ERESULT_SUCCESS;
}

ERESULT CEinkuiSystem::CreateMainWnd(
	IN int niX,		// 主窗口在屏幕上的左上角X坐标
	IN int niY,		// 主窗口在屏幕上的左上角Y坐标
	IN int niWidth,		// 主窗口在屏幕上的宽度
	IN int niHeight,	// 主窗口在屏幕上的高度
	IN const wchar_t* nswWndTittle		// 主窗口标题
	)
{
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc   = CEinkuiSystem::MainWindowProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = sizeof(LONG_PTR);
	wcex.hInstance     = GetModuleHandle(NULL);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName  = NULL;
	wcex.hCursor       = NULL;//LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = L"ESIEINKV1";	// XUI core window class version 1
	wcex.hIconSm = LoadIcon(wcex.hInstance,(LPCWSTR)IDI_MAINWND_ICON); //增加对ICON的指定
	wcex.hIcon = LoadIcon(wcex.hInstance,(LPCWSTR)IDI_MAINWND_ICON);

	RegisterClassEx(&wcex);

	HWND lhWindow = CreateWindow(
		L"ESIEINKV1",
		nswWndTittle,
		WS_OVERLAPPEDWINDOW |WS_MINIMIZEBOX,//WS_OVERLAPPEDWINDOW,WS_POPUP|WS_VISIBLE|WS_MINIMIZEBOX|WS_SYSMENU,
		niX,
		niY,
		niWidth,
		niHeight,
		NULL,
		NULL, 
		GetModuleHandle(NULL),
		this
		);

	if(lhWindow==NULL)
	{
		return ERESULT_UNSUCCESSFUL;
	}

	RegisterTouchWindow(lhWindow, TWF_WANTPALM);

	//这个注册用于接收connect standby消息
	RegisterPowerSettingNotification(EinkuiGetSystem()->GetMainWindow(), &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
	//HRGN rgn = CreateRectRgn(0, 0, niX+niWidth, niY+niHeight); 待查明为什么有这两行调用，可能和d3d有关 Mar.2,2013
	//SetWindowRgn(lhWindow, rgn, FALSE);

	return ERESULT_SUCCESS;
}

// 显示或者隐藏Windows通讯窗口
void __stdcall CEinkuiSystem::ShowMainWindow(bool nbShown)
{
	::PostMessage(mhMainWnd,muiCustomWinMsg,'Show',nbShown?1:0);
}


// 获得当前的界面语言描述字符串，如：简体中文'chn'
const wchar_t* __stdcall CEinkuiSystem::GetCurrentLanguage(void)
{
#define MAX_LANGID_LENGTH 10

	const wchar_t* lswLang= L"enu";

	//LANGID langId = GetUserDefaultUILanguage();
	DWORD ldwLangID = GetUserDefaultUILanguage();
	EiGetUserLagID(ldwLangID);
	switch(ldwLangID)
	{
	case 0x0404: //Chinese (Taiwan)
	case 0x1004: // Chinese (Singapore)
	case 0x1404: // Chinese (Macao SAR)
		lswLang = L"zh_tw";
		break;
	case 0x0c04: //Chinese (Hong Kong SAR, PRC)
		lswLang = L"zh_hk";
		break;
	case 0x0804: //Chinese (PRC)
		lswLang = L"zh_cn";
		break; 
	case 0x0405: //Czech
		lswLang = L"cs";
		break; 
	case 0x0406: //Danish
		lswLang = L"da"; //dk
		break; 
	case 0x0809://United Kingdom
		lswLang = L"en_gb";
		break;
	case 0x0407: //German (Standard)
	case 0x0807: //German (Switzerland) 
	case 0x0c07: //German (Austria) 
	case 0x1007: //German (Luxembourg) 
	case 0x1407: //German (Liechtenstein) 
		lswLang = L"de"; //at
		break;
	case 0x040a: //Spanish (Spain, Traditional Sort) 
	case 0x080a: //Spanish (Mexican) 
	case 0x0c0a: //Spanish (Spain, Modern Sort) 
	case 0x100a: //Spanish (Guatemala) 
	case 0x140a: //Spanish (Costa Rica) 
	case 0x180a: //Spanish (Panama) 
	case 0x1c0a: //Spanish (Dominican Republic) 
	case 0x200a: //Spanish (Venezuela) 
	case 0x240a: //Spanish (Colombia) 
	case 0x280a: //Spanish (Peru) 
	case 0x2c0a: //Spanish (Argentina) 
	case 0x300a: //Spanish (Ecuador) 
	case 0x340a: //Spanish (Chile) 
	case 0x380a: //Spanish (Uruguay) 
	case 0x3c0a: //Spanish (Paraguay) 
	case 0x400a: //Spanish (Bolivia) 
	case 0x440a: //Spanish (El Salvador) 
	case 0x480a: //Spanish (Honduras) 
	case 0x4c0a: //Spanish (Nicaragua) 
	case 0x500a: //Spanish (Puerto Rico) 
		lswLang = L"es_es";
		break;
	case 0x540A: //Spanish (United States (US)) 
		lswLang = L"es_us";
		break;
	case 0x0413: //Dutch (Netherlands) 
	case 0x0813: //Dutch (Belgium) 
		lswLang = L"nl";
		break;
	case 0x040b: //Finnish 
		lswLang = L"fi";
		break;
	case 0x040c: //French (Standard) 
	case 0x080c: //French (Belgian) 
	case 0x100c: //French (Switzerland) 
	case 0x140c: //French (Luxembourg) 
		lswLang = L"fr";
		break;
	case 0x0c0c: //French (Canadian) 
		lswLang = L"fr_ca";
		break;
	case 0x0408: //Greek 
		lswLang = L"el";
		break;
	case 0x040e: //Hungarian 
		lswLang = L"hu";
		break;
	case 0x0410: //Italian (Standard) 
	case 0x0810: //Italian (Switzerland) 
		lswLang = L"it";
		break;
	case 0x0411: //Japanese 
		lswLang = L"ja";
		break;
	case 0x0412: //Korean 
		lswLang = L"ko";
		break;
	case 0x0414: //Norwegian (Bokmal) 
	case 0x0814: //Norwegian (Nynorsk) 
		lswLang = L"nb";
		break;
	case 0x0415: //Polish 
		lswLang = L"pl";
		break;
	case 0x0816: //Portuguese (Portugal) 
		lswLang = L"pt_pt";
		break;
	case 0x0416: //Portuguese (Brazil)  
		lswLang = L"pt_br";
		break;
	case 0x0419: //Russian 
		lswLang = L"ru";
		break;
	case 0x041d: //Swedish 
	case 0x081d: //Swedish (Finland) 
		lswLang = L"sv";
		break;
	case 0x041f: //Turkish
		lswLang = L"tr";
		break;
	case 0x0418://romanian
		lswLang = L"ro";
		break;
 	case 0x0402: //bulgarian
 		lswLang = L"bg";
 		break;
	case 0x041b://slovak
		lswLang = L"sk";
		break;
	case 0x0424://slovenian
		lswLang = L"sl";
		break;
 	case 0x040D://Hebrew (he) Israel (IL)			----HE
 		lswLang = L"iw";
 		break;
 	case 0x0401: // Arabic (ar) Saudi Arabia (SA)	----AR
 		lswLang = L"ar";
 		break;

	case 0x041A: // Croatian (hr) Croatia (HR)		----CR
		lswLang = L"cr";
		break;
	case 0x041E: // 	Thailand (TH)
		lswLang = L"th";
		break;
	case 0x0C1A: //Serbian (SR) Serbia and Montenegro, Former, Cyrillic (CS)
	case 0x081A: //Serbian (SR) Serbia and Montenegro, Former, Latin (CS)
	case 0x241A: //win8.1 下新增的，在MSDN中也没有定义
		lswLang = L"sr";
		break;
	default:
		lswLang = L"enu";

	}

	return lswLang;
}

FLOAT __stdcall CEinkuiSystem::GetDpiX(void)
{
	return mdDpi.width;
}
FLOAT __stdcall CEinkuiSystem::GetDpiY(void)
{
	return mdDpi.height;
}

// 启用画板，当主程序装载完成后，调用这个函数启用画板；在此之前画板并不会绘制出实际图像，以避免启动不完整时，Eink屏幕多次闪动
ERESULT __stdcall CEinkuiSystem::EnablePaintboard(void)
{
	ERESULT luResult = ERESULT_NOT_INITIALIZED;

	if (mpXuiGraphics != NULL)
	{
		mpXuiGraphics->StopPainting(false);
		luResult = ERESULT_SUCCESS;
	}

	EnableRender();

	return luResult;
}

// 重置画板，当屏幕旋转发生时，调用本函数促使系统重置画板
void __stdcall CEinkuiSystem::ResetPaintboard(void)
{
	//EINK获取Eink设备信息
	EiGetSystemInfo(&mdEinkInfo);

	mpXuiGraphics->ResetPaintboard();

}

// 获得主窗口的大小
void __stdcall CEinkuiSystem::GetPaintboardSize(
	OUT EI_SIZE* npSize	// 获取画板大小
	)
{
	mpXuiGraphics->GetPaintboardSize(npSize);
}

// 设置本程序在Eink Panel上的显示位置，只用于自绘程序
void __stdcall CEinkuiSystem::SetPositionInPanel(
	ULONG nuX,
	ULONG nuY
	)
{
	mpElementManager->SetPositionInPanel(nuX,nuY);
}



// 注册所有公共消息
bool CEinkuiSystem::RegisterCommonWindowsMessage(void)
{
	muiCustomWinMsg = RegisterWindowMessage(ES_WINUI_CUSTOM_MSG);

	return muiCustomWinMsg!=0;
}

// 启动Xui
int CEinkuiSystem::Startup(
	STES_START_UP& nrStart
	)
{
	int liProcessResult = -1;
	wchar_t* lswErrorString = NULL;

	ERESULT luResult;

	try
	{
		//mbIsAutoSize = nbIsAutoSize;
		mdDpi.width = nrStart.Dpi.width;
		mdDpi.height = nrStart.Dpi.height;

		muWinThreadID = GetCurrentThreadId();

		//Trace_Point(25643);// 注册公用Windows消息
		if(RegisterCommonWindowsMessage()==false)
			THROW_FALSE;

		//Trace_Point(19481);// 建立通讯窗口
		luResult = CreateMainWnd(0,0,100,100,nrStart.WindowTitle);
		if(ERESULT_FAILED(luResult))
		{
			THROW_UNKNOWN;
		}
		
		mpImeContext = CXsImeContext::CreateInstance();

		//EINK初始化
		EiAppStart(mhMainWnd);

		//EINK获取Eink设备信息
		EiGetSystemInfo(&mdEinkInfo);

		if (nrStart.AutoRotate != 0)
		{
			muAutoRotate = nrStart.AutoRotate;

			EiSetScreenOrient(mdEinkInfo.ulOrient);
		}

		//Trace_Point(24937);// 准备系统Widget的参数
		nrStart.WaitingCaller = CreateEvent(NULL,true,false,NULL);
		if(nrStart.WaitingCaller == NULL)
		{
			THROW_NULL;
		}

		//Trace_Point(20499);// 启动操作线程
		luResult = ExecuteOperationThread(&nrStart);
		if(luResult != ERESULT_SUCCESS)
		{
			THROW_FALSE;
		}

		//Trace_Point(19006);//等待系统Widget建立完成，最多等20秒
		if(WaitForSingleObject(nrStart.WaitingCaller,2000000/*???20000*/) == WAIT_TIMEOUT)
		{
			CloseHandle(nrStart.WaitingCaller);
			THROW_UNKNOWN;
		}
		CloseHandle(nrStart.WaitingCaller);

		if(nrStart.Result != ERESULT_SUCCESS)
			THROW_UNKNOWN;


		if (nrStart.CustomDraw != NULL)
		{
			mpXuiGraphics->SetCustomDraw(nrStart.CustomDraw->CustomDrawFun);
		}

		//ExecuteDetermineThread(NULL);

		// 启动绘制流程

		// 允许执行操作线程探测
		moWidgetContext.EnableTickDetection(true);

		//Trace_Point(15975);// 进入主消息循环
		liProcessResult = HostThreadLoop();

		// 等待操作线程退出
		WaitForSingleObject(mhOperationThread,16000);
	}
	catch(...)
	{
		//Trace_Point(350);//  ???增加对话框提示，直接向用户报错
		//Trace_Flush();
		MessageBox(NULL,L"An unknown error has detected!",L"Error",MB_OK);
	}
	
	return liProcessResult;
}

// 启动绘制定时器
void CEinkuiSystem::EnableRender(void)
{
	DisableRender();

	InterlockedExchange(&miDisableRender, 0);

	//mpElementManager->GetRootElement()->SetTimer(ELMGR_TIMERID_RENDER, MAXULONG32, 15, NULL);
	//mpElementManager->GetRootElement()->SetTimer(ELMGR_TIMERID_LAZY, MAXULONG32, 200, NULL);
	mpElementManager->GetRootElement()->SetTimer(ELMGR_TIMERID_RENDER, MAXULONG32, EINK_RENDER_TIMER, NULL);
	//mpElementManager->GetRootElement()->SetTimer(ELMGR_TIMERID_LAZY, MAXULONG32, 1000, NULL);

	// 设置基本定时器，定期唤醒主线程;主要用于检查操作线程是否会被阻塞
	//::SetTimer(mhMainWnd,0,1000,NULL); $ax$ Nov.27,2018 暂时屏蔽，因为部分操作(打开电子书）耗时过长，没有采取合理的多任务方式，会导致消息处理的暂时阻塞，
					// 所以先屏蔽了消息处理检测

}

// 终止绘制定时器
void CEinkuiSystem::DisableRender(void)
{
	InterlockedExchange(&miDisableRender, 1);

	mpElementManager->GetRootElement()->KillTimer(ELMGR_TIMERID_RENDER);
	//mpElementManager->GetRootElement()->KillTimer(ELMGR_TIMERID_LAZY);

	::KillTimer(mhMainWnd,0);	// 因为联想电脑的电源要求，而增加对这个定时器的修改
}


// Windows界面线程，主循环
int CEinkuiSystem::HostThreadLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		//判断一下是否是没有窗口句柄的线程消息
		if(msg.hwnd == NULL && msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE)
			msg.hwnd = mhMainWnd;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return 0;
}

ERESULT CEinkuiSystem::DoCapture(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam,
	HRESULT& rWinResult
	)	// ??? 没有提供注销捕获的功能
{
	int liMatch;
	CEsWsgCapture loFind;
	ERESULT luReval = ERESULT_WINMSG_SENDTO_NEXT;
	TEsWsgCaptor loAppliers;
	CXuiIterator* lpItrObj;


	moCaptorLock.Enter();

	loFind.muiWsgID = message;
	liMatch = moCaptors.Find(loFind);
	if(liMatch >= 0)
	{
		while(moCaptors[liMatch].muiWsgID == message)
		{
			loAppliers.Insert(moCaptors[liMatch]);
			if(++liMatch >= moCaptors.Size())
				break;
		}
	}

	moCaptorLock.Leave();

	if(loAppliers.Size()<=0)
		return ERESULT_WINMSG_SENDTO_NEXT;

	for(liMatch = 0;liMatch < loAppliers.Size();liMatch++)
	{
		if(mpElementManager->VerifyIterator(loAppliers[liMatch].mpAppItr)!= ERESULT_SUCCESS)
			continue;

		lpItrObj = dynamic_cast<CXuiIterator*>(loAppliers[liMatch].mpAppItr);

		if(lpItrObj == NULL)
			continue;

		moWinWgtContext.PushWidget(lpItrObj->mpWidget);

		try	{
			luReval = (loAppliers[liMatch].mpApplicant->*(loAppliers[liMatch].mpProcedure))(hWnd,message,wParam,lParam,(LRESULT&)rWinResult);
		}
		catch(...)
		{
			luReval = ERESULT_WINMSG_SENDTO_NEXT;
		}

		moWinWgtContext.PopWidget();

		if(luReval != ERESULT_WINMSG_SENDTO_NEXT && luReval != ERESULT_ITERATOR_INVALID)
			break;
	}

	return luReval;
}

// 确认当前线程是否是操作线程
bool CEinkuiSystem::IsRunningInOperationThread(void)
{
	return muOperationThreadID == GetCurrentThreadId();
}

// 确认当前线程是否是Windows界面线程
bool CEinkuiSystem::IsRunningInWindowsUiThread(void)
{
	return muWinThreadID == GetCurrentThreadId();
}

// 检查并且执行Windows线程的Callback请求，本函数只能被执行与Windows线程中的会阻塞住Windows线程的地方调用
bool CEinkuiSystem::RunWindowsUICallback(void)
{
	MSG msg;
	bool lbReval = true;

	if(IsRunningInWindowsUiThread()!=false)
	{
		while(PeekMessage(&msg,mhMainWnd,muiCustomWinMsg,muiCustomWinMsg,PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				PostMessage(mhMainWnd,WM_QUIT,msg.wParam,msg.lParam);
				lbReval = false;
				break;
			}
			//Trace_Point(8590);//确实发生了原来导致死锁的线程间相互依赖

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return lbReval;
}

// 启动操作线程
ERESULT CEinkuiSystem::ExecuteOperationThread(PSTES_START_UP npCreate)
{
	ULONG luThreadID;
	HANDLE lhThread = CreateThread(NULL,128*1024,(LPTHREAD_START_ROUTINE)CEinkuiSystem::OperationThread,npCreate,CREATE_SUSPENDED,&luThreadID);

	if(lhThread == NULL)
		return ERESULT_UNSUCCESSFUL;
	
	InterlockedExchange((LONG*)&muOperationThreadID,luThreadID);

	if(mhOperationThread != NULL)
		CloseHandle(mhOperationThread);

	mhOperationThread = lhThread;

	ResumeThread(lhThread);

	return ERESULT_SUCCESS;
}

// 获得渲染阶段
CEinkuiSystem::ERENDER_STEP CEinkuiSystem::GetRenderStep(void)
{
	return (CEinkuiSystem::ERENDER_STEP)mpXuiGraphics->mlRenderStep;
}

// 操作线程，XUI系统的界面交互线程，如果某个Widget卡住本线程超过一定的时间，本线程间会被Host线程终止
ULONG WINAPI CEinkuiSystem::OperationThread(PSTES_START_UP npStartup)
{
	ERESULT luResult;
	IEinkuiIterator* lpRootItr;

	//Trace_Time(6790);//操作线程启动，进入第一阶段，初始化系统

	CoInitialize(NULL);

	if(npStartup != NULL)
	{
		try
		{
			//Trace_Point(25884);// 建立元素管理器
			gpXuiSystem->mpElementManager = CXelManager::CreateInstance();
			if(gpXuiSystem->mpElementManager == NULL)
			{
				THROW_NULL;
			}

			//Trace_Point(20821);// 建立图像设备
			if(npStartup->CustomDraw != NULL)
				gpXuiSystem->mpXuiGraphics = CXD2dEngine::CreateInstance(npStartup->CustomDraw->Width, npStartup->CustomDraw->Height);
			else
				gpXuiSystem->mpXuiGraphics = CXD2dEngine::CreateInstance(0,0);

			if(gpXuiSystem->mpXuiGraphics == NULL)
			{
				THROW_NULL;
			}

			// 引擎指针赋值给基类指针
			gpXuiSystem->mpPaintResource = gpXuiSystem->mpXuiGraphics;

			// 注册快捷键
			lpRootItr = gpXuiSystem->mpElementManager->GetRootElement();
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_COPY);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_CUT);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_PASTE);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_DELETE);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_SELECT_ALL);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_ENTER);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_ALTF4);
			CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY_ESC);
			//CExHotkey::RegisterHotKey(lpRootItr,lpRootItr,EHOTKEY);

			//Trace_Point(12679);// 准备分配器对象
			gpXuiSystem->mpAllocator = CXelAllocator::CreateInstance(npStartup->ClassesRegPath);
			if(gpXuiSystem->mpAllocator == NULL)
			{
				THROW_NULL;
			}

			//Trace_Point(22798);//设置初始化完成标志
			gpXuiSystem->SetFlags(ESYS_FLAG_INITOK,true);

			//Trace_Point(21957);// 建立系统Widget
			luResult = gpXuiSystem->LaunchWidget(npStartup->ModulePathName,npStartup->HomeTempleteName,L"MainFrame",lpRootItr,NULL,NULL);

		}
		catch(...)
		{
			luResult = ERESULT_UNSUCCESSFUL;
		}

		npStartup->Result = luResult;
		SetEvent(npStartup->WaitingCaller);

		if(ERESULT_FAILED(luResult))
		{
			CoUninitialize();
			//Trace_Time(24705);// 失败了，退出线程
			return -1;
		}

		Sleep(33);
	}

	//Trace_Time(19563);// 操作线程进入第二阶段，等待33毫秒，以让行Host线程

	gpXuiSystem->OpThreadMessageLoop(gpXuiSystem->moThreadBlock.AddBlock());

	CoUninitialize();
	//Trace_Time(22341);//退出线程

	return 0;
}

// 操作线程消息处理循环
ERESULT CEinkuiSystem::OpThreadMessageLoop(ULONG nuBlockID)
{
	ULONG luWait;
	ERESULT luResult;
	ULONG luCrtTick;

	do 
	{
		luCrtTick = GetTickCount();

		// 测试定时器是否需要触发
		KickTimers(luCrtTick);

		try
		{
			// 提取一条消息并分发
			luResult = mpElementManager->ProcessNextMessage();

		}
		catch (...)
		{
			//Trace_Point(28888);//发生异常
			luResult = ERESULT_UNSUCCESSFUL;
		}
		// 如果消息队列为空，则计算最近的待激发定时器间隔，等待这个间隔
		if(luResult == ERESULT_NO_MESSAGE)
		{
			luWait = GetTickCountToKickTimer(luCrtTick);

			luWait = mpElementManager->WaitMessageReach(luWait);
		}

		if(moThreadBlock.ReadBlockState(nuBlockID)!=ERESULT_BLOCK)
			break;

	} while (luResult != ERESULT_QUIT_XUI);

	//if(luResult == ERESULT_QUIT_XUI)
	//	PostQuitMessage(0);

	return luResult;
}


// 运行一个新的Widget
ERESULT __stdcall CEinkuiSystem::LaunchWidget(
	IN const wchar_t* nswModulePathName,	// 该Widget的模块文件的路径名，即实现此Widget的DLL名称
	IN const wchar_t* nswHomeTempleteName,	// 该Widget的HomePage的Templete Key的名字，这个Key必须在ProFile 的Root下
	IN const wchar_t* nswInstanceName,		// 本次运行的实例名，实例名不能相同，如果存在相同的实例名，系统将会返回失败
	IN IEinkuiIterator* npFrame,	// 用于定位和显示待装载Widget的Frame Element
	IN ICfKey* npInstConfig,	// Widget实例专属配置
	OUT IXsWidget** nppWidget	// 可以不填写，用于返回新建立的Widget接口，返回的接口需要释放
	)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	IElementFactory* lpFactory = NULL;
	CXsWidget* lpWidget=NULL;
	IEinkuiIterator* lpHomePage = NULL;
	IConfigFile* lpProfile = NULL;
	ICfKey* lpTemplete = NULL;
	int liWidgetInserted=-1;

	do 
	{
		//Trace_Point(25893);// 建立一个新Widget对象
		lpWidget = CXsWidget::CreateInstance(nswModulePathName,nswInstanceName,npInstConfig);
		BREAK_ON_NULL(lpWidget);

		//Trace_Point(22020);// 检查现在是否是建立系统Widget
		moWidgetLock.Enter();
		if(moAllWidgets.Size()==0)
		{
			//Trace_Point(9073);//设置Root Element的Widget属性
			mpElementManager->SetRootWidget(lpWidget);
		}

		//Trace_Point(11153);// 将新建的Widget加入到列表
		liWidgetInserted = moAllWidgets.Insert(-1,lpWidget);
		moWidgetLock.Leave();
		if(liWidgetInserted<0)	{
			luResult = ERESULT_INSUFFICIENT_RESOURCES;
			break;
		}

		//Trace_Point(32591);// 切换当前Widget
		PushWidget(lpWidget);

		if(moWinWgtContext.GetStackDepth()<=0)
		{
			//Trace_Point(15708);//初始化Windows系统线程的上下文
			moWinWgtContext.PushWidget(lpWidget);
		}

		//Trace_Point(12787);// 装入工厂类
		lpFactory = mpAllocator->LoadFactory(/*lpWidget->GetModuleName()*/);
		BREAK_ON_NULL(lpFactory);

		// 设置Widget的工厂对象
		lpWidget->SetFactory(lpFactory);

		//Trace_Point(12461);// 获取Profile
		lpProfile = lpFactory->GetTempleteFile();
		BREAK_ON_NULL(lpProfile);

		//Trace_Point(29298);// 获得Templete
		lpTemplete = lpProfile->OpenKey(nswHomeTempleteName,false);
		BREAK_ON_NULL(lpTemplete);
		
		//Trace_Point(18621);// 装载Home Page
		lpHomePage = mpAllocator->CreateElement(npFrame,lpTemplete);
		BREAK_ON_NULL(lpHomePage);

		//Trace_Point(28099);//成功了，替moAllWidgets增加对Widget的引用，免得外面的回收代码把它释放掉
		lpWidget->AddRefer();
		luResult = ERESULT_SUCCESS;

	} while (false);

	if(lpWidget!=NULL)
	{
		//Trace_Point(29054);// 可以退出当前Widget了
		PopWidget();
	}

	if(ERESULT_SUCCEEDED(luResult))
	{
		if(nppWidget != NULL && lpWidget!=NULL)
		{
			*nppWidget = lpWidget;
			lpWidget->AddRefer();
		}
	}
	else
	if(lpWidget != NULL && liWidgetInserted >= 0)	// 如果失败，需要清除它
	{
		//Trace_Point(26011);// 将当前Widget从列表中移除
		moWidgetLock.Enter();
		moAllWidgets.RemoveByIndex(liWidgetInserted);
		moWidgetLock.Leave();
	}

	CMM_SAFE_RELEASE(lpFactory);
	CMM_SAFE_RELEASE(lpWidget);
	CMM_SAFE_RELEASE(lpHomePage);
	CMM_SAFE_RELEASE(lpTemplete);
	CMM_SAFE_RELEASE(lpProfile);

	return luResult;
}

// 获得当前系统中的某个Widget接口，如果返回NULL表示此编号之后没有Widget了
// 此函数只能被System Widget调用
IXsWidget* __stdcall CEinkuiSystem::ObtainWidget(
	IN int niNumber		// 从0开始编号得Widegt，编号没有意义，只是Widget的存储位置
	)
{
	CXsWidget* lpWidgetObj = NULL;

	moWidgetLock.Enter();

	if(niNumber < moAllWidgets.Size())
		lpWidgetObj = moAllWidgets[niNumber];

	moWidgetLock.Leave();

	return dynamic_cast<IXsWidget*>(lpWidgetObj);
}

//// 查找Widget
//IXsWidget* __stdcall CEinkuiSystem::FindWidget(
//	IN const wchar_t* nswInstanceName	// 本次运行的实例名
//	)
//{
//	return NULL;
//}

// 获得系统Widget
IXsWidget* __stdcall CEinkuiSystem::GetSystemWidget(void)
{
	CXsWidget* lpWidgetObj = NULL;

	moWidgetLock.Enter();

	if(0 < moAllWidgets.Size())
		lpWidgetObj = moAllWidgets[0];

	moWidgetLock.Leave();

	if(lpWidgetObj == NULL)
		THROW_NULL;

	return dynamic_cast<IXsWidget*>(lpWidgetObj);
}

ERESULT CEinkuiSystem::CloseWidget(
	IN IXsWidget* npWidget
	)
{
	CXsWidget* lpWidget;
	IEinkuiIterator* lpHome;

	if(npWidget == NULL)
		return ERESULT_WRONG_PARAMETERS;

	lpWidget = dynamic_cast<CXsWidget*>(npWidget);
	if(lpWidget == NULL)
		return ERESULT_WRONG_PARAMETERS;

	//Trace_Point(7113);// 关闭此Widget的HomePage
	lpHome = npWidget->GetHomePage();
	if(lpHome!=NULL)
		lpHome->Close();

	//// homepage 必然存在其他引用，我们直接干掉他
	//auto lpToDestroy = dynamic_cast<CXuiIterator*>(lpHome);
	//while (lpToDestroy->KRelease() > 0);

	// 中止此Widget的工作线程

	//Trace_Point(22443);// 将此Widget从widget列表中删除
	moWidgetLock.Enter();

	for(int i=0;i< moAllWidgets.Size();i++)
	{
		if(moAllWidgets[i] == lpWidget)
		{
			moAllWidgets.RemoveByIndex(i);
			break;
		}
	}
	moWidgetLock.Leave();

	lpWidget->Release();

	return ERESULT_SUCCESS;
}


// Widget压栈，供CXelManager调用，当分发一条消息到目标Element时调用，表示切换到新的Widget状态
void CEinkuiSystem::PushWidget(IXsWidget* npWidget)
{
	moWidgetContext.PushWidget(npWidget);
}

// Widget退栈，供ElementManager在完成一个Element的消息是调用，恢复到之前的Widget状态
void CEinkuiSystem::PopWidget(void)
{
	moWidgetContext.PopWidget();
}

// 启动一个新线程，所有的Widget都应该使用这个函数建立自己的额外线程；返回值即Windows线程句柄，可以用于调用SuspendThread/ResumeThread/GetExitCodeThread
// 最终，返回的句柄必须调用CloseHandle关闭；函数的输入参数同Windows API CreateThread一致
HANDLE __stdcall CEinkuiSystem::CreateWidgetWorkThread(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	SIZE_T dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadId
	)
{
	PSTES_CREATE_THREAD lpContext = new STES_CREATE_THREAD;

	if(lpContext == NULL)
		return NULL;

	lpContext->ThreadProc = lpStartAddress;
	lpContext->Context = lpParameter;
	lpContext->Widget = GetCurrentWidget();

	return CreateThread(lpThreadAttributes,dwStackSize,CEinkuiSystem::WidgetWorkThread,lpContext,dwCreationFlags,lpThreadId);
}

// 退出Widget工作线程；当，传入给CreateWidgetWorkThread的线程主函数退出时，系统会自动调用终止线程操作
void __stdcall CEinkuiSystem::ExitWidgetWorkThread(DWORD dwExitCode)
{
	// 删除当前的线程记录
	CEsThreadNode loAdd;

	loAdd.muThreadID = GetCurrentThreadId();

	moWidgetLock.Enter();

	moWidgetWorkThreads.Remove(loAdd);

	moWidgetLock.Leave();
}

// Widget工作线程的入口
ULONG WINAPI CEinkuiSystem::WidgetWorkThread(LPVOID Context)
{
	ULONG luResult = 0;
	PSTES_CREATE_THREAD lpContext = (PSTES_CREATE_THREAD)Context;

	if(Context==NULL)
		return ERROR_INVALID_PARAMETER;

	// 向系统注册自身的线程
	{
		CEsThreadNode loAdd;
		int liPos;

		loAdd.muThreadID = GetCurrentThreadId();
		loAdd.mpOwnerWidget = lpContext->Widget;

		gpXuiSystem->moWidgetLock.Enter();

		liPos = gpXuiSystem->moWidgetWorkThreads.Insert(loAdd);

		gpXuiSystem->moWidgetLock.Leave();

		if(liPos < 0)
			THROW_FALSE;
	}

	// 调用客户的线程过程
	try
	{
		luResult = lpContext->ThreadProc(lpContext->Context);
	}
	catch(...)
	{
	}

	gpXuiSystem->ExitWidgetWorkThread(luResult);

	return luResult;
}

// 打开一个Config文件；用于打开一个config文件，目前应用在Factory接口实现中，用于打开一个Conponent对应的Profile
IConfigFile* __stdcall CEinkuiSystem::OpenConfigFile(
	IN const wchar_t* nszPathName,				// 文件的完整路径名
	IN ULONG nuCreationDisposition	// 同CreateFile API类似，但仅包括CF_CREATE_ALWAYS、CF_CREATE_NEW、CF_OPEN_ALWAYS、CF_OPEN_EXISTING，定义见CfgIface.h
	)
{
	return CfCreateConfig(nszPathName,nuCreationDisposition);
}


LRESULT CALLBACK CEinkuiSystem::MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	
	ERESULT luXuiResult = gpXuiSystem->DoCapture(hwnd,message,wParam,lParam,(HRESULT&)result);

	if(luXuiResult != ERESULT_WINMSG_SENDTO_NEXT)
		return result;

	switch (message)
	{
	case WM_IME_NOTIFY:
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_COMPOSITION:
		if(gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false && gpXuiSystem->mpImeContext != NULL)
			result = gpXuiSystem->mpImeContext->OnImeMessage(hwnd,message,wParam,lParam);
		else
			result = DefWindowProc(hwnd, message, wParam, lParam);
		break;
	case WM_CREATE:
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));

			BOOL lbForceIconic = TRUE;
			BOOL lbHasIconicBitmap = TRUE;
			DwmSetWindowAttribute(
				hwnd,
				DWMWA_FORCE_ICONIC_REPRESENTATION,
				&lbForceIconic,
				sizeof(lbForceIconic));

			DwmSetWindowAttribute(
				hwnd,
				DWMWA_HAS_ICONIC_BITMAP,
				&lbHasIconicBitmap,
				sizeof(lbHasIconicBitmap));

			result = gpXuiSystem->WsgOnCreate(hwnd);
		}
		break;
	case WM_MOVE:
		{
			result = 0;
		}
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
//	case WM_SYSKEYDOWN:
		{
			STELEMGR_WINMSG_FORWARD ldInput;

			if(gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false)
			{
				ldInput.WinMsgID = message;
				ldInput.wParam = wParam;
				ldInput.lParam = lParam;

				gpXuiSystem->mpElementManager->SimplePostMessage(NULL,EMSG_KEYBOARD_FORWARD,&ldInput,sizeof(ldInput),EMSG_POSTTYPE_FAST);
			}
			result = 0;
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
		result = 0;
		break;
	case WM_ERASEBKGND:
		result = 1;
		break;
	case WM_NCPAINT:
	case WM_NCCALCSIZE:
	//case WM_NCACTIVATE:
		result = 0;
		break;
	case WM_TIMER:
		{
			//$ax$ Nov.27, 2018 暂时屏蔽，因为部分操作(打开电子书）耗时过长，没有采取合理的多任务方式，会导致消息处理的暂时阻塞，
			// 所以先屏蔽了定时器申请
			if(wParam == 0 && gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false)
			{
				gpXuiSystem->DetermineOPRunming();
			}
		}
		result = 0;
		break;

	case WM_DESTROY:
		{
			//Trace_Point(19053);// 退出程序
			if(gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false && gpXuiSystem->mhOperationThread != NULL)
			{
				//Trace_Flush();

				gpXuiSystem->mpElementManager->EndMessageQueue();

				CExMessage::PostMessage(NULL,NULL,EMSG_QUIT_XUI,CExMessage::DataInvalid,EMSG_POSTTYPE_QUIT);

				WaitForSingleObject(gpXuiSystem->mhOperationThread,10000);
			}
			PostQuitMessage(0);
		}
		result = 1;
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_MOUSELEAVE:
		{
			if(gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false)
				 gpXuiSystem->OnWinMouse(hwnd,message,wParam,lParam);
			result = 0;
		}
		break;
	case WM_EI_TOUCH:
		{
		if (gpXuiSystem->TestFlag(ESYS_FLAG_INITOK) != false)
			gpXuiSystem->OnEinkTouch(hwnd, message, wParam, lParam);
		result = 0;
		}
		break;
	case WM_EI_DRAW:
	{
		gpXuiSystem->ClearEinkBuffer();
		gpXuiSystem->UpdateView();
	}
	case WM_EI_ACTIVATE:
	{
		if (wParam != 0)
		{
			//说明是被切换到前台了
			SET_TP_AREA ldArea;
			ldArea.Rect.x = 0;
			ldArea.Rect.w = 1920;
			ldArea.Rect.y = 0;
			ldArea.Rect.h = 1080;
			ldArea.Index = 0;
			ldArea.Flag = 0;
			EiSetTpArea(ldArea);

			ldArea.Flag = SET_SP_AREA_TOUCH_PEN;
			EiSetTpArea(ldArea);

			//切换过来先清屏
			EiSetWaveformMode(GI_WAVEFORM_DU2);
			EiCleanupScreen(0xff);
			Sleep(585); //DU260+15*5   分16帧刷，每帧间隔5ms
			EiSetWaveformMode(GI_WAVEFORM_GC16);
			gpXuiSystem->ClearEinkBuffer();

			gpXuiSystem->EnableRender();
			gpXuiSystem->UpdateView();

			gpXuiSystem->moWidgetContext.EnableTickDetection(true);
		}
		else
		{
			gpXuiSystem->DisableRender();

			gpXuiSystem->moWidgetContext.EnableTickDetection(false);
		}
		
	}
	break;
	case WM_EI_ROTATION:
	{
		if (gpXuiSystem->muAutoRotate != 0)
		{
			ULONG lulRoration = (ULONG)wParam;

			gpXuiSystem->mpElementManager->SimplePostMessage(NULL, EMSG_SCREEN_ROTATION,&lulRoration,sizeof(lulRoration), EMSG_POSTTYPE_FAST);

		}
	}
	break;
	case WM_SYSCOMMAND:
		switch(wParam)
		{
		case SC_CLOSE:
			DestroyWindow(gpXuiSystem->GetMainWindow());
			break;
		//case SC_MINIMIZE:
		//	gpXuiSystem->mpXuiGraphics->StopPainting(true);
		//	CExMessage::SendMessage(NULL,NULL,0,CExMessage::DataInvalid);	//为了和绘制线程同步，防止当前仍然在绘制；

		//	gpXuiSystem->mbWindowHide = true;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		break;
	case WM_POWERBROADCAST:
		if(wParam == PBT_APMRESUMEAUTOMATIC)
		{
			//wark up
			gpXuiSystem->moWidgetContext.EnableTickDetection(true);
		}
		else
		if(wParam == PBT_APMSUSPEND)
		{
			//sleep
			gpXuiSystem->moWidgetContext.EnableTickDetection(false);
		}
		else if (wParam == PBT_POWERSETTINGCHANGE)
		{
			//connect standby
			POWERBROADCAST_SETTING* lpSetting = (POWERBROADCAST_SETTING*)lParam;
			BREAK_ON_NULL(lpSetting);
			if (lpSetting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE)
			{
				DWORD ldwStatus = 0;
				memcpy_s(&ldwStatus, sizeof(DWORD), lpSetting->Data, sizeof(DWORD));
				if (ldwStatus == 0)
				{
					//进入connect standby
					OutputDebugString(L"enter connect standby");
					gpXuiSystem->moWidgetContext.EnableTickDetection(false);
				}
				else if (ldwStatus == 1)
				{
					//离开connect standby
					OutputDebugString(L"leave connect standby");
					gpXuiSystem->moWidgetContext.EnableTickDetection(true);
				}
				else if (ldwStatus == 2)
				{
					//关机
					OutputDebugString(L"Os shut down");
				}
			}
		}
		else
			DefWindowProc(hwnd, message, wParam, lParam);
		break;
	case WM_TOUCH:
		{
			gpXuiSystem->mbIsTouch = true;

			break;
		}
	case WM_DWMSENDICONICTHUMBNAIL:
		{
			// This window is being asked to provide its iconic bitmap. This indicates
			// a thumbnail is being drawn.
			//任务栏上的小预览图
			//HBITMAP lhBitmap = gpXuiSystem->mpXuiGraphics->GetCurrentBitmap(HIWORD(lParam), LOWORD(lParam)); 
			HBITMAP lhBitmap = NULL; 
			CExMessage::SendMessage(NULL,NULL,EMSG_MAIN_GET_CURRENT_BITMAP,lParam,&lhBitmap,sizeof(HBITMAP));

			if (lhBitmap != NULL)
			{
				if(gpXuiSystem->mpSmallPreView != NULL)
					DeleteObject(gpXuiSystem->mpSmallPreView);

				gpXuiSystem->mpSmallPreView = lhBitmap;
			}

			DwmSetIconicThumbnail(hwnd, gpXuiSystem->mpSmallPreView, 0);

		}
		break;
// 	case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
// 		break;
	default:
		if(gpXuiSystem->TestFlag(ESYS_FLAG_INITOK)!=false && gpXuiSystem->muiCustomWinMsg == message)
		{
			result = gpXuiSystem->MessageFromXuiToWindowUi(wParam,lParam);
		}
		else
			result = DefWindowProc(hwnd, message, wParam, lParam);
	}

	return result;
}

// 主窗口的Windows消息过程函数
LRESULT CEinkuiSystem::MessageFromXuiToWindowUi(
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch((ULONG)wParam)
	{
	case 'WgCb':
		{
			// 回调
			gpXuiSystem->CallClientProcedure((PSTES_WINTHREAD_CALLBACK)lParam);
		}
		break;
	case 'Exit':
		{
			DestroyWindow(gpXuiSystem->mhMainWnd);
		}
		break;
	case 'Show':
		{
			ShowWindow(gpXuiSystem->mhMainWnd,(ULONG)lParam!=0?SW_SHOW:SW_HIDE);
			if((ULONG)lParam!=0)
				UpdateWindow(gpXuiSystem->mhMainWnd);
		}
		break;
	case 'Idtt':
		{
			// 获取识别码
		}
		break;
	//case 'UdWu':
	//	{
	//		if(lParam != NULL)
	//			InvalidateRect(mhMainWnd,NULL,FALSE);

	//		//因为我们的界面重新绘制了，所以需要激发系统下次仍然向我们索要显示的预览图像
	//		DwmInvalidateIconicBitmaps(mhMainWnd);
	//	}
	//	break;
	//case 'CWSz':
	//	{
	//		SetWindowPos(mhMainWnd,NULL,mlNewPaintboardX,mlNewPaintboardY,mlNewPaintboardWidth,mlNewPaintboardHeight,SWP_NOZORDER);
	//	}
	//	break;
	default:;
	}

	return TRUE;
}


void CEinkuiSystem::RenderProcedule(ULONG nuCrtTick)
{
	if (miDisableRender != 0 || nuCrtTick - muRenderTick < EINK_RENDER_INTERVAL)	// 与前一次渲染的间隔不到间隔要求
	{
		// 设置定时器用渲染标志，而后返回
		InterlockedExchange(&miToRender,1);
		return;
	}

	mpXuiGraphics->DoRender(nuCrtTick, InterlockedCompareExchange(&miRefreshEink, 0, 1) != 0);

	// 阻止消息队列中多余的刷新消息
	InterlockedExchange(&miDiscardRenderMessage, 1);
	ULONG flag = 1;
	mpElementManager->SimplePostMessage(NULL, EMSG_SYSTEM_RENDER,&flag,sizeof(flag));//当系统对象收到此消息时，恢复刷新时序，清除miDiscardRenderMessage标志

	//InterlockedCompareExchange(&miMissedMouseTest, 1, 0);
	if (/*miMissedMouseTest > 0 &&*/ InterlockedIncrement(&miMissedMouseTest) >= 3)
	{
		InterlockedExchange(&miMissedMouseTest, 0);

		//执行一次鼠标检测，确保鼠标下的元素能得到正确的鼠标输入
		mpElementManager->OnMsgMouseForward(NULL);
	}
}

// 收到Windows Mouse消息
void CEinkuiSystem::OnWinMouse(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_LBUTTONDOWN:
		SetWinMouseCapture(EELMGR_WINCAP_LB);
		break;
	case WM_RBUTTONDOWN:
		SetWinMouseCapture(EELMGR_WINCAP_RB);
		break;
	case WM_MBUTTONDOWN:
		SetWinMouseCapture(EELMGR_WINCAP_MB);
		break;
	case WM_LBUTTONUP:
		ReleaseWinMouseCapture(EELMGR_WINCAP_LB);
		break;
	case WM_RBUTTONUP:
		ReleaseWinMouseCapture(EELMGR_WINCAP_RB);
		break;
	case WM_MBUTTONUP:
		ReleaseWinMouseCapture(EELMGR_WINCAP_MB);
		break;
	default:;
	}

	STELEMGR_WINMSG_FORWARD ldMouse;

	ldMouse.WinMsgID = message;
	ldMouse.wParam = wParam;
	ldMouse.lParam = lParam;

	mpElementManager->SimplePostMessage(NULL,EMSG_MOUSE_FORWARD,&ldMouse,sizeof(ldMouse),EMSG_POSTTYPE_FAST);
}

// 收到Eink Touch消息
void CEinkuiSystem::OnEinkTouch(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
// 修改此代码，将触屏消息按照鼠标消息转发，触屏逻辑未完整，Ax.2017.08.16
// 	PSTEMS_TOUCH lpTouch = (PSTEMS_TOUCH)lParam;
// 
// 	if (lpTouch == NULL)
// 		return;
// 
// 	mpElementManager->SimplePostMessage(NULL, EMSG_TOUCH_FORWARD, lpTouch, sizeof(STEMS_TOUCH), EMSG_POSTTYPE_NORMAL);
	STELEMGR_WINMSG_FORWARD ldMouse;
	PEI_TOUCHINPUT_POINT lpTouch = (PEI_TOUCHINPUT_POINT)lParam;

	if (lpTouch == NULL)
		return;

	ldMouse.lParam = MAKELONG((SHORT)lpTouch->x, (SHORT)lpTouch->y);
	switch (lpTouch->Flags)
	{
	case EI_TOUCHEVENTF_DOWN:
		ldMouse.WinMsgID = WM_LBUTTONDOWN;
		ldMouse.wParam = MK_LBUTTON;
		break;
	case EI_TOUCHEVENTF_MOVE:
		ldMouse.WinMsgID = WM_MOUSEMOVE;
		ldMouse.wParam = MK_LBUTTON;
		break;
	case EI_TOUCHEVENTF_UP:
		ldMouse.WinMsgID = WM_LBUTTONUP;
		ldMouse.wParam = 0;
		break;
	default:
		return;
	}

	mpElementManager->SimplePostMessage(NULL, EMSG_MOUSE_FORWARD, &ldMouse, sizeof(ldMouse), EMSG_POSTTYPE_FAST);// EMSG_POSTTYPE_NORMAL);

}


// 回调申请者的函数，成功后给操作线程发送恢复消息
void CEinkuiSystem::CallClientProcedure(PSTES_WINTHREAD_CALLBACK npToCall)
{
	//ULONG ldUlongArr[2];

	//if(npToCall == NULL || npToCall->Signature != 'WgCc')
	//	ldUlongArr[1] = ERESULT_WRONG_PARAMETERS;
	//else
	//	ldUlongArr[1] = (npToCall->Applicant->*npToCall->ProcedureToCall)(npToCall->Flag,npToCall->ConText);

	//// 发送消息给操作线程
	//ldUlongArr[0] = npToCall->BlockID;

	//mpElementManager->SimplePostMessage(NULL,EMSG_WINCALL_COMPLETED,ldUlongArr,sizeof(ldUlongArr),EMSG_POSTTYPE_FAST);

	ERESULT luResult;

	if(npToCall == NULL || npToCall->Signature != 'WgCc')
		luResult = ERESULT_WRONG_PARAMETERS;
	else
		luResult = (npToCall->Applicant->*npToCall->ProcedureToCall)(npToCall->Flag,npToCall->ConText);

	moThreadBlock.SetBlockState(npToCall->BlockID,luResult);

	//if(npToCall->WaitEvent != NULL)
	//{
	//	SetEvent(npToCall->WaitEvent);
	//}

	delete npToCall;
}

// 申请使用Windows交互线程回调，如果Widget开发中需要调用Windows的界面模块，显示额外的Windows界面或者做与Windows界面相关的必须在Windows交互线程中执行的功能，
// 将该功能代码移植到自己Element或者其他输出IBaseObject的类的一个独立函数中，通过调用本方法，可以使得Windows界面线程主动回调设定的独立函数，而该函数的返回值也将
// 通过本函数直接返回给本方法的调用者。完成上述功能的过程中，本处的代码将阻塞在此方法中，并且确保整个界面正常刷行，但，Xui本身的界面将不再相应。
ERESULT __stdcall CEinkuiSystem::CallBackByWinUiThread(
	IBaseObject* npApplicant,		// 回调对象，并不一定要求是调用本方法的对象本身，也可以是别的对象指针
	PXUI_CALLBACK npProcedureToCall,	//回调过程，过程的返回值将会返回给调用者
	ULONG nuFlag,			// 传递给回调函数的参数
	LPVOID npConText,		// 传递给回调函数的参数
	bool nbStall		// 等待返回期间是否需要处理后续的XUI消息，如果nbStal设为ture程序将直接等待返回，而不处理XUI消息循环
						// 使用此方法可以避免调用本方法期间，再次重入调用
	)
{
	STES_WINTHREAD_CALLBACK* lpCall;
	LRESULT luResult;
	ULONG luID = 0;
	//HANDLE lhEvent = NULL;
	bool lbOperationThread = IsRunningInOperationThread();

	lpCall = new STES_WINTHREAD_CALLBACK;
	if(lpCall == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luID = moThreadBlock.AddBlock();

	lpCall->BlockID = luID;
	lpCall->Applicant = npApplicant;
	lpCall->Flag = nuFlag;
	lpCall->ConText = npConText;
	lpCall->ProcedureToCall = npProcedureToCall;
	lpCall->Signature = 'WgCc';
	lpCall->Size = sizeof(STES_WINTHREAD_CALLBACK);

	//if(IsRunningInOperationThread()==false || nbStall != false)
	//{
	//	lhEvent = CreateEvent(NULL,true,false,NULL);
	//	if(lhEvent == NULL)
	//		return ERESULT_INSUFFICIENT_RESOURCES;
	//}
	//lpCall->WaitEvent = lhEvent;
	if(lbOperationThread == false || nbStall != false)
		luResult = (::SendMessage(GetMainWindow(),muiCustomWinMsg,'WgCb',(LPARAM)lpCall)!=FALSE)?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL;
	else
		luResult = (::PostMessage(GetMainWindow(),muiCustomWinMsg,'WgCb',(LPARAM)lpCall)!=FALSE)?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL;

	if(luResult == ERESULT_SUCCESS)
	{
		if(lbOperationThread != false && nbStall == false)
		{
			// 抛弃界面输入
			//InterlockedIncrement(&mlWinCallBack);

			// 分解消息，等待CallID相同的消息到达
			luResult = OpThreadMessageLoop(luID);
			if(luResult == ERESULT_QUIT_XUI)
			{
				// 如果在这个时间点，恰好收到了退出消息，那，这是一个非常不好的情况，直接退出程序
				ExitProcess(-1);
			}

			//InterlockedDecrement(&mlWinCallBack);
		}

		luResult = moThreadBlock.ReadBlockState(luID);

		moThreadBlock.RemoveBlock(luID);

	}

	//if(lhEvent != NULL)
	//	CloseHandle(lhEvent);

	return (ULONG)luResult;
}

// 注册Windows消息拦截，通过本功能可以在XUI系统处理Windows消息之前截取关注的Windows消息
// 处理截取的Windows消息的过程要尽可能短暂，因为此刻XUI系统的两个主要线程处于同步等待状态。
// 返回ERESULT_WINMSG_SENDTO_NEXT，XUI将消息传递给下一个拦截者，或者交由XUI系统解释处理；返回其他值将终止该Windows消息的传递过程以及XUI对该消息的处理
ERESULT __stdcall CEinkuiSystem::CaptureWindowsMessage(
	IN UINT nuiWinMsgID,	// 希望博获的Windows消息的ID
	IN IXsElement* npApplicant,	//申请回调的对象
	IN PWINMSG_CALLBACK npProcedure	//将Windows消息发送给此函数
	)
{
	CEsWsgCapture loAdd;
	ERESULT luReval = ERESULT_UNSUCCESSFUL;

	if(npApplicant == NULL)
		return ERESULT_ELEMENT_INVALID;

	moCaptorLock.Enter();

	loAdd.muiWsgID = nuiWinMsgID;
	loAdd.mpApplicant= npApplicant;
	loAdd.mpAppItr = npApplicant->GetIterator();
	loAdd.mpProcedure = npProcedure;

	if(mpElementManager->VerifyIterator(loAdd.mpAppItr)!=ERESULT_SUCCESS)
		return ERESULT_ITERATOR_INVALID;

	if(moCaptors.Insert(loAdd,false)>=0)
		luReval = ERESULT_SUCCESS;

	moCaptorLock.Leave();

	return luReval;
}


// 产生一个命令，此方法通常为SystemWidget的菜单模块调用，用来模拟一次用户按下组合键行为，该命令将会被发送至当前的键盘焦点以上的具有EITR_STYLE_COMMAND的对象
ERESULT __stdcall CEinkuiSystem::GenerateCommand(
	nes_command::ESCOMMAND neCmd
	)
{
	STEMS_HOTKEY ldHotKey;
	ERESULT luResult = ERESULT_SUCCESS;

	switch(neCmd)
	{
	case nes_command::eCopy:
		ldHotKey.HotKeyID = EHOTKEY_COPY;
		break;
	case nes_command::eCut:
		ldHotKey.HotKeyID = EHOTKEY_CUT;
		break;
	case nes_command::ePaste:
		ldHotKey.HotKeyID = EHOTKEY_PASTE;
		break;
	case nes_command::eDelete:
		ldHotKey.HotKeyID = EHOTKEY_DELETE;
		break;
	case nes_command::eSelAll:
		ldHotKey.HotKeyID = EHOTKEY_SELECT_ALL;
		break;
	default:
		luResult = ERESULT_UNSUCCESSFUL;
	}

	if(luResult == ERESULT_SUCCESS)
		CExMessage::PostMessage(NULL,NULL,EMSG_HOTKEY_PRESSED,ldHotKey,EMSG_POSTTYPE_NORMAL);

	return luResult;
}


//// 启动侦测线程
//ERESULT CEinkuiSystem::ExecuteDetermineThread(LPVOID /*Context*/)
//{
//	ULONG luThreadID;
//	HANDLE lhThread = CreateThread(NULL,128*1024,(LPTHREAD_START_ROUTINE)CEinkuiSystem::DetermineThread,NULL,NULL,&luThreadID);
//
//	if(lhThread == NULL)
//		return ERESULT_UNSUCCESSFUL;
//
//	mhOperationThread = lhThread;
//
//	return ERESULT_SUCCESS;
//}


//// 侦测线程
//ULONG WINAPI CEinkuiSystem::DetermineThread(LPVOID /*Context*/)
//{
//	do 
//	{
//		if(WaitForSingleObject(gpXuiSystem->mhExitDetermine,1000)!= WAIT_TIMEOUT)
//			break;
//
//		gpXuiSystem->DetermineOPRunming();
//
//	} while(1);
//
//	return 0;
//}
// 检查操作线程运行状态
void CEinkuiSystem::DetermineOPRunming(void)
{
	//$ax$ Nov.27, 2018 暂时屏蔽，因为部分操作(打开电子书）耗时过长，没有采取合理的多任务方式，会导致消息处理的暂时阻塞，
	// 所以先屏蔽了对此处的调用，搜索gpXuiSystem->DetermineOPRunming();
	ULONG luElapsed;
	bool lbPullOut = false;
	CFilePathName loFile;

	if(IsDebuggerPresent() != FALSE)
		return ;

	//Trace_ULONG(28132,GetTickCount());//检查线程运行
	moWidgetLock.Enter();

	luElapsed = moWidgetContext.CheckElapsedTick();
	//Trace_ULONG(26368,luElapsed);//用去时间
	if(luElapsed >= 10000)	// 大于10秒没有响应
	{
		if(moWidgetContext.HasTriedPulling()==false)
		{
			//Trace_Point(19018);//激发线程异常
			moWidgetContext.SetTriedPulling();
			lbPullOut = CThreadAbort::PullOut(mhOperationThread);
		}
		//else
		//if(luElapsed <= 10000)
		//{	
		//	Trace_Point(19067);// 小于8秒时，暂时不动作
		//	lbPullOut = true;
		//}
		else
		{
			//Trace_Point(23800);//已经尝试过将线程拖出泥潭，失败啊，那么关闭进程重新启动吧
		}
		//if(lbPullOut != false)
			//Trace_Flush();
	}

	moWidgetLock.Leave();

	return;
	// 本版本不提供自动重新启动程序的功能
	//if(luElapsed < 10000 || lbPullOut != false)
	//	return ;

	//Trace_Point(18027);//直接恢复失败，杀掉线程
	//TerminateThread(mhOperationThread,-1);

	//// reload myself

	//loFile.SetByModulePathName();

	//Trace_Point(18923);//重新启动进程
	//ShellExecute(NULL,L"open",loFile.GetPathName(),L"Reload",NULL,SW_SHOW);

	////ExitProcess(-1);
	//Trace_Flush();

	//DestroyWindow(gpXuiSystem->mhMainWnd);
}


// 处理Windows WM_CREATE消息
LRESULT CEinkuiSystem::WsgOnCreate(HWND nhWindow)
{
	mhMainWnd = nhWindow;

	return 0;
}

// 申请定时器，对于永久触发的定时器，需要注销
ERESULT CEinkuiSystem::SetTimer(
	IN CXuiIterator* npIterator,// 申请者
	IN ULONG nuID,	  // 定时器ID
	IN ULONG nuRepeat,// 需要重复触发的次数，MAXULONG32表示永远重复
	IN ULONG nuDuration,	// 触发周期
	IN void* npContext//上下文，将随着定时器消息发送给申请者
	)
{
	ERESULT luResult = ERESULT_WRONG_PARAMETERS;
	PSTES_TIMER lpTimer = NULL;

	moTimerListLock.Enter();

	do 
	{
		if(npIterator == NULL || nuID == 0 || nuDuration == 0 || nuRepeat == 0)
			break;

		if(FindTimer(npIterator,nuID)!=false)
		{
			luResult = ERESULT_OBJECT_EXISTED;
			break;
		}

		lpTimer = new STES_TIMER;
		if(lpTimer == NULL)
		{
			ERESULT_INSUFFICIENT_RESOURCES;
			break;
		}

		lpTimer->mpIterator = npIterator;
		lpTimer->muID = nuID;
		lpTimer->muBaseTick = GetTickCount();
		lpTimer->muRecentTick = lpTimer->muBaseTick;
		lpTimer->muEndTick = lpTimer->muBaseTick + nuDuration;
		lpTimer->muDuration = nuDuration;
		lpTimer->muKicked = 0;
		lpTimer->muRepeat = nuRepeat;
		lpTimer->mpContext = npContext;

		if(moTimers.Insert(lpTimer)<0)
			luResult = ERESULT_UNSUCCESSFUL;

		// 成功了
		luResult = ERESULT_SUCCESS;

	} while (false);

	moTimerListLock.Leave();

	// 释放消息计数器，以使的定时器尽快做触发测试
	mpElementManager->SimplePostMessage(NULL,EMSG_WAKE_UP,NULL,0);

	return luResult;
}

// 销毁定时器
ERESULT CEinkuiSystem::KillTimer(
	IN CXuiIterator* npIterator,// 申请者
	IN ULONG nuID	  // 定时器ID
	)
{
	ERESULT luResult;

	moTimerListLock.Enter();

	luResult = FindTimer(npIterator,nuID,true)?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL;

	moTimerListLock.Leave();

	return luResult;
}

// 查找或者删除一个定时器，供定时器处理函数内部调用
bool CEinkuiSystem::FindTimer(
	IN CXuiIterator* npIterator,// 申请者
	IN ULONG nuID,	  // 定时器ID
	IN bool nbDelete	// 是否删除
	)
{
	bool lbResult = false;

	moTimerListLock.Enter();

	for (int i=0; i < moTimers.Size();i++)
	{
		if(moTimers[i]->mpIterator == npIterator && moTimers[i]->muID == nuID)
		{
			lbResult = true;
			if(nbDelete != false)
			{
 				STES_TIMER* lpTimer = moTimers[i];
 				CMM_SAFE_DELETE(lpTimer);
				lbResult = moTimers.RemoveByIndex(i);
			}
			break;
		}
	}

	moTimerListLock.Leave();

	return lbResult;
}


// 从定时器队列中取最近一个可能激发的时间间隔，tick（毫秒）为单位
ULONG CEinkuiSystem::GetTickCountToKickTimer(ULONG nuCrtTick)
{
	ULONG luTick = 33;	// 如果不能取得值，就没33毫秒唤醒操作线程一次
	ULONG luCrtTick = nuCrtTick;

	moTimerListLock.Enter();

	if(moTimers.Size() > 0)
	{
		if(luCrtTick >= moTimers[0]->muEndTick)
			luTick = 0;	// 已经到激发的时刻了
		else
			luTick = moTimers[0]->muEndTick - luCrtTick;
	}

	moTimerListLock.Leave();

	return luTick;
}


// 定时器激发测试
void CEinkuiSystem::KickTimers(ULONG nuCrtTick)
{
	STEMS_TIMER ldTimerParam;
	PSTES_TIMER lpTimer;

	moTimerListLock.Enter();

	for (int i = moTimers.Size()-1;i>=0;)	// 从最远开始判断
	{
		if(moTimers[i]->muEndTick <= nuCrtTick)
		{
			// 被激发

			// 首先把定时器从队列中拿下来
			lpTimer = moTimers[i];
			moTimers.RemoveByIndex(i);

			ldTimerParam.TimerID = lpTimer->muID;
			ldTimerParam.BaseTick = lpTimer->muBaseTick;
			ldTimerParam.CurrentTick = nuCrtTick;
			ldTimerParam.ElapsedTick = nuCrtTick - lpTimer->muRecentTick;
			ldTimerParam.Kicked = ++lpTimer->muKicked;
			ldTimerParam.Context = lpTimer->mpContext;

			if(ERESULT_SUCCEEDED(mpElementManager->SimplePostMessage(lpTimer->mpIterator,EMSG_TIMER,&ldTimerParam,sizeof(ldTimerParam))))
			{
				// 修改定时器的状态值
				lpTimer->muRecentTick = nuCrtTick;

				do 
				{	// 准备下一个激发点，跳过所有当前激发点与当前时间之间未激发点
					lpTimer->muEndTick += lpTimer->muDuration;
					if(lpTimer->muRepeat != MAXDWORD)
						lpTimer->muRepeat--;
				} while (lpTimer->muRepeat > 0 && lpTimer->muEndTick <= nuCrtTick);
			}
			else
			{
				//Trace_Point(16926);//发送定时器消息失败
				lpTimer->muRepeat = 0;//不再使用该定时器，关闭它
			}


			// 判断是否需要插回到定时器队列中
			if(lpTimer->muRepeat > 0)
				moTimers.Insert(lpTimer);
			else 
				CMM_SAFE_DELETE(lpTimer);	//要释放资源

			// 重新从队尾检测定时器
			i = moTimers.Size()-1;

			continue;

		}
		i--;
	}

	moTimerListLock.Leave();
}

// 用于接收输入消息
ERESULT CEinkuiSystem::SystemMessageReceiver(IEinkuiMessage* npMsg)
{
	ERESULT luResult=ERESULT_UNEXPECTED_MESSAGE;

	try
	{
		switch (npMsg->GetMessageID())
		{
		case EMSG_SCREEN_ROTATION:
		{
			ULONG lulRoration = *(ULONG*)npMsg->GetInputData();
			ULONG luAccept = MAXULONG32;

			// 询问Widget是否转屏
			CExMessage::SendMessage(gpXuiSystem->mpElementManager->GetDesktop(), NULL, EMSG_QUERY_SWGT_ROTATED, lulRoration, &luAccept, sizeof(luAccept));

			if(luAccept == MAXULONG32)
				break;

			EiSetScreenOrient(luAccept);
			gpXuiSystem->ResetPaintboard();

			CExMessage::SendMessage(gpXuiSystem->mpElementManager->GetDesktop(), NULL, EMSG_SWGT_ROTATED, luAccept);

			gpXuiSystem->UpdateView();
		}
		break;
		case EMSG_SYSTEM_RENDER:
		{
			ULONG luTick;

			if (*(ULONG*)npMsg->GetInputData() == 0)
			{
				if (miDiscardRenderMessage != 0)
				{
					luResult = ERESULT_SUCCESS;
					break;
				}
				luTick = GetTickCount();
				RenderProcedule(luTick);
			}
			else
			if (*(ULONG*)npMsg->GetInputData() == 1)
					InterlockedExchange(&miDiscardRenderMessage, 0);

			luResult = ERESULT_SUCCESS;
		}
			break;
		case EMSG_MOUSE_FORWARD:
			{
				// modified by ax 无需这么做，而且，下面的代码还有严重Bug，当一个Windows线程回调的等待过程中，执行一个模态窗口，就会死锁了
				//if(mlWinCallBack != 0)	// 如果当前处于交互锁定状态，丢弃它
				//	break;

				if(npMsg->GetInputDataSize() != sizeof(STELEMGR_WINMSG_FORWARD))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				const PSTELEMGR_WINMSG_FORWARD lpForward = (const PSTELEMGR_WINMSG_FORWARD)npMsg->GetInputData();
				InterlockedExchange(&miMissedMouseTest,0);
				luResult = mpElementManager->OnMsgMouseForward(lpForward);

				if(lpForward->WinMsgID == WM_LBUTTONUP)
					mbIsTouch = false;
			}
			break;
		case EMSG_KEYBOARD_FORWARD:
			{
				// modified by ax 无需这么做，而且，下面的代码还有严重Bug，当一个Windows线程回调的等待过程中，执行一个模态窗口，就会死锁了
				//if(mlWinCallBack != 0)	// 如果当前处于交互锁定状态，丢弃它
				//	break;

				if(npMsg->GetInputDataSize() != sizeof(STELEMGR_WINMSG_FORWARD))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				luResult = mpElementManager->OnMsgKeyboardForward((const PSTELEMGR_WINMSG_FORWARD)npMsg->GetInputData());
			}
			break;
		case EMSG_INPUT_ENABLE:
			{
				luResult = mpElementManager->OnMsgEnalbeInput();
			}
			break;
		// 修改此代码，将触屏消息按照鼠标消息转发，触屏逻辑未完整，Ax.2017.08.16
		//case EMSG_TOUCH_FORWARD:
		//	{
		//		if (npMsg->GetInputDataSize() != sizeof(STEMS_TOUCH))
		//		{
		//			luResult = ERESULT_WRONG_PARAMETERS;
		//			break;
		//		}

		//		const PSTEMS_TOUCH lpForward = (const PSTEMS_TOUCH)npMsg->GetInputData();
		//		InterlockedExchange(&miMissedMouseTest, 0);
		//		luResult = mpElementManager->OnMsgEinkTouchForward(lpForward);
		//	}
		//	break;
		case EMSG_TIMER:
			{
				if(npMsg->GetInputDataSize() != sizeof(STEMS_TIMER))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				luResult = OnMsgTimer((const PSTEMS_TIMER)npMsg->GetInputData());
			}
			break;
		case  EMSG_QUIT_XUI:
			{
				EndXui();
				luResult = ERESULT_QUIT_XUI;	// 通知线程退出
			}
			break;
		//case EMSG_WINCALL_COMPLETED:
		//	{
		//		if(npMsg->GetInputDataSize() == sizeof(ULONG)*2)
		//		{
		//			moThreadBlock.SetBlockState(*(ULONG*)npMsg->GetInputData(),*((ULONG*)npMsg->GetInputData()+1));
		//		}
		//		luResult = ERESULT_QUIT_OPLOOP;
		//	}
		//	break;
		//case EMSG_MODAL_COMPLETED:
		//	{
		//		if(npMsg->GetInputDataSize() == sizeof(ULONG))
		//		{
		//			muOpLoopResult = *(ULONG*)npMsg->GetInputData();
		//		}
		//		luResult = ERESULT_QUIT_OPLOOP;
		//	}
		//	break;
		case EMSG_WAKE_UP:
			luResult = ERESULT_SUCCESS;
			break;
		case EMSG_SET_KEYBOARD_FOCUS:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*) || (*(IEinkuiIterator**)npMsg->GetInputData())->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				luResult = mpElementManager->ApplyKeyBoard(*(IEinkuiIterator**)npMsg->GetInputData());
			}
			break;
		case EMSG_SET_ACTIVE:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*) || (*(IEinkuiIterator**)npMsg->GetInputData())->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				mpElementManager->AssignActivation(*(IEinkuiIterator**)npMsg->GetInputData());
				luResult = ERESULT_SUCCESS;
			}
			break;
		case EMSG_RELEASE_KEYBOARD_FOCUS:
			{
				if(npMsg->GetInputDataSize() != sizeof(ST_RELEASE_KEYFOCUS))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				mpElementManager->ReleaseKeyBoard((PST_RELEASE_KEYFOCUS)npMsg->GetInputData());
				luResult =ERESULT_SUCCESS;
			}
			break;
		//case EMSG_MAIN_RESIZED:
		//	{
		//		if(npMsg->GetInputDataSize() != sizeof(ULONG)*3)
		//		{
		//			luResult = ERESULT_WRONG_PARAMETERS;
		//			break;
		//		}
		//		if(mpXuiGraphics != NULL)
		//		{
		//			if(*((ULONG*)npMsg->GetInputData()+2) != 0)
		//				mpXuiGraphics->Resize(-1,-1);

		//			ULONG luSize[2];
		//			Sleep(50);	//稍稍休息一会
		//			luSize[0] = *(ULONG*)npMsg->GetInputData();
		//			luSize[1] = *((ULONG*)npMsg->GetInputData()+1);

		//			mpXuiGraphics->Resize(luSize[0],luSize[1]);

		//			//使用SendMessage，否则会使界面看起来调整的比较慢
		//			CExMessage::SendMessage(mpElementManager->GetDesktop(),NULL,EMSG_SWGT_MW_RESIZED,luSize);
		//		}
		//		luResult = ERESULT_SUCCESS;
		//	}
		//	break;
		case EMSG_OPTHREAD_LOCK:
			{
				if(npMsg->GetInputDataSize() != sizeof(STEMS_OPT_LOCK))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				SetEvent(((PSTEMS_OPT_LOCK)npMsg->GetInputData())->OptToRelease);

				mbLocked = true;
				WaitForSingleObject(((PSTEMS_OPT_LOCK)npMsg->GetInputData())->OptToWait,INFINITE);

				mbLocked = false;
				SetEvent(((PSTEMS_OPT_LOCK)npMsg->GetInputData())->OptToRelease);
			}
			break;
		case EMSG_APPLY_DESTROY:
			{
				if(npMsg->GetMessageSender() == NULL)
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				luResult = mpElementManager->DestroyElement(npMsg->GetMessageSender());
			}
			break;
		case EMSG_CLOSE_WIDGET:
			{
				if(npMsg->GetInputDataSize() != sizeof(IXsWidget*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				luResult = CloseWidget(*(IXsWidget**)npMsg->GetInputData());
			}
			break;
		case EMSG_HOTKEY_PRESSED:
			{
				//快捷键消息
				const STEMS_HOTKEY* lpHotKey;
				nes_command::ESCOMMAND leCmd;

				luResult = CExMessage::GetInputDataBuffer(npMsg,lpHotKey);
				if(luResult != ERESULT_SUCCESS)
					break;
				switch(lpHotKey->HotKeyID)
				{
				case EHOTKEY_COPY:
					leCmd = nes_command::eCopy;
					break;
				case EHOTKEY_CUT:
					leCmd = nes_command::eCut;
					break;
				case EHOTKEY_PASTE:
					leCmd = nes_command::ePaste;
					break;
				case EHOTKEY_DELETE:
					leCmd = nes_command::eDelete;
					break;
				case EHOTKEY_SELECT_ALL:
					leCmd = nes_command::eSelAll;
					break;
				case EHOTKEY_ENTER:
					leCmd = nes_command::eEnter;
					break;
				case EHOTKEY_ESC:
					leCmd = nes_command::eEsc;
					break;
				case EHOTKEY_ALTF4:
					leCmd = nes_command::eAltF4;
					break;
				default:
					leCmd = nes_command::eInvalid;
					luResult = ERESULT_KEY_ACCEPTED;
				}
				if(leCmd != nes_command::eInvalid)
					mpElementManager->SendCommand(leCmd);
			}
			break;
		case EMSG_MAIN_GET_CURRENT_BITMAP:
			{
				LPARAM llParam = *(LPARAM*)npMsg->GetInputData();
				HBITMAP* lppHbitmap = (HBITMAP*)npMsg->GetOutputBuffer();
				if(lppHbitmap != NULL)
					*lppHbitmap = gpXuiSystem->mpXuiGraphics->GetCurrentBitmap(HIWORD(llParam), LOWORD(llParam));

				luResult = ERESULT_SUCCESS;

				break;
			}
		default:;
		}
	}
	catch(...)
	{
		luResult = ERESULT_UNKNOWN_ERROR;
	}



	return luResult;
}

// 定时器消息
ERESULT CEinkuiSystem::OnMsgTimer(const PSTEMS_TIMER npTimerInput)
{
	//static cmmVector<ULONG> TimerLog;
	if (npTimerInput->TimerID == ELMGR_TIMERID_RENDER)
	{

		// 判断是否需要绘制
		//if (InterlockedCompareExchange(&miToRender, 0, 1) != 0 || InterlockedIncrement(&miLostToRender)*EINK_RENDER_TIMER >= 20)
		//{	// 当前有绘制要求，或者有10秒钟(500ms*20)的时间没有刷新界面
		if (InterlockedCompareExchange(&miToRender, 0, 1) != 0)
		{
			//TimerLog.Insert(-1,npTimerInput->CurrentTick);

			//InterlockedExchange(&miLostToRender, 0);

			RenderProcedule(npTimerInput->CurrentTick);
		}

		return ERESULT_SUCCESS;
	}
	else
		return ERESULT_UNEXPECTED_MESSAGE;
}

// 给某个Element及其所有children拍照，???暂时还没有做线程同步，要防止工作线程调用
IEinkuiBitmap* __stdcall CEinkuiSystem::TakeSnapshot(
	IEinkuiIterator* npToShot,
	const D2D1_RECT_F& crSourceRegion,	// 采样区域，目标元素的局部坐标系
	const D2D_SIZE_F& crBriefSize,		// 缩略图尺寸，快照的结果是一副缩略图
	const FLOAT* ColorRGBA
	)
{

	return mpXuiGraphics->TakeSnapshot(npToShot,crSourceRegion,crBriefSize,ColorRGBA);
}

// 申请重新绘制以更新整个视图，Idealife的模式是整个视图重绘，只要调用一次就会更新整个输出视图；
// 重复调用本函数并不会导致重复的绘制；
void __stdcall CEinkuiSystem::UpdateView(
	IN bool nbRefresh	// 必须提交全屏
	)
{
	if(nbRefresh != false)
		InterlockedExchange(&miRefreshEink, 1);

	LONG flag = 0;
	mpElementManager->SimplePostMessage(NULL, EMSG_SYSTEM_RENDER,&flag,sizeof(flag));
}

// 设置Eink绘制回调，每当UI系统向Eink服务提交时调用指定的回调函数
ERESULT __stdcall CEinkuiSystem::SetEinkUpdatingCallBack(
	IN PXUI_EINKUPDATING_CALLBACK npFunction,
	IN PVOID npContext
)
{
	if(mpXuiGraphics->mpEinkUpdatingCallBack!=NULL)
		return  ERESULT_OBJECT_EXISTED;

	mpXuiGraphics->mpEinkUpdatingCallBack = npFunction;
	mpXuiGraphics->mpEinkUpdatingContext = npContext;

	return ERROR_SUCCESS;
}


// 重置Eink缓存；本库的工作原理是通过Eink缓存中的遗留内容来比对待显示内容，只将不同的部分发给Eink；如果Eink屏幕需要全部重绘
//    就需要重置Eink缓存，使得全部内容绘制到Eink；主要用于App重新活动Eink屏幕的情况
void __stdcall CEinkuiSystem::ClearEinkBuffer(void)
{
	mpXuiGraphics->ClearEinkBuffer(true);
}

// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
ID2D1Factory* __stdcall CEinkuiSystem::GetD2dFactory(void)
{
	return mpXuiGraphics->GetD2dFactory();
}

// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
IWICImagingFactory* __stdcall CEinkuiSystem::GetWICFactory(void)
{
	return mpXuiGraphics->GetWICFactory();
}

// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
// 通过调用CXuiGraphy类的同名函数实现，同其完全相同
IDWriteFactory* __stdcall CEinkuiSystem::GetDWriteFactory(void)
{
	return mpXuiGraphics->GetDWriteFactory();
}

void __stdcall CEinkuiSystem::SystemTest(ULONG nuValue)
{
	//Sleep(1000);

	//mbThreadKilled = true;

	//TerminateThread(mhOperationThread,0);
	GUID guid = EGUID_EMGR_DROP_TEST;

	//Trace_ULONG(19122,nuValue);//系统测试记录
}

// 鼠标落点检测预处理
ERESULT __stdcall CEinkuiSystem::EnterForLazyUpdate(IEinkuiIterator* npRecipient)
{
	CXuiIterator* lpRecipient = dynamic_cast<CXuiIterator*>(npRecipient);

	//if(lpRecipient->CheckStyle(EITR_STYLE_LAZY_UPDATE)!= false)
		//mpElementManager->SimplePostMessage(npRecipient,EMSG_LAZY_UPATE,&mdLazyUpdate,sizeof(mdLazyUpdate));
	if(lpRecipient->CheckStyle(EITR_STYLE_LAZY_UPDATE)!= false)
		mpElementManager->SimplePostMessage(npRecipient,EMSG_LAZY_UPATE,&mdLazyUpdate,sizeof(mdLazyUpdate));
		//mpElementManager->SimpleSendMessage(npRecipient,EMSG_LAZY_UPATE,&mdLazyUpdate,sizeof(mdLazyUpdate),NULL,0);

	return ERESULT_ENUM_CHILD;
}

// 鼠标落点检测后处理
ERESULT __stdcall CEinkuiSystem::LeaveForLazyUpdate(IEinkuiIterator* npRecipient)
{
	return ERESULT_SUCCESS;
}

// 设置某页成为模态对话方式，即用户必须完成该对话，此时界面上的其他部分都无法使用。
// XUI的模态对话方式是全局的，处于模态对话下，所有的Widget（包括System Widget)都不能响应用户输入；所有，尽可能避免使用模式对话，除非它是必须的。
// 使用方法是，首先创建模式对话的主对象（默认隐藏)，然后调用本函数进入模态对话方式，此时该模态对话元素对象将会收到一条EMSG_MODAL_ENTER消息，它处理相关事务后将自己显示出来；
// 当该模态对话对象判断用户完成了对话后，隐藏自己，而后ExitModal退出模态对话方式
// 注意，模态对话方式是可以重叠进入的，在模态对话下，可以打开子模态对话，而后一层层退出
ERESULT __stdcall CEinkuiSystem::DoModal(
	IEinkuiIterator* npModalElement		// 此元素是模态对话的主对象
	)
{
	if(IsRunningInOperationThread()==false)
	{
		return ERESULT_WRONG_THREAD;
	}

	if(mpElementManager->VerifyIterator(npModalElement)!=ERESULT_SUCCESS)
		return ERESULT_ITERATOR_INVALID;

	if(((CXuiIterator*)npModalElement)->CheckStyle(EITR_STYLE_POPUP)==false)
		return ERETULT_WRONG_STYLE;

	// 给模态对象发送一条消息
	ERESULT luResult = mpElementManager->SimpleSendMessage(npModalElement,EMSG_MODAL_ENTER,NULL,0,NULL,0);
	if(luResult != ERESULT_SUCCESS)
		return luResult;

	// 设置为激活
	npModalElement->SetActive();

	// 设置模态对话主对象
	CXuiModalState loState;

	loState.muBlock = moThreadBlock.AddBlock();
	loState.mpTarget = npModalElement;
	mpElementManager->EnterModal();
	moModalStack.AddModal(loState);
	
	// 进入消息循环
	luResult = OpThreadMessageLoop(loState.muBlock);
	if(luResult == ERESULT_QUIT_XUI && IsRunningInOperationThread()!=false)
	{
		// 如果在这个时间点，恰好收到了退出消息，那，这是一个非常不好的情况，直接退出程序
		ExitProcess(-1);
	}

	moModalStack.RemoveModal(npModalElement);

	luResult = moThreadBlock.ReadBlockState(loState.muBlock);
	moThreadBlock.RemoveBlock(loState.muBlock);

	return luResult;
}

// 退出模态对话方式
void __stdcall CEinkuiSystem::ExitModal(
	IEinkuiIterator* npModalElement,	// 此元素是模态对话的主对象
	ERESULT nuResult
	)
{
	//// 给系统发送退出消息
	//mpElementManager->SimplePostMessage(NULL,EMSG_MODAL_COMPLETED,&nuResult,sizeof(nuResult),EMSG_POSTTYPE_FAST);

	// 清除当前缓冲的全部Windows原始输入消息
	//mpElementManager->moFastMessages.RemoveMessages(LMSG_TP_WIN_INPUT,0,0,XSMSG_REMOVE_TYPE);
	mpElementManager->CleanHumanInput(false);

	// 通过元素找到阻点号，然后设置它的状态
	ULONG luBlockID = moModalStack.GetBlockIDOfModal(npModalElement);
	if(luBlockID != 0)
		moThreadBlock.SetBlockState(luBlockID,nuResult);
}

// 设置windows鼠标捕获
void CEinkuiSystem::SetWinMouseCapture(USHORT nsuFlag)
{
	if(InterlockedOr16((SHORT*)&msuWinCap,nsuFlag)==0)
		SetCapture(EinkuiGetSystem()->GetMainWindow());
}

// 释放windows鼠标捕获
void CEinkuiSystem::ReleaseWinMouseCapture(USHORT nsuFlag)
{
	if(msuWinCap == 0)
		return ;

	if(InterlockedAnd16((SHORT*)&msuWinCap,~nsuFlag)!=0 && msuWinCap==0)
		ReleaseCapture();
}

void CEinkuiSystem::WsgOnGetMinMaxInfo(MINMAXINFO *pMinMaxInfo)
{
	pMinMaxInfo->ptMinTrackSize.x = GetSystemMetrics(SM_CXMINTRACK);
	pMinMaxInfo->ptMinTrackSize.y = GetSystemMetrics(SM_CXMAXTRACK);
}

//void CEinkuiSystem::WsgOnSettingChanged()
//{
//	if (mbIsAutoSize != false)
//	{
//		RECT rcWorkArea;
//		SystemParametersInfo(SPI_GETWORKAREA, sizeof(rcWorkArea), &rcWorkArea, 0);
//		MoveWindow(mhMainWnd, rcWorkArea.left, rcWorkArea.top, rcWorkArea.right-rcWorkArea.left, rcWorkArea.bottom-rcWorkArea.top, TRUE);
//	}
//}

// Client调用，退出整个XUI系统
void __stdcall CEinkuiSystem::ExitXui(void)
{
	gpXuiSystem->mpElementManager->EndMessageQueue();

	::PostMessage(mhMainWnd,muiCustomWinMsg,'Exit',NULL);
}


// 实际退出Xui的时刻
void CEinkuiSystem::EndXui(void)
{
	bool exception = false;
	__try {
		// 关闭全部的Widget
		for (int i = moAllWidgets.Size() - 1; i >= 0; i--)
		{
			CloseWidget(moAllWidgets[i]);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		exception = true;
	}
}

//// 设置闪烁Domodal的窗口
//void CEinkuiSystem::FlashModalElement(IEinkuiIterator* npDodal){
//	mpXuiGraphics->FlashModalElement(npDodal);
//}

// 获得最前面的模态窗口
IEinkuiIterator* CEinkuiSystem::GetTopModalElement(void)
{
	CXuiModalState loState;

	if(moModalStack.GetTopModel(loState)==false)
		return NULL;

	return loState.mpTarget;
}

//本次操作是否是TOUCH操作
bool __stdcall CEinkuiSystem::IsTouch(void)
{
	return mbIsTouch;
}