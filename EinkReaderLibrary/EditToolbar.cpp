/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "EditToolbar.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(EditToolbar)


CEditToolbar::CEditToolbar(void)
{
	mpIterBtUndo = NULL;
	mpIterBtRedo = NULL;

	mdwPenColorIndex = 0;
	mdwPenWidthIndex = 1;

	mpMenuPen = NULL;
	mpIterGroup = NULL;
	mbIsHand = false;

	mulSelectID = PEN_MODE_PEN;
}


CEditToolbar::~CEditToolbar(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEditToolbar::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CEditToolbar::InitOnCreate(
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
		mpIterBtUndo = mpIterator->GetSubElementByID(ET_BT_UNDO);
		BREAK_ON_NULL(mpIterBtUndo);
		mpIterBtUndo->SetEnable(false);
		//mpIterBtUndo->SetPosition(mpIterBtUndo->GetPositionX() - 100.0f, mpIterBtUndo->GetPositionY()); //开启选择按钮后，删除该代码

		mpIterBtRedo = mpIterator->GetSubElementByID(ET_BT_REDO);
		BREAK_ON_NULL(mpIterBtRedo);
		mpIterBtRedo->SetEnable(false);
		//mpIterBtRedo->SetPosition(mpIterBtRedo->GetPositionX() - 100.0f, mpIterBtRedo->GetPositionY()); //开启选择按钮后，删除该代码

		mpIterBtHand = mpIterator->GetSubElementByID(ET_BT_HAND);
		BREAK_ON_NULL(mpIterBtHand);
		//mpIterBtHand->SetPosition(mpIterBtHand->GetPositionX() - 100.0f, mpIterBtHand->GetPositionY()); //开启选择按钮后，删除该代码
		
		mpIterLine2 = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterLine2);
		//mpIterLine2->SetPosition(mpIterLine2->GetPositionX() - 100.0f, mpIterLine2->GetPositionY()); //开启选择按钮后，删除该代码

		mpIterLine1 = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterLine1);
		//mpIterLine1->SetPosition(mpIterLine1->GetPositionX() - 100.0f, mpIterLine1->GetPositionY()); //开启选择按钮后，删除该代码

		//暂时隐藏选择按钮
		mpIterGroup = mpIterator->GetSubElementByID(3);
		//mpIterGroup->GetSubElementByID(3)->SetVisible(false);


		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}


//按钮单击事件
ERESULT CEditToolbar::OnCtlButtonClick(IEinkuiIterator* npSender)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case ET_BT_UNDO:
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_UNDO,CExMessage::DataInvalid);
			mpIterBtRedo->SetEnable(true);

			break;
		}
		case ET_BT_REDO:
		{

			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_REDO, CExMessage::DataInvalid);
			mpIterBtUndo->SetEnable(true);

			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//设置redo,undo按钮状态
void CEditToolbar::SetUndoRedoStatus(bool nbUndoEnable, bool nbRedoEnable)
{
	if(mpIterBtRedo->IsEnable() != nbRedoEnable)
		mpIterBtRedo->SetEnable(nbRedoEnable);

	if (mpIterBtUndo->IsEnable() != nbUndoEnable)
		mpIterBtUndo->SetEnable(nbUndoEnable);
}

// 设置初始化数据
void CEditToolbar::SetData(DWORD ndwPenWidthIndex, DWORD ndwPenColorIndex)
{
	mdwPenWidthIndex = ndwPenWidthIndex;
	mdwPenColorIndex = ndwPenColorIndex;
}

//设置按钮选中状态
void CEditToolbar::SetSelect(ULONG nulID)
{
	if (mulSelectID != nulID)
	{
		mulSelectID = nulID;
		CExMessage::PostMessage(mpIterGroup, mpIterator, EACT_RBG_SET_SELECT, mulSelectID);
	}
}

//消息处理函数
ERESULT CEditToolbar::ParseMessage(IEinkuiMessage* npMsg)
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
	case EEVT_RBG_SELECTED_ITEM_CLICK:
	{
		//被选中的情况下再次被点击
		//CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		ULONG lulSelectID = 0;
		luResult = CExMessage::GetInputData(npMsg, lulSelectID);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulSelectID == ET_BT_PEN)
		{
			//弹出笔迹菜单
			if (mpMenuPen != NULL)
			{
				mpMenuPen->ExitModal();
				mpMenuPen = NULL;
			}

			ICfKey* lpSubKey = mpTemplete->OpenKey(L"MenuPen");
			mpMenuPen = CMenuPen::CreateInstance(mpIterator, lpSubKey);
			mpMenuPen->SetData(mdwPenWidthIndex, mdwPenColorIndex);
			EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			mpMenuPen->GetIterator()->SetSize(float(ldPaintSize.w), float(ldPaintSize.h));

			//弹出菜单时就关闭输入
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_NONE);
			mpMenuPen->DoModal();
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);

			CMM_SAFE_RELEASE(lpSubKey);
			mpMenuPen = NULL;

			EinkuiGetSystem()->UpdateView(true);
		}
		break;
	}
	case EEVT_RBG_SELECTED_CHANGED:
	{
		//项选择变化
		//CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_RESET_HIDE_TIME, true);

		
		luResult = CExMessage::GetInputData(npMsg, mulSelectID);
		if (luResult != ERESULT_SUCCESS)
			break;

		//设置笔状态
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, mulSelectID);

		break;
	}
	case EEVT_SET_PEN_WIDTH:
	{
		//设置笔宽
		luResult = CExMessage::GetInputData(npMsg, mdwPenWidthIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		//PostMessageToParent(EEVT_SET_PEN_WIDTH, mdwPenWidthIndex);
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_SET_PEN_WIDTH, mdwPenWidthIndex);
		break;
	}
	case EEVT_SET_PEN_COLOR:
	{
		//设置笔颜色
		luResult = CExMessage::GetInputData(npMsg, mdwPenColorIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		//PostMessageToParent(EEVT_SET_PEN_COLOR, mdwPenColorIndex);
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_SET_PEN_COLOR, mdwPenColorIndex);
		break;
	}
	case EEVT_BUTTON_CHECKED:
	{
		if (npMsg->GetMessageSender()->GetID() == ET_BT_HAND) //是否手可画线按钮
		{
			mbIsHand = true;
			CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_HAND_WRITE, mbIsHand);
		}
			
		break;
	}
	case EEVT_BUTTON_UNCHECK:
	{
		if (npMsg->GetMessageSender()->GetID() == ET_BT_HAND) //是否手可画线按钮
		{
			mbIsHand = false;
			CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_HAND_WRITE, mbIsHand);
		}
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
void CEditToolbar::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CEditToolbar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	return ERESULT_SUCCESS;
}

//初始化，默认笔状态
void CEditToolbar::init(void)
{
	//默认是笔写模式
	mulSelectID = PEN_MODE_PEN;
	CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, mulSelectID);
	CExMessage::SendMessage(mpIterGroup, mpIterator, EACT_RBG_SET_SELECT, mulSelectID);
}

//通知元素【显示/隐藏】发生改变
ERESULT CEditToolbar::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_HAND_WRITE_MUTE, mbIsHand);
	}
	else
	{
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_NONE);
		CExMessage::PostMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_HAND_WRITE_MUTE, false);
	}

	return ERESULT_SUCCESS;
}

//任务栏隐藏
void CEditToolbar::ToolbarShow(bool nbIsShow)
{
	if (nbIsShow == false && mpMenuPen != NULL)
		mpMenuPen->ExitModal();
}