/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "Highlight.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(Highlight)

CHighlight::CHighlight(void)
{
	mpPdfPicture = NULL;
	mbIsPressed = false;
	mbIsTouchDown = false;

	miTopHeight = 0;
	miBottomHeight = 0;
	mpHighlightToolbar = NULL;
	mpIterBtBottom = NULL;
	mpIterBtTop = NULL;
	mbModifyStatus = false;
}


CHighlight::~CHighlight(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CHighlight::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_TOUCH, this, (PWINMSG_CALLBACK)&CHighlight::OnTouchMsg);// OnActivity);

		mpIterator->ModifyStyles(EITR_STYLE_TOP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CHighlight::InitOnCreate(
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

		mpIterBtTop = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterBtTop);
		LONG llStyle = ES_SLIDER_BUTTON_STYLE_ANYWAY;
		CExMessage::SendMessage(mpIterBtTop, mpIterator, EACT_SLIDERBUTTON_SET_STYLE, llStyle);
		mpIterBtTop->SetVisible(false);

		mpIterBtBottom = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterBtBottom);
		CExMessage::SendMessage(mpIterBtBottom, mpIterator, EACT_SLIDERBUTTON_SET_STYLE, llStyle);
		mpIterBtBottom->SetVisible(false);

		lpSubKey = mpTemplete->OpenKey(L"HighlightToolbar");
		mpHighlightToolbar = CHighlightToolbar::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpHighlightToolbar);
		mpHighlightToolbar->GetIterator()->SetVisible(false);
		//mpHighlightToolbar->GetIterator()->SetPosition(100.0f, 100.0f);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

// 鼠标落点检测
ERESULT CHighlight::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (mbModifyStatus != false &&(
		!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
				|| rPoint.y < miTopHeight || rPoint.y >= mpIterator->GetSizeY() - miBottomHeight))
		)
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}

	return luResult;
}



//处理移动完成事件
void CHighlight::DragEndPro(D2D1_POINT_2F ndTop, D2D1_POINT_2F ndBottom)
{
	do 
	{
		D2D1_RECT_F ldRect = { 0.0f,0.0f,0.0f,0.0f };
		if (fabs(ndTop.x-ndBottom.x) < 3.0f && fabs(ndTop.y - ndBottom.y) < 3.0f)
		{
			mdHighlightStruct.QuadCount = 0;
		}
		else
		{
			ldRect.left = ndTop.x;
			ldRect.top = ndTop.y;
			ldRect.right = ndBottom.x;
			ldRect.bottom = ndBottom.y;
			
			/*if (ndTop.x < ndBottom.x)
			{
				ldRect.left = ndTop.x;
				ldRect.right = ndBottom.x;
			}
			else
			{
				ldRect.left = ndBottom.x;
				ldRect.right = ndTop.x;
			}

			if (ndTop.y < ndBottom.y)
			{
				ldRect.top = ndTop.y;
				ldRect.bottom = ndBottom.y;
			}
			else
			{
				ldRect.top = ndBottom.y;
				ldRect.bottom = ndTop.y;
			}*/

			/*if (ldRect.bottom - ldRect.top < 1.0f)
				ldRect.bottom = ldRect.top + 1.0f;*/

		}
		SendMessageToParent(EEVT_SELECT_HIGHLIGHT, ldRect, &mdHighlightStruct, sizeof(mdHighlightStruct));
		mpHighlightToolbar->SetHighlightCount(mdHighlightStruct.AnnotCount);

		if (mdHighlightStruct.QuadCount > 0)
		{
			mpHighlightToolbar->GetIterator()->SetPosition(mdHighlightStruct.PointA.x, mdHighlightStruct.PointA.y - 100.0f);
			mpIterBtTop->SetPosition(mdHighlightStruct.PointA.x - mpIterBtTop->GetSizeX(), mdHighlightStruct.PointA.y);
			mpIterBtBottom->SetPosition(mdHighlightStruct.PointB);

			mpHighlightToolbar->GetIterator()->SetVisible(true);
			mpIterBtTop->SetVisible(true);
			mpIterBtBottom->SetVisible(true);

			Relocation();

			//进入修改状态
			mbModifyStatus = true;

			ULONG lulMode = PEN_MODE_NONE;
			SendMessageToParent(EEVT_PEN_MODE, lulMode, NULL, 0);
		}
		else
		{
			HideSelect();
		}

	} while (false);
}

//隐藏选中
void CHighlight::HideSelect(void)
{
	if (mpIterBtTop != NULL)
	{
		mbModifyStatus = false;
		mpIterBtTop->SetVisible(false);
		mpIterBtBottom->SetVisible(false);

		mpHighlightToolbar->GetIterator()->SetVisible(false);

		D2D1_RECT_F ldRect = { 0.0f,0.0f,0.0f,0.0f };
		SendMessageToParent(EEVT_SELECT_HIGHLIGHT, ldRect, &mdHighlightStruct, sizeof(mdHighlightStruct));

		if (mpIterator->IsVisible() != false)
		{
			ULONG lulMode = PEN_MODE_SELECT;
			SendMessageToParent(EEVT_PEN_MODE, lulMode, NULL, 0);
		}
	}
}



//输入事件
void CHighlight::TouchDown(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand)
{
	do
	{
		mpPdfPicture->SvcDebugOutFmt("highlight touch down");
		EinkuiGetSystem()->EnablePaintboard(true); //笔迹输入时禁用刷新

		mdPressPos = { (FLOAT)npPoint->x,(FLOAT)npPoint->y };
		mdwClickTicount = GetTickCount();

		mbIsTouchDown = true;

	} while (false);
}


void CHighlight::TouchMove(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand)
{
	if (mbIsTouchDown == false)
	{
		mbIsTouchDown = true;
		mdPressPos = { (FLOAT)npPoint->x,(FLOAT)npPoint->y };
		mdwClickTicount = GetTickCount();
	}

}

void  CHighlight::PenLeave(ULONG nulPenMode)
{

}

void CHighlight::TouchUp(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand)
{
	try
	{
		EinkuiGetSystem()->EnablePaintboard(false); //笔迹输入时禁用刷新

		do
		{
			if(mbModifyStatus != false)
				break;

			mpPdfPicture->SvcDebugOutFmt("highlight touch up");
			
			D2D1_POINT_2F ldEndPos = { (FLOAT)npPoint->x,(FLOAT)npPoint->y };
			DragEndPro(mdPressPos, ldEndPos);

		} while (false);
	}
	catch (...)
	{
		mpPdfPicture->SvcDebugOutFmt("highlight touch up try");
	}

	mbIsTouchDown = false;

	mpPdfPicture->SvcDebugOutFmt("highlight touch up end");
}

//
////鼠标移动
//ERESULT CHighlight::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	do
//	{
//		BREAK_ON_NULL(npInfo);
//
//		if (MOUSE_LB(npInfo->KeyFlag) == false)  //如果不是鼠标左键就不处理
//			break;
//
//		if (mbIsPressed == false)
//		{
//			mbIsPressed = true;
//			mdPressPos = npInfo->Position;
//			mdwClickTicount = GetTickCount();
//		}
//
//		lResult = ERESULT_SUCCESS;
//
//	} while (false);
//
//	return lResult;
//}
//
//鼠标按下
ERESULT CHighlight::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
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
			if (mbIsPressed != false)
			{
				//到这里才算一次Click
				mbIsPressed = false;
				
				//取消选中状态
				D2D1_POINT_2F ldA = { 0.0f,0.0f };
				DragEndPro(ldA, ldA);
			}
		}
		else
		{
			//鼠标按下
			mdPressPos = npInfo->Position;
			mdwClickTicount = GetTickCount();

			//鼠标按下
			if (mbIsPressed == false)
			{
				mbIsPressed = true;
			}
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//重新定位
void CHighlight::Relocation(void)
{
	do 
	{
		BREAK_ON_NULL(mpHighlightToolbar);

		IEinkuiIterator* lpToolbar = mpHighlightToolbar->GetIterator();
		if(lpToolbar->IsVisible() == false)
			break; //没有显示

		SendMessageToParent(EEVT_GET_SELECT_HIGHLIGHT_INFO, CExMessage::DataInvalid, &mdHighlightStruct, sizeof(mdHighlightStruct));
		mpHighlightToolbar->GetIterator()->SetPosition(mdHighlightStruct.PointA.x, mdHighlightStruct.PointA.y - 100.0f);

		D2D1_POINT_2F ldOldPos = lpToolbar->GetPosition();
		D2D1_POINT_2F ldPos = ldOldPos;
		if (ldPos.x < 10.0f)
			ldPos.x = 10.0f;
		if (ldPos.y < 10.0f)
			ldPos.y = 10.0f;
		if (ldPos.x + lpToolbar->GetSizeX() + 10.0f > mpIterator->GetSizeX())
			ldPos.x = mpIterator->GetSizeX() - lpToolbar->GetSizeX() - 10.0f;
		if (ldPos.y + lpToolbar->GetSizeY() + 10.0f > mpIterator->GetSizeY())
			ldPos.y = mpIterator->GetSizeY() - lpToolbar->GetSizeY() - 10.0f;

		if (ldPos.x != ldOldPos.x || ldPos.y != ldOldPos.y)
			lpToolbar->SetPosition(ldPos);

		mpIterBtTop->SetPosition(mdHighlightStruct.PointA.x - mpIterBtTop->GetSizeX(), mdHighlightStruct.PointA.y);
		mpIterBtBottom->SetPosition(mdHighlightStruct.PointB);
		 
	} while (false);
}

//设置上任务栏高度和下任务栏高度
void CHighlight::SetToolbarHeight(int niTopHeight, int niBottomHeight)
{
	miTopHeight = niTopHeight;
	miBottomHeight = niBottomHeight;
}

//设置文档对象指针
void CHighlight::SetPdfPicture(CPdfPicture* npPdfPicture)
{
	mpPdfPicture = npPdfPicture;
}

//消息处理函数
ERESULT CHighlight::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EEVT_TOUCH_INPUT:
	{
		

		luResult = ERESULT_SUCCESS;

		break;
	}
	case EEVT_HIGHLIGHT_BT_EVENT:
	{
		//工具栏按钮
		int liIndex = 0;
		luResult = CExMessage::GetInputData(npMsg, liIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_HIGHLIGHT_BT_EVENT, liIndex);

		HideSelect();

		break;
	}
	case EACT_SLIDERBUTTON_DRAG_END:
	{
		//按钮拖动结束
		D2D1_POINT_2F ldPosTop = mpIterBtTop->GetPosition();
		ldPosTop.x += mpIterBtTop->GetSizeX();
		D2D1_POINT_2F ldPosBottom = mpIterBtBottom->GetPosition();


		DragEndPro(ldPosTop, ldPosBottom);

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

//按钮单击事件
ERESULT CHighlight::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//ULONG llBtnID = npSender->GetID();
		//switch (llBtnID)
		//{
		//case H_BT_COPY:
		//{
		//	//复制到剪切版本
		//	

		//	break;
		//}
		//default:
		//	break;
		//}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//定时器
void CHighlight::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CHighlight::OnElementResized(D2D1_SIZE_F nNewSize)
{


	if (mpIterBtTop != NULL)
	{
		D2D1_RECT_F lSliderRect;
		lSliderRect.left = 0.0f;
		lSliderRect.top = 0.0f;
		lSliderRect.right = nNewSize.width;
		lSliderRect.bottom = nNewSize.height;
		CExMessage::PostMessage(mpIterBtTop, mpIterator, EACT_SLIDERBUTTON_SET_SLIDERRECT, lSliderRect);
		CExMessage::PostMessage(mpIterBtBottom, mpIterator, EACT_SLIDERBUTTON_SET_SLIDERRECT, lSliderRect);
	}

	Relocation();

	//if (mpIterBtPre != NULL)
	//{
	//	if (nNewSize.width > nNewSize.height)
	//	{
	//		//横屏
	//		//设置中间按钮有效区域
	//		float lfBtWidth = nNewSize.width * 0.09f;
	//		D2D1_SIZE_F ldActionSize;
	//		ldActionSize.width = nNewSize.width - lfBtWidth*2; //上一页下一页各占400
	//		ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
	//		CExMessage::SendMessage(mpIterBtMiddle, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);
	//		mpIterBtMiddle->SetPosition(lfBtWidth, 0.0f);
	//		

	//		//上一页
	//		ldActionSize.width = lfBtWidth; //上一页下一页各占屏幕的9%
	//		ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
	//		CExMessage::SendMessage(mpIterBtPre, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

	//		//下一页
	//		CExMessage::SendMessage(mpIterBtNext, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

	//		float lfX = mpIterator->GetSizeX() - lfBtWidth;
	//		mpIterBtNext->SetPosition(lfX, mpIterBtNext->GetPositionY());
	//		
	//		mfLeftButton = lfBtWidth;
	//	}
	//	else
	//	{
	//		//坚屏
	//		//设置中间按钮有效区域
	//		float lfBtWidth = nNewSize.width * 0.16f;
	//		D2D1_SIZE_F ldActionSize;
	//		ldActionSize.width = nNewSize.width - lfBtWidth*2; //上一页下一页各占250
	//		ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
	//		CExMessage::SendMessage(mpIterBtMiddle, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);
	//		mpIterBtMiddle->SetPosition(lfBtWidth, 0.0f);


	//		//上一页
	//		ldActionSize.width = lfBtWidth; //上一页下一页各占屏幕的16%
	//		ldActionSize.height = nNewSize.height - mpIterator->GetPositionY();
	//		CExMessage::SendMessage(mpIterBtPre, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

	//		//下一页
	//		CExMessage::SendMessage(mpIterBtNext, mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldActionSize);

	//		float lfX = mpIterator->GetSizeX() - lfBtWidth;
	//		mpIterBtNext->SetPosition(lfX, mpIterBtNext->GetPositionY());
	//		mfLeftButton = lfBtWidth;
	//	}
	//	//mbIsFwRect = false;
	//	ResetFwRect(true);
	//}

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CHighlight::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow == false)
	{
		HideSelect();
	}
	return ERESULT_SUCCESS;
}