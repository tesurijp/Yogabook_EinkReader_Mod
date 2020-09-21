/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "XCtl.h"
#include "EvPictureFrameImp.h"


//using namespace D2D1;

DEFINE_BUILTIN_NAME(PictureFrame)

CEvPictureFrame::CEvPictureFrame()
{
	mlMaxFrame = mlCurrentIndex = 0;
	mdFrameSize.width = mdFrameSize.height = 0.0f;
	mulMethod = ESPB_DRAWBMP_LINEAR;	//缩放方式
	mfBeginPos = 0.0f;
	mbIsAutoPlay = false;
	mbIsPlayLoop = false;
	mlPlayTimerElapse = 0;
}
CEvPictureFrame::~CEvPictureFrame()
{

}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvPictureFrame::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CXuiElement::OnElementDestroy();	//调用基类

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CEvPictureFrame::InitOnCreate(
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

		//装载一些必要的配置资源
		LoadResource();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}


//装载配置资源
ERESULT CEvPictureFrame::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;

	do 
	{
		//获取帧信息
		mlMaxFrame = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_PIC_FRAME_COUNT,1);
		mbIsAutoPlay = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_PIC_AUTO_PLAY, 0)==0?false:true;
		mbIsPlayLoop = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_PIC_PLAY_LOOP, 0) == 0 ? false : true;
		mlPlayTimerElapse = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_PIC_PLAY_TIMER_ELAPSE, 0);

		BREAK_ON_FALSE(Resize());

		if (mbIsAutoPlay != false && mlMaxFrame > 1)
		{
			//说明要自动播放动画
			mpIterator->SetTimer(TF_ID_PIC_TIMER_ID_PLAY, MAXULONG64 , mlPlayTimerElapse, NULL);
		}

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//重新计算帧大小,nbResize为真才重新设置大小
bool CEvPictureFrame::Resize(bool nbResize)
{
	bool lbret = false;

	do 
	{
		if (mlMaxFrame <= 0 || mpBgBitmap == NULL)
			break;

		//计算每帧大小
		UINT luiWidth = mpBgBitmap->GetWidth();
		UINT luiHeight = mpBgBitmap->GetHeight();
		mlCurrentIndex = 0;

		mdFrameSize.width = float(luiWidth / mlMaxFrame);
		mdFrameSize.height = (float)luiHeight;

		if (nbResize != false)
		{
			//读取参考尺寸设置
			LONG llWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_WIDTH,0);
			LONG llHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_HEIGHT,0);

			if (llWidth == 0 || llHeight == 0)
				mpIterator->SetSize(mdFrameSize.width,mdFrameSize.height); //如果没有设置参考尺寸，就以计算出来的为准
			else
				mpIterator->SetSize((FLOAT)llWidth,(FLOAT)llHeight);

		}

		if((mpBgBitmap->GetExtnedLineX() != 0 && mpBgBitmap->GetExtnedLineY() != 0)/* && ((mpIterator->GetSizeX() - mdFrameSize.width > 1) || mpIterator->GetSizeY() - mdFrameSize.height > 1)*/)
			mulMethod = ESPB_DRAWBMP_EXTEND;	//如果设置了延展线并且参考尺寸大于实际尺寸，那就使用延展方式

		mfBeginPos = mdFrameSize.width * mlCurrentIndex;	//绘制时的X坐标起始点

		lbret = true;

	} while (false);

	return lbret;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvPictureFrame::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		case EACT_PICTUREFRAME_SET_INDEX:	
			{
				//切换显示帧
				if(npMsg->GetInputDataSize() != sizeof(LONG))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				LONG* lpValue = (LONG*)npMsg->GetInputData();

				luResult = OnChangeIndex(*lpValue);

				break;
			}
		case EACT_PICTUREFRAME_CHANGE_PIC:	
			{
				//更换显示图片,相对路径
				wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

				luResult = OnChangePic(lpValue,false);

				if (mpBgBitmap != NULL && npMsg->GetOutputBufferSize() == sizeof(D2D1_SIZE_F))
				{
					//如果发送者想要得到帧大小
					D2D1_SIZE_F* lpOut = (D2D1_SIZE_F*)npMsg->GetOutputBuffer();
					*lpOut = mdFrameSize;
					npMsg->SetOutputDataSize(sizeof(D2D1_SIZE_F));
				}

				break;
			}
		case EACT_PICTUREFRAME_CHANGE_PIC_FULLPATH:	
			{
				//更换显示图片,全路径
				wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

				luResult = OnChangePic(lpValue,true);

				if (mpBgBitmap != NULL && npMsg->GetOutputBufferSize() == sizeof(D2D1_SIZE_F))
				{
					//如果发送者想要得到帧大小
					D2D1_SIZE_F* lpOut = (D2D1_SIZE_F*)npMsg->GetOutputBuffer();
					(*lpOut).width = mpBgBitmap->GetWidth();
					(*lpOut).height = mpBgBitmap->GetHeight();
					npMsg->SetOutputDataSize(sizeof(D2D1_SIZE_F));
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

//绘制
ERESULT CEvPictureFrame::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
		{
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
				D2D1::RectF(mfBeginPos,0,mfBeginPos + mdFrameSize.width,mdFrameSize.height),
				mpBgBitmap,
				mulMethod
				);
		}

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvPictureFrame::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(/*EITR_STYLE_CONTROL*/NULL,EITR_STYLE_MOUSE);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//切换显示帧,第一帧为1
ERESULT CEvPictureFrame::OnChangeIndex(LONG nlIndex)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(nlIndex <= 0 || nlIndex > mlMaxFrame)
			break;

		mlCurrentIndex = nlIndex - 1;

		mfBeginPos = mdFrameSize.width * mlCurrentIndex;	//绘制时的X坐标起始点

		EinkuiGetSystem()->UpdateView();

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//更换显示图片
ERESULT CEvPictureFrame::OnChangePic(wchar_t* npswPicPath,bool nbIsFullPath)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(npswPicPath == NULL || npswPicPath[0] == UNICODE_NULL)
			break;

		CMM_SAFE_RELEASE(mpBgBitmap);	//去除原来的图片

		mpBgBitmap = EinkuiGetSystem()->GetAllocator()->LoadImageFile(npswPicPath,nbIsFullPath);
		BREAK_ON_NULL(mpBgBitmap);

		mlMaxFrame = 1;	//目前只支持切换为一帧的图

		Resize(false);

		EinkuiGetSystem()->UpdateView();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

// 鼠标落点检测
ERESULT CEvPictureFrame::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do 
	{
		BREAK_ON_NULL(mpBgBitmap);

		float lfX = mpIterator->GetSizeX() * mlCurrentIndex;  	//从哪个位置开始显示

		if(rPoint.x < 0.0f || (UINT)rPoint.x >= mpIterator->GetSizeX() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpIterator->GetSizeY())
			break;

		//????通过像素Alpha值检测

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}

//通知元素【显示/隐藏】发生改变
ERESULT CEvPictureFrame::OnElementShow(bool nbIsShow)
{
	if (mbIsAutoPlay != false)
	{
		//如果是自动播放动画的元素
		if (nbIsShow == false)
		{
			//如果进入隐藏状态，就关掉定时器
			mpIterator->KillTimer(TF_ID_PIC_TIMER_ID_PLAY);
		}
		else
		{
			//开始动画，并且显示第1帧
			mpIterator->SetTimer(TF_ID_PIC_TIMER_ID_PLAY, MAXULONG64, mlPlayTimerElapse, NULL);
			mlCurrentIndex = 1;
		}
	}
	

	return ERESULT_SUCCESS;
}

//定时器
void CEvPictureFrame::OnTimer(PSTEMS_TIMER npStatus)
{
	do
	{
		if (npStatus->TimerID == TF_ID_PIC_TIMER_ID_PLAY)
		{
			//播放动画

			if (mlCurrentIndex < mlMaxFrame - 1)
			{
				mlCurrentIndex++;
			}
			else
			{
				mlCurrentIndex = 1;	//超过最大页后，返回第一帧显示
				if (mbIsPlayLoop == false)
					mpIterator->KillTimer(TF_ID_PIC_TIMER_ID_PLAY); //如果是非循环，就只播放一次
			}
			OnChangeIndex(mlCurrentIndex);

		}

	} while (false);

}