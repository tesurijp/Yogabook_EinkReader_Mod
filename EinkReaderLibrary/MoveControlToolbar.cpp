/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "MoveControlToolbar.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(MoveControlToolbar)

CMoveControlToolbar::CMoveControlToolbar(void)
{
}

CMoveControlToolbar::~CMoveControlToolbar(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CMoveControlToolbar::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CMoveControlToolbar::InitOnCreate(
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
		mpIterBtUP = mpIterator->GetSubElementByID(ZC_BT_UP);
		BREAK_ON_NULL(mpIterBtUP);

		mpIterBtDown = mpIterator->GetSubElementByID(ZC_BT_DOWN);
		BREAK_ON_NULL(mpIterBtDown);

		mpIterBtLeft = mpIterator->GetSubElementByID(ZC_BT_LEFT);
		BREAK_ON_NULL(mpIterBtLeft);

		mpIterBtRight = mpIterator->GetSubElementByID(ZC_BT_RIGHT);
		BREAK_ON_NULL(mpIterBtRight);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//上下左右移动
void CMoveControlToolbar::MovePage(ULONG nulID)
{
	POINT ldPos;
	RECT ldRect;
	if (nulID == ZC_BT_UP)
	{
		ldPos.y = 300;
		ldPos.x = 0;
	}
	else if (nulID == ZC_BT_DOWN)
	{
		ldPos.y = -300;
		ldPos.x = 0;
	}
	if (nulID == ZC_BT_LEFT)
	{
		ldPos.x = 300;
		ldPos.y = 0;
	}
	if (nulID == ZC_BT_RIGHT)
	{
		ldPos.x = -300;
		ldPos.y = 0;
	}

	CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_PAGE_MOVE, ldPos, &ldRect, sizeof(RECT));
	ShowMoveButton(ldRect);
}

//设置4个移动按钮的状态
void CMoveControlToolbar::ShowMoveButton(RECT ldRect)
{
	if (mMoveForward == MoveForward::HORIZONTAL)
	{
		ldRect.top = 0;
		ldRect.bottom = 0;
	}
	else if (mMoveForward == MoveForward::VERTICAL)
	{
		ldRect.left = 0;
		ldRect.right = 0;
	}
	
	mpIterBtLeft->SetEnable(ldRect.left != 0);
	mpIterBtRight->SetEnable(ldRect.right != 0);
	mpIterBtUP->SetEnable(ldRect.top != 0);
	mpIterBtDown->SetEnable(ldRect.bottom != 0);
}

//设置滑动方向
void CMoveControlToolbar::SetMoveForward(MoveForward forward)
{
	mMoveForward = forward;
}

//按钮单击事件
ERESULT CMoveControlToolbar::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case ZC_BT_UP:
		case ZC_BT_DOWN:
		case ZC_BT_LEFT:
		case ZC_BT_RIGHT:
		{
			//上下左右移动
			MovePage(llBtnID);

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
void CMoveControlToolbar::initData(void)
{
}

//消息处理函数
ERESULT CMoveControlToolbar::ParseMessage(IEinkuiMessage* npMsg)
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
void CMoveControlToolbar::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
}

//元素参考尺寸发生变化
ERESULT CMoveControlToolbar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CMoveControlToolbar::OnElementShow(bool nbIsShow)
{
	return ERESULT_SUCCESS;
}