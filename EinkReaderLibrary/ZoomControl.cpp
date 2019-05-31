/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ZoomControl.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "..\ECtl\EvButtonImp.h"

// 下面三行，从PreNextButton.h文件中借来
#define PNB_BT_MIDDLE 100  //中间
#define PNB_BT_PRE 101	//上一页
#define PNB_BT_NEXT 102 //下一页

DEFINE_BUILTIN_NAME(ZoomControl)

CZoomControl::CZoomControl(void)
{
	mpIterBtUP = NULL;
	mbIsBeginMove = false;
}


CZoomControl::~CZoomControl(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CZoomControl::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CZoomControl::InitOnCreate(
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
		
		mpIterScrH = mpIterator->GetSubElementByID(105);
		BREAK_ON_NULL(mpIterScrH);
		mpIterScrH->GetSubElementByID(2)->SetEnable(false);

		mpIterScrV = mpIterator->GetSubElementByID(106);
		BREAK_ON_NULL(mpIterScrV);
		mpIterScrV->GetSubElementByID(2)->SetEnable(false);

		CExMessage::SendMessage(mpIterScrH, mpIterator, EACT_SCROLLBAR_HVSCROLL_RELACATION, CExMessage::DataInvalid);
		CExMessage::SendMessage(mpIterScrV, mpIterator, EACT_SCROLLBAR_HVSCROLL_RELACATION, CExMessage::DataInvalid);

		//工具栏
		lpSubKey = mpTemplete->OpenKey(L"ZoomControlToolbar");
		mpZoomControlToolbar = CZoomControlToolbar::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpZoomControlToolbar);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置ScrollBar的位置
bool CZoomControl::SetScrollBarPositionAndSize()
{
	{
		//设置ScrollBar位置
		if (mpIterScrH)
		{
			mpIterScrH->SetPosition((mpIterator->GetSizeX()- mpIterScrH->GetSizeX())/2.0f, mpIterator->GetSizeY() - 20.0f);
			mpIterScrV->SetPosition(mpIterator->GetSizeX() - 20.0f, (mpIterator->GetSizeY() - mpIterScrH->GetSizeY()) / 2.0f);

			CExMessage::SendMessage(mpIterScrH, mpIterator, EACT_SCROLLBAR_HVSCROLL_RELACATION, CExMessage::DataInvalid);
			CExMessage::SendMessage(mpIterScrV, mpIterator, EACT_SCROLLBAR_HVSCROLL_RELACATION, CExMessage::DataInvalid);
		}

	}
	return true;
}

void CZoomControl::SetFatRatio(float fatRatio)
{
	mpZoomControlToolbar->SetFatRatio(fatRatio);
}

//按钮单击事件
ERESULT CZoomControl::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
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
			mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
			mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);

			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//上下左右移动
void CZoomControl::MovePage(ULONG nulID)
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

	CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_PAGE_MOVE, ldPos, &ldRect, sizeof(RECT));
	ShowMoveButton(ldRect);
}

//消息处理函数
ERESULT CZoomControl::ParseMessage(IEinkuiMessage* npMsg)
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
		RECT ldRect;
		float lfRatio = 1.0f;
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, lfRatio, &ldRect, sizeof(RECT));
		ShowMoveButton(ldRect);

		PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);

		break;
	}
	case EEVT_ER_SET_ZOOM:
	{
		//设置放大倍数
		float lfRatio = 1.0f;
		luResult = CExMessage::GetInputData(npMsg, lfRatio);
		if (luResult != ERESULT_SUCCESS)
			break;

		RECT ldRect;
		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_ZOOM, lfRatio,&ldRect, sizeof(RECT));
		ShowMoveButton(ldRect);

		mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);

		break;
	}
	case EEVT_ER_ENTER_SNAPSHOT:
	{
		//需要截屏
		//隐藏自己的控件
		ShowItem(false);
		
		SendMessageToParent(EEVT_ER_ENTER_SNAPSHOT, CExMessage::DataInvalid, NULL, 0);


		//显示自己的控件
		//ShowItem(true);

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
void CZoomControl::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
	if (npStatus->TimerID == ZC_TIMER_ID_HIDE)
	{
		mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		ShowItem(false);
	}
}

//元素参考尺寸发生变化
ERESULT CZoomControl::OnElementResized(D2D1_SIZE_F nNewSize)
{
	if (mpIterBtUP != NULL)
	{
		D2D1_SIZE_F ldSize = mpIterator->GetSize();
		
		D2D1_POINT_2F ldPos;

		//定位
		ldPos.x = 00.0f;
		ldPos.y = (ldSize.height - mpIterBtLeft->GetSizeY()) / 2.0f;
		mpIterBtLeft->SetPosition(ldPos);

		ldPos.x = ldSize.width - mpIterBtRight->GetSizeX();
		mpIterBtRight->SetPosition(ldPos);

		ldPos.y = 00.0f;
		ldPos.x = (ldSize.width - mpIterBtLeft->GetSizeX()) / 2.0f;
		mpIterBtUP->SetPosition(ldPos);

		ldPos.y = ldSize.height - mpIterBtRight->GetSizeY() - 170.0f;
		mpIterBtDown->SetPosition(ldPos);

		ldPos.x = (ldSize.width - mpZoomControlToolbar->GetIterator()->GetSizeX()) / 2.0f;
		ldPos.y = ldSize.height - mpZoomControlToolbar->GetIterator()->GetSizeY() - 80.0f;
		mpZoomControlToolbar->GetIterator()->SetPosition(ldPos);

		if (mpIterScrH)
		{
			mpIterScrH->SetPosition((mpIterator->GetSizeX() - mpIterScrH->GetSizeX()) / 2.0f, mpIterator->GetSizeY() - 20.0f);
			mpIterScrV->SetPosition(mpIterator->GetSizeX() - 20.0f, (mpIterator->GetSizeY() - mpIterScrV->GetSizeY()) / 2.0f);
		}
	}

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CZoomControl::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);
	}
	return ERESULT_SUCCESS;
}


// 鼠标落点检测
ERESULT CZoomControl::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
		|| rPoint.y < 0.0f || rPoint.y >= mpIterator->GetSizeY()))
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}

	return luResult;
}

//鼠标按下
ERESULT CZoomControl::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if (mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if (MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if (npInfo->Presssed == false)
		{
			//鼠标抬起
			POINT ldPos;
			ldPos.x = LONG(npInfo->Position.x - mdPressPos.x);
			ldPos.y = LONG(npInfo->Position.y - mdPressPos.y);

			mbIsBeginMove = false;
			if (abs(mdPressPos.x - npInfo->Position.x) >= 20.0f || abs(mdPressPos.y - npInfo->Position.y) >= 20.0f)
			{
				//是滑动操作
	
				do 
				{
					// 判断是否操作翻页
					if (abs(mdPressPos.x - npInfo->Position.x) >= abs(mdPressPos.y - npInfo->Position.y))	// 要求横向滑动
					{
						if ((mdPressPos.x - npInfo->Position.x) >= 50.0f && mpIterBtRight->IsVisible()==false)
						{
							//后一页
							//PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
							CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
							ldPos.x = 100;
							//break;
						}
						else if ((npInfo->Position.x - mdPressPos.x) >= 50.0f && mpIterBtLeft->IsVisible()==false)
						{
							//前一页
							//PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
							CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
							ldPos.x = 100;
							//break;
						}
					}

					RECT ldRect;
					CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_PAGE_MOVE, ldPos, &ldRect, sizeof(RECT));
					ShowMoveButton(ldRect);

				} while (false);

			}
			else
			{
				//认为是点击操作
				//显示或隐藏自己的按钮
				ShowItem(!mpZoomControlToolbar->GetIterator()->IsVisible());
			}
		}
		else
		{
			//鼠标按下
			mbIsBeginMove = true;
			mdPressPos = npInfo->Position;
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}


//显示或隐藏所有控件
void CZoomControl::ShowItem(bool nbIsShow)
{
	

	if (nbIsShow == false)
	{
		mpIterBtUP->SetVisible(nbIsShow);
		mpIterBtDown->SetVisible(nbIsShow);
		mpIterBtLeft->SetVisible(nbIsShow);
		mpIterBtRight->SetVisible(nbIsShow);
		mpZoomControlToolbar->GetIterator()->SetVisible(nbIsShow);

		mpIterScrH->SetVisible(nbIsShow);
		mpIterScrV->SetVisible(nbIsShow);
	}
	else
	{
		ShowMoveButton(mdMoveBtShow);

		mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);
	}
}

//鼠标移动
ERESULT CZoomControl::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
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

		mpIterator->KillTimer(ZC_TIMER_ID_HIDE);
		mpIterator->SetTimer(ZC_TIMER_ID_HIDE, 1, 1000 * 5, NULL);

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//设置4个移动按钮的状态
void CZoomControl::ShowMoveButton(RECT ldRect)
{
	mdMoveBtShow = ldRect;

	mpIterBtUP->SetVisible(ldRect.top == 0?false:true);
	mpIterBtDown->SetVisible(ldRect.bottom == 0 ? false : true);
	mpIterBtLeft->SetVisible(ldRect.left == 0 ? false : true);
	mpIterBtRight->SetVisible(ldRect.right == 0 ? false : true);

	mpZoomControlToolbar->GetIterator()->SetVisible(true);
	mpIterScrH->SetVisible(true);
	mpIterScrV->SetVisible(true);
}

//设置放大后的图大小及显示区域
void CZoomControl::SetRectOfViewportOnPage(D2D1_SIZE_F& nrImageSize, D2D1_RECT_F& nrViewPort)
{
	if (mpIterScrV != NULL)
	{
		float lfLen = nrImageSize.height - mpIterator->GetSizeY();
		if (lfLen <= 0.0f)
			lfLen = 0.0f;
		CExMessage::SendMessage(mpIterScrV, mpIterator, EACT_SCROLLBAR_HVSCROLL_SET_DELTA_SIZE, lfLen);
		CExMessage::SendMessage(mpIterScrV, mpIterator, EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION, nrViewPort.top);

		lfLen = nrImageSize.width - mpIterator->GetSizeX();
		if (lfLen <= 0.0f)
			lfLen = 0.0f;
		CExMessage::SendMessage(mpIterScrH, mpIterator, EACT_SCROLLBAR_HVSCROLL_SET_DELTA_SIZE, lfLen);
		CExMessage::SendMessage(mpIterScrH, mpIterator, EACT_SCROLLBAR_HSCROLL_SET_SLIDER_POSTION, nrViewPort.left);
	}
}