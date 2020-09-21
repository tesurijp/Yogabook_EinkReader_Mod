/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "MenuTxt.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "PreNextButton.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(MenuTxt)

CMenuTxt::CMenuTxt(void)
{
	mpIterMenuBase = NULL;
	mpIterFontSizeGroup = NULL;
	mbIsPressed = false;
}


CMenuTxt::~CMenuTxt(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CMenuTxt::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);
		mpIterMenuBase->BringToTop();

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


ULONG CMenuTxt::InitOnCreate(
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

		mpIterFontSizeGroup = mpIterMenuBase->GetSubElementByID(4);
		BREAK_ON_NULL(mpIterFontSizeGroup);


		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

// 鼠标落点检测
ERESULT CMenuTxt::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
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
ERESULT CMenuTxt::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
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
ERESULT CMenuTxt::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
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
		else if(npInfo->Presssed != false)
		{
			//鼠标按下
			mbIsPressed = true;
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}


//按钮单击事件
ERESULT CMenuTxt::OnCtlButtonClick(IEinkuiIterator* npSender)
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


void CMenuTxt::DoModal()
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

void CMenuTxt::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);
}

//设置txt字号
void CMenuTxt::SetTxtFontSizeIndex(DWORD ndwIndex)
{
	CExMessage::SendMessage(mpIterFontSizeGroup, mpIterator, EACT_RBG_SET_SELECT, ndwIndex + 1);
}

//设置屏幕方向
void CMenuTxt::SetScreenOritent(ULONG nulIndex)
{
	CExMessage::SendMessage(mpIterMenuBase->GetSubElementByID(6), mpIterator, EACT_RBG_SET_SELECT, nulIndex);
}

//消息处理函数
ERESULT CMenuTxt::ParseMessage(IEinkuiMessage* npMsg)
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

		if (lulID == MT_BT_SHAPSHOT)
		{
			//截屏
			PostMessageToParent(EEVT_ER_ENTER_SNAPSHOT, CExMessage::DataInvalid);
		}

		ExitModal();

		break;
	}
	case EEVT_RBG_SELECTED_CHANGED:
	{
		//项选择变化
		ULONG lulSelectID = 0;
		luResult = CExMessage::GetInputData(npMsg, lulSelectID);
		if (luResult != ERESULT_SUCCESS)
			break;
		
		if (lulSelectID < 10)
		{
			//文字大小
			DWORD ldwIndex = 0;
			if (lulSelectID == 1)
				ldwIndex = 0;
			else if (lulSelectID == 2)
				ldwIndex = 1;
			else if (lulSelectID == 3)
				ldwIndex = 2;
			else if (lulSelectID == 4)
				ldwIndex = 3;

			PostMessageToParent(EEVT_ER_SET_TXT_ZOOM, ldwIndex);
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
void CMenuTxt::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CMenuTxt::OnElementResized(D2D1_SIZE_F nNewSize)
{

	if (mpIterMenuBase != NULL)
	{
		mpIterMenuBase->SetPosition(nNewSize.width - mpIterMenuBase->GetSizeX() - 120.0f, mpIterMenuBase->GetPositionY());
	}

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CMenuTxt::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}
