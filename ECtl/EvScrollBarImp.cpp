/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvScrollBarImp.h"
#include "EvButtonImp.h"


DEFINE_BUILTIN_NAME(ScrollBar)

CEvScrollBar::CEvScrollBar()
{
	mpBtUp = mpBtDrag = mpBtDown = NULL;
	mfSVScrollBarWidth = 0.0;
	mfSHScrollBarHeigth = 0.0;
	mbVertical = true;
	mfDestPixelPerScrollPix = 1.0f;
	mfMinBarSize = 0.0f;
}

CEvScrollBar::~CEvScrollBar()
{
	//CMM_SAFE_RELEASE(mpBtUp);
	//CMM_SAFE_RELEASE(mpBtDown);
	//CMM_SAFE_RELEASE(mpBtDrag);

}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvScrollBar::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CXuiElement::OnElementDestroy();	//调用基类

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CEvScrollBar::InitOnCreate(
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
		//创建Button
		ICfKey * lpUp = npTemplete->GetSubKey(SCB_BT_UP);
		ICfKey * lpDown = npTemplete->GetSubKey(SCB_BT_DOWN);
		ICfKey * lpDrag = npTemplete->GetSubKey(SCB_BT_DRAG);
	
		//根据List的配置文件，创建纵向或横向滚动条
		if(lpUp)
			mpBtUp = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpUp);
		if(lpDown)
			mpBtDown = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpDown);
		if(lpDrag)
			mpBtDrag = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpDrag);
	

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}
ERESULT CEvScrollBar::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npSender);
		if (npSender->GetID() == SCB_BT_UP)
		{			
			SetPosition(true);
		}
		if (npSender->GetID() == SCB_BT_DOWN)
		{			
			SetPosition(false);
		}

		lResult = ERESULT_SUCCESS;
	}while(false);
	return lResult;
}

FLOAT CEvScrollBar::GetVScrollBarWidth()
{
	return mfSVScrollBarWidth;
}
//获取横向滚动条的高度
FLOAT CEvScrollBar::GetHScrollBarHeigth()
{
	return mfSHScrollBarHeigth;
}
//设置滚动的范围
bool CEvScrollBar:: SetDeltaSize(FLOAT nfSize)
{
	mfDeltaSize = nfSize;

	//先得到最大的可滚动像素
	FLOAT mfMaxScrollPixel = 0.0;
	if(mbVertical)
		mfMaxScrollPixel = mRectSlider.bottom - mRectSlider.top - mfMinBarSize;
	else
		mfMaxScrollPixel = mRectSlider.right - mRectSlider.left - mfMinBarSize;

	//计算目标像素和滚动像素的对应关系
    mfDestPixelPerScrollPix = mfDeltaSize / mfMaxScrollPixel;

	//如果mfDestPixelPerScrollPix < 1.0  则需要放大滑块
	//不然则将滑块缩小的最小值
	if(mfDestPixelPerScrollPix <= 1.0)
	{
		mfDestPixelPerScrollPix = 1.0;
		if(mbVertical)
			mpBtDrag->SetSize(mpBtDrag->GetSizeX(),mfMinBarSize + mfMaxScrollPixel - mfDeltaSize);
		else
			mpBtDrag->SetSize(mfMinBarSize + mfMaxScrollPixel - mfDeltaSize,mpBtDrag->GetSizeY());
	}
	else
	{
		if(mbVertical)
			mpBtDrag->SetSize(mpBtDrag->GetSizeX(),mfMinBarSize);
		else
			mpBtDrag->SetSize(mfMinBarSize,mpBtDrag->GetSizeY());
	}


	return true;
}
bool CEvScrollBar:: SetPosition(bool bUp)
{
	FLOAT lfDeta  = -20.0f;
	if(bUp == false)
		lfDeta = -lfDeta;
	if(mpBtDrag)
	{
		if(mbVertical)
		{
			FLOAT lfTempPosition = mpBtDrag->GetPositionY() + lfDeta;
			if(lfTempPosition < mRectSlider.top)
				lfTempPosition = mRectSlider.top;
			else if((lfTempPosition + mpBtDrag->GetSizeY() ) > mRectSlider.bottom)
				lfTempPosition = mRectSlider.bottom - mpBtDrag->GetSizeY();
			
			mpBtDrag->SetPosition(mpBtDrag->GetPositionX(),lfTempPosition);
		}
		else
		{
			FLOAT lfTempPosition = mpBtDrag->GetPositionX() + lfDeta ;
			if(lfTempPosition < mRectSlider.left)
				lfTempPosition = mRectSlider.left;
			else if((lfTempPosition + mpBtDrag->GetSizeX()) > mRectSlider.right)
				lfTempPosition = mRectSlider.right - mpBtDrag->GetSizeX();
		
			mpBtDrag->SetPosition(lfTempPosition,mpBtDrag->GetPositionY());

		}
	}
	EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,
		EACT_SLIDERBUTTON_DRAGING,NULL,0);
	
	return true;
}
//装载配置资源
ERESULT CEvScrollBar::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;	

	do 
	{
		LONG lHor = mpTemplete->QuerySubKeyValueAsLONG(SCROLLBAR_HOR,0);
		if(lHor != 0)
			mbVertical = false;

		leResult = ERESULT_SUCCESS;

	} while (false);


	return leResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvScrollBar::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	
	case EACT_SLIDERBUTTON_DRAGING:
		{
			//计算Position,给父窗口发定位消息
			if(mpBtDrag)
			{
				if(mbVertical)
				{
					FLOAT lfy = mpBtDrag->GetPositionY();
					if(mpBtUp)
						lfy -= mpBtUp->GetSizeY();
					lfy *= mfDestPixelPerScrollPix;
					EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),
						EACT_SCROLLBAR_VSCROLL_THUMB_POSITION,&lfy,sizeof(FLOAT));
				}
				else
				{
					FLOAT lfX = mpBtDrag->GetPositionX();
					if(mpBtUp)
						lfX -= mpBtUp->GetSizeX();
					lfX *= mfDestPixelPerScrollPix;
					EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),
						EACT_SCROLLBAR_HSCROLL_THUMB_POSITION,&lfX,sizeof(FLOAT));
				}
			}
			break;
		}
	case EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION:
		{
			if(npMsg->GetInputDataSize() != sizeof(FLOAT))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}
			FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
			FLOAT lfy = (*lpValue)/mfDestPixelPerScrollPix;
			if(mpBtDrag)
			{   
				if(lfy < mRectSlider.top)
					lfy = mRectSlider.top;
				else if(lfy > (mRectSlider.bottom - mpBtDrag->GetSizeY() ))
					lfy = mRectSlider.bottom - mpBtDrag->GetSizeY();
				else
					lfy += mRectSlider.top;
				
				mpBtDrag->SetPosition(mpBtDrag->GetPositionX(),lfy);
			}
			break;
		}
	case EACT_SCROLLBAR_HSCROLL_SET_SLIDER_POSTION:
		{
			if(npMsg->GetInputDataSize() != sizeof(FLOAT))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}
			FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
			FLOAT lfX = (*lpValue)/mfDestPixelPerScrollPix;
			if(mpBtDrag)
			{
				if(lfX < mRectSlider.left)
					lfX = mRectSlider.left;
				else if(lfX > (mRectSlider.right - mpBtDrag->GetSizeX() ))
					lfX = mRectSlider.right - mpBtDrag->GetSizeX();
				else
					lfX += mRectSlider.left;

			
				mpBtDrag->SetPosition(lfX,mpBtDrag->GetPositionY());
			}
			break;
		}
	case EACT_SCROLLBAR_HVSCROLL_SET_DELTA_SIZE:
		{
			//设置滚动范围
			if(npMsg->GetInputDataSize() != sizeof(FLOAT))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}
			SetDeltaSize(*(FLOAT*)npMsg->GetInputData());

			break;
		}
	
	case EACT_SCROLLBAR_HSCROLL_GET_HEIGTH:
		{
			if(npMsg->GetOutputBufferSize() != sizeof(float))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			// 设置输出数据
			float* lpOut = (float*)npMsg->GetOutputBuffer();

			*lpOut = GetHScrollBarHeigth();

			npMsg->SetOutputDataSize(sizeof(float));
			break;
			
		}
	case EACT_SCROLLBAR_VSCROLL_GET_WIDTH:
		{
			if(npMsg->GetOutputBufferSize() != sizeof(float))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			// 设置输出数据
			float* lpOut = (float*)npMsg->GetOutputBuffer();

			*lpOut = GetVScrollBarWidth();

			npMsg->SetOutputDataSize(sizeof(float));
			break;
			
		}
	case EACT_SCROLLBAR_HVSCROLL_RELACATION:
		{
			Relocation();
			break;
		}
	case EACT_SCROLLBAR_BT_CLICK:
		{
			//模拟上下按钮点击
			bool lbUp = true;
			CExMessage::GetInputData(npMsg,lbUp,lbUp);

			SetPosition(lbUp);

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

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvScrollBar::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//装载一些必要的配置资源
		//LoadResource();
		mpIterator->ModifyStyles(EITR_STYLE_TOP);			
		LoadResource();

		//Relocation();
		if(mpBtDrag)
		{
			if(mbVertical)
			{
				//设置纵向
				mfSVScrollBarWidth = mpIterator->GetSizeX();
				LONG lnStyle = ES_SLIDER_BUTTON_STYLE_VER;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpBtDrag,EACT_SLIDERBUTTON_SET_STYLE,&lnStyle,sizeof(LONG));
				CExMessage::PostMessage(mpBtDrag,mpIterator,EACT_SLIDERBUTTON_SET_STYLE,lnStyle);
			}
			else
			{
				//设置横向
				mfSHScrollBarHeigth= mpIterator->GetSizeY();
				LONG lnStyle = ES_SLIDER_BUTTON_STYLE_HOR;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpBtDrag,EACT_SLIDERBUTTON_SET_STYLE,&lnStyle,sizeof(LONG));
				CExMessage::PostMessage(mpBtDrag,mpIterator,EACT_SLIDERBUTTON_SET_STYLE,lnStyle);
			}
		}
		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}
//元素Enbale
ERESULT CEvScrollBar::OnElementEnable(bool nbIsEnable)
{
	if(mpBtUp) mpBtUp->SetEnable(nbIsEnable);
	if(mpBtDown) mpBtDown->SetEnable(nbIsEnable);
	if(mpBtDrag) mpBtDrag->SetEnable(nbIsEnable);
	return ERESULT_SUCCESS;
}
//元素参考尺寸发生变化
ERESULT CEvScrollBar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	Relocation();
	return ERESULT_SUCCESS;
}
//修改元素布局
void CEvScrollBar::Relocation(void)
{
	D2D1_POINT_2F ldPoint;	//修改元素位置

	do 
	{
		BREAK_ON_NULL(mpBtDrag);
	

		ldPoint.x = ldPoint.y = 0.0f;
        //如果有向上按钮
		if(mpBtUp)
		{
			if(mbVertical)
				ldPoint.y = mpBtUp->GetPositionY();
			else
				ldPoint.x =mpBtUp->GetPositionX();
			mpBtUp->SetPosition(ldPoint);
		}	
	    //如果有向下按钮
		if(mpBtDown)
		{	
			if(mbVertical)
				ldPoint.y =mpIterator->GetSizeY() - mpBtDown->GetSizeY();
			else
				ldPoint.x =mpIterator->GetSizeX() - mpBtDown->GetSizeX();
			mpBtDown->SetPosition(ldPoint);
		}
		
		//设置滑槽的位置和大小
		if(mbVertical)
    	{
			ldPoint.x = mpBtDrag->GetPositionX();
			ldPoint.y = 0;
			if(mpBtUp)
				ldPoint.y += mpBtUp->GetSizeY();	//滚动条
			
			FLOAT lfHeigth = mpIterator->GetSizeY();
			if(mpBtUp)
				lfHeigth -= mpBtUp->GetSizeY();
			if(mpBtDown)
				lfHeigth -= mpBtDown->GetSizeY();

			mRectSlider.left = ldPoint.x;
			mRectSlider.top = ldPoint.y;
			mRectSlider.right = mRectSlider.left + mpIterator->GetSizeX();
			mRectSlider.bottom = mRectSlider.top + lfHeigth;
			
		}
		else
		{
			ldPoint.x = 0;
			ldPoint.y = mpBtDrag->GetPositionY();
			if(mpBtUp)
				ldPoint.x += mpBtUp->GetSizeX();	//滚动条
			FLOAT lfWidth = mpIterator->GetSizeX();
			if(mpBtUp)
				lfWidth -= mpBtUp->GetSizeX();
			if(mpBtDown)
				lfWidth -= mpBtDown->GetSizeX();
			mRectSlider.left = ldPoint.x;
			mRectSlider.top = ldPoint.y;
			mRectSlider.right = mRectSlider.left + lfWidth;
			mRectSlider.bottom = mRectSlider.top + mpIterator->GetSizeY();
		}

		//设置滑块的位置和滑动范围属性
		if(mpBtDrag)
		{
			if(mbVertical)
			{
				mpBtDrag->SetPosition(mRectSlider.left,mRectSlider.top);
				if(mfMinBarSize == 0.0f)
					mfMinBarSize = mpBtDrag->GetSizeY();
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpBtDrag,EACT_SLIDERBUTTON_SET_SLIDERRECT,&mRectSlider,sizeof(D2D1_RECT_F));
				CExMessage::PostMessage(mpBtDrag,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,mRectSlider);
			}
			else
			{
				mpBtDrag->SetPosition(mRectSlider.left,mpBtDrag->GetPositionY());
				if(mfMinBarSize == 0.0f)
					mfMinBarSize = mpBtDrag->GetSizeX();
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpBtDrag,EACT_SLIDERBUTTON_SET_SLIDERRECT,&mRectSlider,sizeof(D2D1_RECT_F));
				CExMessage::PostMessage(mpBtDrag,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,mRectSlider);
			}
		}



	} while (false);
}

//鼠标按下
ERESULT CEvScrollBar::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
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
			if(npInfo->Position.x < mRectSlider.left ||
				npInfo->Position.x > mRectSlider.right ||
				npInfo->Position.y < mRectSlider.top ||
				npInfo->Position.y > mRectSlider.bottom)
				break;
			FLOAT lf = 0.0;
			if(mbVertical)
			{
				FLOAT lfPos = npInfo->Position.y;
			
				if(lfPos > mRectSlider.bottom - mpBtDrag->GetSizeY())
					lfPos = mRectSlider.bottom - mpBtDrag->GetSizeY();
						

				lf = (lfPos - mRectSlider.top) * mfDestPixelPerScrollPix;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_SLIDERBAR_SET_POSITION,&lf,sizeof(FLOAT));
				CExMessage::PostMessage(mpIterator,mpIterator,EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION,lf);
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_SCROLLBAR_VSCROLL_THUMB_POSITION,lf);
			}
			else
			{
				FLOAT lfPos = npInfo->Position.x;
			
				if(lfPos > mRectSlider.right - mpBtDrag->GetSizeX())
					lfPos = mRectSlider.right - mpBtDrag->GetSizeX();
				lf = (lfPos - mRectSlider.left) * mfDestPixelPerScrollPix;
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_SLIDERBAR_SET_POSITION,&lf,sizeof(FLOAT));
				CExMessage::PostMessage(mpIterator,mpIterator,EACT_SCROLLBAR_HSCROLL_SET_SLIDER_POSTION,lf);
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_SCROLLBAR_HSCROLL_THUMB_POSITION,lf);
			}

			//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),EACT_SLIDERBAR_THUMB_POSITION,&lf,sizeof(FLOAT));
			
		}



		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

// 鼠标落点检测
ERESULT CEvScrollBar::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do 
	{	
		if(mpIterator->IsVisible() == false)
			break;

		//进入非滑块区域
		if(rPoint.x < 0.0f ||
			(UINT)rPoint.x >= mpIterator->GetSizeX() ||
			rPoint.y < 0.0f || 
			(UINT)rPoint.y >= mpIterator->GetSizeY())
			break;

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}
//绘制消息
ERESULT CEvScrollBar::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		// 只有启用时时，才绘画背景条，add by colin
		if(false != mpIterator->IsEnable())
		{
			if(mpBgBitmap != NULL)
				npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
				mpBgBitmap,
				ESPB_DRAWBMP_EXTEND);
		}



		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}