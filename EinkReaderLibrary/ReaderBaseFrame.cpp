/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ReaderBaseFrame.h"
#include "XCtl.h"
#include <shellapi.h>

#include "SmCuveImp.h"
#include "stdio.h"
#include "HellFun.h"
#include <tlhelp32.h>
#include "winsock.h"

#include "cmmStrHandle.h"
#include "math.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <Shlobj.h>


DEFINE_BUILTIN_NAME(Reader_BaseFrame)

CReaderBaseFrame::CReaderBaseFrame(void)
{
	mpFileOpenDlg = NULL;
	mpTipFrame = NULL;
	mpPdfPicture = NULL;
	mpJumpPage = NULL;
	mpIterBackground = NULL;
	mpPreNextButton = NULL;
	mpZoomControl = NULL;
	mpZoomControlTxt = NULL;
	mpSnapShot = NULL;
	mpIterToast = NULL;
	mlHoldInput = 0;
	mxLastGcUiNumber = 0;
	muLastGcTick = 0;
	mbGcMode = false;
	mpToolbarH = NULL;
	mbIsTxt = false;

	//txt字号
	mdwFontsizeIndex = 1;
	mdwFontSizeArray[0] = 9;
	mdwFontSizeArray[1] = 12;
	mdwFontSizeArray[2] = 15;
	mdwFontSizeArray[3] = 18;
	mdwFontSizeArray[4] = 21;

	mszTempFile[0] = UNICODE_NULL;
	mszSrcFile[0] = UNICODE_NULL;

	mbIsSetPartial = true;
	mulPageIndex = 0;

	//Sleep(1000 * 20);
}


CReaderBaseFrame::~CReaderBaseFrame(void)
{
	while (mdHistroyPath.Size() > 0)
	{
		delete mdHistroyPath.GetEntry(0);
		mdHistroyPath.RemoveByIndex(0);
	}
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CReaderBaseFrame::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		// 设置风格支持拖拽
		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		

		EinkuiGetSystem()->SetEinkUpdatingCallBack((PXUI_EINKUPDATING_CALLBACK)&EinkUpdating, this);
		EinkuiGetSystem()->EnablePaintboard();
#ifdef SHOW_BCOVER_WINDOW
		EinkuiGetSystem()->ShowMainWindow(true);
#endif

		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_ACTIVATE, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnActivity);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_HOMEBAR_MODE_CHANGE, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnHomebarChanged);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_RESET_TP_AREA, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnNotifyResetTpArea);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_POWERBROADCAST, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnPowerChangeMsg);
		//mpIterator->SetTimer(EINK_SETTINGS_TIMER_ID_PAINT, MAXULONG32, 1000, NULL);
		//ImmDisableIME(-1);	//关闭输入法

		//mpIterator->SetTimer(1,MAXULONG32,3000,NULL);

		EiSetWaveformMode(GI_WAVEFORM_DU2);
		EiCleanupScreen(0xff);
		Sleep(585); //DU260+15*5   分16帧刷，每帧间隔5ms
		EiSetWaveformMode(GI_WAVEFORM_GC16);

		ShowToolBar(true);
		mpIterator->SetTimer(RBF_TIMER_ENABL_PARTIAL, 1, 1800, NULL);
		//mpIterator->SetTimer(RBF_TIMER_INIT, 1, 100, NULL);
		Init();

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CReaderBaseFrame::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		EI_SYSTEM_INFO ldSysInfo;
		EiGetSystemInfo(&ldSysInfo);
		EiSetScreenOrient(ldSysInfo.ulOrient);
		

		//EI_SYSTEM_INFO ldInfo;
		//EiGetSystemInfo(&ldInfo);
		EiSetHandWritingMode(GIHW_OWNER_DRAW);
		

		//EiGetSystemInfo(&ldInfo);

		mpIterToast = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterToast);
		mpIterToast->SetVisible(false);
		mpIterToast->SetDefaultZOrder(900000);
		
		mpIterBackground = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterBackground);

		//翻页
		lpSubKey = mpTemplete->OpenKey(L"PreNextButton");
		mpPreNextButton = CPreNextButton::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPreNextButton);

		//PDF图片
		lpSubKey = mpTemplete->OpenKey(L"PicturePreview");
		mpPdfPicture = CPdfPicture::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPdfPicture);

		//横屏工具栏
		lpSubKey = mpTemplete->OpenKey(L"ToolbarH");
		mpToolbarH = CToolbarH::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpToolbarH);
		mpToolbarH->GetIterator()->BringToTop();
		
		//缩放
		lpSubKey = mpTemplete->OpenKey(L"ZoomControl");
		mpZoomControl = CZoomControl::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpZoomControl);
		mpZoomControl->GetIterator()->SetVisible(false);

		//Txt缩放
		lpSubKey = mpTemplete->OpenKey(L"ZoomControlTxt");
		mpZoomControlTxt = CZoomControlTxt::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpZoomControlTxt);
		mpZoomControlTxt->GetIterator()->SetVisible(false);

		
		OnRotated(ldSysInfo.ulOrient);

		leResult = ERESULT_SUCCESS;


	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}


//进入系统消息
ERESULT __stdcall CReaderBaseFrame::OnPowerChangeMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		if (wParam == PBT_POWERSETTINGCHANGE)
		{
			POWERBROADCAST_SETTING* lpSetting = (POWERBROADCAST_SETTING*)lParam;
			if (lpSetting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE)
			{
				DWORD ldwStatus = 0;
				memcpy_s(&ldwStatus, sizeof(DWORD), lpSetting->Data, sizeof(DWORD));
				/*if (ldwStatus == 0x1)
				{
					EiSetPartialUpdate(FALSE);
					EinkuiGetSystem()->ClearEinkBuffer();
					EinkuiGetSystem()->UpdateView(true);
				}*/
				
			}
		}
		else if (wParam == PBT_APMRESUMEAUTOMATIC)
		{
			OutputDebugString(L"PBT_APMRESUMEAUTOMATIC a");
			//ExitProcess(0);
		}
		else if (wParam == PBT_APMSUSPEND)
		{
			OutputDebugString(L"PBT_APMSUSPEND ");
		}



	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//服务通知应用切换到前台或后台
ERESULT __stdcall CReaderBaseFrame::OnActivity(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		CExMessage::SendMessage(mpIterator, mpIterator, EEVT_HB_ACTIVITE, (ULONG)wParam);

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//homebar状态变化通知
ERESULT __stdcall CReaderBaseFrame::OnHomebarChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		CExMessage::SendMessage(mpIterator, mpIterator, EEVT_ER_HOMEBAR_CHANGED, (ULONG)wParam);

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//Tp被reset了，需要重新设置tp area
ERESULT __stdcall CReaderBaseFrame::OnNotifyResetTpArea(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
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

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//按钮单击事件
ERESULT CReaderBaseFrame::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;


	return lResult;
}


void CReaderBaseFrame::HoldInput(bool nbSet)
{
	if (nbSet != false)
	{
		EinkuiGetSystem()->GetElementManager()->CleanHumanInput(true);
		InterlockedExchange(&mlHoldInput, 1);
		mpIterator->SetTimer(RBF_TIMER_HOLDINPUT, 1, 2000, NULL);	// 如果没有刷新，2秒后也将自动解除
	}
	else
	{
		if (InterlockedExchange(&mlHoldInput, 0) == 1) // 只有当前处于锁定状态，才调用管理器的清理服务
			EinkuiGetSystem()->GetElementManager()->CleanHumanInput(false);
	}
}

//消息处理函数
ERESULT CReaderBaseFrame::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_QUERY_SWGT_ROTATED:
	{
		ULONG luOrient = 0;
		luResult = CExMessage::GetInputData(npMsg, luOrient);
		if (luResult != ERESULT_SUCCESS || npMsg->GetOutputBufferSize() < sizeof(ULONG))
			break;

		*(ULONG*)npMsg->GetOutputBuffer() = luOrient;
		npMsg->SetOutputDataSize(sizeof(ULONG));

		luResult = ERESULT_SUCCESS;
		break;
	}
	break;
	case EMSG_SWGT_ROTATED:
	{
		ULONG luOrient = 0;
		luResult = CExMessage::GetInputData(npMsg, luOrient);
		if (luResult != ERESULT_SUCCESS)
			break;

		OnRotated(luOrient);

		luResult = ERESULT_SUCCESS;
		break;
	}
	case EEVT_ER_OPEN_FILE:
	{
		//选择文件
		if (mpFileOpenDlg != NULL)
		{
			mpFileOpenDlg->ExitModal();
			mpFileOpenDlg = NULL;
		}
			

		{
			ICfKey* lpSubKey = mpTemplete->OpenKey(L"OpenFile");
			mpFileOpenDlg = CFileOpenDlg::CreateInstance(mpIterator, lpSubKey);;
			//mpFileOpenDlg->SetSelect(mulKeyboardLanguageIndex);

			SetOpenFilePos();
			mpFileOpenDlg->SetHistoryList(&mdHistroyPath);

			//EiSetPartialUpdate(TRUE);
			//如果当前没有打开文件，文件打开对话框的取消按钮就置灰
			bool lbEnable = true;
			if (mszSrcFile[0] == UNICODE_NULL || GetFileAttributes(mszSrcFile) == INVALID_FILE_ATTRIBUTES)
				lbEnable = false;
			mpFileOpenDlg->DoModal(lbEnable);
			//EiSetPartialUpdate(FALSE);
			CMM_SAFE_RELEASE(lpSubKey);
			mpFileOpenDlg = NULL;

			if (mpPdfPicture->GetPageCount() == 0)
			{
				//重新打开文件
				Init();
			}
		}
	
		break;
	}
	case EEVT_ER_OPEN_PAGE_JUMP:
	{
		//页码跳转
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"JumpPage");
		mpJumpPage = CJumpPage::CreateInstance(mpIterator, lpSubKey);
		ULONG secondPage;
		mpJumpPage->SetCurrentPage(mpPdfPicture->GetCurrentPageNumber(secondPage), mpPdfPicture->GetPageCount());
		SetJumpPagePos();

		//EiSetPartialUpdate(TRUE);
		mpJumpPage->DoModal();
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpJumpPage = NULL;
		break;
	}
	case EEVT_ER_SHOW_TIP:
	{
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"TipFrame");
		mpTipFrame = CTipFrame::CreateInstance(mpIterator, lpSubKey);;
		//mpFileOpenDlg->SetSelect(mulKeyboardLanguageIndex);
		mpTipFrame->GetIterator()->SetSize(mpIterator->GetSize());
		//EiSetPartialUpdate(TRUE);
		mpTipFrame->DoModal();
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpTipFrame = NULL;

		//mpTipFrame->GetIterator()->SetVisible(false);
		break;
	}
	case EEVT_HB_ACTIVITE:
	{
		//应用切换到前台或后台
		ULONG lulActivity = 0;
		luResult = CExMessage::GetInputData(npMsg, lulActivity);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulActivity == 1)
		{
			//切换到前台
			//mpIterator->KillTimer(RBF_TIMER_EXIT);

			EiSetWaveformMode(GI_WAVEFORM_DU2);
			EiCleanupScreen(0xff);
			Sleep(585); //DU260+15*5   分16帧刷，每帧间隔5ms
			EiSetWaveformMode(GI_WAVEFORM_GC16);
			EiSetPartialUpdate(FALSE);

			mpIterator->SetTimer(RBF_TIMER_ENABL_PARTIAL, 1, 1800, NULL);
			mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR, 1, 5000, NULL);

			//如果刚才打开的文件被删除了，就进入打开文件对话框
			if (mszSrcFile[0] != UNICODE_NULL && GetFileAttributes(mszSrcFile) == INVALID_FILE_ATTRIBUTES)
			{
				mpPdfPicture->CloseFile();

				if(mpFileOpenDlg == NULL) //只有在没有弹出的情况下才打开
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			}
		}
		else
		{
			//切换到后台
			//保存数据
			//ExitProcess(0);
			mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
			//mpToolbarH->GetIterator()->SetVisible(true);
			 //这样就可以在切换过来第一帧时关闭partial刷新一次

			
			//mpIterator->SetTimer(RBF_TIMER_EXIT, 1, 400, NULL);
			//ExitProcess(0);
		}

		break;
	}
	case EEVT_ER_HOMEBAR_CHANGED:
	{
		//homebar状态发生变化
		ULONG lulHomebarMode = 0;
		luResult = CExMessage::GetInputData(npMsg, lulHomebarMode);
		if (luResult != ERESULT_SUCCESS)
			break;

		//EiSetPartialUpdate(TRUE);
		if (lulHomebarMode == GI_HOMEBAR_EXPAND)
		{
			//homebar展开就隐藏自己的工具栏
			mpToolbarH->GetIterator()->SetVisible(false);
		}
		else if(lulHomebarMode == GI_HOMEBAR_COLLAPSE)
		{
			//homebar收缩就显示自己的工具栏
			mpToolbarH->GetIterator()->SetVisible(true);
			mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
			mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR, 1, 5000, NULL);
		}
		else if (lulHomebarMode == GI_HOMEBAR_HIDE)
		{
			//隐藏了
		}
		//EiSetPartialUpdate(FALSE);

		break;
	}
	case EEVT_ER_PAGE_JUMP:
	{
		//页码跳转
		int liPage = 1;
		luResult = CExMessage::GetInputData(npMsg, liPage);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->GoToPage(liPage);
		ShowPageInfo();

		//GotoPage(liPage);

		break;
	}
	case EEVT_ER_OPEN_FILE_PATH:
	{
		//要打开的文件路径
		wchar_t* lpszFilePath = (wchar_t*)npMsg->GetInputData();
		bool lbRet = OpenFile(lpszFilePath);

		// 设置输出数据
		bool* lpOut = (bool*)npMsg->GetOutputBuffer();
		*lpOut = lbRet;
		npMsg->SetOutputDataSize(sizeof(bool));

		break;
	}
	case EEVT_ER_PRE_NEXT_CLICKED:
	{
		//页码跳转
		ULONG lulFlag = 1;
		luResult = CExMessage::GetInputData(npMsg, lulFlag);
		if (luResult != ERESULT_SUCCESS)
			break;
		do 
		{
			if (lulFlag == PNB_BT_PRE)
			{
				mpPdfPicture->PageFoward(false);
				ShowPageInfo();			
			}
			else if (lulFlag == PNB_BT_NEXT)
			{
				mpPdfPicture->PageFoward(true);
				ShowPageInfo();
			}
			else if (lulFlag == PNB_BT_MIDDLE)
			{
				//显示或隐藏工作栏
				ShowToolBar(!mpToolbarH->GetIterator()->IsVisible());
			}
			else
				break;

			HoldInput(true);

		} while (false);

		//???niu 增加对PageFoward()失败的判断和处理

		break;
	}
	case EEVT_ER_TWO_SCREEN:
	{
		//切换双屏或单屏
		bool lbIsDoubleScreen = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsDoubleScreen);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->EnableDuopageView(lbIsDoubleScreen);
		ShowPageInfo();

		CHellFun::SetRegData(RbF_REG_DOUBLE_SCREEN, lbIsDoubleScreen==false?0:1);

		mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
		mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR, 1, 5000, NULL);

		HoldInput(true);

		break;
	}
	case EEVT_ER_ENTER_ZOOM:
	{
		//进入缩放模式
		bool lbIsZoom = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsZoom);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lbIsZoom == false)
		{
			//显示工具栏
			//mpToolbarH->GetIterator()->SetVisible(true);

			mpZoomControl->GetIterator()->SetVisible(false);
			mpZoomControlTxt->GetIterator()->SetVisible(false);

			mpPreNextButton->GetIterator()->SetVisible(true);
		}
		else
		{
			//隐藏工具栏
			mpToolbarH->GetIterator()->SetVisible(false);

			if (mbIsTxt == false)
			{
				mpZoomControl->SetFatRatio(mpPdfPicture->GetFatRatio());
				mpZoomControl->GetIterator()->SetVisible(true);

			}
			else
			{
				mpZoomControlTxt->SetFontsize(mdwFontsizeIndex);
				mpZoomControlTxt->GetIterator()->SetVisible(true);

			}
			
			mpPreNextButton->GetIterator()->SetVisible(false);
		}

		break;
	}
	case EEVT_ER_SET_PAGE_MOVE:
	{
		//页面移动
		POINT ldPos;
		luResult = CExMessage::GetInputData(npMsg, ldPos);
		if (luResult != ERESULT_SUCCESS)
			break;

		RECT ldRect;
		mpPdfPicture->MovePage(ldPos.x, ldPos.y, ldRect);
		// 设置输出数据
		RECT* lpOut = (RECT*)npMsg->GetOutputBuffer();
		*lpOut = ldRect;
		npMsg->SetOutputDataSize(sizeof(RECT));

		//获取真实大小
		D2D1_SIZE_F ldMaxSize;
		D2D1_RECT_F ldViewRect;
		mpPdfPicture->GetRectOfViewportOnPage(ldMaxSize, ldViewRect);
		mpZoomControl->SetRectOfViewportOnPage(ldMaxSize, ldViewRect);

		break;
	}
	case EEVT_ER_SET_ZOOM:
	{
		//设置放大比例
		float lfRotio = 1.0f;
		luResult = CExMessage::GetInputData(npMsg, lfRotio);
		if (luResult != ERESULT_SUCCESS)
			break;

		RECT ldRect;
		mpPdfPicture->SetScaleRatio(lfRotio, ldRect);
		CHellFun::SetRegData(RbF_REG_RATIO, DWORD(lfRotio*100));

		// 设置输出数据
		RECT* lpOut = (RECT*)npMsg->GetOutputBuffer();
		*lpOut = ldRect;
		npMsg->SetOutputDataSize(sizeof(RECT));

		//获取真实大小
		D2D1_SIZE_F ldMaxSize;
		D2D1_RECT_F ldViewRect;
		mpPdfPicture->GetRectOfViewportOnPage(ldMaxSize, ldViewRect);
		mpZoomControl->SetRectOfViewportOnPage(ldMaxSize, ldViewRect);

		break;
	}
	case EEVT_ER_ENTER_SNAPSHOT:
	{
		//截屏
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"SnapShot");
		mpSnapShot = CSnapShot::CreateInstance(mpIterator, lpSubKey);;
		//mpFileOpenDlg->SetSelect(mulKeyboardLanguageIndex);
		mpSnapShot->GetIterator()->SetSize(mpIterator->GetSize());
		//EiSetPartialUpdate(TRUE);
		mpSnapShot->DoModal();
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpSnapShot = NULL;

		break;
	}
	case EEVT_ER_SNAPSHOT_TO_CLIPBRD:
	{
		D2D1_RECT_F ldRect;
		luResult = CExMessage::GetInputData(npMsg, ldRect);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->CopyToClipboard(ldRect);
		ShowToast(L"SnapShot");

		break;
	}
	case EEVT_ER_SET_TXT_ZOOM:
	{
		//设置txt放大
		luResult = CExMessage::GetInputData(npMsg, mdwFontsizeIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->SetFontSize(mdwFontSizeArray[mdwFontsizeIndex]);
		ShowPageInfo();

		//把设置保存到注册表中
		CHellFun::SetRegData(RbF_REG_TXT_FONT_SIZE_INDEX, mdwFontsizeIndex);

		break;
	}
	case EEVT_TXT_ARRANGED_START:
	{
		//txt开始页面加载
		
		
		break;
	}
	case EEVT_TXT_ARRANGED_DOING:
	{
		//txt页面正在加载中
		ShowPageInfo();
		mbIsSetPartial = false;

		int32 liPageIndex = 0;
		luResult = CExMessage::GetInputData(npMsg, liPageIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->SetLoadingPageIndex(liPageIndex);

		break;
	}
	case EEVT_ARRANGED_COMPLETE:
	{
		//页面加载完毕
		mbIsSetPartial = true;
		break;
	}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if (luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//绘制
ERESULT CReaderBaseFrame::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	HoldInput(false);

	return CXuiElement::OnPaint(npPaintBoard);
}

////切页
//void CReaderBaseFrame::GotoPage(ULONG nulPage)
//{
//	mpPdfPicture->GoToPage(nulPage);
//
//	wchar_t lszString[MAX_PATH] = { 0 };
//	swprintf_s(lszString, MAX_PATH, L"(第%d页/共%d页)", nulPage, mpPdfPicture->GetPageCount());
//	mpToolbarH->SetPage(lszString);
//
//	mulCurrentPageNumber = nulPage;
//}

//显示或隐藏工具栏
void CReaderBaseFrame::ShowToolBar(bool nbIsShow)
{
	if (nbIsShow == false)
	{
		//隐藏
		mpToolbarH->GetIterator()->SetVisible(false);
		mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
	}
	else
	{
		//显示
		mpToolbarH->GetIterator()->SetVisible(true);
		mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
		mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR,1,5000,NULL);

		EiSetHomebarStatus(GI_HOMEBAR_COLLAPSE);
	}
}

void CReaderBaseFrame::ShowPageInfo(void)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	ULONG pageNo1=0, pageNo2=0;

	pageNo1 = mpPdfPicture->GetCurrentPageNumber(pageNo2);
	if (pageNo1 == 0)
		pageNo1 = 1;

	{
		IConfigFile* lpProfile = NULL;
		wchar_t lszText[MAX_PATH] = { 0 };

		do
		{
			//获取多语言字符串
			//为了翻译方便，字符串存放在root/string
			lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
			ICfKey* lpCfKey = NULL;
			if (lpProfile != NULL)
			{
				if (pageNo2 != 0)
					lpCfKey = lpProfile->OpenKey(L"String2/Title2");
				else
					lpCfKey = lpProfile->OpenKey(L"String2/Title1");

				if (lpCfKey != NULL)
					lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));
				
			}

			CMM_SAFE_RELEASE(lpCfKey);

		} while (false);

		CMM_SAFE_RELEASE(lpProfile);
		

		if(pageNo2 != 0)
			swprintf_s(lszString, MAX_PATH, lszText, pageNo1,pageNo2, mpPdfPicture->GetPageCount());
		else
			swprintf_s(lszString, MAX_PATH, lszText, pageNo1, mpPdfPicture->GetPageCount()==0?1: mpPdfPicture->GetPageCount());

		mpToolbarH->SetPage(lszString);

		if (mulPageIndex != pageNo1)
		{
			//更新记录数据
			mulPageIndex = pageNo1;

			
			PAGE_PDF_CONTEXT ldContent;
			if (mpPdfPicture->GetCrtPageContext(&ldContent) != false)
			{
				CHellFun::SetRegData(RbF_REG_PAGE_NUMBER, pageNo1);
				CHellFun::SetRegData(RbF_REG_PAGE_CONTENT, ldContent.pageContext);
				CHellFun::SetRegData(RbF_REG_PAGE_CONTENT2, ldContent.pageContext2);
			}
		}
		

	}
}

//显示toast
void CReaderBaseFrame::ShowToast(wchar_t* npszKeyName)
{
	IConfigFile* lpProfile = NULL;

	do 
	{
		BREAK_ON_NULL(npszKeyName);

		//获取多语言字符串
		//为了翻译方便，字符串存放在root/string
		wchar_t lszText[MAX_PATH] = { 0 };
		lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		ICfKey* lpCfKey = NULL;
		if (lpProfile != NULL)
		{
			swprintf_s(lszText, MAX_PATH, L"String/%s", npszKeyName);
			lpCfKey = lpProfile->OpenKey(lszText);
			if (lpCfKey != NULL)
			{
				lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));
				CMM_SAFE_RELEASE(lpCfKey);
				CExMessage::SendMessageWithText(mpIterToast, mpIterator, EACT_LABEL_SET_TEXT, lszText);
				FLOAT lfX = (mpIterator->GetSizeX() - mpIterToast->GetSizeX()) / 2;
				mpIterToast->SetPosition(lfX, mpIterator->GetSizeY() - 200);

				mpIterToast->SetVisible(true);
				mpIterToast->BringToTop();
				mpIterator->SetTimer(RBF_TIMER_TOAST, 1, 3000, NULL);
			}
		}
		CMM_SAFE_RELEASE(lpCfKey);

	} while (false);

	CMM_SAFE_RELEASE(lpProfile);
}


// 用于给eink工作线程发送消息，避免阻塞服务主线程
bool CReaderBaseFrame::CopyFileThread(LPVOID npData)
{
	CReaderBaseFrame* lpThis = (CReaderBaseFrame*)npData;
	
	CopyFile(lpThis->mszSrcFile, lpThis->mszTempFile, FALSE);

	return true;
}

//把要打开的文件copy到临时目录
void CReaderBaseFrame::CopyFileToTemp(IN wchar_t* npszSrc, OUT wchar_t* npszDest, IN LONG nlLen)
{
	do 
	{
		BREAK_ON_NULL(npszSrc);
		BREAK_ON_NULL(npszDest);
		if (npszDest[0] != UNICODE_NULL)
		{
			wchar_t lszTemp[MAX_PATH] = { 0 };
			wcscpy_s(lszTemp, MAX_PATH, npszDest);
			lszTemp[wcslen(lszTemp) + 1] = UNICODE_NULL; //双0结尾
			//如此已存在，先删除
			SHFILEOPSTRUCT ldShfile;
			memset(&ldShfile, 0, sizeof(ldShfile));
			ldShfile.pFrom = lszTemp;
			ldShfile.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
			ldShfile.wFunc = FO_DELETE;
			SHFileOperation(&ldShfile);
		}

		SHGetSpecialFolderPath(NULL, npszDest, CSIDL_COMMON_APPDATA, FALSE);
		wcscat_s(npszDest, nlLen, L"\\EinkSrv\\");
		wcscat_s(npszDest, nlLen, wcsrchr(npszSrc, L'\\') + 1);


		DWORD ldwProtectThreadID = 0;
		HANDLE lhCopyfileThread = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)&CReaderBaseFrame::CopyFileThread,
			this,
			0,
			&ldwProtectThreadID
		);

		if (WaitForSingleObject(lhCopyfileThread, 2000) == WAIT_TIMEOUT)
		{
			ICfKey* lpSubKey = mpTemplete->OpenKey(L"PicturePreview/Loading");
			mpLoadingView = CLoadingView::CreateInstance(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), lpSubKey);
			wchar_t* lpszFileName = wcsrchr(npszSrc, L'\\');
			if (lpszFileName != NULL)
				lpszFileName = lpszFileName + 1;
			mpLoadingView->SetData(lpszFileName);
			mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());
			mpLoadingView->DoModal(NULL,lhCopyfileThread);
			mpLoadingView = NULL;
			CMM_SAFE_RELEASE(lpSubKey);

			CloseHandle(lhCopyfileThread);
		}

	} while (false);

}

//用户选择了要打开的文件
bool CReaderBaseFrame::OpenFile(wchar_t* npszFilePath)
{
	bool lbRet = false;

	do 
	{
		BREAK_ON_NULL(npszFilePath);
		mpPdfPicture->CloseFile();
		wcscpy_s(mszSrcFile, MAX_PATH, npszFilePath);

		CopyFileToTemp(npszFilePath, mszTempFile, MAX_PATH);
		
		mulPageIndex = 0;

		wchar_t* lpszFileName = wcsrchr(npszFilePath, L'\\');
		if (lpszFileName == NULL)
			lpszFileName = npszFilePath;
		else
			lpszFileName = lpszFileName + 1;
		if (lpszFileName[wcslen(lpszFileName) - 1] == 't')
			mbIsTxt = true;
		else
			mbIsTxt = false;

		ULONG lulRet = mpPdfPicture->OpenFile(mszTempFile);
		mbIsSetPartial = true;
		if (lulRet != EDERR_SUCCESS/* && lulRet != EDERR_EMPTY_CONTENT*/)
		{
			if (mbIsTxt == false || GetFileSize(npszFilePath) > 0)
			{
				//打开文件失败
				ShowToast(L"OpenFileFail");
				mszSrcFile[0] = UNICODE_NULL;

				//CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);

				break;
			}
		}
		mpIterToast->SetVisible(false);

		mpPdfPicture->GoToPage((IEdPage_ptr)NULL);
		mpPdfPicture->EnableDuopageView(mpToolbarH->GetDuopageStatus());

		ShowPageInfo();
		mpToolbarH->SetFileName(lpszFileName);

		//记录打开历史
		//先看下这个是不是已经记录过了
		bool lbIsHave = false;
		for (int i=0;i<mdHistroyPath.Size();i++)
		{
			wchar_t* lpszPath = mdHistroyPath.GetEntry(i);
			if (_wcsicmp(npszFilePath, lpszPath) == 0)
			{
				//已经有了，把它提到第一就行了
				mdHistroyPath.RemoveByIndex(i);
				mdHistroyPath.Insert(0, lpszPath);

				lbIsHave = true;
				break;
			}
		}

		if (lbIsHave == false)
		{
			//增加到列表
			wchar_t* lpszPath = new wchar_t[MAX_PATH];
			wcscpy_s(lpszPath, MAX_PATH, npszFilePath);
			mdHistroyPath.Insert(0, lpszPath);

			if (mdHistroyPath.Size() > RBF_HISTROY_MAX)
			{
				//超出最大数了，删除最后一个
				delete[] mdHistroyPath.GetEntry(mdHistroyPath.Size() - 1);
				mdHistroyPath.RemoveByIndex(mdHistroyPath.Size() - 1);
			}
		}

		//保存到注册表
		wchar_t lszText[MAX_PATH];
		DWORD ldwLen = 0;
		for (int i = 0; i < mdHistroyPath.Size(); i++)
		{
			swprintf_s(lszText, MAX_PATH, L"history%d", i);
			CHellFun::SetRegData(lszText, 0, mdHistroyPath.GetEntry(i));
		}

		lbRet = true;

	} while (false);

	return lbRet;
}

//定时器
void CReaderBaseFrame::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
	if (npStatus->TimerID == RBF_TIMER_TOAST)
	{
		mpIterator->KillTimer(RBF_TIMER_TOAST);
		mpIterToast->SetVisible(false);
	}
	else if (npStatus->TimerID == RBF_TIMER_TSHOW_TOOLBAR)
	{
		ShowToolBar(false);
	}
	else if (npStatus->TimerID == RBF_TIMER_ENABL_PARTIAL)
	{
		mpIterator->KillTimer(npStatus->TimerID);
		EiSetPartialUpdate(TRUE);
	}
	else if (npStatus->TimerID == RBF_TIMER_INIT)
	{
		mpIterator->KillTimer(npStatus->TimerID);
		Init();
	}
	else if (npStatus->TimerID == RBF_TIMER_EXIT)
	{
		ExitProcess(0);
	}
	else if (npStatus->TimerID == RBF_TIMER_HOLDINPUT)
	{
		HoldInput(false);
	}
}



//系统屏幕发生旋转
void CReaderBaseFrame::OnRotated(ULONG nuOrient)
{
	EI_SIZE ldPaintSize;

	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
	
	if (mpPdfPicture != NULL)
	{
		//mpPdfPicture->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);上一行设置本窗体大小后，本窗体的大小改变相应函数已经对pdfpicture对象做了大小调整了
		mpPdfPicture->SetRotation(nuOrient);
	}

	mpIterator->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);

	if (mpZoomControl != NULL)
	{
		mpZoomControl->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		if(mpZoomControl->GetIterator()->IsVisible() != false)
			mpZoomControl->SetFatRatio(mpPdfPicture->GetFatRatio());
	}

	if (mpZoomControlTxt != NULL)
	{
		mpZoomControlTxt->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		/*if (mpZoomControlTxt->GetIterator()->IsVisible() != false)
			mpZoomControlTxt->SetFatRatio(mpPdfPicture->GetFatRatio());*/
	}

	if (mpPreNextButton != NULL)
	{
		mpPreNextButton->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		//mpPreNextButton->SetRotation(nuOrient);
	}

	if (mpToolbarH != NULL)
		mpToolbarH->GetIterator()->SetSize((FLOAT)ldPaintSize.w, 60.0f);

	if (nuOrient == GIR_90 || nuOrient == GIR_270)
	{
		//坚屏
	}
	else
	{
		//横屏
			
	}

	ShowPageInfo();
	////重绘自己
	//EinkuiGetSystem()->UpdateView();
}

//元素参考尺寸发生变化
ERESULT CReaderBaseFrame::OnElementResized(D2D1_SIZE_F nNewSize)
{
	do 
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		//if(mbIsInit == false)
		//	break;

		
		if (mpPdfPicture != NULL)
		{
			mpPdfPicture->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		}

		if (mpLoadingView != NULL)
			mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());

		if (mpToolbarH != NULL)
			mpToolbarH->GetIterator()->SetSize((FLOAT)ldPaintSize.w, 60.0f);

		if(mpIterBackground != NULL)
			mpIterBackground->SetSize(nNewSize);
		//mpTipFrame->GetIterator()->SetSize(nNewSize);
		SetOpenFilePos();
		SetJumpPagePos();

		if(mpSnapShot != NULL)
			mpSnapShot->GetIterator()->SetSize(mpIterator->GetSize());
	
		////横坚屏下左侧按钮宽度不一样
		//ULONG lulWidth = 600;
		//if (nNewSize.width > nNewSize.height)
		//	lulWidth = 600;
		//else
		//	lulWidth = 400;

		//mpIterLineTwo->SetSize(mpIterLineTwo->GetSize().width, nNewSize.height);
		//mpIterLineTwo->SetPosition(lulWidth, mpIterLineTwo->GetPositionY());

		//mpIterLineOne->SetSize(nNewSize.width, mpIterLineOne->GetSize().height);
		//mpIterBackground->SetSize(nNewSize);

		////mpIterBtGroup->SetSize(lulWidth, mpIterBtGroup->GetSizeY());
		//D2D1_RECT_F ldRect;
		//ldRect.left = 0;
		//ldRect.top = 0;
		//ldRect.right = lulWidth;
		//ldRect.bottom = 1000;
		//mpIterBtGroup->SetVisibleRegion(ldRect);

		//mpPicturePreview->GetIterator()->SetSize(nNewSize);

		//mpSettingHelp->GetIterator()->SetSize(nNewSize.width - lulWidth, nNewSize.height);
		//mpCoverSet->GetIterator()->SetSize(nNewSize.width - lulWidth, nNewSize.height);
		//mpCHaloKeyboard->GetIterator()->SetSize(nNewSize.width - lulWidth, nNewSize.height);

		//mpSettingHelp->GetIterator()->SetPosition(lulWidth, mpSettingHelp->GetIterator()->GetPositionY());
		//mpCoverSet->GetIterator()->SetPosition(lulWidth, mpCoverSet->GetIterator()->GetPositionY());
		//mpCHaloKeyboard->GetIterator()->SetPosition(lulWidth, mpCHaloKeyboard->GetIterator()->GetPositionY());

	} while (false);

	return ERESULT_SUCCESS;
}


//设置打开文件窗口的位置
void CReaderBaseFrame::SetOpenFilePos(void)
{
	if (mpFileOpenDlg != NULL)
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		FLOAT lfX = (ldPaintSize.w - mpFileOpenDlg->GetIterator()->GetSizeX()) / 2;
		lfX = lfX - mpIterator->GetPosition().x;
		D2D1_POINT_2F ldPos;
		ldPos.x = lfX;
		if (ldPaintSize.w > ldPaintSize.h)
		{
			ldPos.y = 100.0f;
		}
		else
		{
			ldPos.y = 482.0f;
		}
		mpFileOpenDlg->GetIterator()->SetPosition(ldPos);
	}

}

//设置页码跳转窗口的位置
void CReaderBaseFrame::SetJumpPagePos(void)
{
	if (mpJumpPage != NULL)
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		FLOAT lfX = (ldPaintSize.w - mpJumpPage->GetIterator()->GetSizeX()) / 2;
		lfX = lfX - mpIterator->GetPosition().x;
		D2D1_POINT_2F ldPos;
		ldPos.x = lfX;
		if (ldPaintSize.w > ldPaintSize.h)
		{
			ldPos.y = 200.0f;
		}
		else
		{
			ldPos.y = 522.0f;
		}
		mpJumpPage->GetIterator()->SetPosition(ldPos);
	}

}

//获取文件大小
ULONG CReaderBaseFrame::GetFileSize(wchar_t* npszFilePath)
{
	ULONG lulRet = 1;
	HANDLE lhTxtFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lFileSize;

	do 
	{
		lhTxtFile = CreateFile(npszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		if (lhTxtFile == INVALID_HANDLE_VALUE)
			break;

		if (GetFileSizeEx(lhTxtFile, &lFileSize) == FALSE)
			break;

		lulRet = lFileSize.LowPart;

	} while (false);

	if (lhTxtFile != INVALID_HANDLE_VALUE)
		CloseHandle(lhTxtFile);

	return lulRet;
}

//初始化
void CReaderBaseFrame::Init(void)
{
	DWORD ldwRet = 0, ldwLen = 0;;
	HKEY lhKey = NULL;
	wchar_t lszText[MAX_PATH] = { 0 };
	
	do 
	{
		
		DWORD ldwValue = 0;
		ldwRet = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
		if (ldwRet == ERROR_SUCCESS)
		{
			//获取历史记录
			for (int i=0;i<RBF_HISTROY_MAX; i++)
			{
				swprintf_s(lszText, MAX_PATH, L"history%d", i);
				ldwLen = MAX_PATH * sizeof(wchar_t);
				if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lszText, &ldwLen) == ERROR_SUCCESS)
				{
					wchar_t* lpszPath = new wchar_t[MAX_PATH];
					wcscpy_s(lpszPath, MAX_PATH, lszText);
					mdHistroyPath.Insert(-1, lpszPath);
				}
			}

			
		}

		if (mdHistroyPath.Size() > 0)
		{
			//上次看到第几页了
			ldwLen = sizeof(DWORD);
			DWORD ldwPageNumber = 1;
			RegQueryValueEx(lhKey, RbF_REG_PAGE_NUMBER, NULL, NULL, (BYTE*)&ldwPageNumber, &ldwLen);

			DWORD ldwPageContent = 0;
			RegQueryValueEx(lhKey, RbF_REG_PAGE_CONTENT, NULL, NULL, (BYTE*)&ldwPageContent, &ldwLen);
			DWORD ldwPageContent2 = 0;
			RegQueryValueEx(lhKey, RbF_REG_PAGE_CONTENT2, NULL, NULL, (BYTE*)&ldwPageContent2, &ldwLen);

			//上次设置的txt字号
			mdwFontsizeIndex = 1;
			RegQueryValueEx(lhKey, RbF_REG_TXT_FONT_SIZE_INDEX, NULL, NULL, (BYTE*)&mdwFontsizeIndex, &ldwLen);
			if (mdwFontsizeIndex > ZCT_FONTSIZE_LEVEL || mdwFontsizeIndex < 0)
				mdwFontsizeIndex = 1;

			//说明打开过文件,恢复上次打开状态
			mpPdfPicture->SetFontSize(mdwFontSizeArray[mdwFontsizeIndex]);
			if (GetFileAttributes(mdHistroyPath.GetEntry(0)) == INVALID_FILE_ATTRIBUTES)
			{
				//文件不存在了
				CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
				break;
			}
			wcscpy_s(mszSrcFile, MAX_PATH, mdHistroyPath.GetEntry(0));
			CopyFileToTemp(mdHistroyPath.GetEntry(0), mszTempFile, MAX_PATH);
			

			//文件名
			wchar_t* lpszFileName = wcsrchr(mdHistroyPath.GetEntry(0), L'\\');
			if (lpszFileName == NULL)
				lpszFileName = mdHistroyPath.GetEntry(0);
			else
				lpszFileName = lpszFileName + 1;

			if (lpszFileName[wcslen(lpszFileName) - 1] == 't')
				mbIsTxt = true;
			else
				mbIsTxt = false;

			PAGE_PDF_CONTEXT ldPageContent;
			ldPageContent.pageContext = ldwPageContent;
			ldPageContent.pageContext2 = ldwPageContent2;
			ldPageContent.pageIndex = ldwPageNumber;
			mulPageIndex = 0;
			ULONG lulRet = mpPdfPicture->OpenFile(mszTempFile, &ldPageContent);
			mbIsSetPartial = true;
			if (lulRet != EDERR_SUCCESS/* && lulRet != EDERR_EMPTY_CONTENT*/)
			{
				if (mbIsTxt == false || GetFileSize(mdHistroyPath.GetEntry(0)) > 0)
				{
					//打开失败了，可能文件不存在了
					mszSrcFile[0] = UNICODE_NULL;
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
					break;
				}
			}

			//是否双屏
			ldwLen = sizeof(DWORD);
			RegQueryValueEx(lhKey, RbF_REG_DOUBLE_SCREEN, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			mpPdfPicture->EnableDuopageView(ldwValue==0?false:true);
			mpToolbarH->SetDuopageButton(ldwValue == 0 ? false : true);

			EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			if (ldPaintSize.w > ldPaintSize.h)
			{
				mpPdfPicture->EnableDuopageView(ldwValue == 0 ? false : true);
				mpToolbarH->SetDuopageButton(ldwValue == 0 ? false : true);
			}
			else
			{
				mpToolbarH->SetDuopageButton(false);
				mpPdfPicture->EnableDuopageView(false);
			}

			//不能变换这个代码和上面代码的位置，切记
			if (ldwPageNumber > 1 && ldwPageContent == 0 && ldwPageContent2 == 0)
			{
				mpPdfPicture->GoToPage(ldwPageNumber);
			}
			else
			{
				//只有txt格式文件才适用于这种方式
				mpPdfPicture->GoToPage((IEdPage_ptr)NULL);
			}

			ShowPageInfo();
			mpToolbarH->SetFileName(lpszFileName);
		}
		else
		{
			//说明是第一次打开，直接弹出打开文件框，其它的就不需要初始化了
			CExMessage::PostMessage(mpIterator,mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			break;
		}

	} while (false);
}

ERESULT __stdcall CReaderBaseFrame::EinkUpdating(ULONGLONG nxNumber, CReaderBaseFrame* npThis)	// nuNumber是更新序号，每次更新加一，达到最大值后回到零
{
	do 
	{
		if (npThis->mbIsSetPartial == false)
		{
			//npThis->mbIsSetPartial = true;
			break; //此时不要关闭partial,正在处于界面频繁变化阶段
		}
			

		ULONG luCrtTick = GetTickCount();

		if ((nxNumber - npThis->mxLastGcUiNumber >= 10 && luCrtTick - npThis->muLastGcTick > 1000*5) || luCrtTick - npThis->muLastGcTick > 1000 * 60 * 5)	// 十帧或者五分钟
		{
			// 设置进入GC模式 ???niu
			EinkuiGetSystem()->ClearEinkBuffer();
			EiSetPartialUpdate(FALSE);

			npThis->mbGcMode = true;
			npThis->muLastGcTick = luCrtTick;
			npThis->mxLastGcUiNumber = nxNumber;
		}
		else
		{
			if (npThis->mbGcMode != FALSE)
			{
				// 设置退出GC模式 ???niu
				EiSetPartialUpdate(TRUE);

				npThis->mbGcMode = false;
			}
		}

	} while (false);
	

	return ERESULT_SUCCESS;
}
