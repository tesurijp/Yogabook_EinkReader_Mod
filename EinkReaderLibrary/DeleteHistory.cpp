/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "DeleteHistory.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "PreNextButton.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(DeleteHistory)

CDeleteHistory::CDeleteHistory(void)
{
	mpIterBtDelete = NULL;
	mbIsPressed = false;
}


CDeleteHistory::~CDeleteHistory(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CDeleteHistory::OnElementCreate(IEinkuiIterator* npIterator)
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

// 鼠标落点检测
ERESULT CDeleteHistory::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
		|| rPoint.y < 0.0f || rPoint.y >= mpIterator->GetSizeY()))
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}
	else
	{
		luResult = ERESULT_SUCCESS;
	}

	

	return luResult;
}


//鼠标移动
ERESULT CDeleteHistory::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);

		if (MOUSE_LB(npInfo->KeyFlag) == false)  //如果不是鼠标左键就不处理
			break;

		//if (mbIsBeginMove != false)
		//{
		//	//开始滑动记录下起始坐标
		//	mdDropBeginPos = npInfo->Position;
		//	mbIsBeginMove = false;
		//	break;
		//}

		//mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		//mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//鼠标按下
ERESULT CDeleteHistory::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if (mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if (MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if (npInfo->Presssed == false && mbIsPressed != false)
		{
			//鼠标抬起

			ExitModal();

		}
		else if (npInfo->Presssed != false)
		{
			//鼠标按下
			mbIsPressed = true;
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

ULONG CDeleteHistory::InitOnCreate(
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
		mpIterBtDelete = mpIterator->GetSubElementByID(DH_BT_DELETE);
		BREAK_ON_NULL(mpIterBtDelete);
		
	

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//按钮单击事件
ERESULT CDeleteHistory::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		PostMessageToParent(EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case DH_BT_DELETE:
		{
			//清除
			PostMessageToParent(EEVT_DELETE_READ_HISTORY,CExMessage::DataInvalid);
			ExitModal();
			//mpIterator->SetVisible(false);
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


void CDeleteHistory::DoModal()
{
	do
	{
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->EnablePaintboard(false);
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);


		mpIterator->Close();

	} while (false);
}

void CDeleteHistory::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);
}

//消息处理函数
ERESULT CDeleteHistory::ParseMessage(IEinkuiMessage* npMsg)
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
void CDeleteHistory::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CDeleteHistory::OnElementResized(D2D1_SIZE_F nNewSize)
{

	if (mpIterBtDelete != NULL)
	{
		mpIterBtDelete->SetPosition(nNewSize.width - mpIterBtDelete->GetSizeX() - 10.0f, mpIterBtDelete->GetPositionY());
	}

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CDeleteHistory::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}
