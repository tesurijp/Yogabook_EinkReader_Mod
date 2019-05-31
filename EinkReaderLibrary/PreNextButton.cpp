/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "PreNextButton.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(PreNextButton)

CPreNextButton::CPreNextButton(void)
{
	mpIterBtPre = NULL;
	mpIterBtMiddle = NULL;
	mpIterBtNext = NULL;
	mfLeftButton = 400.0f;
}


CPreNextButton::~CPreNextButton(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CPreNextButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CPreNextButton::InitOnCreate(
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
		mpIterBtPre = mpIterator->GetSubElementByID(PNB_BT_PRE);
		BREAK_ON_NULL(mpIterBtPre);
		mpIterBtPre->SetEnable(false);

		mpIterBtMiddle = mpIterator->GetSubElementByID(PNB_BT_MIDDLE);
		BREAK_ON_NULL(mpIterBtMiddle);
		mpIterBtMiddle->SetEnable(false);

		mpIterBtNext = mpIterator->GetSubElementByID(PNB_BT_NEXT);
		BREAK_ON_NULL(mpIterBtNext);
		mpIterBtNext->SetEnable(false);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}



// 鼠标落点检测
ERESULT CPreNextButton::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
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
ERESULT CPreNextButton::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if (mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if (mpIterator->IsVisible() == false)
			break;	//如果是隐藏状态，就不接收输入

		if (MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if (npInfo->Presssed == false)
		{
			//鼠标抬起
			if (abs(mdPressPos.x - npInfo->Position.x) >= 20.0f || abs(mdPressPos.y - npInfo->Position.y) >= 20.0f)
			{
				//是滑动操作
				if (abs(mdPressPos.x - npInfo->Position.x) >= abs(mdPressPos.y - npInfo->Position.y))	// 过滤掉垂直滑
				{
					if ((mdPressPos.x - npInfo->Position.x) >= 50.0f)
					{
						//后一页
						PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
					}
					else if ((npInfo->Position.x - mdPressPos.x) >= 50.0f)
					{
						//前一页
						PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
					}
				}
				
			}
			else
			{
				//认为是点击操作
				if ((GetTickCount() - mdwClickTicount) > 500)
				{
					//超过500ms算长按，不处理
					break;
				}

				//判断一下点击区域
				if (npInfo->Position.x < mfLeftButton)
				{
					//上一页
					PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
				}
				else if (npInfo->Position.x > mpIterBtNext->GetPositionX())
				{
					//下一页
					PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
				}
				else
				{
					//显示或隐藏工具栏
					PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_MIDDLE);
				}
				
			}
		}
		else
		{
			//鼠标按下
			mdPressPos = npInfo->Position;
			mdwClickTicount = GetTickCount();
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}


//按钮单击事件
ERESULT CPreNextButton::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case PNB_BT_PRE:
		case PNB_BT_MIDDLE:
		case PNB_BT_NEXT:
		{
			//打开文件对话框
			PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, llBtnID);

			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//
////消息处理函数
//ERESULT CPreNextButton::ParseMessage(IEinkuiMessage* npMsg)
//{
//	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
//
//	switch (npMsg->GetMessageID())
//	{
//	case EMSG_MODAL_ENTER:
//	{
//		//// 创建要弹出的对话框
//		//mpIterator->SetVisible(true);
//		luResult = ERESULT_SUCCESS;
//		break;
//	}
//	default:
//		luResult = ERESULT_NOT_SET;
//		break;
//	}
//
//	if (luResult == ERESULT_NOT_SET)
//	{
//		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
//	}
//
//	return luResult;
//}

//定时器
void CPreNextButton::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CPreNextButton::OnElementResized(D2D1_SIZE_F nNewSize)
{
	if (mpIterBtPre != NULL)
	{
		if (nNewSize.width > nNewSize.height)
		{
			//横屏
			//设置中间按钮有效区域
			float lfBtWidth = nNewSize.width * 0.09f;
			D2D1_SIZE_F ldActionSize;
			ldActionSize.width = nNewSize.width - lfBtWidth*2; //上一页下一页各占400
			ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
			CExMessage::SendMessage(mpIterBtMiddle, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);
			mpIterBtMiddle->SetPosition(lfBtWidth, 0.0f);
			

			//上一页
			ldActionSize.width = lfBtWidth; //上一页下一页各占屏幕的9%
			ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
			CExMessage::SendMessage(mpIterBtPre, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

			//下一页
			CExMessage::SendMessage(mpIterBtNext, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

			float lfX = mpIterator->GetSizeX() - lfBtWidth;
			mpIterBtNext->SetPosition(lfX, mpIterBtNext->GetPositionY());
			
			mfLeftButton = lfBtWidth;
		}
		else
		{
			//坚屏
			//设置中间按钮有效区域
			float lfBtWidth = nNewSize.width * 0.16f;
			D2D1_SIZE_F ldActionSize;
			ldActionSize.width = nNewSize.width - lfBtWidth*2; //上一页下一页各占250
			ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
			CExMessage::SendMessage(mpIterBtMiddle, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);
			mpIterBtMiddle->SetPosition(lfBtWidth, 0.0f);


			//上一页
			ldActionSize.width = lfBtWidth; //上一页下一页各占屏幕的16%
			ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
			CExMessage::SendMessage(mpIterBtPre, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

			//下一页
			CExMessage::SendMessage(mpIterBtNext, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

			float lfX = mpIterator->GetSizeX() - lfBtWidth;
			mpIterBtNext->SetPosition(lfX, mpIterBtNext->GetPositionY());
			mfLeftButton = lfBtWidth;
		}
	}

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CPreNextButton::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}