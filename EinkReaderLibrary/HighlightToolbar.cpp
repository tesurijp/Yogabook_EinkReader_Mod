/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "HighlightToolbar.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <shellapi.h>

DEFINE_BUILTIN_NAME(HighlightToolbar)

CHighlightToolbar::CHighlightToolbar(void)
{
	mpIterBtCopy = NULL;
	mpIterBtHighlight = NULL;
	mpIterBtDeleteLine = NULL;
	mpIterBtLine = NULL;
	mpIterBtDelete = NULL;
}


CHighlightToolbar::~CHighlightToolbar(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CHighlightToolbar::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CHighlightToolbar::InitOnCreate(
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
		mpIterBtDelete = mpIterator->GetSubElementByID(HIGHLIGHT_BT_DELETE);
		BREAK_ON_NULL(mpIterBtDelete);
		//
		//mpIterBtAdd = mpIterator->GetSubElementByID(ZC_BT_ADD);
		//BREAK_ON_NULL(mpIterBtAdd);

		//mpIterBtSub = mpIterator->GetSubElementByID(ZC_BT_SUB);
		//BREAK_ON_NULL(mpIterBtSub);


		////获取对像句柄
		//mpIterBtUP = mpIterator->GetSubElementByID(ZC_BT_UP);
		//BREAK_ON_NULL(mpIterBtUP);

		//mpIterBtDown = mpIterator->GetSubElementByID(ZC_BT_DOWN);
		//BREAK_ON_NULL(mpIterBtDown);

		//mpIterBtLeft = mpIterator->GetSubElementByID(ZC_BT_LEFT);
		//BREAK_ON_NULL(mpIterBtLeft);

		//mpIterBtRight = mpIterator->GetSubElementByID(ZC_BT_RIGHT);
		//BREAK_ON_NULL(mpIterBtRight);

		/*mpIterBtDefault = mpIterator->GetSubElementByID(ZC_BT_DEFAULT);
		BREAK_ON_NULL(mpIterBtDefault);*/

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//按钮单击事件
ERESULT CHighlightToolbar::OnCtlButtonClick(IEinkuiIterator* npSender)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		int liBtnID = (int)npSender->GetID();
		switch (liBtnID)
		{
		case HIGHLIGHT_BT_COPY:
		case HIGHLIGHT_BT_HIGHLIGHT:
		case HIGHLIGHT_BT_DELETE_LINE:
		case HIGHLIGHT_BT_UNDER_LINE:
		case HIGHLIGHT_BT_DELETE:
		case HIGHLIGHT_BT_TRANSLATE:
		{
			//复制到剪切板
			//PostMessageToParent(EEVT_COPY_TEXT, CExMessage::DataInvalid);
			CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_HIGHLIGHT_BT_EVENT, liBtnID);

			mpIterator->SetVisible(false);
			//initData();

			break;
		}
	
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//设置highlight数量
void CHighlightToolbar::SetHighlightCount(int niCount)
{
	if (niCount > 0)
	{
		mpIterBtDelete->SetEnable(true);
	}
	else
	{
		mpIterBtDelete->SetEnable(false);
	}
}

//初始化自己
void CHighlightToolbar::initData(void)
{

}


//消息处理函数
ERESULT CHighlightToolbar::ParseMessage(IEinkuiMessage* npMsg)
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
void CHighlightToolbar::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CHighlightToolbar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CHighlightToolbar::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		mpIterator->BringToTop();
	}
	return ERESULT_SUCCESS;
}