/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "cmmstruct.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "XCtl.h"

#include "EvAnimatorImp.h"

DEFINE_BUILTIN_NAME(Animator)

CEvAnimatorImp::CEvAnimatorImp()
{
	mpFrontFrame=NULL;
	mpSweepBrush = NULL;
}

CEvAnimatorImp::~CEvAnimatorImp()
{

}

ULONG CEvAnimatorImp::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	//LONG llValue;

	do 
	{
		if(npTemplete == NULL)
			return ERESULT_UNSUCCESSFUL;

		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//mpIterator->ModifyStyles(EITR_STYLE_LAZY_UPDATE);

		//llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_EDGE_LEFT,0);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvAnimatorImp::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	do
	{
		luResult = CXuiElement::OnElementCreate(npIterator);
		if(luResult != ERESULT_SUCCESS)
			break;

		//moMask.SetBase(false,D2D1::Point2F(160.0f,120.0f),120.0f,0.0f);
		//moMask.SetAngle(360.0f);

		luResult = ERESULT_SUCCESS;

	}while(false);

	return luResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvAnimatorImp::OnElementDestroy()
{
	CMM_SAFE_RELEASE(mpFrontFrame);
	CMM_SAFE_RELEASE(mpSweepBrush);

	CXuiElement::OnElementDestroy();

	return ERESULT_SUCCESS;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvAnimatorImp::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_ANIMATOR_SET_FRAME:
		{
			if(npMsg->GetInputDataSize() < sizeof(STCTL_ANIMATOR_FRAME))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const PSTCTL_ANIMATOR_FRAME lpFrame = (const PSTCTL_ANIMATOR_FRAME)npMsg->GetInputData();

			luResult = UpdateFrame(lpFrame);
		}
		break;
	case EACT_ANIMATOR_SET_TRANSFORM:
		{
			if(npMsg->GetInputDataSize() < sizeof(STCTL_ANIMATOR_TRANSFORM))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const PSTCTL_ANIMATOR_TRANSFORM lpTrans = (const PSTCTL_ANIMATOR_TRANSFORM)npMsg->GetInputData();

			luResult = SetTransform(lpTrans);
		}
		break;
	default:
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

// 更新帧
ERESULT CEvAnimatorImp::UpdateFrame(PSTCTL_ANIMATOR_FRAME npFrame)
{
	CMM_SAFE_RELEASE(mpFrontFrame);

	// 像素宽度必须能被4整除，目前假定宽度不能大于1万，高度假定不能大于5000
	if((npFrame->PixelWidth/4)*4 != npFrame->PixelWidth || npFrame->PixelWidth > 10000 || npFrame->PixelHeight > 5000)
		return ERESULT_WRONG_PARAMETERS;

	mpFrontFrame = EinkuiGetSystem()->GetAllocator()->CreateBitmapFromMemory(npFrame->PixelWidth,npFrame->PixelHeight,3, npFrame->PixelWidth * 3, npFrame->PixelData);

	if(mpFrontFrame == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	EinkuiGetSystem()->UpdateView();

	return ERESULT_SUCCESS;
}

// 设定变换
ERESULT CEvAnimatorImp::SetTransform(PSTCTL_ANIMATOR_TRANSFORM npTransform)
{
	return ERESULT_SUCCESS;
}

//绘制消息
ERESULT CEvAnimatorImp::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	D2D1_RECT_F ldDestRect;
	D2D1::Matrix3x2F ldFlip(-1,0,0,1,0,0); // 以局部X=0.0f的Y轴翻转
	D2D1::Matrix3x2F ldOld;

	// 获取目标区域
	ldDestRect.left = 0.0f;
	ldDestRect.top = 0.0f;
	ldDestRect.right = mpIterator->GetSizeX();
	ldDestRect.bottom = mpIterator->GetSizeY();

	{
		// 下面建立画刷
 		if(mpSweepBrush ==NULL)
 		{
 			mpSweepBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush,D2D1::ColorF(D2D1::ColorF::Black,1.0f));
 		}

//  		if(mpSweepBrush != NULL)
//  			npPaintBoard->DrawRect(ldDestRect,mpSweepBrush);

	}

	CExMessage::SendMessage(mpIterator->GetParent(),mpIterator,EEVT_ANIMATOR_BEFORE_PAINT,npPaintBoard,NULL,0);

//	moMask.Enter(npPaintBoard->GetD2dRenderTarget());

	// 设置镜像矩阵
	npPaintBoard->GetD2dRenderTarget()->GetTransform(&ldOld);
	ldFlip = ldFlip*D2D1::Matrix3x2F::Translation(ldDestRect.left+ldDestRect.right,0.0f);	// 从镜像位置移动到原来位置
	ldFlip = ldFlip*D2D1::Matrix3x2F(1,0,0,-1,0,0);											// 以局部Y=0.0f的X轴反转
	ldFlip = ldFlip*D2D1::Matrix3x2F::Translation(0.0f,ldDestRect.top+ldDestRect.bottom);	// 从镜像位置移动到原来位置
	ldFlip = ldFlip*ldOld;			// 转换到全局位置
	npPaintBoard->GetD2dRenderTarget()->SetTransform(ldFlip);

	if(mpFrontFrame != NULL)
	{
		//计算绘制矩形大小
		D2D1_RECT_F ldRect;
		ZeroMemory(&ldRect,sizeof(D2D1_RECT_F));
		if(mpIterator->GetSizeX()/mpIterator->GetSizeY() > mpFrontFrame->GetWidth() / mpFrontFrame->GetHeight())
		{
			ldRect.right = (FLOAT)mpFrontFrame->GetWidth();
			ldRect.bottom = mpIterator->GetSizeY()*mpFrontFrame->GetWidth()/mpIterator->GetSizeX();
		}
		else
		{
			ldRect.bottom = (FLOAT)mpFrontFrame->GetHeight();
			ldRect.right = mpIterator->GetSizeX()*mpFrontFrame->GetHeight()/mpIterator->GetSizeY();
		}

		npPaintBoard->DrawBitmap(ldDestRect,ldRect,mpFrontFrame,ESPB_DRAWBMP_NEAREST);
	}
	//else
 	//if(mpSweepBrush != NULL)
 	//	npPaintBoard->FillRect(ldDestRect,mpSweepBrush);

//	moMask.Leave(npPaintBoard->GetD2dRenderTarget());

	CExMessage::SendMessage(mpIterator->GetParent(),mpIterator,EEVT_ANIMATOR_AFTER_PAINT,npPaintBoard,NULL,0);

	// 恢复矩阵
	npPaintBoard->GetD2dRenderTarget()->SetTransform(ldOld);

	return ERESULT_SUCCESS;
}

