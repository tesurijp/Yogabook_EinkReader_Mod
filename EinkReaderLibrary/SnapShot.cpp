/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "SnapShot.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(SnapShot)

CSnapShot::CSnapShot(void)
{
}


CSnapShot::~CSnapShot(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CSnapShot::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//CExMessage::PostMessage(mpIterSelectFrame, mpIterator, EMSG_SET_EDIT_STATUS, false);
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CSnapShot::InitOnCreate(
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
		mpIterSelectFrame = mpIterator->GetSubElementByID(104);
		BREAK_ON_NULL(mpIterSelectFrame);
		

		mpIterBtSnap = mpIterator->GetSubElementByID(SS_BT_SNAP);
		BREAK_ON_NULL(mpIterBtSnap);

		mpIterBtReturn = mpIterator->GetSubElementByID(SS_BT_RETURN);
		BREAK_ON_NULL(mpIterBtReturn);


		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//按钮单击事件
ERESULT CSnapShot::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case SS_BT_SNAP:
		{
			//截屏
			D2D1_POINT_2F ldPos = mpIterSelectFrame->GetPosition();
			D2D1_SIZE_F ldSize = mpIterSelectFrame->GetSize();
			D2D1_RECT_F ldRect;
			ldRect.left = ldPos.x<1.0f?1.0f:ldPos.x;
			ldRect.top = ldPos.y<1.0f?1.0f:ldPos.y;
			ldRect.right = ldPos.x + ldSize.width;
			ldRect.bottom = ldPos.y + ldSize.height;

			EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			if (ldPaintSize.w > ldPaintSize.h)
			{
				//横屏
				if (ldRect.right >= 1920.0f)
					ldRect.right = 1919.0f;
				if (ldRect.bottom >= 1080.0f)
					ldRect.bottom = 1079.0f;
			}
			else
			{
				//坚屏
				if (ldRect.right >= 1080.0f)
					ldRect.right = 1079.0f;
				if (ldRect.bottom >= 1920.0f)
					ldRect.bottom = 1919.0f;
			}
			
			PostMessageToParent(EEVT_ER_SNAPSHOT_TO_CLIPBRD, ldRect);

			ExitModal();

			break;
		}
		case SS_BT_RETURN:
		{
			//退出
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
ERESULT CSnapShot::ParseMessage(IEinkuiMessage* npMsg)
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
	case EMSG_SELECTFRAME_BEGIN:
	case EMSG_SELECTPOINT_BEGIN:
	{
		//选择框开始移动
		mdBeginSize = mpIterSelectFrame->GetSize();
		mdDropBeginPos = mpIterSelectFrame->GetPosition();

		mpIterBtSnap->SetVisible(false);
		mpIterBtReturn->SetVisible(false);

		break;
	}
	case EMSG_SELECTPOINT_CHANGE_POSITION_SIZE:
	{
		//选择框变化
		STCTL_CHANGE_POSITION_SIZE ldPositionSize;
		luResult = CExMessage::GetInputData(npMsg, ldPositionSize);
		if (luResult != ERESULT_SUCCESS)
			break;

		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
		D2D1_POINT_2F ldNewPos;
		ldNewPos.x = mdDropBeginPos.x + ldPositionSize.mdPositionOffset.x;
		ldNewPos.y = mdDropBeginPos.y + ldPositionSize.mdPositionOffset.y;
		if (ldNewPos.x <= 0.0f)
			ldNewPos.x = 0.0f;
		if (ldNewPos.y <= 0.0f)
			ldNewPos.y = 0.0f;
		if (ldNewPos.x + mpIterSelectFrame->GetSizeX() > ldPaintSize.w)
			ldNewPos.x = ldPaintSize.w - mpIterSelectFrame->GetSizeX();
		if (ldNewPos.y + mpIterSelectFrame->GetSizeY() > ldPaintSize.h)
			ldNewPos.y = ldPaintSize.h - mpIterSelectFrame->GetSizeY();
		mpIterSelectFrame->SetPosition(ldNewPos.x, ldNewPos.y);

		mpIterSelectFrame->SetSize(mdBeginSize.width + ldPositionSize.mdSizeVariation.width,
			mdBeginSize.height + ldPositionSize.mdSizeVariation.height);

		break;
	}
	case EMSG_SELECTFRAME_MOVING:
	{
		//选择框自己移动
		D2D1_POINT_2F ldPositionSize;
		luResult = CExMessage::GetInputData(npMsg, ldPositionSize);
		if (luResult != ERESULT_SUCCESS)
			break;

		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
		D2D1_POINT_2F ldNewPos;
		ldNewPos.x = mdDropBeginPos.x + ldPositionSize.x;
		ldNewPos.y = mdDropBeginPos.y + ldPositionSize.y;
		if (ldNewPos.x <= 0.0f)
			ldNewPos.x = 0.0f;
		if (ldNewPos.y <= 0.0f)
			ldNewPos.y = 0.0f;
		if (ldNewPos.x + mpIterSelectFrame->GetSizeX() > ldPaintSize.w)
			ldNewPos.x = ldPaintSize.w - mpIterSelectFrame->GetSizeX();
		if (ldNewPos.y + mpIterSelectFrame->GetSizeY() > ldPaintSize.h)
			ldNewPos.y = ldPaintSize.h - mpIterSelectFrame->GetSizeY();

		mpIterSelectFrame->SetPosition(ldNewPos.x,ldNewPos.y);

		break;
	}
	case EMSG_SELECTFPOINT_MOVED:
	case EMSG_SELECTFRAME_DRAGED:
	{
		//拖动结束
		SetButtonPos();

		mpIterBtSnap->SetVisible(true);
		mpIterBtReturn->SetVisible(true);

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
void CSnapShot::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CSnapShot::OnElementResized(D2D1_SIZE_F nNewSize)
{
	InitSnapRect(nNewSize);
	//if (mpIterBtRight != NULL)
	//{
	//	D2D1_SIZE_F ldSize = mpIterator->GetSize();
	//	
	//	D2D1_POINT_2F ldPos;

	//	//定位
	//	ldPos.x = 60.0f;
	//	ldPos.y = (ldSize.height - mpIterBtLeft->GetSizeY()) / 2.0f;
	//	mpIterBtLeft->SetPosition(ldPos);

	//	ldPos.x = ldSize.width - mpIterBtRight->GetSizeX() - 60.0f;
	//	mpIterBtRight->SetPosition(ldPos);

	//	ldPos.y = 60.0f;
	//	ldPos.x = (ldSize.width - mpIterBtLeft->GetSizeX()) / 2.0f;

	//	ldPos.y = ldSize.height - mpIterBtRight->GetSizeY() - 220.0f;
	//	mpIterBtDown->SetPosition(ldPos);

	//}

	return ERESULT_SUCCESS;
}

//初始化截图区域
void CSnapShot::InitSnapRect(D2D1_SIZE_F ndParentSize)
{
	if (mpIterSelectFrame != NULL)
	{
		mpIterSelectFrame->SetSize(ndParentSize.width, ndParentSize.height - mnToolBarBottomHeight - mnToolBarTopHeight);
		mpIterSelectFrame->SetPosition(0, mnToolBarTopHeight);
		SetButtonPos();
	}
}

//设置两个按钮位置
void CSnapShot::SetButtonPos()
{
	if (mpIterSelectFrame != NULL)
	{
		D2D1_POINT_2F ldCurrentPos = mpIterSelectFrame->GetPosition();
		D2D1_POINT_2F ldPos;
		ldPos.x = ldCurrentPos.x + mpIterSelectFrame->GetSizeX() - mpIterBtSnap->GetSizeX() * 2;
		ldPos.y = ldCurrentPos.y + mpIterSelectFrame->GetSizeY() + 20.0f;
		if (ldPos.y + mpIterBtSnap->GetSizeY() > mpIterator->GetSizeY())
		{
			//超出屏幕范围了，那就放到最边缘的位置
			ldPos.y = mpIterator->GetSizeY() - mpIterBtSnap->GetSizeY();
			ldPos.x -= 50.0f;
		}

		mpIterBtSnap->SetPosition(ldPos);
		ldPos.x += mpIterBtSnap->GetSizeX();
		mpIterBtReturn->SetPosition(ldPos);
	}
	
}

//通知元素【显示/隐藏】发生改变
ERESULT CSnapShot::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}

void CSnapShot::DoModal()
{
	do
	{
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->EnablePaintboard(false);
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);


		mpIterator->Close();
		//mpIterator->Release();

	} while (false);
}

void CSnapShot::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);

}


void CSnapShot::SetToolBarHeight(int nTopH, int nBottomH)
{
	mnToolBarTopHeight = nTopH;
	mnToolBarBottomHeight = nBottomH;
}