/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "XUIx.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvSliderButtonImp.h"


//using namespace D2D1;

DEFINE_BUILTIN_NAME(SliderButton)

CEvSliderButton::CEvSliderButton()
{
	mnMoveStyle = ES_SLIDER_BUTTON_STYLE_HOR;

}
CEvSliderButton::~CEvSliderButton()
{
	//if(mpBarButton)
	//	mpBarButton->Release();
}


ULONG CEvSliderButton::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CEvButton::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_DRAG);
		leResult = ERESULT_SUCCESS;

	} while (false);
	return leResult;
}
// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvSliderButton::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
	switch (npMsg->GetMessageID())
	{
	case EACT_SLIDERBUTTON_SET_STYLE:
		{
			if(npMsg->GetInputDataSize() != sizeof(LONG))
			{
				break;
			}
			LONG * lpValue = (LONG*)npMsg->GetInputData();
			mnMoveStyle = * lpValue;
			luResult = ERESULT_SUCCESS;
			break;
		}
	case EACT_SLIDERBUTTON_SET_SLIDERRECT:
		{
			if(npMsg->GetInputDataSize() != sizeof(D2D1_RECT_F))
			{
				break;
			}
			D2D1_RECT_F * lpRect = (D2D1_RECT_F*)npMsg->GetInputData();
			if(lpRect)
			{
				RtlCopyMemory(&mRectSlider,lpRect,sizeof(D2D1_RECT_F));
				luResult = ERESULT_SUCCESS;
			}
			break;
		}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}
	if(luResult == ERESULT_NOT_SET)		
		luResult = CEvButton::ParseMessage(npMsg);

	return luResult;
}
//元素拖拽
ERESULT CEvSliderButton::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	//
	if(mnMoveStyle == ES_SLIDER_BUTTON_STYLE_VER)
	{		
		FLOAT lfy = npInfo->Offset.y + mDragStartPoint.y;		
		//计算位置，不能超出范围
		if(lfy < mRectSlider.top)
			lfy = mRectSlider.top;
		FLOAT lfMaxY = mRectSlider.top + ((mRectSlider.bottom - mRectSlider.top) - mpIterator->GetSizeY());
		if(lfy > lfMaxY)
			lfy = lfMaxY;

		mpIterator->SetPosition(mpIterator->GetPositionX(),lfy);
	}
	else if(mnMoveStyle == ES_SLIDER_BUTTON_STYLE_HOR)
	{
		FLOAT lfx = npInfo->Offset.x + mDragStartPoint.x;		
		//计算位置，不能超出范围
		if(lfx < mRectSlider.left)
			lfx = mRectSlider.left;
		//int n = mpIterator->GetSizeX();
		FLOAT lfMaxX = mRectSlider.left + ((mRectSlider.right - mRectSlider.left) - mpIterator->GetSizeX());
		if(lfx > lfMaxX)
			lfx =  lfMaxX;
		mpIterator->SetPosition(lfx,mpIterator->GetPositionY());
	}
	else if(mnMoveStyle == ES_SLIDER_BUTTON_STYLE_ANYWAY)
	{
		FLOAT lfy = npInfo->Offset.y + mDragStartPoint.y;		
		//计算位置，不能超出Y范围
		if(lfy < mRectSlider.top)
			lfy = mRectSlider.top;
		if(lfy > ((mRectSlider.bottom - mRectSlider.top) - mpIterator->GetSizeY()) )
			lfy = ((mRectSlider.bottom - mRectSlider.top) - mpIterator->GetSizeY());

		FLOAT lfx = npInfo->Offset.x + mDragStartPoint.x;		
		//计算位置，不能超出X范围
		if(lfx < mRectSlider.left)
			lfx = mRectSlider.left;
		if(lfx > ((mRectSlider.right - mRectSlider.left) - mpIterator->GetSizeX()) )
			lfx = ((mRectSlider.right - mRectSlider.left) - mpIterator->GetSizeX());

		mpIterator->SetPosition(lfx,lfy);
	}
	
	//
// 	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpIterator->GetParent(),
// 		EACT_SLIDERBUTTON_DRAGING,
// 		(LPVOID)npInfo,
// 		sizeof(STMS_DRAGGING_ELE),NULL,0);

	CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_SLIDERBUTTON_DRAGING,npInfo,EMSG_POSTTYPE_REDUCE);

	return ERESULT_SUCCESS;
}

//拖拽开始
ERESULT CEvSliderButton::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	mDragStartPoint.x = mpIterator->GetPositionX();
	mDragStartPoint.y = mpIterator->GetPositionY();

	CExMessage::SendMessage(mpIterator->GetParent(),mpIterator,EACT_SLIDERBUTTON_DRAG_START,CExMessage::DataInvalid);
	return ERESULT_SUCCESS;
}

//拖拽结束
ERESULT CEvSliderButton::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	CExMessage::SendMessage(mpIterator->GetParent(),mpIterator,EACT_SLIDERBUTTON_DRAG_END,CExMessage::DataInvalid);
	return ERESULT_SUCCESS;
}

//元素Enbale
ERESULT CEvSliderButton::OnElementEnable(bool nbIsEnable)
{
	if (nbIsEnable)
	{
		if (mpIterator) mpIterator->ModifyStyles(EITR_STYLE_MOUSE, NULL);
		mpIterator->SetAlpha(1.0f);
	}
	else
	{
		if (mpIterator) mpIterator->ModifyStyles(NULL, EITR_STYLE_MOUSE);
		mpIterator->SetAlpha(1.0f);
	}

	return ERESULT_SUCCESS;
}
