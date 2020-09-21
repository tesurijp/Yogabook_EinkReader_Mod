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
	mpPdfPicture = NULL;
	mulPenMode = PEN_MODE_NONE;
	mbIsPressed = false;
	mbIsHand = false;

	miTopHeight = 0;
	miBottomHeight = 0;
	mdwAttrib = 0;
	mbIsFwRect = false;
	ZeroMemory(&mdFwLineRect, sizeof(ED_RECT));
	ZeroMemory(&mdPrePoint, sizeof(mdPrePoint));
	mulPointCount = 0;
	mdwLastPenTicket = 0;
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

		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_TOUCH, this, (PWINMSG_CALLBACK)&CPreNextButton::OnTouchMsg);// OnActivity);

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

//笔写消息

ERESULT __stdcall CPreNextButton::OnTouchMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	ERESULT luResult = ERESULT_WINMSG_SENDTO_NEXT;
	PEI_TOUCHINPUT_POINT lpPoint = (PEI_TOUCHINPUT_POINT)lParam;

	do
	{
		if(mpIterator->IsVisible() == false)
			break; //自己没有显示，就不起作用
		//控制开关Fw画线功能 
		//if (lpPoint->Flags != EI_TOUCHEVENTF_HOVERING_LEAVE)
		//{
		//	//? ? ? ? 临时代码，等待ite增加接口参数后，取消这里的代码
		//	if (mbIsFwRect == false && mulPenMode == PEN_MODE_PEN && mbIsHand == false)
		//	{
		//		mbIsFwRect = true;
		//		ResetFwRect();
		//	}
		//}

		if (lpPoint->PenButton == EI_TOUCH_PEN_BUTTON_ABOVE && mulPenMode != PEN_MODE_SELECT && (lpPoint->Flags == EI_TOUCHEVENTF_DOWN || lpPoint->Flags == EI_TOUCHEVENTF_MOVE))
		{
			if (mulPenMode == PEN_MODE_PEN || mulPenMode == PEN_MODE_ERASER)
			{
				//切换到高亮模式
				ResetFwRect(true);
				SendMessageToParent(EEVT_PEN_MODE, PEN_MODE_SELECT, NULL, 0);

				break;
			}
		}
		//? ? ? ? 临时代码，等待ite增加接口参数后，取消这里的代码
		if (lpPoint->Flags == EI_TOUCHEVENTF_HOVERING_LEAVE && mbIsHand == false)
		{
			//关闭fw画线
			//mbIsFwRect = false;
			//ResetFwRect(true);

		}
		else
		{
			if (mbIsFwRect == false)
			{
				if (mulPenMode == PEN_MODE_PEN && ((mbIsHand != false && lpPoint->FingerOrPen == EI_TOUCH_EVENT_FINGER) || (mbIsHand == false && lpPoint->FingerOrPen == EI_TOUCH_EVENT_PEN)))
				{
					//开启ite 画线
					//mbIsFwRect = true;
					//ResetFwRect();
				}
			}
			else
			{
				if (mulPenMode != PEN_MODE_PEN || (mbIsHand == false && lpPoint->FingerOrPen == EI_TOUCH_EVENT_FINGER) || (mbIsHand != false && lpPoint->FingerOrPen == EI_TOUCH_EVENT_PEN))
				{
					//关闭fw画线
					//mbIsFwRect = false;
					//ResetFwRect(true);
				}
			}
		}

		

		if (lpPoint->FingerOrPen == EI_TOUCH_EVENT_PEN && mulPenMode != PEN_MODE_NONE)
		{
			//如果是笔输入，则只有在非写字状态下才把消息继续传递给其它人
			if (!(lpPoint->x < 0.0f || lpPoint->x >= mpIterator->GetSizeX()
				|| lpPoint->y < (unsigned long)miTopHeight || lpPoint->y >= mpIterator->GetSizeY() - miBottomHeight))
			{
				luResult = ERESULT_SUCCESS;
			}

		}

		if (mulPenMode == PEN_MODE_SELECT)
			luResult = ERESULT_WINMSG_SENDTO_NEXT; //文字选择状态下，消息继续传递

		if ((lpPoint->x < 0.0f || lpPoint->x >= mpIterator->GetSizeX()
			|| lpPoint->y < (unsigned long)miTopHeight || lpPoint->y >= mpIterator->GetSizeY() - miBottomHeight))
		{
			if(mulPenMode != PEN_MODE_PEN || (lpPoint->Flags != EI_TOUCHEVENTF_UP && lpPoint->Flags != EI_TOUCHEVENTF_HOVERING_LEAVE))
				break; //不在自己范围，不处理
		}

		if (lpPoint->FingerOrPen == EI_TOUCH_EVENT_FINGER && mbIsHand == false)
			break; //没有开启手写，就不处理手输入


		if (lpPoint->FingerOrPen == EI_TOUCH_EVENT_PEN && mbIsHand != false)
			break; //手写状态下，不处理笔输入

		
		//if(lpPoint->Flags == EI_TOUCHEVENTF_DOWN) //按下使用send
		//	CExMessage::SendMessage(mpIterator, mpIterator, EEVT_TOUCH_INPUT, (PEI_TOUCHINPUT_POINT)lParam);
		//else 
		if (lpPoint->Flags != EI_TOUCHEVENTF_HOVERING) //hovering不传
			CExMessage::PostMessage(mpIterator, mpIterator, EEVT_TOUCH_INPUT, (PEI_TOUCHINPUT_POINT)lParam, EMSG_POSTTYPE_FAST);

		
		//if (!(lpPoint->x < 0.0f || lpPoint->x >= mpIterator->GetSizeX()
		//	|| lpPoint->y < (unsigned long)miTopHeight || lpPoint->y >= mpIterator->GetSizeY() - miTopHeight - miBottomHeight))
		//{
		//	luResult = ERESULT_SUCCESS; //不再传递
		//}

	} while (false);

	

	return luResult;
}


//设置文件属性
void CPreNextButton::SetFileAttrib(DWORD ndwAttrib)
{
	mdwAttrib = ndwAttrib;
}

// 鼠标落点检测
ERESULT CPreNextButton::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
		|| rPoint.y < miTopHeight || rPoint.y >= mpIterator->GetSizeY() - miBottomHeight))
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
			if (mbIsPressed != false)
			{
				//到这里才算一次Click
				mbIsPressed = false;
				
				if(GetTickCount() - mdwLastPenTicket <= 700)
					break; //说明在写字过程中，不处理手

				if (mZoomStatus == ZoomStatus::NONE)
					ProcessPageNext(npInfo->Position); //翻页处理
				else
					ProcessPageMove(npInfo->Position); //页面移动处理
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

//处理翻页事件
void CPreNextButton::ProcessPageNext(D2D1_POINT_2F ndPos)
{
	do 
	{
		if (abs(mdPressPos.x - ndPos.x) >= 20.0f || abs(mdPressPos.y - ndPos.y) >= 20.0f)
		{
			if (mbIsHand != false)
				break; //如果开启了手画线，就不响应翻页和移动

			//是滑动操作
			if (abs(mdPressPos.x - ndPos.x) >= abs(mdPressPos.y - ndPos.y))	// 过滤掉垂直滑
			{
				if ((mdPressPos.x - ndPos.x) >= 50.0f)
				{
					//后一页
					PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
				}
				else if ((ndPos.x - mdPressPos.x) >= 50.0f)
				{
					//前一页
					PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
				}
			}

		}
		else
		{
			if (mbIsHand != false && (abs(mdPressPos.x - ndPos.x) >= 3.0f || abs(mdPressPos.y - ndPos.y) >= 3.0f))
				break;

			//认为是点击操作
			if ((GetTickCount() - mdwClickTicount) > 500)
			{
				//超过500ms算长按，不处理
				break;
			}

			//判断一下点击区域
			//if (ndPos.x < mfLeftButton)
			//{
			//	//上一页
			//	if (mbIsHand != false)
			//		break; //如果开启了手画线，就不响应翻页和移动

			//	PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
			//}
			//else if (ndPos.x > mpIterBtNext->GetPositionX())
			//{
			//	//下一页
			//	if (mbIsHand != false)
			//		break; //如果开启了手画线，就不响应翻页和移动

			//	PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
			//}
			//else
			{
				//显示或隐藏工具栏
				if ((abs(mdPressPos.x - ndPos.x) >= 12.0f || abs(mdPressPos.y - ndPos.y) >= 12.0f))
					break;

				PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_MIDDLE);
			}

		}
	} while (false);
}

//处理页面移动
void CPreNextButton::ProcessPageMove(D2D1_POINT_2F ndpPos)
{
	do 
	{
		if (mbIsHand != false)
			break; //如果开启了手画线，就不响应翻页和移动

		//鼠标抬起
		POINT ldPos;
		ldPos.x = LONG(ndpPos.x - mdPressPos.x);
		ldPos.y = LONG(ndpPos.y - mdPressPos.y);

		if (abs(ldPos.x) >= 20.0f || abs(ldPos.y) >= 20.0f)
		{
			if (mMoveForward == MoveForward::HORIZONTAL)
				ldPos.y = 0;
			else if (mMoveForward == MoveForward::VERTICAL)
				ldPos.x = 0;

			RECT ldRect;
			CExMessage::SendMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_SET_PAGE_MOVE, ldPos, &ldRect, sizeof(RECT));
		}
		else
		{
			//点击操作，显示或隐藏工具栏
			if (abs(ldPos.x) >= 12.0f || abs(ldPos.y) >= 12.0f)
				break;
			PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_MIDDLE_ZOOM);
		}

	} while (false);
}

//设置上任务栏高度和下任务栏高度
void CPreNextButton::SetToolbarHeight(int niTopHeight, int niBottomHeight)
{
	miTopHeight = niTopHeight;
	miBottomHeight = niBottomHeight;

	ResetFwRect(false);
}

//设置是否是缩放状态,nbIsZoom为真表示当前是缩放状态
void CPreNextButton::SetZoomStatus(ZoomStatus status)
{
	mZoomStatus = status;
}

//设置滑动方向
void CPreNextButton::SetMoveForward(MoveForward forward)
{
	mMoveForward = forward;
}

//设置文档对象指针
void CPreNextButton::SetPdfPicture(CPdfPicture* npPdfPicture, CHighlight* npHighlight)
{
	mpPdfPicture = npPdfPicture;
	mpHighlight = npHighlight;
}

//设置笔状态
void CPreNextButton::SetPenMode(ULONG nulPenMode)
{
	mulPenMode = nulPenMode;

	if (mulPenMode != PEN_MODE_PEN)
	{
		//关闭画线
		//mbIsFwRect = false;
		ResetFwRect(true);
	}
	else
	{
		//开启画线
		ResetFwRect(false);
	}
		
}


//设置画线区域
void CPreNextButton::SetFwLineRect(ED_RECT ndRect)
{
	mdFwLineRect = ndRect;
	ResetFwRect(false);
}

//设置手是否可以用于画线
void CPreNextButton::SetHandWrite(bool nbIsHand)
{
	mbIsHand = nbIsHand;

	//mbIsFwRect = mbIsHand;
	ResetFwRect(false);
}

//获取手写状态
bool CPreNextButton::GetHandWrite(void)
{
	return mbIsHand;
}

//重置FW画线区域
void CPreNextButton::ResetFwRect(bool nbIsClose)
{
	mbIsFwRect = !nbIsClose;

	EI_SIZE ldPaintSize;
	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

	EI_RECT ld;
	SET_TP_AREA ldTpArea;
	ldTpArea.Flag = SET_SP_AREA_NO_REPORT;
	ldTpArea.Index = 0;
	ldTpArea.Rect = { 0,0,0,0 };
	EiSetTpArea(ldTpArea); //先恢复

	if (mbIsHand != false)
	{
		//开启了手写模式，中间区域禁用笔
		if (miTopHeight > 0 && mulPenMode != PEN_MODE_NONE)
		{
			//有任务栏的情况下分三块处理
			ldTpArea.Flag = SET_SP_AREA_TOUCH_PEN; //上任务栏
			ldTpArea.Index = 0;
			ldTpArea.Rect.x = 0;
			ldTpArea.Rect.y = 0;
			ldTpArea.Rect.w = ldPaintSize.w;
			ldTpArea.Rect.h = miTopHeight;
			EiSetTpArea(ldTpArea);

			ldTpArea.Flag = SET_SP_AREA_TOUCH_ONLY; //中间区域
			ldTpArea.Index = 1;
			ldTpArea.Rect.x = 0;
			ldTpArea.Rect.y = miTopHeight;
			ldTpArea.Rect.w = ldPaintSize.w;
			ldTpArea.Rect.h = ldPaintSize.h - miTopHeight - miBottomHeight;
			EiSetTpArea(ldTpArea);

			ldTpArea.Flag = SET_SP_AREA_TOUCH_PEN; //下任务栏
			ldTpArea.Index = 2;
			ldTpArea.Rect.x = 0;
			ldTpArea.Rect.y = ldPaintSize.h - miBottomHeight;
			ldTpArea.Rect.w = ldPaintSize.w;
			ldTpArea.Rect.h = miBottomHeight;
			EiSetTpArea(ldTpArea);
		}
		else
		{
			//没有任务栏，一块就够了
			if(mulPenMode == PEN_MODE_NONE)
				ldTpArea.Flag = SET_SP_AREA_TOUCH_PEN; //非输入状态
			else
				ldTpArea.Flag = SET_SP_AREA_TOUCH_ONLY;

			ldTpArea.Index = 0;
			ldTpArea.Rect = { 0,0,ldPaintSize.w,ldPaintSize.h };
			EiSetTpArea(ldTpArea);
		}
	}
	else
	{
		//没有开启手写
		ldTpArea.Flag = SET_SP_AREA_TOUCH_PEN; //一块就够了
		ldTpArea.Index = 0;
		ldTpArea.Rect = { 0, 0, ldPaintSize.w, ldPaintSize.h};
		EiSetTpArea(ldTpArea);
	}

	if (nbIsClose != false || mulPenMode != PEN_MODE_PEN)
	{
		//关闭fw画线
		ld.x = 0;
		ld.y = 0;
		ld.w = 0;
		ld.h = 0;
	}
	else
	{
		

		ld.x = mdFwLineRect.left;
		ld.y = mdFwLineRect.top<miTopHeight? miTopHeight: mdFwLineRect.top;
		ld.w = mdFwLineRect.right - mdFwLineRect.left;
		ld.h = mdFwLineRect.bottom - ld.y;
		if (ld.h > (ldPaintSize.h - miBottomHeight - miTopHeight))
			ld.h = ldPaintSize.h - miBottomHeight - miTopHeight;


		APITconBoolValues ldValue;
		ldValue.EnableHandInHWDraw = mbIsHand == false ? FALSE : TRUE;
		ldValue.EnableAxisTransformInPenMouse = FALSE;
		ldValue.EnablePressureInHWDraw = FALSE;
		ldValue.EnableUpperButtonDrawInHWDraw = FALSE;
		EiTconBoolSetting(ldValue);

	}

	EiSetHandwritingRect(ld);
}

//输入事件
void CPreNextButton::TouchMsgPro(PEI_TOUCHINPUT_POINT npTouch)
{
	do
	{
		BREAK_ON_NULL(npTouch);

		//if(npTouch->FingerOrPen != 2)
		//	break;
		if (mulPenMode != PEN_MODE_PEN && mulPenMode != PEN_MODE_ERASER && mulPenMode != PEN_MODE_SELECT)
			break; //不是笔输入状态就不处理

		BREAK_ON_NULL(mpPdfPicture);
		BREAK_ON_NULL(mpHighlight);

		//touch输入事件
		ULONG lulPenMode = mulPenMode;
		if (npTouch->PenButton == EI_TOUCH_PEN_BUTTON_BELOW || mdPrePoint.PenButton == EI_TOUCH_PEN_BUTTON_BELOW)
			lulPenMode = PEN_MODE_ERASER; //如果按了下键了就当橡皮处理,FW有bug，按住下键滑动，最后一个抬笔会清掉按键标志，只能自己判断

		mdPrePoint.PenButton = npTouch->PenButton;

		if (lulPenMode == PEN_MODE_SELECT)
		{
			if (npTouch->Flags == EI_TOUCHEVENTF_DOWN)
			{
				//按下
				mpHighlight->TouchDown(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_MOVE)
			{
				//移动
				mpHighlight->TouchMove(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_UP)
			{
				//抬起
				mpHighlight->TouchUp(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_HOVERING_LEAVE)
			{
				//离开
				mpHighlight->PenLeave(lulPenMode);
			}
		}
		else
		{
			if (npTouch->Flags == EI_TOUCHEVENTF_DOWN)
			{
				//按下
				mpPdfPicture->TouchDown(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_MOVE)
			{
				//移动
				mpPdfPicture->TouchMove(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_UP)
			{
				//抬起
				mpPdfPicture->TouchUp(npTouch, lulPenMode, mbIsHand);
			}
			else if (npTouch->Flags == EI_TOUCHEVENTF_HOVERING_LEAVE)
			{
				//离开
				mpPdfPicture->PenLeave(lulPenMode);
			}
		}
		

	} while (false);
}

//消息处理函数
ERESULT CPreNextButton::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EEVT_TOUCH_INPUT:
	{
		if ((mdwAttrib&FILE_ATTRIBUTE_READONLY) != false)
			break; //只读文档

		PEI_TOUCHINPUT_POINT ldInput = *(PEI_TOUCHINPUT_POINT*)npMsg->GetInputData();
		if (ldInput->FingerOrPen == EI_TOUCH_EVENT_PEN && (ldInput->Flags == EI_TOUCHEVENTF_MOVE || ldInput->Flags == EI_TOUCHEVENTF_DOWN || ldInput->Flags == EI_TOUCHEVENTF_UP))
		{
			mdwLastPenTicket = GetTickCount();
		}

		TouchMsgPro(ldInput);

		//if (ldInput->Flags == EI_TOUCHEVENTF_DOWN || ldInput->Flags == EI_TOUCHEVENTF_MOVE)
		//{
		//	OutputDebugString(L"downdowndowndowndowndowndowndowndowndowndowndowndowndowndowndowndowndowndowndowndownvv");
		//}
		//else if (ldInput->Flags == EI_TOUCHEVENTF_UP)
		//{
		//	OutputDebugString(L"upupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupupvv");
		//}

		//if (ldInput->Flags == EI_TOUCHEVENTF_DOWN || ldInput->Flags == EI_TOUCHEVENTF_UP)
		//{
		//	mulPointCount = 0;
		//}
		//else if (ldInput->Flags == EI_TOUCHEVENTF_MOVE)
		//{
		//	mulPointCount++;
		//}

		//if (mulPointCount > 30 && mulPenMode == PEN_MODE_ERASER)
		//{
		//	//如果是橡皮模式，并且一笔的点数量大于指定数量了就插入一个抬笔
		//	EI_TOUCHINPUT_POINT ldNewPoint;
		//	ldNewPoint.x = ldInput->x;
		//	ldNewPoint.y = ldInput->y;
		//	ldNewPoint.z = ldInput->z;
		//	ldNewPoint.FingerOrPen = ldInput->FingerOrPen;
		//	ldNewPoint.PenButton = ldInput->PenButton;
		//	ldNewPoint.Flags = EI_TOUCHEVENTF_UP;
		//	TouchMsgPro(&ldNewPoint);
		//	ldNewPoint.Flags = EI_TOUCHEVENTF_DOWN;
		//	TouchMsgPro(&ldNewPoint);
		//	mulPointCount = 0;
		//}

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

//按钮单击事件
//ERESULT CPreNextButton::OnCtlButtonClick(IEinkuiIterator* npSender)
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	do
//	{
//		ULONG llBtnID = npSender->GetID();
//		switch (llBtnID)
//		{
//		case PNB_BT_PRE:
//		case PNB_BT_MIDDLE:
//		case PNB_BT_NEXT:
//		{
//			//翻页处理
//			PostMessageToParent(EEVT_ER_PRE_NEXT_CLICKED, llBtnID);
//
//			break;
//		}
//		default:
//			break;
//		}
//
//
//		lResult = ERESULT_SUCCESS;
//	} while (false);
//
//	return lResult;
//}

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
		//mbIsFwRect = false;
		//ResetFwRect(false);
	}

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CPreNextButton::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}