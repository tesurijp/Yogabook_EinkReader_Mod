/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"

#include "xSelectPoint.h"
#include "xSelectFrame.h"
#include "xSelectFrameMacro.h"

DEFINE_BUILTIN_NAME(System_SelectFrame)

CSelectFrame::CSelectFrame(void)
{
	for (int i=0;i<SF_POINT_MAX;i++)
		mpArrayPoint[i] = NULL;

	mhCursorNesw = NULL;
	mhCursorNwse = NULL;
	mpXuiBrush = NULL;
	mbModifying = false;

	mcProportionalScaling = false;
	mcProportionalScalingByKey = false;

	mcLastHTurn = false;
	mcLastVTurn = false;

	// 默认允许编辑
	mcIsEdit = true;

}


CSelectFrame::~CSelectFrame(void)
{
	CMM_SAFE_RELEASE(mpXuiBrush);
}


ULONG CSelectFrame::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID				// 如果不为0，则指定该元素的EID，否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动那个分配
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

		for (int liID=0;liID<SF_POINT_MAX;liID++)
		{

			CSelectPoint* lpPoint = CSelectPoint::CreateInstance(mpIterator,npTemplete,liID+1);
			BREAK_ON_NULL(lpPoint);

			mpArrayPoint[liID] = lpPoint->GetIterator();
			if (liID == 1 || liID == 3 || liID == 5 || liID == 7)
				lpPoint->GetIterator()->SetVisible(false);
		}

		// 创建画刷
		mpXuiBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush, D2D1::ColorF(0.0f,0.0f,0.0f));
		BREAK_ON_NULL(mpXuiBrush);

		mpXuiBrush->SetStrokeWidth(1.0f);
		mpXuiBrush->SetStrokeType(
			D2D1::StrokeStyleProperties(
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_ROUND,
			D2D1_LINE_JOIN_MITER,
			10.0f,
			D2D1_DASH_STYLE_DASH,//D2D1_DASH_STYLE_DASH_DOT_DOT,
			0.0f),
			0,
			NULL);

		mhCursorNwse = LoadCursor(NULL,IDC_SIZENWSE);
		mhCursorNesw = LoadCursor(NULL,IDC_SIZENESW);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}


//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CSelectFrame::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpToolTipKey = NULL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//修改鼠标移动要自己身上时的鼠标样式
		CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_SIZEALL);
		mpIterator->ModifyStyles(EITR_STYLE_ALL_DRAG|EITR_STYLE_ALL_KEY);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


//分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
//本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CSelectFrame::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_SELECTPOINT_BEGIN:
		{
			if (mcIsEdit != false)
			{
				SetPointVisible(false);

				mdOriginPosition = mpIterator->GetPosition();
				mdOriginSize = mpIterator->GetSize();

				//判断按键信息
				if(npMsg->GetInputDataSize() != sizeof(STMS_DRAGGING_ELE))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				STMS_DRAGGING_ELE* lpInfo = (STMS_DRAGGING_ELE*)npMsg->GetInputData();
				if ((lpInfo->KeyFlag & MK_SHIFT) != 0)
					mcProportionalScalingByKey = true;

				// 为父窗口发送开始拖动的消息，不携带任何数据
				SendMessageToParent(EMSG_SELECTPOINT_BEGIN, CExMessage::DataInvalid, NULL, 0);
			}
			else
			{
				SendMessageToParent(EMSG_SELECTFRAME_BEGIN, CExMessage::DataInvalid, NULL, 0);
			}

			break;
		}
	case EMSG_SELECTPOINT_MOVING:
		{
			// 移动某个点，带动选择框进行缩放翻转
			if(npMsg->GetInputDataSize() != sizeof(D2D1_SIZE_F))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			D2D1_SIZE_F* lpInfo = ((D2D1_SIZE_F*)npMsg->GetInputData());
			if (mcIsEdit != false)
			{
				luResult = OnPointDrag(npMsg->GetMessageSender(),lpInfo);
			}
			else
			{
				// 不允许编辑，则所有锚点都处于移动状态	
				SendMessageToParent(EMSG_SELECTFRAME_MOVING, *lpInfo,NULL,0);			
			}
			break;
		}
	case EMSG_SELECTFPOINT_MOVED:
		{
			// 每完成一次拖动，重置该标志
			mcProportionalScaling = false;
			mcProportionalScalingByKey = false;

			// 重置翻转状态
			mcLastHTurn = false;
			mcLastVTurn = false;

			SetPointVisible(true);
			mpIterator->SetVisible(true);

			Relocation();

			if (mcIsEdit != false)
			{
				// 为父窗口发送拖动结束的消息，不携带任何数据
				SendMessageToParent(EMSG_SELECTFPOINT_MOVED, CExMessage::DataInvalid, NULL, 0);
			}
			else
			{
				SendMessageToParent(EMSG_SELECTFRAME_DRAGED, CExMessage::DataInvalid, NULL, 0);
			}
			break;
		}
		// 处理归一化命令
	case EMSG_SELECTFRAME_REGULAR:
		{
			// 等待父窗口的命令，如果是任意比缩放，才重置锚点
			//if (mcProportionalScaling == false)
			//	EinkuiGetSystem()->GetElementManager()->ResetDragBegin(mpArrayPoint[miActivePoint-1]);
			break;
		}
	case EMSG_SET_PROPORTIONAL_SCALING:
		{
			// 是否执行等比例缩放
			if(npMsg->GetInputDataSize() != sizeof(bool))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			bool* lpInfo = ((bool*)npMsg->GetInputData());
			mcProportionalScaling ^= *lpInfo;
			break;
		}
	case EMSG_SET_EDIT_STATUS:
		{
			if(npMsg->GetInputDataSize() != sizeof(bool))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			bool* lpInfo = ((bool*)npMsg->GetInputData());
			mcIsEdit = *lpInfo;
		}
		break;
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//键盘消息
ERESULT CSelectFrame::OnKeyPressed(const STEMS_KEY_PRESSED* npInfo)
{
	ERESULT luResult;

	luResult = SendMessageToParent(EMSG_SELECTFRAME_KEYBOARD,*npInfo,NULL,0);
	if(luResult == ERESULT_KEY_ACCEPTED)
		return luResult;

	if(false != npInfo->IsPressDown)		// 只处理按下
	{
		D2D1_POINT_2F ldOffset = {0.0f,0.0f};

		luResult = ERESULT_SUCCESS;

		switch (npInfo->VirtualKeyCode)
		{
		case VK_LEFT:
				ldOffset.x = -1.0f;
			break;
		case VK_RIGHT:
				ldOffset.x = 1.0f;
			break;
		case VK_UP:
				ldOffset.y = -1.0f;
			break;
		case VK_DOWN:
				ldOffset.y = 1.0f;
			break;
		default:
			luResult = ERESULT_KEY_UNEXPECTED;
		}

		if(luResult == ERESULT_SUCCESS)
		{
			SendMessageToParent(EMSG_SELECTFRAME_BEGIN, CExMessage::DataInvalid, NULL, 0);
			SendMessageToParent(EMSG_SELECTFRAME_MOVING,ldOffset,NULL,0);
			SendMessageToParent(EMSG_SELECTFRAME_DRAGED, CExMessage::DataInvalid, NULL, 0);
		}
	}
	else
		luResult = ERESULT_KEY_UNEXPECTED;

	return luResult;
}


//元素参考尺寸发生变化
ERESULT CSelectFrame::OnElementResized(D2D1_SIZE_F nNewSize)
{
	Relocation();

	return ERESULT_SUCCESS;
}


// 设置对应点的鼠标状态
void CSelectFrame::SetCursor(D2D1_POINT_2F ndPoint, D2D1_RECT_F ndNormalRect, int niIndex)
{
	float lfHalfWidth = mpArrayPoint[0]->GetSizeX()/2;
	float lfHalfHeight = mpArrayPoint[0]->GetSizeY()/2;
	float lfWidth = mpArrayPoint[0]->GetSizeX();
	float lfHeight = mpArrayPoint[0]->GetSizeY();

	HCURSOR lhCursorNwse = LoadCursor(NULL,IDC_SIZENWSE);
	HCURSOR lhCursorNesw = LoadCursor(NULL,IDC_SIZENESW);

	// 是否是1，5点
	if (ndPoint.x == (ndNormalRect.left-lfHalfWidth) && ndPoint.y == (ndNormalRect.top-lfHalfHeight) ||
		ndPoint.x == (ndNormalRect.right - lfHalfWidth) && ndPoint.y ==(ndNormalRect.bottom - lfHalfHeight)
		)
	{
		CExMessage::SendMessage(mpArrayPoint[niIndex],mpIterator,EMSG_SELECTPOINT_RESET_CURSOR,lhCursorNwse);
	}

	// 是否是3，7点
	if (ndPoint.x == ndNormalRect.right -lfHalfWidth && ndPoint.y == (ndNormalRect.top-lfHalfHeight) ||
		ndPoint.x == ndNormalRect.left-lfHalfWidth && ndPoint.y == (ndNormalRect.bottom-lfHalfHeight)
		)
	{
		CExMessage::SendMessage(mpArrayPoint[niIndex],mpIterator,EMSG_SELECTPOINT_RESET_CURSOR,lhCursorNesw);
	}

	if (lhCursorNwse != NULL)
		DestroyCursor(lhCursorNwse);

	if (lhCursorNesw != NULL)
		DestroyCursor(lhCursorNesw);
	
}

//调整八个点的位置
void CSelectFrame::Relocation(void)
{
	if (mpArrayPoint[0] != NULL)
	{
		float lfHalfWidth = mpArrayPoint[0]->GetSizeX() / 2;
		float lfHalfHeight = mpArrayPoint[0]->GetSizeY() / 2;
		float lfWidth = mpArrayPoint[0]->GetSizeX();
		float lfHeight = mpArrayPoint[0]->GetSizeY();

		D2D1_POINT_2F ldPoint = mpIterator->GetPosition();
		D2D1_RECT_F	ldNormalRect = CExRect::GetNormalizedRectangle(mpIterator->GetPositionX(), mpIterator->GetPositionY(), mpIterator->GetSizeX(), mpIterator->GetSizeY());

		HCURSOR lhCursorSizeAll = LoadCursor(NULL, IDC_SIZEALL);

		do
		{

			mpArrayPoint[0]->SetPosition(-lfHalfWidth, -lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[0], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);
			else
				SetCursor(D2D1::Point2F(-lfHalfWidth, -lfHalfHeight), ldNormalRect, 0);

			mpArrayPoint[1]->SetPosition(FLOAT((mpIterator->GetSizeX() - lfHalfWidth) / 2.0f), -lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[1], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);

			mpArrayPoint[2]->SetPosition(mpIterator->GetSizeX() - lfHalfWidth, -lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[2], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);
			else
				SetCursor(D2D1::Point2F(mpIterator->GetSizeX() - lfHalfWidth, -lfHalfHeight), ldNormalRect, 2);

			mpArrayPoint[3]->SetPosition(mpIterator->GetSizeX() - lfHalfWidth, FLOAT((mpIterator->GetSizeY() - lfWidth) / 2.0f));
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[3], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);

			mpArrayPoint[4]->SetPosition(mpIterator->GetSizeX() - lfHalfWidth, mpIterator->GetSizeY() - lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[4], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);
			else
				SetCursor(D2D1::Point2F(mpIterator->GetSizeX() - lfHalfWidth, mpIterator->GetSizeY() - lfHalfHeight), ldNormalRect, 4);

			mpArrayPoint[5]->SetPosition(FLOAT((mpIterator->GetSizeX() - lfWidth) / 2.0f), mpIterator->GetSizeY() - lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[5], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);

			mpArrayPoint[6]->SetPosition(-lfHalfWidth, mpIterator->GetSizeY() - lfHalfHeight);
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[6], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);
			else
				SetCursor(D2D1::Point2F(-lfHalfWidth, mpIterator->GetSizeY() - lfHalfHeight), ldNormalRect, 6);

			mpArrayPoint[7]->SetPosition(-lfHalfWidth, FLOAT((mpIterator->GetSizeY() - lfHalfHeight) / 2.0f));
			if (mcIsEdit == false)
				CExMessage::SendMessage(mpArrayPoint[7], mpIterator, EMSG_SELECTPOINT_RESET_CURSOR, lhCursorSizeAll);

		} while (false);

		if (lhCursorSizeAll != NULL)
			DestroyCursor(lhCursorSizeAll);
	}

}


//绘制
ERESULT CSelectFrame::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

 		if (mpBgBitmap != NULL)
 		{
 			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
 				mpBgBitmap, 
				ESPB_DRAWBMP_LINEAR
 				);
 		}

		//画选择框的虚线
		float lfHalfWidth = mpArrayPoint[0]->GetSizeX()/2;
		float lfHalfHeight = mpArrayPoint[0]->GetSizeY()/2;

		for (ULONG liLoop = 0; liLoop < SF_POINT_MAX-1; liLoop++)
		{
			npPaintBoard->DrawLine(
				D2D1::Point2F(
					CExFloat::HalfPixel(mpArrayPoint[liLoop]->GetPosition().x + lfHalfWidth), 
					CExFloat::HalfPixel(mpArrayPoint[liLoop]->GetPosition().y + lfHalfHeight)),
				D2D1::Point2F(
					CExFloat::HalfPixel(mpArrayPoint[liLoop+1]->GetPosition().x + lfHalfWidth), 
					CExFloat::HalfPixel(mpArrayPoint[liLoop+1]->GetPosition().y + lfHalfHeight)),
				mpXuiBrush);
		}

		npPaintBoard->DrawLine(
			D2D1::Point2F
				(CExFloat::HalfPixel(mpArrayPoint[7]->GetPosition().x + lfHalfWidth), 
				CExFloat::HalfPixel(mpArrayPoint[7]->GetPosition().y + lfHalfHeight)),
			D2D1::Point2F
				(CExFloat::HalfPixel(mpArrayPoint[0]->GetPosition().x + lfHalfWidth), 
				CExFloat::HalfPixel(mpArrayPoint[0]->GetPosition().y + lfHalfHeight)),
			mpXuiBrush);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

// 鼠标落点检测
ERESULT CSelectFrame::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do 
	{
		if (rPoint.x < 0.0f || (UINT)rPoint.x >= mpIterator->GetSizeX() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpIterator->GetSizeY())
			break;

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);
	
	return luResult;

	// 检测八个点所围成的矩形区域

	// 顶部矩形框
	D2D1_RECT_F ldTopRect = D2D1::RectF(
		mpArrayPoint[0]->GetPositionX(), 
		mpArrayPoint[0]->GetPositionY(), 
		mpArrayPoint[0]->GetPositionX() + mpIterator->GetSizeX(), 
		mpArrayPoint[0]->GetSizeY()
		);
	if (CExRect::PtInRect(rPoint, ldTopRect))
		return ERESULT_MOUSE_OWNERSHIP;

	// 右边矩形框
	D2D1_RECT_F ldRightRect = D2D1::RectF(
		mpArrayPoint[2]->GetPositionX(), 
		mpArrayPoint[2]->GetPositionY(), 
		mpArrayPoint[2]->GetPositionX() + mpArrayPoint[2]->GetSizeX(), 
		mpIterator->GetSizeY() + mpArrayPoint[2]->GetSizeY()
		);
	if (CExRect::PtInRect(rPoint, ldRightRect))
		return ERESULT_MOUSE_OWNERSHIP;

	// 底部矩形框
	D2D1_RECT_F ldBottomRect = D2D1::RectF(
		mpArrayPoint[6]->GetPositionX(), 
		mpArrayPoint[6]->GetPositionY(), 
		mpArrayPoint[6]->GetPositionX() + mpIterator->GetSizeX(), 
		mpArrayPoint[6]->GetPositionY() + mpArrayPoint[6]->GetSizeY());
	if (CExRect::PtInRect(rPoint, ldBottomRect))
		return ERESULT_MOUSE_OWNERSHIP;

	// 左边矩形框
	D2D1_RECT_F ldLeftRect = D2D1::RectF(
		mpArrayPoint[0]->GetPositionX(), 
		mpArrayPoint[0]->GetPositionY(), 
		mpArrayPoint[0]->GetPositionX() + mpArrayPoint[0]->GetSizeX(), 
		mpIterator->GetSizeY() + mpArrayPoint[0]->GetSizeY()
		);
	if (CExRect::PtInRect(rPoint, ldLeftRect))
		return ERESULT_MOUSE_OWNERSHIP;

	return ERESULT_SUCCESS;

}

//禁用或启用
ERESULT CSelectFrame::OnElementEnable(bool nbIsEnable)
{
	for (int i=0;i<SF_POINT_MAX;i++)
	{
		mpArrayPoint[i]->SetVisible(nbIsEnable);	//如果是禁用就不显示八个点
	}

	return ERESULT_SUCCESS;
}

ERESULT CSelectFrame::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);
		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		SendMessageToParent(EMSG_SELECTFRAME_MOVING,npInfo->Offset,NULL,0);
		
		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽开始
ERESULT CSelectFrame::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(mpIterator->IsEnable() == false)
			break;	//如果是禁用状态就不能拖动

		BREAK_ON_NULL(npInfo);
		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;
		
		if(npInfo->DragOn != mpIterator) //只有在自己上面才可以
			break;
	
		SendMessageToParent(EMSG_SELECTFRAME_BEGIN, CExMessage::DataInvalid, NULL, 0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽结束
ERESULT CSelectFrame::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);

		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		SendMessageToParent(EMSG_SELECTFRAME_DRAGED, CExMessage::DataInvalid, NULL, 0);

		Relocation();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//处理八个点拖动
ERESULT CSelectFrame::OnPointDrag(IEinkuiIterator* npDragItem,D2D1_SIZE_F* npOffset)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	STCTL_CHANGE_POSITION_SIZE	ldPositionSize;	// 拖动点，改变位置和大小
	D2D1_POINT_2F	ldPositionOffset;		// 记录位置偏移量
	D2D1_SIZE_F		ldSizeVariation;		// 大小改变量


	float lfSizeX = 0.0f;
	float lfSizeY = 0.0f;

	do 
	{

		// 先假定不需要翻转
		ldPositionSize.mcHTurn = false;
		ldPositionSize.mcVTurn = false;
		mcHTurn = false;
		mcVTurn = false;

		BREAK_ON_NULL(npDragItem);
		BREAK_ON_NULL(npOffset);

		// 计算不同锚点所对应的位置偏移和大小变化量
		switch(npDragItem->GetID())
		//switch(miActivePoint)
		{
		// 左上点
		case XuiLeftTop:	
			{
				// 计算位置偏移量
				ldPositionOffset = D2D1::Point2F(npOffset->width, npOffset->height);
				ldSizeVariation = D2D1::SizeF(-npOffset->width, -npOffset->height);
				break;
			}
		// 中上点
		case XuiMidTop:	
			{
				ldPositionOffset = D2D1::Point2F(0, npOffset->height);
				ldSizeVariation = D2D1::SizeF(0, -npOffset->height);		// 不允许左右缩放
				break;
			}
		// 右上点
		case XuiRightTop: 
			{
				ldPositionOffset = D2D1::Point2F(0, npOffset->height);
				ldSizeVariation = D2D1::SizeF(npOffset->width, -npOffset->height);
				break;
			}
		// 右中点
		case XuiRightMid: 
			{
				ldPositionOffset = D2D1::Point2F(0, 0);
				ldSizeVariation = D2D1::SizeF(npOffset->width, 0);	// 不允许上下缩放
				break;
			}
		// 右下点
		case XuiRightBottom: 
			{
				ldPositionOffset = D2D1::Point2F(0, 0);
				ldSizeVariation = D2D1::SizeF(npOffset->width, npOffset->height);
				break;
			}
		// 中下点
		case XuiMidBottom:	
			{
				ldPositionOffset = D2D1::Point2F(0, 0);
				ldSizeVariation = D2D1::SizeF(0, npOffset->height);	// 不允许左右缩放
				break;
			}
		// 左下点
		case XuiLeftBottom:	
			{
				ldPositionOffset = D2D1::Point2F(npOffset->width, 0);
				ldSizeVariation = D2D1::SizeF(-npOffset->width, npOffset->height);
				break;
			}
		// 左中点
		case XuiLeftMid:
			{
				ldPositionOffset = D2D1::Point2F(npOffset->width, 0);
				ldSizeVariation = D2D1::SizeF(-npOffset->width, 0);	// 不允许上下缩放
				break;
			}

		}

		// 获取改变后的大小
		lfSizeX = mdOriginSize.width + ldSizeVariation.width;
		lfSizeY = mdOriginSize.height + ldSizeVariation.height;

		// 如果size=0,则本次拖动不处理
		if (lfSizeX == 0 || lfSizeY == 0)
			return ERESULT_SUCCESS;
		
		// 这里有个效率问题
		// 1，设计上，当翻转后，需要归一化矩形，此时会重新设置拖动的锚点；如果不这样操作，则翻转后的拖动每次都会进入判断翻转的流程，从而增加了运算量；
		// 2，等比缩放的情况下，如果归一化后，计算斜率时运算将及其复杂，所以不归一化；
		// 3，将来可能改成支持负矩形，使行为统一。这样，父窗口在拖动时如果发生翻转，宽高可能会变负值，拖动完成后，在归一化坐标即可。
		// 4，现在的模型是，任意比例缩放采用归一化模型；等比缩放采用负矩形运算，向上层输出的结果转换为正矩形。


		// 第一步计算出的position偏移和size变化量
		ldPositionSize.mdPositionOffset = ldPositionOffset;
		ldPositionSize.mdSizeVariation = ldSizeVariation;

		// 处理等比缩放逻辑，根据缩放后的结果，计算出position偏移和size变化量
		if (mcProportionalScaling != false || mcProportionalScalingByKey != false)
		{
			OnProportionalScaling(npDragItem, ldPositionSize);			
		}
		else
		{
			OnNormalScaling(npDragItem, ldPositionSize);
		}

		// 向父窗口报告移动状态{位置偏移量，大小该变量，水平翻转，垂直翻转}
		SendMessageToParent(EMSG_SELECTPOINT_CHANGE_POSITION_SIZE, ldPositionSize,NULL,0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}


ERESULT CSelectFrame::OnNormalScaling(IEinkuiIterator* npDragItem, STCTL_CHANGE_POSITION_SIZE& noChangePositionSize)
{

	float lfSizeX = 0;
	float lfSizeY = 0;

	// 获取改变后的大小
	lfSizeX = mdOriginSize.width + noChangePositionSize.mdSizeVariation.width;
	lfSizeY = mdOriginSize.height + noChangePositionSize.mdSizeVariation.height;


	// 判断有无翻转
	do 
	{
		// 判断是否垂直翻转
		if (lfSizeY < 0)
		{
			noChangePositionSize.mcVTurn = true;
			mcVTurn = true;
		}
		// 判断是否水平翻转
		if (lfSizeX < 0)
		{
			noChangePositionSize.mcHTurn = true;
			mcHTurn = true;
		}

		// 如果没有翻转，则直接跳出
		if (mcHTurn == false && mcVTurn == false)
			break;

		// 如果有翻转，则针对每个点，单独处理
		switch(npDragItem->GetID())
		{
			// 左上点
		case XuiLeftTop:	
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;
				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;

				break;
			}
			// 中上点
		case XuiMidTop:	
			{
				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;

				break;
			}
			// 右上点
		case XuiRightTop: 
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = lfSizeX;

				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;

				break;
			}
			// 右中点
		case XuiRightMid: 
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = lfSizeX;					

				break;
			}
			// 右下点
		case XuiRightBottom: 
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = lfSizeX;

				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = lfSizeY;

				break;
			}
			// 中下点
		case XuiMidBottom:	
			{
				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = lfSizeY;

				break;
			}
			// 左下点
		case XuiLeftBottom:	
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;

				if (mcVTurn != false)
					noChangePositionSize.mdPositionOffset.y = lfSizeY;

				break;
			}
			// 左中点
		case XuiLeftMid:
			{
				if (mcHTurn != false)
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;

				break;
			}

		}// end switch

	} while (false);


	if (mcHTurn != false || mcVTurn != false)
	{

		// 翻转之后的大小
		D2D1_SIZE_F ldAfterTurnedSize;

		ldAfterTurnedSize.width= lfSizeX>0 ? lfSizeX : -lfSizeX;
		ldAfterTurnedSize.height = lfSizeY>0 ? lfSizeY : -lfSizeY;

		// 翻转前后的变化量 = 翻转后的大小 - 原始大小
		noChangePositionSize.mdSizeVariation.width = ldAfterTurnedSize.width - mdOriginSize.width;
		noChangePositionSize.mdSizeVariation.height = ldAfterTurnedSize.height - mdOriginSize.height;

	}

	// 根据当前状态与上一次的翻转状态，判断实际是否发生翻转
	if (mcLastHTurn != mcHTurn)
	{
		noChangePositionSize.mcHTurn = true;		
		mcLastHTurn = mcHTurn;
	}
	else
	{
		noChangePositionSize.mcHTurn = false;
	}

	if (mcLastVTurn != mcVTurn)
	{
		noChangePositionSize.mcVTurn = true;
		mcLastVTurn = mcVTurn;
	}
	else
	{
		noChangePositionSize.mcVTurn = false;
	}

	return ERESULT_SUCCESS;

}



ERESULT CSelectFrame::OnProportionalScaling(IEinkuiIterator* npDragItem, STCTL_CHANGE_POSITION_SIZE& noChangePositionSize)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	// 记录大小的变化量
	float lfSizeX = noChangePositionSize.mdSizeVariation.width;
	float lfSizeY = noChangePositionSize.mdSizeVariation.height;

	// 等比变形前的理论宽度和高度
	float lfBeforeScaledWidth = 0.0f;
	float lfBeforeScaledHeight = 0.0f;

	// 基准斜率
	float lfBaseLineSlope;
	// 当前斜率
	float lfCurLineSlope;

	// 获取改变后的大小
	lfBeforeScaledWidth = mdOriginSize.width + lfSizeX;
	lfBeforeScaledHeight = mdOriginSize.height + lfSizeY;

	{
		// 计算基准斜率,由原始矩形的数据确定，由于起始点始终是(0.0f, 0.0f)，所以斜率就是宽度和高度的比值
		lfBaseLineSlope = mdOriginSize.width/mdOriginSize.height;

		// 计算当前斜率，为当前宽度与高度的比值
		lfCurLineSlope = lfBeforeScaledWidth/lfBeforeScaledHeight;
		// 取绝对值，考虑的翻转的情况
		lfCurLineSlope = lfCurLineSlope>0 ?lfCurLineSlope:-lfCurLineSlope;
	}


	{
		// 判断是否水平翻转
		if (lfBeforeScaledWidth < 0)
		{
			noChangePositionSize.mcHTurn = true;
			mcHTurn = true;
		}

		// 判断是否向垂直翻转
		if (lfBeforeScaledHeight < 0)
		{
			noChangePositionSize.mcVTurn = true;
			mcVTurn = true;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 下面5句是精华代码，统一计算等比缩放后的变化量，兼顾各种情况
	//////////////////////////////////////////////////////////////////////////
	{
		if (lfCurLineSlope >= lfBaseLineSlope)
		{
			// 取X轴的缩放比例，保持与X轴对齐
			lfSizeX = (fabs(lfSizeX + mdOriginSize.width)-mdOriginSize.width);
			noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfSizeX, lfSizeX/lfBaseLineSlope);
		}
		else
		{
			// 取Y轴的缩放比例
			lfSizeY = (fabs(lfSizeY + mdOriginSize.height)-mdOriginSize.height);		
			noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfSizeY*lfBaseLineSlope, lfSizeY);
		}
	}

	// 定义等比变形后的X轴和Y轴的变化量，也就是宽度和高度的变化量，这两个变化量需要根据不同的翻转情况进行修正
	float lfSizeVarX = 0.0f;
	float lfSizeVarY = 0.0f;

	lfSizeVarX = noChangePositionSize.mdSizeVariation.width;
	lfSizeVarY = noChangePositionSize.mdSizeVariation.height;

	do 
	{


		switch(npDragItem->GetID())
		{
			// 左上点
		case XuiLeftTop:	
			{
				if (mcHTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;
					noChangePositionSize.mdPositionOffset.y = -lfSizeVarY;
				}
				if (mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -lfSizeVarX;
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;
				}

				if (mcHTurn != false && mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;
				}

				if (mcVTurn == false && mcHTurn == false)
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(-lfSizeVarX, -lfSizeVarY);
				}

				break;
			}
			// 中上点
		case XuiMidTop:	
			{
				float lfWidthOffset = (lfSizeY/mdOriginSize.height)*mdOriginSize.width;

				if (mcVTurn != false)
				{
					// 这里的计算逻辑很变态，画图以辅助理解20120701
					float xTmp = 0.0f;				
					if (mdOriginSize.width/2+lfWidthOffset/2 < 0)
					{
						xTmp = mdOriginSize.width + lfWidthOffset/2;		
					}
					else
					{
						xTmp = -lfWidthOffset/2;
					}

					noChangePositionSize.mdPositionOffset = D2D1::Point2F(xTmp, mdOriginSize.height);
				}
				else
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(-lfWidthOffset/2, noChangePositionSize.mdPositionOffset.y);
				}
	
				noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfWidthOffset, lfSizeY);				
				break;
			}
			// 右上点
		case XuiRightTop: 
			{
				if (mcHTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -(mdOriginSize.width + lfSizeVarX);		
					noChangePositionSize.mdPositionOffset.y = -lfSizeVarY;
				}

				if (mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = 0.0f;
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;
				}

				if (mcHTurn != false && mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -(mdOriginSize.width + lfSizeVarX);
					noChangePositionSize.mdPositionOffset.y = mdOriginSize.height;
				}

				if(mcHTurn == false && mcVTurn == false)
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(0, -lfSizeVarY);
				}

				break;
			}
			// 右中点
		case XuiRightMid: 
			{
				float lfHeightOffset = (lfSizeX/mdOriginSize.width)*mdOriginSize.height;
				if (mcHTurn != false)
				{
					float xTmp = 0.0f;				
					if (mdOriginSize.height/2+lfHeightOffset/2 < 0)
					{
						xTmp = mdOriginSize.height + lfHeightOffset/2;		
					}
					else
					{
						xTmp = -lfHeightOffset/2;
					}
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(lfBeforeScaledWidth, xTmp);	
				}
				else
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(0.0f, -lfHeightOffset/2);
				}
				
				noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfSizeX, lfHeightOffset);				
				break;
			}
			// 右下点
		case XuiRightBottom: 
			{

				if (mcHTurn != false && mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -(mdOriginSize.width + lfSizeVarX);
					noChangePositionSize.mdPositionOffset.y = -(mdOriginSize.height + lfSizeVarY);
					break;
				}

				if (mcHTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -(mdOriginSize.width + lfSizeVarX);
					noChangePositionSize.mdPositionOffset.y = 0;
				}

				if (mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = 0;
					noChangePositionSize.mdPositionOffset.y = -(mdOriginSize.height + lfSizeVarY);
				}

				if(mcHTurn == false && mcVTurn == false)
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(0, 0);
				}
				
				break;
			}
			// 中下点
		case XuiMidBottom:	
			{
				float lfWidthOffset = (lfSizeY/mdOriginSize.height)*mdOriginSize.width;
				if (mcVTurn != false)
				{

					float xTmp = 0.0f;				
					if (mdOriginSize.width/2+lfWidthOffset/2 < 0)
					{
						xTmp = mdOriginSize.width + lfWidthOffset/2;		
					}
					else
					{
						xTmp = -lfWidthOffset/2;
					}

					noChangePositionSize.mdPositionOffset = D2D1::Point2F(xTmp, lfBeforeScaledHeight);
				}
				else
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(-lfWidthOffset/2, noChangePositionSize.mdPositionOffset.y);
				}

				//noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfWidthOffset, fabs(lfHeight) - mdOriginSize.height);	
				noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfWidthOffset, lfSizeY);	

				break;
			}
			// 左下点
		case XuiLeftBottom:	
			{

				if (mcHTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;
					noChangePositionSize.mdPositionOffset.y = 0.0f;
				}

				if (mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = -lfSizeVarX;
					noChangePositionSize.mdPositionOffset.y = -(mdOriginSize.height + lfSizeVarY);

				}

				if (mcHTurn != false && mcVTurn != false)
				{
					noChangePositionSize.mdPositionOffset.x = mdOriginSize.width;
					noChangePositionSize.mdPositionOffset.y = -(mdOriginSize.height + lfSizeVarY);
				}

				if(mcHTurn == false && mcVTurn == false)
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(-lfSizeVarX, 0);	
				}
				
				break;
			}
			// 左中点
		case XuiLeftMid:
			{
				float lfHeightOffset = (lfSizeX/mdOriginSize.width)*mdOriginSize.height;

				if (mcHTurn != false)
				{
					float xTmp = 0.0f;				
					if (mdOriginSize.height/2+lfHeightOffset/2 < 0)
					{
						xTmp = mdOriginSize.height + lfHeightOffset/2;		
					}
					else
					{
						xTmp = -lfHeightOffset/2;
					}

					noChangePositionSize.mdPositionOffset = D2D1::Point2F(mdOriginSize.width, xTmp);
				}
				else
				{
					noChangePositionSize.mdPositionOffset = D2D1::Point2F(noChangePositionSize.mdPositionOffset.x, -lfHeightOffset/2);
				}

				noChangePositionSize.mdSizeVariation = D2D1::SizeF(lfSizeX, lfHeightOffset);	

				break;
			}
		}

	} while (false);


	// 定义等比变形后的宽度和高度
	float lfAfterScaledWidth = 0.0f;
	float lfAfterScaledHeight = 0.0f;

	lfAfterScaledWidth = mdOriginSize.width + noChangePositionSize.mdSizeVariation.width;
	lfAfterScaledHeight = mdOriginSize.height + noChangePositionSize.mdSizeVariation.height;

	// 修正水平翻转和垂直翻转后的变化量
	if (mcHTurn != false || mcVTurn != false)
	{
		// 翻转之后的大小
		D2D1_SIZE_F ldAfterTurnedSize;

		ldAfterTurnedSize.width= lfAfterScaledWidth>0 ? lfAfterScaledWidth : -lfAfterScaledWidth;
		ldAfterTurnedSize.height = lfAfterScaledHeight>0 ? lfAfterScaledHeight : -lfAfterScaledHeight;

		// 翻转前后的变化量 = 翻转后的大小 - 原始大小
		noChangePositionSize.mdSizeVariation.width = ldAfterTurnedSize.width - mdOriginSize.width;
		noChangePositionSize.mdSizeVariation.height = ldAfterTurnedSize.height - mdOriginSize.height;

	}

	// 根据当前状态与上一次的翻转状态，判断实际是否发生翻转
	if (mcLastHTurn != mcHTurn)
	{
		noChangePositionSize.mcHTurn = true;		
		mcLastHTurn = mcHTurn;
	}
	else
	{
		noChangePositionSize.mcHTurn = false;
	}

	if (mcLastVTurn != mcVTurn)
	{
		noChangePositionSize.mcVTurn = true;
		mcLastVTurn = mcVTurn;
	}
	else
	{
		noChangePositionSize.mcVTurn = false;
	}

	return ERESULT_SUCCESS;

}


// 设置八个点的显示状态
void CSelectFrame::SetPointVisible(bool nbFlag)
{
	/*if (nbFlag == false)
	{
		mbModifying = true;
		for (int liLoop = 0; liLoop < SF_POINT_MAX; liLoop++)
		{
			mpArrayPoint[liLoop]->SetVisible(false);
		}
	}
	else
	{
		mbModifying = false;
		for (int liLoop = 0; liLoop < SF_POINT_MAX; liLoop++)
		{
			mpArrayPoint[liLoop]->SetVisible(true);
		}
	}*/

}