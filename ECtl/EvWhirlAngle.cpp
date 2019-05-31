/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "EvWhirlAngle.h"

DEFINE_BUILTIN_NAME(WhirlAngle)

	CEvWhirlAngleImp::CEvWhirlAngleImp(void)
{
	mpoWhirlBg = NULL;
	mpoWhirlDot = NULL;

	memset(&mdWhirlCenter, 0, sizeof(D2D1_POINT_2F));
	memset(&mdDotPoint, 0, sizeof(D2D1_POINT_2F));

	mdbRadian = 0.0f;
}


CEvWhirlAngleImp::~CEvWhirlAngleImp(void)
{
	
}


ULONG CEvWhirlAngleImp::InitOnCreate(
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

		mpIterator->ModifyStyles(EITR_STYLE_ALL_DRAG);

		LoadResource();

		leResult = ERESULT_SUCCESS;
	}while(false);


	return leResult;
}

//绘制消息
ERESULT CEvWhirlAngleImp::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
			mpBgBitmap,ESPB_DRAWBMP_LINEAR);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}



ERESULT CEvWhirlAngleImp::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_WHIRLANGLE_GET_ANGLE:
		{
			if(npMsg->GetOutputBufferSize() != sizeof(double)
				|| NULL == npMsg->GetOutputBuffer())
				break;

			double* lppAngle = (double*)npMsg->GetOutputBuffer();
			*lppAngle = RadianToAngle(mdbRadian);

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EACT_WHIRLANGLE_SET_ANGLE:
		{
			if(npMsg->GetInputDataSize() != sizeof(double)
				|| NULL == npMsg->GetInputData())
				break;

			double ldbAngle = *(double*)npMsg->GetInputData();
			mdbRadian = AngleToRadian(ldbAngle);
			UpdateDotPosition(mdbRadian);

			luResult = ERESULT_SUCCESS;
			luResult = ERESULT_SUCCESS;
		}
		break;

	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		//luResult = ERESULT_UNEXPECTED_MESSAGE;	// 这儿没有基类，派生本类时，删除本句；
	}

	return luResult;
}

// 鼠标落点检测
ERESULT CEvWhirlAngleImp::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if(mpoWhirlBg != NULL && !(rPoint.x < 0.0f || rPoint.x >= mpoWhirlBg->GetSizeX() || rPoint.y < 0.0f || rPoint.y >= mpoWhirlBg->GetSizeY()))
	{
		//SetPicIndex(3);
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}

	return luResult;
}
// 
// //鼠标按下
// ERESULT CEvWhirlAngleImp::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
// {
// 	ERESULT lResult = ERESULT_UNSUCCESSFUL;
// 
// 	do
// 	{
// 
// 		lResult = ERESULT_SUCCESS;
// 	}while(false);
// 
// 	return lResult;
// }


ERESULT CEvWhirlAngleImp::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);
		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		mdbRadian = GetRadian(npInfo->CurrentPos);
		if(-1 != mdbRadian)
		{
			UpdateDotPosition(mdbRadian);

			double ldbAngle = RadianToAngle(mdbRadian);
			PostMessageToParent(EEVT_WHIRLANGEL_ANGLE_CHANGE, ldbAngle);
		}

		EinkuiGetSystem()->UpdateView();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽开始
ERESULT CEvWhirlAngleImp::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);
		if(false == mpIterator->IsEnable())
			break;

		if ((npInfo->ActKey&MK_LBUTTON) == 0)	//只有左键可以拖动
			break;

		if(npInfo->DragOn != mpIterator) //只有在自己上面才可以
			break;

		// 拖动开始
		SetPicIndex(4);

		PostMessageToParent(EEVT_WHIRLANGEL_ANGLE_CHANGE_BEGIN, NULL);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽结束
ERESULT CEvWhirlAngleImp::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		BREAK_ON_NULL(npInfo);

		// 拖动结束
		// 如果当前鼠标还在对象内，则转入焦点态
		if(mpoWhirlBg != NULL && !(npInfo->CurrentPos.x < 0.0f || npInfo->CurrentPos.x >= mpoWhirlBg->GetSizeX() 
			|| npInfo->CurrentPos.y < 0.0f || npInfo->CurrentPos.y >= mpoWhirlBg->GetSizeY()))
		{
			SetPicIndex(3);
		}
		else
		{
			SetPicIndex(2);
		}

		PostMessageToParent(EEVT_WHIRLANGEL_ANGLE_CHANGE_END, NULL);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;


}

//鼠标进入或离开
void CEvWhirlAngleImp::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	do 
	{
		if(false == mpIterator->IsEnable())
			break;

		if(0 != npState->State)		// 获得焦点
		{
			SetPicIndex(3);
		}
		else
		{
			SetPicIndex(2);
		}
	} while (false);
}

//启用或禁用
ERESULT CEvWhirlAngleImp::OnElementEnable(bool nbIsShow)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(false != nbIsShow)
		{
			SetPicIndex(2);
		}
		else
		{
			SetPicIndex(1);
		}

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}


void CEvWhirlAngleImp::LoadResource()
{
	mpoWhirlBg = mpIterator->GetSubElementByID(ID_OF_SUB_WHIRL_BG);
	mpoWhirlDot = mpIterator->GetSubElementByID(ID_OF_SUB_WHIRL_DOT);

	if(NULL != this->mpTemplete)
	{
		mdWhirlCenter.x = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_WHIRLANGLE_WHIRL_CENTER_X, -1);
		if(-1 == mdWhirlCenter.x
			&& NULL != mpoWhirlBg)
		{
			mdWhirlCenter.x = CExFloat::Round(mpoWhirlBg->GetSizeX() / 2);
		}

		mdWhirlCenter.y = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_WHIRLANGLE_WHIRL_CENTER_Y, -1);
		if(-1 == mdWhirlCenter.y
			&& NULL != mpoWhirlBg)
		{
			mdWhirlCenter.y = CExFloat::Round(mpoWhirlBg->GetSizeY() / 2);
		}

		mdDotPoint.x = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_WHIRLANGLE_DOT_POINT_X, -1);
		if(-1 == mdDotPoint.x
			&& NULL != mpoWhirlDot)
		{
			mdDotPoint.x = CExFloat::Round((mpoWhirlDot->GetSizeX() - mpoWhirlDot->GetPositionX()) / 2);
		}

		mdDotPoint.y = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_WHIRLANGLE_DOT_POINT_Y, -1);
		if(-1 == mdDotPoint.y
			&& NULL != mpoWhirlDot)
		{
			mdDotPoint.y = CExFloat::Round((mpoWhirlDot->GetSizeY() - mpoWhirlDot->GetPositionY()) / 2);
		}

		SetPicIndex(2);
	}
}

// 描述：
//		更改图片帧数
void CEvWhirlAngleImp::SetPicIndex(
	IN size_t niIndex			// 索引
	)
{
	if(NULL != mpoWhirlBg)
	{
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpoWhirlBg, EACT_PICTUREFRAME_SET_INDEX, &niIndex, sizeof(size_t), NULL, 0);
	}

	if(NULL != mpoWhirlDot)
	{
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpoWhirlDot, EACT_PICTUREFRAME_SET_INDEX, &niIndex, sizeof(size_t), NULL, 0);
	}
}


// 描述：
//		通过当前位置，计算旋转了的弧度值
double CEvWhirlAngleImp::GetRadian(
	IN D2D1_POINT_2F nfCrtPoint
	)
{
	double ldbResult = -1;
	do 
	{
		// 计算三边长度
		double ldbCrtPointToCenter = GetDistance(nfCrtPoint, mdWhirlCenter);

		double ldbDotPointToCenter = GetDistance(mdDotPoint, mdWhirlCenter);

		double ldbCrtPointToDotPoint = GetDistance(nfCrtPoint, mdDotPoint);

		if(0 == ldbCrtPointToCenter || 0 == ldbDotPointToCenter || 0 == ldbCrtPointToDotPoint)
			break;

		// 利用余弦定理计算出夹角的余弦值
		double ldb = ldbCrtPointToCenter * ldbCrtPointToCenter + ldbDotPointToCenter * ldbDotPointToCenter
			- ldbCrtPointToDotPoint * ldbCrtPointToDotPoint;
		double ldbCosAngle = ldb / (2 * ldbCrtPointToCenter * ldbDotPointToCenter);

		ldbResult = acos(ldbCosAngle);

		// 如果在圆心以下，则为钝角
		if(nfCrtPoint.y > mdWhirlCenter.y)
		{
			ldbResult = 2 * CONST_VALUE_PI - ldbResult;
		}

	} while (false);
	return ldbResult;
}

// 描述：
//		获取两点之间的距离
double CEvWhirlAngleImp::GetDistance(
	IN D2D1_POINT_2F ndfPointA,
	IN D2D1_POINT_2F ndfPointB
	)
{
	double llResult = -1;
	do 
	{
		// 计算三边长度
		llResult = sqrt((ndfPointA.y-ndfPointB.y)*(ndfPointA.y-ndfPointB.y)
			+ (ndfPointA.x-ndfPointB.x)*(ndfPointA.x-ndfPointB.x));

	} while (false);
	return llResult;

}

// 描述：
//		更新旋转点位置
void CEvWhirlAngleImp::UpdateDotPosition(
	IN double ndbRadian
	)
{
	D2D1_POINT_2F ldPosition;
	double ldbRadian = 2 * CONST_VALUE_PI - ndbRadian;			// 因为角度变化是逆时针的。所以这里取反角
	ldPosition.x = mdWhirlCenter.x + (mdDotPoint.x - mdWhirlCenter.x) * (float)cos(ldbRadian) 
		- (mdDotPoint.y - mdWhirlCenter.y) * (float)sin(ldbRadian);
	ldPosition.y = mdWhirlCenter.y + (mdDotPoint.x - mdWhirlCenter.x) * (float)sin(ldbRadian) 
		+ (mdDotPoint.y - mdWhirlCenter.y) * (float)cos(ldbRadian);

	D2D1_POINT_2F ldLeftTop;
	ldLeftTop.x = ldPosition.x - mpoWhirlDot->GetSizeX() / 2;
	ldLeftTop.y = ldPosition.y - mpoWhirlDot->GetSizeY() / 2;
	if(NULL != mpoWhirlDot)
		mpoWhirlDot->SetPosition(ldLeftTop);
}

// 描述：
//		弧度转换成角度
double CEvWhirlAngleImp::RadianToAngle(
	IN double ndbRandian
	)
{
	return ndbRandian * 180.0 / CONST_VALUE_PI;
}

// 描述：
//		角度转换成弧度
double CEvWhirlAngleImp::AngleToRadian(
	IN double ndbAngle
	)
{
	return ndbAngle * CONST_VALUE_PI / 180.0;
}