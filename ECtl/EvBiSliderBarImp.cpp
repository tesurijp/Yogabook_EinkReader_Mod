/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvBiSliderBarImp.h"


//using namespace D2D1;

DEFINE_BUILTIN_NAME(BiSliderBar)

CEvBiSliderBar::CEvBiSliderBar()
{
	mulMethod = ESPB_DRAWBMP_LINEAR;	//缩放方式
//	mpBarButton = NULL;
	mbIsMouseFocus = false;	
	mbIsPressed = false;
	mnVertical = 0;
	mfLabelLength = 40.0f;
}
CEvBiSliderBar::~CEvBiSliderBar()
{
	//if(mpBarButton)
	//	mpBarButton->Release();
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvBiSliderBar::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;
	do
	{
		CXuiElement::OnElementDestroy();	//调用基类
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CEvBiSliderBar::InitOnCreate(
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

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_LEFT_SLIDERBUTTON);			
		if(lpKey)
			mpDragButtonLeft = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);
		BREAK_ON_NULL(mpDragButtonLeft);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_RIGHT_SLIDERBUTTON);			
		if(lpKey)
			mpDragButtonRigth = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);
		BREAK_ON_NULL(mpDragButtonRigth);

		lpKey = npTemplete->GetSubKey(BI_SLIDER_BAR_LEFT_PICTURE);			
		if(lpKey)
			mpLeftBarPicture = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_RIGHT_PICTURE);			
		if(lpKey)
			mpRightBarPicture = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_MID_SLIDERBUTTON);			
		if(lpKey)
			mpDragButtonMid = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);

		lpKey = npTemplete->GetSubKey(SLIDER_BAR_MID_LABEL);			
		if(lpKey)
			mpDragButtonMidLable = EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpKey);

		//装载一些必要的配置资源
		LoadResource();

	

		leResult = ERESULT_SUCCESS;

	} while (false);
	return leResult;
}
//元素Enbale
ERESULT CEvBiSliderBar::OnElementEnable(bool nbIsEnable)
{
	if(mpDragButtonLeft) mpDragButtonLeft->SetEnable(nbIsEnable);
	if(mpDragButtonRigth) mpDragButtonRigth->SetEnable(nbIsEnable);	
	if(mpDragButtonMid) mpDragButtonMid->SetEnable(nbIsEnable);
//	if(mpDragButtonMid) mpDragButtonMid->SetEnable(nbIsEnable);

	//if (nbIsEnable)
	//{
	//	if (mpIterator) mpIterator->ModifyStyles(EITR_STYLE_MOUSE, NULL);
	//}
	//else
	//{
	//	if (mpIterator) mpIterator->ModifyStyles(NULL, EITR_STYLE_MOUSE);
	//}

	return ERESULT_SUCCESS;
}
//装载配置资源
ERESULT CEvBiSliderBar::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;
	do 
	{	
		//装载LeftOrBack的图片 

		//装载Min Max

		//设置Bar的Rect
		leResult = ERESULT_SUCCESS;
	} while (false);

	return leResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvBiSliderBar::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
	do 
	{
		BREAK_ON_NULL(npMsg);
		switch (npMsg->GetMessageID())
		{
		case EACT_BISLIDERBAR_SET_MIDLABEL_LEGTH:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
				mfLabelLength = * lpValue;
				break;
			}
		
		case EACT_BISLIDERBAR_SET_MIDLABEL_VALUE:
			{
				wchar_t* lpswText = (wchar_t*)npMsg->GetInputData();
				CExMessage::SendMessageWithText(mpDragButtonMidLable,mpIterator,EACT_LABEL_SET_TEXT,lpswText,NULL,0);
				break;
			}
		case EACT_SLIDERBUTTON_DRAG_START:
			{
				//设置左右的滑动区域
				if(mpDragButtonLeft && mpDragButtonRigth && mpDragButtonMid)
				{
					FLOAT lfPositionLeft,lfPositionRight,lfPositionMid ;
					lfPositionLeft = mpDragButtonLeft->GetPositionX()+mpDragButtonLeft->GetSizeX()/2 - mBarRect.left ;
					lfPositionRight = mpDragButtonRigth->GetPositionX()+mpDragButtonRigth->GetSizeX()/2 - mBarRect.left ;
					lfPositionMid = mpDragButtonMid->GetPositionX() - mBarRect.left ;
					D2D1_RECT_F lLeftSliderRect;
					lLeftSliderRect.left = mSliderRect.left;
					lLeftSliderRect.top = mSliderRect.top;
					lLeftSliderRect.bottom = mSliderRect.bottom;
					lLeftSliderRect.right = lfPositionRight+mpDragButtonLeft->GetSizeX()/2;
					CExMessage::PostMessage(mpDragButtonLeft,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,lLeftSliderRect);

					D2D1_RECT_F lRightSliderRect;
					lRightSliderRect.left = lfPositionLeft-mpDragButtonRigth->GetSizeX()/2;
					lRightSliderRect.top = mSliderRect.top;
					lRightSliderRect.bottom = mSliderRect.bottom;
					lRightSliderRect.right = mSliderRect.right;
					CExMessage::PostMessage(mpDragButtonRigth,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,lRightSliderRect);
		
					//设置label的位置
					if(mpDragButtonMidLable && !mnVertical)
					{
						if(mpDragButtonMid->GetPositionX() < (mBarRect.right-mBarRect.left) / 2)
						{
							mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX() + 5,mpDragButtonMidLable->GetPositionY());
						}
						else
						{
							mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX()-mfLabelLength- 5 ,mpDragButtonMidLable->GetPositionY());
						}
					}
				}
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator, EACT_BISLIDERBAR_DRAG_START, 0);
				break;
			}
		case EACT_SLIDERBUTTON_DRAG_END:
			{
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator, EACT_BISLIDERBAR_DRAG_END, 0);
				break;
			}
		case EACT_SLIDERBUTTON_DRAGING:
			{
				//计算Position,给父窗口发定位消息
				//定位DragButton的中心点位置
				if(mpDragButtonLeft && mpDragButtonRigth && mpDragButtonMid)
				{
					FLOAT lfPositionLeft,lfPositionRight,lfPositionMid ;
					if(mnVertical)
					{
						lfPositionLeft = mpDragButtonLeft->GetPositionY() - mBarRect.top ;	
						lfPositionRight = mpDragButtonRigth->GetPositionY() - mBarRect.top ;	
						lfPositionMid = mpDragButtonMid->GetPositionY() - mBarRect.top ;	
					}
					else
					{
						lfPositionLeft = mpDragButtonLeft->GetPositionX()+mpDragButtonLeft->GetSizeX()/2 - mBarRect.left ;
						lfPositionRight = mpDragButtonRigth->GetPositionX()+mpDragButtonRigth->GetSizeX()/2 - mBarRect.left ;
						lfPositionMid = mpDragButtonMid->GetPositionX() - mBarRect.left ;

					}

					//如果有LeftorUP picture 则应该放大或缩小该图像
					if(mpLeftBarPicture)
					{
						if(mnVertical)
							mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(),lfPositionLeft);
						else
						{
							mpLeftBarPicture->SetSize(lfPositionLeft,mpLeftBarPicture->GetSizeY());
							mpLeftBarPicture->SetPosition(mBarRect.left,mpLeftBarPicture->GetPositionY());
						}
					}
					lfPositionLeft *= mfDestPixelPerScrollPixLeft;


	                //如果有LeftorUP picture 则应该放大或缩小该图像
					if(mpRightBarPicture)
					{
						if(mnVertical)
							mpRightBarPicture->SetSize(mpRightBarPicture->GetSizeX(),lfPositionRight);
						else
						{
							mpRightBarPicture->SetSize(mBarRect.right- lfPositionRight,mpRightBarPicture->GetSizeY());
							mpRightBarPicture->SetPosition(lfPositionRight+mBarRect.left,mpRightBarPicture->GetPositionY());
						}
					}
					lfPositionRight *= mfDestPixelPerScrollPixRight;

					//设置label的位置
					if(mpDragButtonMidLable && !mnVertical)
					{
						if(mpDragButtonMid->GetPositionX() < (mBarRect.right-mBarRect.left) / 2)
						{
							mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX() + 5,mpDragButtonMidLable->GetPositionY());
						}
						else
						{
							mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX()-mfLabelLength -5 ,mpDragButtonMidLable->GetPositionY());
						}
					}
					lfPositionMid *= mfDestPixelPerScrollPixMid;
					_sBiSliderBarStruct lsPos;
					lsPos.mfLeftPos = lfPositionLeft;
					lsPos.mfRightPos = lfPositionRight;
					lsPos.mfMidPos = lfPositionMid;
					//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),EACT_BISLIDERBAR_THUMB_POSITION,&lsPos,sizeof(_sBiSliderBarStruct));
					//CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_BISLIDERBAR_THUMB_POSITION,lsPos);
					CExMessage::SendMessageW(mpIterator->GetParent(),mpIterator,EACT_BISLIDERBAR_THUMB_POSITION,lsPos);
				}
				break;
			}
		case EACT_BISLIDERBAR_SET_RANGE:
			{
				if(npMsg->GetInputDataSize() != sizeof(_sBiSliderBarStruct))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				_sBiSliderBarStruct* lpValue = (_sBiSliderBarStruct*)npMsg->GetInputData();
				BREAK_ON_NULL(lpValue);
				//FLOAT lfy = (*lpValue)/mfDestPixelPerScrollPix;
				SetDeltaSizeLeft(lpValue->mfLeftPos);
				SetDeltaSizeRigth(lpValue->mfRightPos);
				SetDeltaSizeMid(lpValue->mfMidPos);
				break;
			}
		case EACT_BISLIDERBAR_SET_POS:
			{
				if(npMsg->GetInputDataSize() != sizeof(_sBiSliderBarStruct))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				_sBiSliderBarStruct* lpValue = (_sBiSliderBarStruct*)npMsg->GetInputData();
				BREAK_ON_NULL(lpValue);

				FLOAT lfPositionLeft;
				FLOAT lfDeltaLeft = lpValue->mfLeftPos / mfDestPixelPerScrollPixLeft;
				if(mnVertical)
				{
					lfPositionLeft = mBarRect.top + lfDeltaLeft;	
					mpDragButtonLeft->SetPosition(mpDragButtonLeft->GetPositionX(),lfPositionLeft - mpDragButtonLeft->GetSizeY()/2);
				}
				else
				{
					lfPositionLeft =  mBarRect.left + lfDeltaLeft ;
					mpDragButtonLeft->SetPosition(lfPositionLeft-mpDragButtonLeft->GetSizeX()/2,mpDragButtonLeft->GetPositionY());
				}
				//如果有LeftorUP picture 则应该放大或缩小该图像
				if(mpLeftBarPicture)
				{
					if(mnVertical)
						mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(),lfPositionLeft - mBarRect.top);
					else
						mpLeftBarPicture->SetSize(lfPositionLeft - mBarRect.left,mpLeftBarPicture->GetSizeY());
				}
			
				FLOAT lfPositionRight;
				FLOAT lfDeltaRight = lpValue->mfRightPos / mfDestPixelPerScrollPixRight;
				if(mnVertical)
				{
					lfPositionRight = mBarRect.top + lfDeltaRight;	
					mpDragButtonRigth->SetPosition(mpDragButtonRigth->GetPositionX(),lfDeltaRight - mpDragButtonRigth->GetSizeY()/2);
				}
				else
				{
					lfPositionRight =  mBarRect.left + lfDeltaRight ;
					mpDragButtonRigth->SetPosition(lfDeltaRight-mpDragButtonLeft->GetSizeX()/2,mpDragButtonRigth->GetPositionY());
				}
				//如果有LeftorUP picture 则应该放大或缩小该图像
				if(mpRightBarPicture)
				{

					if(mnVertical)
						mpRightBarPicture->SetSize(mpDragButtonRigth->GetSizeX(),lfPositionRight - mBarRect.top);
					else
					{
						mpRightBarPicture->SetSize(mBarRect.right - lfPositionRight,mpRightBarPicture->GetSizeY());
					    mpRightBarPicture->SetPosition(lfPositionRight+mBarRect.left,mpRightBarPicture->GetPositionY());
					}
				}

				FLOAT lfPositionMid;
				FLOAT lfDeltaMid = lpValue->mfMidPos / mfDestPixelPerScrollPixMid;
				if(mnVertical)
				{
					lfPositionMid = mBarRect.top + lfDeltaMid;	
					mpDragButtonMid->SetPosition(mpDragButtonMid->GetPositionX(),lfDeltaRight);
				}
				else
				{
					lfPositionMid =  mBarRect.left + lfDeltaMid ;
					mpDragButtonMid->SetPosition(lfDeltaMid,mpDragButtonMid->GetPositionY());
				}

				//设置label的位置
				if(mpDragButtonMidLable && !mnVertical)
				{
					if(mpDragButtonMid->GetPositionX() < (mBarRect.right-mBarRect.left) / 2)
					{
						mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX() + 5,mpDragButtonMidLable->GetPositionY());
					}
					else
					{
						mpDragButtonMidLable->SetPosition(mpDragButtonMid->GetPositionX()-mfLabelLength -5 ,mpDragButtonMidLable->GetPositionY());
					}
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
bool CEvBiSliderBar:: SetDeltaSizeLeft(FLOAT nfSize)
{
	mfMaxLeft = nfSize;

	//先得到最大的可滚动像素
	FLOAT mfMaxScrollPixelLeft = 0.0;
	if(mnVertical)
		mfMaxScrollPixelLeft = mBarRect.bottom - mBarRect.top  ;
	else
		mfMaxScrollPixelLeft = mBarRect.right - mBarRect.left ;

	//计算目标像素和滚动像素的对应关系
	mfDestPixelPerScrollPixLeft= mfMaxLeft / mfMaxScrollPixelLeft;

	return true;
}


//设置滚动的范围
bool CEvBiSliderBar:: SetDeltaSizeRigth(FLOAT nfSize)
{
	mfMaxRight = nfSize;

	//先得到最大的可滚动像素
	FLOAT mfMaxScrollPixelRight= 0.0;
	if(mnVertical)
		mfMaxScrollPixelRight = mBarRect.bottom - mBarRect.top  ;
	else
		mfMaxScrollPixelRight = mBarRect.right - mBarRect.left ;

	//计算目标像素和滚动像素的对应关系
	mfDestPixelPerScrollPixRight = mfMaxRight / mfMaxScrollPixelRight;

	return true;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvBiSliderBar::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;		

		if(mpDragButtonLeft && mpDragButtonRigth)
		{
			LONG lnStyle = ES_SLIDER_BUTTON_STYLE_HOR;
			if(mnVertical)
				lnStyle = ES_SLIDER_BUTTON_STYLE_VER;
			//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpDragButton,EACT_SLIDERBUTTON_SET_STYLE,&lnStyle,sizeof(LONG));
			CExMessage::PostMessage(mpDragButtonLeft,mpIterator,EACT_SLIDERBUTTON_SET_STYLE,lnStyle);
			CExMessage::PostMessage(mpDragButtonRigth,mpIterator,EACT_SLIDERBUTTON_SET_STYLE,lnStyle);
			if(mpDragButtonMid)
				CExMessage::PostMessage(mpDragButtonMid,mpIterator,EACT_SLIDERBUTTON_SET_STYLE,lnStyle);
			//获取BarRect
			mBarRect.left = mpBarPicture->GetPositionX() ;
			mBarRect.top = mpBarPicture->GetPositionY();
			mBarRect.right = mBarRect.left + mpBarPicture->GetSizeX();
			mBarRect.bottom =mBarRect.top + mpBarPicture->GetSizeY();

			//设置可以滑动的范围
			BREAK_ON_NULL(mpBarPicture);
			mSliderRect.left = mpBarPicture->GetPositionX() -  mpDragButtonLeft->GetSizeX()/2 ;
			mSliderRect.top = mpBarPicture->GetPositionY();
			mSliderRect.right = mpBarPicture->GetPositionX() + mpBarPicture->GetSizeX() + mpDragButtonLeft->GetSizeX()/2;
			mSliderRect.bottom =mSliderRect.top + mpBarPicture->GetSizeY();
			CExMessage::PostMessage(mpDragButtonRigth,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,mSliderRect);
			CExMessage::PostMessage(mpDragButtonLeft,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,mSliderRect);
			if(mpDragButtonMid)
				CExMessage::PostMessage(mpDragButtonMid,mpIterator,EACT_SLIDERBUTTON_SET_SLIDERRECT,mBarRect);
			//如果有LeftOrUP Picture则大小设为0
			if(mpLeftBarPicture)
			{
				if(mnVertical)
					mpLeftBarPicture->SetSize(mpLeftBarPicture->GetSizeX(),0.0);
				else
					mpLeftBarPicture->SetSize(0.0,mpLeftBarPicture->GetSizeY());
			}	
			if(mpRightBarPicture)
			{
				if(mnVertical)
					mpRightBarPicture->SetSize(mpRightBarPicture->GetSizeX(),0.0);
				else
					mpRightBarPicture->SetSize(0.0,mpRightBarPicture->GetSizeY());
			}	
		}
		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


// 鼠标落点检测
ERESULT CEvBiSliderBar::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
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
bool CEvBiSliderBar::SetRangMid(FLOAT nMin,FLOAT nMax)
{
	mfMinMid = nMin;
	mfMaxMid = nMax;
	return true;
}
bool CEvBiSliderBar::SetPosMid(FLOAT nPos)
{
	mfPosMid = nPos;
	return true;
}
bool CEvBiSliderBar::SetDeltaSizeMid(FLOAT nfSize)
{
	mfMaxMid = nfSize;

	//先得到最大的可滚动像素
	FLOAT mfMaxScrollPixelMid= 0.0;
	if(mnVertical)
		mfMaxScrollPixelMid = mBarRect.bottom - mBarRect.top  ;
	else
		mfMaxScrollPixelMid = mBarRect.right - mBarRect.left ;

	//计算目标像素和滚动像素的对应关系
	mfDestPixelPerScrollPixMid = mfMaxMid / mfMaxScrollPixelMid;

	return true;
}
bool CEvBiSliderBar::SetRangeLeft(FLOAT nMin,FLOAT nMax)
{
	mfMinLeft = nMin;
	mfMaxLeft = nMax;
	return true;
}
bool CEvBiSliderBar::SetRangeRight(FLOAT nMin,FLOAT nMax)
{
	mfMinRigth = nMin;
	mfMaxRight = nMax;
	return true;
}

bool CEvBiSliderBar::SetPosLeft(FLOAT nPos)
{
	mfPosLeft = nPos;
	return false;
}

bool CEvBiSliderBar::SetPosRight(FLOAT nPos)
{
	mfPosRight = nPos;
	return false;
}
//鼠标按下
ERESULT CEvBiSliderBar::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if(mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if(MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		//if (npInfo->Presssed != false)
		//{
		//	//鼠标按下，如果不落在mBarRect 则处理
		//	if(npInfo->Position.x < mBarRect.left ||
		//			npInfo->Position.x > mBarRect.right ||
		//			npInfo->Position.y < mBarRect.top ||
		//			npInfo->Position.y > mBarRect.bottom)
		//			break;
		//	FLOAT lf = 0.0;
		//	if(mnVertical)
		//	{
		//		lf = (npInfo->Position.y - mBarRect.top) * mfDestPixelPerScrollPix;
		//		//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_BiSliderBar_SET_POSITION,&lf,sizeof(FLOAT));
		//		CExMessage::PostMessage(mpIterator,mpIterator,EACT_SLIDERBAR_SET_POSITION,lf);
		//	}
		//	else
		//	{
		//		lf = (npInfo->Position.x - mBarRect.left) * mfDestPixelPerScrollPix;
		//		//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator,EACT_BiSliderBar_SET_POSITION,&lf,sizeof(FLOAT));
		//		CExMessage::PostMessage(mpIterator,mpIterator,EACT_SLIDERBAR_SET_POSITION,lf);
		//	}

		//	//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(mpIterator->GetParent(),EACT_BiSliderBar_THUMB_POSITION,&lf,sizeof(FLOAT));
		//	CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EACT_SLIDERBAR_THUMB_POSITION,lf);
		//}



		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//绘制消息
ERESULT CEvBiSliderBar::OnPaint(IEinkuiPaintBoard* npPaintBoard)
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