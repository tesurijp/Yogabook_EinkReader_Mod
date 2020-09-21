/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "PageProgress.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <math.h>

DEFINE_BUILTIN_NAME(PageProgress)


CPageProgress::CPageProgress(void)
{
	mpIterProgress = NULL;
	mpIterArrow = NULL;
	mfCurrentPage = 0.0f;
	mfPageCount = 0.0f;
}


CPageProgress::~CPageProgress(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CPageProgress::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP | EITR_STYLE_KEYBOARD);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CPageProgress::InitOnCreate(
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
		mpIterProgress = mpIterator->GetSubElementByID(PP_PROGRESS);
		BREAK_ON_NULL(mpIterProgress);
		
		mpIterArrow = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIterArrow);
		
		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

// 设置数据
void CPageProgress::SetData(FLOAT nfCurrentPage, FLOAT nfPageCount)
{
	mfCurrentPage = nfCurrentPage;
	mfPageCount = nfPageCount;

	if (nfPageCount > 1)
	{
		CExMessage::SendMessage(mpIterProgress, mpIterator, EACT_SLIDERBAR_SET_RANGE, mfPageCount - 1.0f);
		CExMessage::SendMessage(mpIterProgress, mpIterator, EACT_SLIDERBAR_SET_POSITION, mfCurrentPage - 1.0f);
		mpIterProgress->SetEnable(true);
	}
	else
	{
		CExMessage::SendMessage(mpIterProgress, mpIterator, EACT_SLIDERBAR_SET_RANGE, mfPageCount);
		CExMessage::SendMessage(mpIterProgress, mpIterator, EACT_SLIDERBAR_SET_POSITION, mfCurrentPage);

		mpIterProgress->SetEnable(false);
	}

}

//设置箭头位置
void CPageProgress::SetArrowPos(D2D1_POINT_2F ndPos)
{
	mpIterArrow->SetPosition(ndPos.x - mpIterArrow->GetSizeX()/2.0f,ndPos.y);
}

//按钮单击事件
ERESULT CPageProgress::OnCtlButtonClick(IEinkuiIterator* npSender)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case PP_BT_CLOSE:
		{
			//关闭界面
			mpIterator->SetVisible(false);
			
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//消息处理函数
ERESULT CPageProgress::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_SLIDERBAR_DRAG_END:
	case EACT_SLIDERBAR_THUMB_CLICK:
	//case EACT_SLIDERBAR_DRAGING:
	{
		// 拖动进度条
		//CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		if (npMsg->GetInputDataSize() != sizeof(FLOAT))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
		int liCurrent = (int)round(*lpValue)+1;
		if (liCurrent < 1)
			liCurrent = 1;
		//PostMessageToParent(EEVT_ER_PAGE_JUMP, liCurrent);
		CExMessage::PostMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_PAGE_JUMP, liCurrent);

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
void CPageProgress::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CPageProgress::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CPageProgress::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		mpIterator->BringToTop();
	}
	return ERESULT_SUCCESS;
}

//鼠标进入或离开
void  CPageProgress::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	if (npState->State == 0)
	{
		mpIterator->BringToTop();
	}
}