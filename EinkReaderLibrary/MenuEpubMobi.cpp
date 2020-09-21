/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "MenuEpubMobi.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "PreNextButton.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(MenuEpubMobi)

CMenuEpubMobi::CMenuEpubMobi(void)
{
	mpIterMenuBase = NULL;
	mpIterPageGroup = NULL;
	mbIsPressed = false;
}


CMenuEpubMobi::~CMenuEpubMobi(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CMenuEpubMobi::OnElementCreate(IEinkuiIterator* npIterator)
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


ULONG CMenuEpubMobi::InitOnCreate(
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
		leResult = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
		if (leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄
		mpIterMenuBase = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterMenuBase);

		mpIterPageGroup = mpIterMenuBase->GetSubElementByID(4);
		BREAK_ON_NULL(mpIterPageGroup);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

// 鼠标落点检测
ERESULT CMenuEpubMobi::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
		|| rPoint.y < 0.0f || rPoint.y >= mpIterator->GetSizeY()))
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}
	else if (rPoint.x < 0.0f || rPoint.y < 0.0f)
	{
		//说明出错了
		ExitModal();
	}
	else
	{
		luResult = ERESULT_SUCCESS;
	}

	

	return luResult;
}


//鼠标移动
ERESULT CMenuEpubMobi::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
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
ERESULT CMenuEpubMobi::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
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


//按钮单击事件
ERESULT CMenuEpubMobi::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//ULONG llBtnID = npSender->GetID();
		//switch (llBtnID)
		//{
		//case MT_BT_SHAPSHOT:
		//{
		//	//截屏
		//	PostMessageToParent(EEVT_ER_ENTER_SNAPSHOT,CExMessage::DataInvalid);
		//	
		//	//mpIterator->SetVisible(false);
		//	break;
		//}
		//default:
		//	break;
		//}

		

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


void CMenuEpubMobi::DoModal()
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

//设置单双页
void CMenuEpubMobi::SetPageTwo(bool nbIsDouble, bool nbIsEnable)
{
	CExMessage::SendMessage(mpIterPageGroup, mpIterator, EACT_RBG_SET_SELECT, nbIsDouble==false?1:2);

	mpIterPageGroup->GetSubElementByID(2)->SetEnable(nbIsEnable);
}

//设置屏幕方向
void CMenuEpubMobi::SetScreenOritent(ULONG nulIndex)
{
	CExMessage::SendMessage(mpIterMenuBase->GetSubElementByID(6), mpIterator, EACT_RBG_SET_SELECT, nulIndex);
}

void CMenuEpubMobi::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);
}


//消息处理函数
ERESULT CMenuEpubMobi::ParseMessage(IEinkuiMessage* npMsg)
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
	case EEVT_MENU_ITEM_CLICKED:
	{
		ULONG lulID = 0;
		luResult = CExMessage::GetInputData(npMsg, lulID);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulID == ME_BT_SHAPSHOT)
		{
			//截屏
			PostMessageToParent(EEVT_ER_ENTER_SNAPSHOT, CExMessage::DataInvalid);
		}

		ExitModal();

		break;
	}
	case EEVT_RBG_SELECTED_CHANGED:
	{
		//单双页项选择变化
		ULONG lulSelectID = 0;
		luResult = CExMessage::GetInputData(npMsg, lulSelectID);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulSelectID < 10)
		{
			bool lbIsDouble = false;
			if (lulSelectID == 1)
				lbIsDouble = false;
			else
				lbIsDouble = true;

			PostMessageToParent(EEVT_ER_TWO_SCREEN, lbIsDouble);
		}
		else
		{
			//页面方向

			CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_SET_SCREEN_STATUS, lulSelectID);
		}
		ExitModal();

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
void CMenuEpubMobi::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CMenuEpubMobi::OnElementResized(D2D1_SIZE_F nNewSize)
{

	if (mpIterMenuBase != NULL)
	{
		mpIterMenuBase->SetPosition(nNewSize.width - mpIterMenuBase->GetSizeX() - 120.0f, mpIterMenuBase->GetPositionY());
		
	}

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CMenuEpubMobi::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}
