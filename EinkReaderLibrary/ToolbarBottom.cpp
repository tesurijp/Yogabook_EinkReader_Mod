/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ToolbarBottom.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "PreNextButton.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(ToolbarBottom)

CToolbarBottom::CToolbarBottom(void)
{
	mpIterBackground = NULL;
	miDocType = DOC_TYPE_PDF;
	mpPageProgress = NULL;
}

CToolbarBottom::~CToolbarBottom(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CToolbarBottom::OnElementCreate(IEinkuiIterator* npIterator)
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


ULONG CToolbarBottom::InitOnCreate(
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

		//移动
		lpSubKey = mpTemplete->OpenKey(L"MoveControlToolbar");
		mpMoveControlToolbar = CMoveControlToolbar::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpMoveControlToolbar);
		mpMoveControlToolbar->GetIterator()->SetVisible(false);

		//获取对像句柄
		mpIterBtPageNumber = mpIterator->GetSubElementByID(TBB_BT_NUMBER);
		BREAK_ON_NULL(mpIterBtPageNumber);

		mpIterBtPre = mpIterator->GetSubElementByID(TBB_BT_PRE);
		BREAK_ON_NULL(mpIterBtPre);

		mpIterBtNext = mpIterator->GetSubElementByID(TBB_BT_NEXT);
		BREAK_ON_NULL(mpIterBtNext);

		mpIterBackground = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterBackground);

		//工具栏
		lpSubKey = mpTemplete->OpenKey(L"ZoomControlToolbar");
		mpZoomControlToolbar = CZoomControlToolbar::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpZoomControlToolbar);
		mpZoomControlToolbar->GetIterator()->SetVisible(false);

		//页面跳转
		lpSubKey = mpTemplete->OpenKey(L"PageProgress");
		mpPageProgress = CPageProgress::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPageProgress);
		mpPageProgress->GetIterator()->SetVisible(false);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//隐藏/显示翻页滚动条
void CToolbarBottom::HidePageProcess(bool nbIsShow)
{
	if (mpPageProgress != NULL)
		mpPageProgress->GetIterator()->SetVisible(nbIsShow);
}

//显示或隐藏元素
void CToolbarBottom::ShowItem(bool nbIsShow)
{
	mpIterator->SetVisible(nbIsShow);
}

//按钮单击事件
ERESULT CToolbarBottom::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		PostMessageToParent(EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case TBB_BT_PRE:
		{
			//上一页
			PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);

			//mpIterator->SetVisible(false);
			break;
		}
		case TBB_BT_NEXT:
		{
			//下一页
			PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);

			//mpIterator->SetVisible(false);
			break;
		}
		case TBB_BT_NUMBER:
		{
			//调整页码
			//PostMessageToParent(EEVT_ER_TWO_SCREEN, true);
			if (mpPageProgress != NULL)
			{
				mpPageProgress->SetData((FLOAT)mulCurrentPage, (FLOAT)mulPageCount);
				mpPageProgress->GetIterator()->SetVisible(!mpPageProgress->GetIterator()->IsVisible());
			}
				

			break;
		}
		case TBB_BT_ZOOM:
		{
			//进入缩放
			ActiveZoom();

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
ERESULT CToolbarBottom::ParseMessage(IEinkuiMessage* npMsg)
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
	case EEVT_ER_ENTER_ZOOM:
	{
		//退出缩放，恢复到100%
		SetZoomStatus(ZoomStatus::NONE);
		break;
	}
	case EEVT_ER_SET_ZOOM:
	{
		//设置放大倍数
		double lfRatio = 1.0;
		luResult = CExMessage::GetInputData(npMsg, lfRatio);
		if (luResult != ERESULT_SUCCESS)
			break;

		RECT ldRect;
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, lfRatio, &ldRect, sizeof(RECT));
		mpMoveControlToolbar->ShowMoveButton(ldRect);

		break;
	}
	case EEVT_ER_AUTO_ZOOM:
	{
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_AUTO_ZOOM, NULL, NULL, NULL);
		break;
	}
	case EEVT_ER_SET_ZOOMLEVEL:
	{
		int scalingLevel = 0;
		luResult = CExMessage::GetInputData(npMsg, scalingLevel);
		if (luResult != ERESULT_SUCCESS)
			break;
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, scalingLevel);
		break;
	}
	case EEVT_ER_PAGE_JUMP:
	{
		//页码跳转
		int liPage = 1;
		luResult = CExMessage::GetInputData(npMsg, liPage);
		if (luResult != ERESULT_SUCCESS)
			break;

		CExMessage::PostMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_PAGE_JUMP, liPage);

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
void CToolbarBottom::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//设置缩放模式
void CToolbarBottom::SetZoomStatus(ZoomStatus status, int scaleLevel /*= 0*/)
{
	if (status == ZoomStatus::NONE)
	{
		mZoomStatus = status;
		mpZoomControlToolbar->initData();

		RECT ldRect;
		double lfRatio = 1.0;
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, lfRatio, &ldRect, sizeof(RECT));
		
		RelocationItem();
		PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);
	}
	else if (miDocType != DOC_TYPE_TXT)
	{
		if (status == ZoomStatus::ZOOM)
			ActiveZoom(scaleLevel);
		else if (status == ZoomStatus::AUTO_ZOOM)
			CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_AUTO_ZOOM, NULL, NULL, NULL);
	}
}

//设置当前打开的文件类型
void CToolbarBottom::SetDoctype(int niType)
{
	miDocType = niType;
	RelocationItem();

	//关闭跳页滚动条
	mpPageProgress->GetIterator()->SetVisible(false);
}

//重新定位元素
void CToolbarBottom::RelocationItem(void)
{
	if (mpIterBackground != NULL)
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		float lfWidth = 0.0f;
		D2D1_POINT_2F ldPos;
		ldPos.y = 10.0f;

		float lfBackHeight = 80.0f;
		mpIterator->SetPosition(0.0f, ldPaintSize.h - lfBackHeight);

		//横屏
		CExMessage::SendMessageWithText(mpIterBackground, mpIterator, EACT_PICTUREFRAME_CHANGE_PIC, L".\\Pic\\toolbar_h.png");
		mpIterBackground->SetSize(float(ldPaintSize.w), lfBackHeight);

		if (miDocType == DOC_TYPE_TXT)
		{
			//如果是txt文件，就只有翻页按钮
			lfWidth = mpIterBtPre->GetSizeX() + mpIterBtNext->GetSizeX() + mpIterBtPageNumber->GetSizeX();
			ldPos.x = (ldPaintSize.w - lfWidth) / 2.0f;

			mpMoveControlToolbar->GetIterator()->SetVisible(false);
			mpZoomControlToolbar->GetIterator()->SetVisible(false);
		}
		else if (miDocType == DOC_TYPE_PDF)
		{
			mpMoveControlToolbar->GetIterator()->SetVisible(true);
			mpZoomControlToolbar->GetIterator()->SetVisible(true);
		}
		else
		{
			mpMoveControlToolbar->GetIterator()->SetVisible(true);
			mpZoomControlToolbar->GetIterator()->SetVisible(true);
		}

		lfWidth =  mpIterBtPre->GetSizeX() + mpIterBtNext->GetSizeX() + mpIterBtPageNumber->GetSizeX();
		ldPos.x = (ldPaintSize.w - lfWidth) / 2.0f;
		ldPos.y = 0.0f;

		if (mpZoomControlToolbar->GetIterator()->IsVisible())
		{
			lfWidth += mpZoomControlToolbar->GetIterator()->GetSizeX();
			ldPos.x = (ldPaintSize.w - lfWidth) / 2.0f;

			if (mpMoveControlToolbar->GetIterator()->IsVisible())
			{
				FLOAT fPosX = mpMoveControlToolbar->GetIterator()->GetPositionX();
				FLOAT fSizeX = mpMoveControlToolbar->GetIterator()->GetSizeX();
				if (ldPos.x < fPosX + fSizeX + 5.0f)
					ldPos.x = fPosX + fSizeX + 5.0f;
			}

			mpZoomControlToolbar->GetIterator()->SetPosition(ldPos);
			ldPos.x += mpZoomControlToolbar->GetIterator()->GetSizeX() + 30.0f;
			ldPos.y = 10.0f;
		}

		mpIterBtPre->SetPosition(ldPos);
		ldPos.x += mpIterBtPre->GetSizeX();
		mpIterBtPageNumber->SetPosition(ldPos);
		ldPos.x += mpIterBtPageNumber->GetSizeX();
		mpIterBtNext->SetPosition(ldPos);

		ldPos.x = mpIterBtPageNumber->GetPositionX() - (mpPageProgress->GetIterator()->GetSizeX() - mpIterBtPageNumber->GetSizeX()) / 2.0f;
		ldPos.y = -80.0f;
		if (ldPos.x + mpPageProgress->GetIterator()->GetSizeX() > ldPaintSize.w)
			ldPos.x = ldPaintSize.w - mpPageProgress->GetIterator()->GetSizeX()-75.0f; //如果超出屏幕就往回来点

		mpPageProgress->GetIterator()->SetPosition(ldPos);
		ldPos.x = mpIterBtPageNumber->GetPositionX() - ldPos.x + mpIterBtPageNumber->GetSizeX()/2.0f;
		ldPos.y = 58.0f;
		mpPageProgress->SetArrowPos(ldPos);
	}
}

//设置滑动方向
void CToolbarBottom::SetMoveForward(MoveForward forward)
{
	mpMoveControlToolbar->SetMoveForward(forward);
}

//按照自适应缩放比例，对应到合适的缩放等级
float CToolbarBottom::AjustAutoZoomLevel(float ratio)
{
	return mpZoomControlToolbar->AjustAutoZoomLevel(ratio);
}

void CToolbarBottom::EnableAutoZoomButton(bool enable)
{
	return mpZoomControlToolbar->EnableAutoZoomButton(enable);
}

void CToolbarBottom::SetRatioString(double ratio)
{
	mpZoomControlToolbar->SetRatioString(ratio);
}

//元素参考尺寸发生变化
ERESULT CToolbarBottom::OnElementResized(D2D1_SIZE_F nNewSize)
{
	RelocationItem();

	return ERESULT_SUCCESS;
}

//设置页码字符串
void CToolbarBottom::SetPage(ULONG nulCurrentPage, ULONG nulPageCount)
{
	do
	{
		wchar_t lszString[MAX_PATH] = { 0 };

		swprintf_s(lszString, MAX_PATH, L"%d/%d", nulCurrentPage, nulPageCount);

		CExMessage::PostMessageWithText(mpIterBtPageNumber, mpIterator, EACT_BUTTON_SETTEXT, lszString);

		mpIterBtPre->SetEnable(nulCurrentPage > 1?true:false);
		mpIterBtNext->SetEnable(nulCurrentPage < nulPageCount ? true : false);

		mulCurrentPage = nulCurrentPage;
		mulPageCount = nulPageCount;

		if (mpPageProgress != NULL)
			mpPageProgress->SetData((FLOAT)mulCurrentPage, (FLOAT)mulPageCount);

	} while (false);
}


//通知元素【显示/隐藏】发生改变
ERESULT CToolbarBottom::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		mpIterator->BringToTop();
	}
	else
	{
		//自己隐藏时把翻页滚动条也隐藏了，下次显示时不显示，需要用户再次点击页码区域才会显示
		mpPageProgress->GetIterator()->SetVisible(false);
	}

	return ERESULT_SUCCESS;
}

void CToolbarBottom::SetFatRatio(float fatRatio)
{
	mpZoomControlToolbar->SetFatRatio(fatRatio);
}

//设置4个移动按钮的状态
void CToolbarBottom::ShowMoveButton(RECT ldRect)
{
	mpMoveControlToolbar->ShowMoveButton(ldRect);
}

// To active Zoom mode. [zhuhl5@20200116:ZoomRecovery]
void CToolbarBottom::ActiveZoom(int scaleLevel /*= 0*/)
{
	mZoomStatus = ZoomStatus::ZOOM;
	SendMessageToParent(EEVT_ER_ENTER_ZOOM, true, NULL, 0);

	mpZoomControlToolbar->EnterZoomStatus(scaleLevel);
	RelocationItem();
}

void CToolbarBottom::SetMoveToolbarVisible(bool visible)
{
	mpMoveControlToolbar->GetIterator()->SetVisible(visible);
}
