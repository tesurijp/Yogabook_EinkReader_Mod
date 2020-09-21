/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ZoomControlToolbar.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "ToolbarBottom.h"

DEFINE_BUILTIN_NAME(ZoomControlToolbar)

CZoomControlToolbar::CZoomControlToolbar(void)
{
	mpIterBili = NULL;

	mlCurrentZoomLevel = 0;
	mfZoom.Insert(-1, 1.0);
}

CZoomControlToolbar::~CZoomControlToolbar(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CZoomControlToolbar::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CZoomControlToolbar::InitOnCreate(
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
		mpIterBili = mpIterator->GetSubElementByID(110);
		BREAK_ON_NULL(mpIterBili);
		
		mpIterBtAdd = mpIterator->GetSubElementByID(ZC_BT_ADD);
		BREAK_ON_NULL(mpIterBtAdd);

		mpIterBtSub = mpIterator->GetSubElementByID(ZC_BT_SUB);
		BREAK_ON_NULL(mpIterBtSub);

		mpIterBtAutoZoom = mpIterator->GetSubElementByID(ZC_BT_AUTO_ZOOM);
		BREAK_ON_NULL(mpIterBtAutoZoom);

		mpIterBtResetZoom = mpIterator->GetSubElementByID(ZC_BT_RESET_ZOOM);
		BREAK_ON_NULL(mpIterBtResetZoom);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置显示比例
void CZoomControlToolbar::SetString(ULONG nulLevel)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	swprintf_s(lszString, MAX_PATH, L"%d%s", int(100 + nulLevel*10), L"%");

	CExMessage::SendMessageWithText(mpIterBili, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);
}

//设置显示比例
void CZoomControlToolbar::SetRatioString(double ratio)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	swprintf_s(lszString, MAX_PATH, L"%d%s", int(ratio * 100.0), L"%");
	CExMessage::SendMessageWithText(mpIterBili, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);
}

//按钮单击事件
ERESULT CZoomControlToolbar::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case ZC_BT_DEFAULT:
		case ZC_BT_RESET_ZOOM:
		{
			//恢复为100%显示
			PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);
			initData();
			break;
		}
		case ZC_BT_ADD:
		{
			//放大
			SetLevel(true);
			break;
		}
		case ZC_BT_SUB:
		{
			//缩小
			SetLevel(false);
			break;
		}
		case ZC_BT_AUTO_ZOOM:
		{
			SendMessageToParent(EEVT_ER_AUTO_ZOOM, NULL, NULL, NULL);
			break;
		}
		default:
			break;
		}

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//初始化自己
void CZoomControlToolbar::initData(void)
{
	mlCurrentZoomLevel = 0;
	mpIterBtSub->SetEnable(true);
	mpIterBtAdd->SetEnable(true);
	SetString(mlCurrentZoomLevel);
}

void CZoomControlToolbar::SetFatRatio(double fatRatio)
{
	mfFatRatio = 1.0;
	bool fatSaved = false;
	mfZoom.Clear();
	//100%，120%，140%，160%
	mfZoom.Insert(-1, mfFatRatio);
	for(int i=11;i<=40;i+=1)
		mfZoom.Insert(-1, mfFatRatio*i*0.1);
}

//进入缩放状态,默认120%显示
// [zhuhl5@20200116:ZoomRecovery]
// Modified function params for initiating zoom level.
void CZoomControlToolbar::EnterZoomStatus(int scaleLevel /*= 0*/)
{
	mlCurrentZoomLevel = 1;
	if (scaleLevel >= 1)
	{
		mlCurrentZoomLevel = scaleLevel; //默认120%
	}
	RECT ldRect;
	mpIterBtAdd->SetEnable(true);
	mpIterBtSub->SetEnable(true);
	CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, mfZoom[mlCurrentZoomLevel], &ldRect, sizeof(RECT));

	// Add zoom level notification, for recording
	CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_ZOOMLEVEL, mlCurrentZoomLevel);
}

//按照自适应缩放比例，对应到合适的缩放等级
float CZoomControlToolbar::AjustAutoZoomLevel(float ratio)
{
	wchar_t* lszString = L"Auto";
	CExMessage::SendMessageWithText(mpIterBili, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

	mlCurrentZoomLevel = -1;
	for (int i = 0; i < mfZoom.Size(); ++i)
	{
		if (ratio >= mfZoom[i])
			++mlCurrentZoomLevel;
		else
			break;
	}

	if (mlCurrentZoomLevel >= mfZoom.Size() - 1)
		mpIterBtAdd->SetEnable(false);
	else
		mpIterBtAdd->SetEnable(true);

	return mfZoom[mlCurrentZoomLevel];
}

void CZoomControlToolbar::EnableAutoZoomButton(bool enable)
{
	mpIterBtAutoZoom->SetEnable(enable);
}

//设置放大级别
void CZoomControlToolbar::SetLevel(bool nbIsAdd)
{
	do
	{
		if (nbIsAdd == false)
		{
			//降低
			if (mlCurrentZoomLevel <= 0)
			{
				PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);
				initData();
				break; //已经是最低了
			}

			if (--mlCurrentZoomLevel <= 0)
				mpIterBtSub->SetEnable(false);

			mpIterBtAdd->SetEnable(true);
		}
		else
		{
			//增加
			if (mlCurrentZoomLevel >= mfZoom.Size()-1)
				break; //已经是最高了

			if (++mlCurrentZoomLevel >= mfZoom.Size()-1)
				mpIterBtAdd->SetEnable(false);

			mpIterBtSub->SetEnable(true);
		}

		RECT ldRect;
		CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, mfZoom[mlCurrentZoomLevel], &ldRect, sizeof(RECT));
		// Add zoom level notification, for recording
		CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_ZOOMLEVEL, mlCurrentZoomLevel);
		
		if (mlCurrentZoomLevel <= 0)
		{
			PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);
			initData();
		}

	} while (false);
}

//消息处理函数
ERESULT CZoomControlToolbar::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_MODAL_ENTER:
	{
		// 创建要弹出的对话框
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
void CZoomControlToolbar::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
}

//元素参考尺寸发生变化
ERESULT CZoomControlToolbar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CZoomControlToolbar::OnElementShow(bool nbIsShow)
{
	return ERESULT_SUCCESS;
}