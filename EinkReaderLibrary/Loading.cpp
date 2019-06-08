/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "Loading.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <Shlobj.h>

DEFINE_BUILTIN_NAME(LoadingView)

CLoadingView::CLoadingView(void)
{
	mpIterFileName = NULL;
	mpIterDi = NULL;
	mpIterDd = NULL;
	mplStep = NULL;
	mWaitHandle = NULL;
	mpIterIndex = NULL;
}


CLoadingView::~CLoadingView(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CLoadingView::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CLoadingView::InitOnCreate(
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


		//获取对像句柄
		mpIterDd = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterDd);

		mpIterDi = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterDi);

		mpIterFileName = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIterFileName);

		mpIterIndex = mpIterator->GetSubElementByID(10);
		BREAK_ON_NULL(mpIterIndex);
		mpIterIndex->SetVisible(false);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置已加载页数
void CLoadingView::SetPage(LONG niIndex)
{
	IConfigFile* lpProfile = NULL;
	wchar_t lszText[MAX_PATH] = { 0 };

	do 
	{
		if(mpIterIndex == NULL)
			break;

		wchar_t lszString[MAX_PATH] = { 0 };

		//获取多语言字符串
		//为了翻译方便，字符串存放在root/string
		lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		ICfKey* lpCfKey = NULL;
		if (lpProfile != NULL)
		{
			lpCfKey = lpProfile->OpenKey(L"String2/LoadPageIndex");

			if (lpCfKey != NULL)
				lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));

		}
		CMM_SAFE_RELEASE(lpCfKey);

		swprintf_s(lszString, MAX_PATH, lszText, niIndex);
		CExMessage::PostMessageWithText(mpIterIndex, mpIterator, EACT_LABEL_SET_TEXT, lszString);
		
		if (mpIterIndex->IsVisible() == false && mpIterIndex->GetSizeX() > 50.0f)
		{
			mpIterIndex->SetPosition((mpIterator->GetSizeX() - mpIterIndex->GetSizeX()) / 2.0f, mpIterIndex->GetPositionY());
			mpIterIndex->SetVisible(true);
		}
			

	} while (false);
	
}

//按钮单击事件
ERESULT CLoadingView::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


void CLoadingView::DoModal(volatile LONG* nplStep, HANDLE nWaitHandle)
{
	do
	{
		mplStep = nplStep;
		mWaitHandle = nWaitHandle;

		mpIterator->SetVisible(true);
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->UpdateView(true);
		mpIterator->SetTimer(LOAD_TIMER_MOVE, MAXULONG32, 1000, NULL);

		EinkuiGetSystem()->DoModal(mpIterator);


		mpIterator->Close();

	} while (false);
}

void CLoadingView::ExitModal()
{
	mpIterator->KillTimer(LOAD_TIMER_MOVE);
	//mpIterator->SetVisible(false);
	EinkuiGetSystem()->ExitModal(mpIterator, 0);
}

//消息处理函数
ERESULT CLoadingView::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_MODAL_ENTER:
	{
		//// 创建要弹出的对话框
		//mpIterator->SetVisible(true);
		luResult = ERESULT_SUCCESS;
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

//定时器
void CLoadingView::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
	if (npStatus->TimerID == LOAD_TIMER_MOVE)
	{
		//移动
		float lfMoved = mpIterDd->GetPositionX() + 51.0f;
		if ((lfMoved + mpIterDd->GetSizeX()) > (mpIterDi->GetPositionX() + mpIterDi->GetSizeX()))
			lfMoved = mpIterDi->GetPositionX()+2.0f;

		EiSetPartialUpdate(TRUE);
		EinkuiGetSystem()->ClearEinkBuffer();

		mpIterDd->SetPosition(lfMoved, mpIterDd->GetPositionY());

		if (mplStep != NULL && *mplStep == 0)
			ExitModal(); //退出

		if (mWaitHandle != NULL && WaitForSingleObject(mWaitHandle,100) == WAIT_OBJECT_0)
			ExitModal(); //退出
	}
}

//绘制消息
ERESULT CLoadingView::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	// 绘制背景
	if (mpBgBitmap != NULL)
		npPaintBoard->DrawBitmap(D2D1::RectF(0, 0, mpIterator->GetSizeX(), mpIterator->GetSizeY()),
			mpBgBitmap,
			ESPB_DRAWBMP_EXTEND);

	return ERESULT_SUCCESS;
}

//元素参考尺寸发生变化
ERESULT CLoadingView::OnElementResized(D2D1_SIZE_F nNewSize)
{
	//定位
	if(mpIterFileName != NULL)
		mpIterFileName->SetPosition((nNewSize.width - mpIterFileName->GetSizeX()) / 2.0f, nNewSize.height/2.0f-100);

	if(mpIterDi != NULL)
		mpIterDi->SetPosition((nNewSize.width - mpIterDi->GetSizeX()) / 2.0f, mpIterFileName->GetPositionY() + 100.0f);
	if(mpIterDd != NULL)
		mpIterDd->SetPosition(mpIterDi->GetPositionX()+2.0f, mpIterDi->GetPositionY()+2.0f);

	if (mpIterIndex != NULL)
		mpIterIndex->SetPosition((nNewSize.width - mpIterIndex->GetSizeX()) / 2.0f, mpIterDi->GetPositionY() + 100.0f);

	return ERESULT_SUCCESS;
}


//设置文件名
void CLoadingView::SetData(wchar_t* npszFileName)
{
	do 
	{
		BREAK_ON_NULL(npszFileName);
		CExMessage::SendMessageWithText(mpIterFileName, mpIterator, EACT_LABEL_SET_TEXT, npszFileName);

		//重新定位
		mpIterFileName->SetPosition((mpIterator->GetSizeX()-mpIterFileName->GetSizeX()) / 2.0f,mpIterFileName->GetPositionY());

	} while (false);
}

//通知元素【显示/隐藏】发生改变
ERESULT CLoadingView::OnElementShow(bool nbIsShow)
{
	return ERESULT_SUCCESS;
}