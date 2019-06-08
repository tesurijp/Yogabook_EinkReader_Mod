/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvSliderBarImp.h"


//using namespace D2D1;

DEFINE_BUILTIN_NAME(SliderBar)

CEvSliderBar::CEvSliderBar()
{
	mulMethod = ESPB_DRAWBMP_LINEAR;	//缩放方式
//	mpBarButton = NULL;
	mbIsMouseFocus = false;	
	mbIsPressed = false;
	mnVertical = 0;

}
CEvSliderBar::~CEvSliderBar()
{
	//if(mpBarButton)
	//	mpBarButton->Release();
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvSliderBar::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;
	do
	{
		CXuiElement::OnElementDestroy();	//调用基类
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CEvSliderBar::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//创建滑块SliderButton/BAR picture /Bar left Picture
		ICfKey * lpKey = npTemplete->GetSubKey(SLIDER_BAR_PICTURE);			
		if(lpKey)
			mpBarPicture = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);
		BREAK_ON_NULL(mpBarPicture);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_LEFT_PICTURE);			
		if(lpKey)
			mpLeftBarPicture = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_SLIDERBUTTON);			
		if(lpKey)
			mpDragButton = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);
		BREAK_ON_NULL(mpDragButton);
		//装载一些必要的配置资源
		LoadResource();

	

		leResult = ERESULT_SUCCESS;

	} while (false);
	return leResult;
}
//元素Enbale
ERESULT CEvSliderBar::OnElementEnable(bool nbIsEnable)
{
	if(mpDragButton) mpDragButton->SetEnable(nbIsEnable);
	
	return ERESULT_SUCCESS;
}
//装载配置资源
ERESULT CEvSliderBar::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;
	do 
	{	
		//装载LeftOrBack的图片 

		//装载Min Max

		//设置Bar的Rect
		if (mpIterator->GetSizeX() > 0)
		{
			mpBarPicture->SetSize(mpIterator->GetSizeX(), mpBarPicture->GetSizeY());
			mpLeftBarPicture->SetSize(mpIterator->GetSizeX(), mpLeftBarPicture->GetSizeY());
		}

		// 读取鼠标有效区域 add by colin
		mdEffectiveRect.left = (FLOAT)this->mpTemplete->QuerySubKeyValueAsLONG(TF_ID_SLIDERBAR_EFFECTIVERECT_LEFT, 0);
		mdEffectiveRect.top = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_SLIDERBAR_EFFECTIVERECT_TOP, 0);
		mdEffectiveRect.right = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_SLIDERBAR_EFFECTIVERECT_RIGHT, mpBarPicture->GetSizeX());
		mdEffectiveRect.bottom = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_SLIDERBAR_EFFECTIVERECT_BOTTOM, mpBarPicture->GetSizeY());

		if (mpDragButton)
		{
			LONG lnStyle = ES_SLIDER_BUTTON_STYLE_HOR;
			if (mnVertical)
				lnStyle = ES_SLIDER_BUTTON_STYLE_VER;
			//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpDragButton,EACT_SLIDERBUTTON_SET_STYLE,&lnStyle,sizeof(LONG));
			CExMessage::PostMessage(mpDragButton, mpIterator, EACT_SLIDERBUTTON_SET_STYLE, lnStyle);
			//获取BarRect
			mBarRect.left = mpBarPicture->GetPositionX();
			mBarRect.top = mpBarPicture->GetPositionY();
			mBarRect.right = mBarRect.left + mpBarPicture->GetSizeX();
			mBarRect.bottom = mBarRect.top + mpBarPicture->GetSizeY();

			//设置可以滑动的范围
			BREAK_ON_NULL(mpBarPicture);
			mSliderRect.left = mpBarPicture->GetPositionX();
			mSliderRect.top = mpBarPicture->GetPositionY();
			mSliderRect.right = mSliderRect.left + mpBarPicture->GetSizeX();
			mSliderRect.bottom = mSliderRect.top + mpBarPicture->GetSizeY();

			////需加上1/2 dragButton的打下
			//if(mnVertical)
			//{		
			//	mSliderRect.top -= mpDragButton->GetSizeY() /2 ;
			//	mSliderRect.bottom += mpDragButton->GetSizeY() / 2;
			//}
			//else
			//{	
			//	mSliderRect.left -= mpDragButton->GetSizeX() /2 ;
			//	mSliderRect.right += mpDragButton->GetSizeX() /2 ;
			//}
			//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpDragButton,EACT_SLIDERBUTTON_SET_SLIDERRECT,&mSliderRect,sizeof(D2D1_RECT_F));
			CExMessage::PostMessage(mpDragButton, mpIterator, EACT_SLIDERBUTTON_SET_SLIDERRECT, mSliderRect);
		}
		//如果有LeftOrUP Picture则大小设为0
		if (mpLeftBarPicture)
		{
			if (mnVertical)
				mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(), 0.0);
			else
				mpLeftBarPicture->SetSize(0.0, mpLeftBarPicture->GetSizeY());
		}

		leResult = ERESULT_SUCCESS;
	} while (false);

	return leResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvSliderBar::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
	do 
	{
		BREAK_ON_NULL(npMsg);
		switch (npMsg->GetMessageID())
		{
		case EACT_SLIDERBUTTON_DRAG_START:
			{
				FLOAT lfPosition;
				if(mnVertical)
					lfPosition = mpDragButton->GetPositionY() - mBarRect.top ;		
				else
					lfPosition = mpDragButton->GetPositionX() - mBarRect.left ;
				lfPosition *= mfDestPixelPerScrollPix;

				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator, EACT_SLIDERBAR_DRAG_START, lfPosition);

				break;
			}
		case EACT_SLIDERBUTTON_DRAG_END:
			{
				// 这里知道了拖拽结束，应该将此时的位置也发送过去  add by colin
				FLOAT lfPosition;
				if(mnVertical)
					lfPosition = mpDragButton->GetPositionY() - mBarRect.top ;		
				else
					lfPosition = mpDragButton->GetPositionX() - mBarRect.left ;
				lfPosition *= mfDestPixelPerScrollPix;

				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator, EACT_SLIDERBAR_DRAG_END, lfPosition);
				break;
			}
		case EACT_SLIDERBUTTON_DRAGING:
			{
				//计算Position,给父窗口发定位消息
				//定位DragButton的中心点位置
				if(mpDragButton)
				{
					FLOAT lfPosition ;
					if(mnVertical)
						lfPosition = mpDragButton->GetPositionY() - mBarRect.top ;		
					else
						lfPosition = mpDragButton->GetPositionX() - mBarRect.left ;

					//如果有LeftorUP picture 则应该放大或缩小该图像
					if(mpLeftBarPicture)
					{
						if(mnVertical)
							mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(),lfPosition);
						else
							mpLeftBarPicture->SetSize(lfPosition,mpLeftBarPicture->GetSizeY());
					}
					lfPosition *= mfDestPixelPerScrollPix;

					CExMessage::PostMessage(mpIterator->GetParent(),mpIterator, EACT_SLIDERBAR_DRAGING, lfPosition);

				}
				break;
			}
		case EACT_SLIDERBAR_SET_RANGE:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
				//FLOAT lfy = (*lpValue)/mfDestPixelPerScrollPix;
				SetDeltaSize(*lpValue);
				break;
			}
		case EACT_SLIDERBAR_SET_POSITION:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();

				FLOAT lfPosition ;
				FLOAT lfDelta = (*lpValue) / mfDestPixelPerScrollPix;
				if(mnVertical)
				{
					lfPosition = mBarRect.top + lfDelta;	
					mpDragButton->SetPosition(mpDragButton->GetPositionX(),lfPosition);
				}
				else
				{
					lfPosition =  mBarRect.left + lfDelta ;
					mpDragButton->SetPosition(lfPosition ,mpDragButton->GetPositionY());
				}

				//如果有LeftorUP picture 则应该放大或缩小该图像
				if(mpLeftBarPicture)
				{
					if(mnVertical)
						mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(),lfPosition - mBarRect.top);
					else
						mpLeftBarPicture->SetSize(lfPosition - mBarRect.left,mpLeftBarPicture->GetSizeY());
				}
			
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

	} while (false);
	
	return luResult;
}

//设置滚动的范围
bool CEvSliderBar:: SetDeltaSize(FLOAT nfSize)
{
	mfMax = nfSize;

	//先得到最大的可滚动像素
	FLOAT mfMaxScrollPixel = 0.0;
	if(mnVertical)
		mfMaxScrollPixel = mBarRect.bottom - mBarRect.top - mpDragButton->GetSizeY() ;
	else
		mfMaxScrollPixel = mBarRect.right - mBarRect.left - mpDragButton->GetSizeX();

	//计算目标像素和滚动像素的对应关系
	mfDestPixelPerScrollPix = mfMax / mfMaxScrollPixel;

	


	return true;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvSliderBar::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;		

		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


// 鼠标落点检测
ERESULT CEvSliderBar::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do 
	{	
		if(mpIterator->IsVisible() == false)
			break;

		////进入非滑块区域
		//if(rPoint.x < 0.0f ||
		//	(UINT)rPoint.x >= mpIterator->GetSizeX() ||
		//	rPoint.y < 0.0f || 
		//	(UINT)rPoint.y >= mpIterator->GetSizeY())
		//	break;

		// 进入无效区域
		if(rPoint.x < mdEffectiveRect.left ||
			rPoint.x > mdEffectiveRect.right ||
			rPoint.y < mdEffectiveRect.top ||
			rPoint.y > mdEffectiveRect.bottom)
			break;
	
		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}

bool CEvSliderBar::SetRange(FLOAT nMin,FLOAT nMax)
{
	mfMin = nMin;
	mfMax = nMin;
	return true;
}
bool CEvSliderBar::SetPos(FLOAT nPos)
{
	mfPos = nPos;
	return false;
}


//鼠标按下
ERESULT CEvSliderBar::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if(mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if(MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if (npInfo->Presssed != false)
		{
			//鼠标按下，如果不落在mBarRect 则处理
			if(npInfo->Position.x < mBarRect.left ||
					npInfo->Position.x > mBarRect.right ||
					npInfo->Position.y < mBarRect.top ||
					npInfo->Position.y > mBarRect.bottom)
					break;
			FLOAT lf = 0.0;
			if(mnVertical)
			{
				lf = (npInfo->Position.y - mBarRect.top) * mfDestPixelPerScrollPix;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_SLIDERBAR_SET_POSITION,&lf,sizeof(FLOAT));
				CExMessage::PostMessage(mpIterator,mpIterator,EACT_SLIDERBAR_SET_POSITION,lf);
			}
			else
			{
				lf = (npInfo->Position.x - mBarRect.left) * mfDestPixelPerScrollPix;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_SLIDERBAR_SET_POSITION,&lf,sizeof(FLOAT));
				CExMessage::PostMessage(mpIterator,mpIterator,EACT_SLIDERBAR_SET_POSITION,lf);
			}

			//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),EACT_SLIDERBAR_THUMB_POSITION,&lf,sizeof(FLOAT));
			CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_SLIDERBAR_THUMB_CLICK,lf);
		}



		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//绘制消息
ERESULT CEvSliderBar::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
			mpBgBitmap,
			ESPB_DRAWBMP_EXTEND);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//元素参考尺寸发生变化
ERESULT CEvSliderBar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return LoadResource();
}