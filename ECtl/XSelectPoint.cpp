/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"

#include "xSelectPoint.h"
#include "xSelectFrameMacro.h"

DEFINE_BUILTIN_NAME(System_SelectPoint)

CSelectPoint::CSelectPoint(void)
{
	mfWidth = 80.0f;
	mfHeight = 80.0f;

	mpXuiBrush = NULL;
	mpFillBrush = NULL;
}


CSelectPoint::~CSelectPoint(void)
{
	CMM_SAFE_RELEASE(mpXuiBrush);
}


ULONG CSelectPoint::InitOnCreate(
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

		mpXuiBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush, D2D1::ColorF(0.0f,0.0f,0.0f));
		BREAK_ON_NULL(mpXuiBrush);
		mpFillBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush, D2D1::ColorF(0.0f, 0.0f, 0.0f));
		BREAK_ON_NULL(mpFillBrush);

		mpIterator->SetSize(mfWidth, mfHeight);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}


//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CSelectPoint::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpToolTipKey = NULL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		// 这里元素ID不能为0
		if(mpIterator->GetID() == 1 || mpIterator->GetID() == 5)
			CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_SIZENWSE);
		else if(mpIterator->GetID() == 3 || mpIterator->GetID() == 7)
			CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_SIZENESW);
		else if(mpIterator->GetID() == 2 || mpIterator->GetID() == 6)
			CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_SIZENS);
		else if(mpIterator->GetID() == 4 || mpIterator->GetID() == 8)
			CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_SIZEWE);

		mpIterator->ModifyStyles(EITR_STYLE_ALL_DRAG);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//修改鼠标样式
void CSelectPoint::SetCursor(HCURSOR nhInnerCursor)
{
// 	if(CXuiElement::mhInnerCursor != NULL)
// 		CloseHandle(CXuiElement::mhInnerCursor);

	CXuiElement::mhInnerCursor = nhInnerCursor;
}

//分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
//本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CSelectPoint::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_SELECTPOINT_RESET_CURSOR:
		{
			if(npMsg->GetInputDataSize() != sizeof(HCURSOR))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			HCURSOR* lpVoid = (HCURSOR *)npMsg->GetInputData();
			SetCursor(*lpVoid);

			break;
		}
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

// 鼠标落点检测
ERESULT CSelectPoint::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	if (mpIterator->IsVisible() == false || mpIterator->IsEnable() == false)
		return ERESULT_SUCCESS; //隐藏或禁用时不检测

	if(rPoint.x < 0 || rPoint.x > mpIterator->GetSizeX() || rPoint.y < 0 || rPoint.y > mpIterator->GetSizeY())
		return ERESULT_SUCCESS;
	else
		return ERESULT_MOUSE_OWNERSHIP;
}

//绘制消息
ERESULT CSelectPoint::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{

	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	
	do 
	{
		BREAK_ON_NULL(mpXuiBrush);
		mpXuiBrush->SetStrokeWidth(1.0f);
		
		//npPaintBoard->FillRect(D2D1::RectF(0.5f, 0.5f, CExFloat::UnderHalf(mfWidth),CExFloat::UnderHalf(mfHeight)), mpFillBrush);
		//npPaintBoard->DrawRect(D2D1::RectF(0.5f, 0.5f, CExFloat::UnderHalf(mfWidth),CExFloat::UnderHalf(mfHeight)), mpXuiBrush);
		npPaintBoard->FillEllipse(D2D1::RectF(25.0f, 25.0f, CExFloat::UnderHalf(55.0f), CExFloat::UnderHalf(55.0f)), mpFillBrush);

		luResult = ERESULT_SUCCESS;

	} while (false);

	return luResult;
}

ERESULT CSelectPoint::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);
		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		SendMessageToParent(EMSG_SELECTPOINT_MOVING,npInfo->Offset,NULL,0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽开始
ERESULT CSelectPoint::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);
		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		if(npInfo->DragOn != mpIterator) //只有在自己上面才可以
			break;

		//通知父窗口，开始拖动
		SendMessageToParent(EMSG_SELECTPOINT_BEGIN,*npInfo,NULL,0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽结束
ERESULT CSelectPoint::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);

		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		//通知父窗口，自己移动完成
		SendMessageToParent(EMSG_SELECTFPOINT_MOVED,CExMessage::DataInvalid,NULL,0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}