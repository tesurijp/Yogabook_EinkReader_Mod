/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "TipFrame.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(TipFrame)

CTipFrame::CTipFrame(void)
{

}


CTipFrame::~CTipFrame(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CTipFrame::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CTipFrame::InitOnCreate(
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
		mpIterBtClose = mpIterator->GetSubElementByID(TIP_F_BT_CLOSE);
		BREAK_ON_NULL(mpIterBtClose);

		mpIterPicBack = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterPicBack);

		mpIterPicLeft = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterPicLeft);

		mpIterPicRight = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIterPicRight);

		mpIterPicPic = mpIterator->GetSubElementByID(4);
		BREAK_ON_NULL(mpIterPicPic);

		mpIterText = mpIterator->GetSubElementByID(5);
		BREAK_ON_NULL(mpIterText);
		
		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//按钮单击事件
ERESULT CTipFrame::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case TIP_F_BT_CLOSE:
		{
			ExitModal();
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
ERESULT CTipFrame::ParseMessage(IEinkuiMessage* npMsg)
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
void CTipFrame::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

void CTipFrame::DoModal()
{
	do
	{
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);


		mpIterator->Close();

	} while (false);
}

void CTipFrame::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);

}

//元素参考尺寸发生变化
ERESULT CTipFrame::OnElementResized(D2D1_SIZE_F nNewSize)
{
	CExMessage::SendMessage(mpIterBtClose, mpIterator, EACT_BUTTON_SET_ACTION_RECT, nNewSize);
	mpIterPicBack->SetSize(nNewSize);

	//左右两个图片
	FLOAT lfY = (nNewSize.height - mpIterPicLeft->GetSizeY()) / 2;
	mpIterPicLeft->SetPosition(mpIterPicLeft->GetPositionX(), lfY);
	mpIterPicRight->SetPosition(nNewSize.width-mpIterPicRight->GetSizeX() - 40.0f, lfY);

	//中间图片
	FLOAT lfX = (nNewSize.width - mpIterPicPic->GetSizeY()) / 2;
	lfY = (nNewSize.height - mpIterPicPic->GetSizeY()) / 2 + 40;
	mpIterPicPic->SetPosition(lfX, lfY);

	//中间文字
	lfX = (nNewSize.width - mpIterText->GetSizeY()) / 2;
	lfY = (nNewSize.height - mpIterText->GetSizeY()) / 2 - 40;
	mpIterText->SetPosition(lfX, lfY);

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CTipFrame::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}