/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ThumbnailListItem.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <time.h>

DEFINE_BUILTIN_NAME(ThumbnailListItem)

CThumbnailListItem::CThumbnailListItem(void)
{
	mdwClickTicount = 0;
	mpIterPicture = NULL;
	mpIterPageNumber = NULL;
	mpIterBtClick = NULL;
	mulPageNumber = 0;
	mpLineBrush = NULL;
	mdPicSize.width = mdPicSize.height = 80.0f;
	mbIsCurrent = false;
}


CThumbnailListItem::~CThumbnailListItem(void)
{

}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CThumbnailListItem::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CThumbnailListItem::OnElementDestroy()
{
	CMM_SAFE_RELEASE(mpLineBrush);

	CXuiElement::OnElementDestroy();

	return ERESULT_SUCCESS;
}

ULONG CThumbnailListItem::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;
	IConfigFile* lpProfile = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄
		mpIterPageNumber = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterPageNumber);

		mpIterPicture = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterPicture);

		mpIterBtClick = mpIterator->GetSubElementByID(TL_BT_CLICK);
		BREAK_ON_NULL(mpIterBtClick);

		CExMessage::SendMessageWithText(mpIterPicture, mpIterator, EACT_PICTUREFRAME_CHANGE_PIC, L".\\Pic\\loading.png");

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);
	CMM_SAFE_RELEASE(lpProfile);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置属性
void CThumbnailListItem::SetData(wchar_t* npszFilePath, ULONG nulPageNumber, bool nbIsCurrent)
{
	do 
	{
		BREAK_ON_NULL(npszFilePath);
		mulPageNumber = nulPageNumber;
		mbIsCurrent = nbIsCurrent;

		wchar_t lszNumber[MAX_PATH] = { 0 };
		swprintf_s(lszNumber, MAX_PATH, L"%d", nulPageNumber);
		CExMessage::SendMessageWithText(mpIterPageNumber, mpIterator, EACT_LABEL_SET_TEXT, lszNumber);

		ZeroMemory(&mdPicSize, sizeof(mdPicSize));
		CExMessage::SendMessageWithText(mpIterPicture, mpIterator, EACT_PICTUREFRAME_CHANGE_PIC_FULLPATH, npszFilePath, &mdPicSize, sizeof(D2D1_SIZE_F));
		mpIterPicture->SetSize(mdPicSize);
		if (mdPicSize.width < 10.0f)
		{
			//说明文件坏了，删除它
			DeleteFile(npszFilePath);
		}

		RelocationItem();

	} while (false);
}

//按钮单击事件
ERESULT CThumbnailListItem::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case TL_BT_CLICK:
		{
			//自己被点击
			if (GetTickCount() - mdwClickTicount > 400)
			{
				//为了防止双击
				mdwClickTicount = GetTickCount();
				int liPagenumber = mulPageNumber;
				CExMessage::PostMessage(mpIterator->GetParent(), mpIterator, EEVT_THUMBNAIL_CLICK, liPagenumber);
				//CExMessage::PostMessageWithText(mpIterator->GetParent(), mpIterator, EEVT_ER_LIST_CLICK, mpFileAttrib->FilePath);
			}
			
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


//消息处理函数
//ERESULT CThumbnailListItem::ParseMessage(IEinkuiMessage* npMsg)
//{
//	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
//
//	switch (npMsg->GetMessageID())
//	{
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
void CThumbnailListItem::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}


//绘制
ERESULT CThumbnailListItem::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		leResult = CXuiElement::OnPaint(npPaintBoard);
		if (ERESULT_FAILED(leResult))
			break;

		// 下面建立画刷
		if (mpLineBrush == NULL)
		{
			mpLineBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush, D2D1::ColorF(D2D1::ColorF::Black, 2.0f));
			if (mpLineBrush == NULL)
				break;
		}

		if (mbIsCurrent != false)
			mpLineBrush->SetStrokeWidth(4.0f);
		else
			mpLineBrush->SetStrokeWidth(2.0f);

		D2D1_POINT_2F pt1, pt2;
		
		if (mbIsCurrent != false)
		{	// 左边
			pt1.x = 0.0f;
			pt1.y = 0.0f;
			pt2.x = 1.0f;
			pt2.y = mpIterator->GetSizeY();
			npPaintBoard->DrawLine(pt1, pt2, mpLineBrush);
		}
		//if ((impact & 0x2) == 0)
		{	// 右边
			pt1.x = mpIterator->GetSizeX();
			pt1.y = 0.0f;
			pt2.x = pt1.x - 1.0f;
			pt2.y = mpIterator->GetSizeY();
			npPaintBoard->DrawLine(pt1, pt2, mpLineBrush);
		}
		if (mbIsCurrent != false)
		{	// 上边
			pt1.x = 0.0f;
			pt1.y = 0.0f;
			pt2.x = mpIterator->GetSizeX();
			pt2.y = 1.0f;
			npPaintBoard->DrawLine(pt1, pt2, mpLineBrush);
		}
		//if ((impact & 0x8) == 0)
		{	// 下边
			pt1.x = 0.0f;
			pt1.y = mpIterator->GetSizeY();
			pt2.x = mpIterator->GetSizeX();
			pt2.y = pt1.y - 1.0f;
			npPaintBoard->DrawLine(pt1, pt2, mpLineBrush);
		}

		leResult = ERESULT_SUCCESS;
	} while (false);

	return leResult;
}


//等比缩放，ndSizeDest原始图片大小  ndSizeSrc要绘制的区域
//返回等比缩放的大小
D2D1_RECT_F CThumbnailListItem::GetProportionalScaling(D2D1_SIZE_F ndSizeSrc, D2D1_SIZE_F ndSizeDest)
{
	D2D1_RECT_F ldRetRect;

	// 记录大小的变化量
	float lfSizeX = ndSizeSrc.width;
	float lfSizeY = ndSizeSrc.height;

	// 等比变形前的理论宽度和高度
	float lfBeforeScaledWidth = 0.0f;
	float lfBeforeScaledHeight = 0.0f;

	// 基准斜率
	float lfBaseLineSlope;
	// 当前斜率
	float lfCurLineSlope;

	// 获取改变后的大小
	lfBeforeScaledWidth = ndSizeDest.width + lfSizeX;
	lfBeforeScaledHeight = ndSizeDest.height + lfSizeY;

	{
		// 计算基准斜率,由原始矩形的数据确定，由于起始点始终是(0.0f, 0.0f)，所以斜率就是宽度和高度的比值
		lfBaseLineSlope = ndSizeDest.width / ndSizeDest.height;

		// 计算当前斜率，为当前宽度与高度的比值
		lfCurLineSlope = lfBeforeScaledWidth / lfBeforeScaledHeight;
		// 取绝对值，考虑的翻转的情况
		lfCurLineSlope = lfCurLineSlope > 0 ? lfCurLineSlope : -lfCurLineSlope;
	}

	D2D1_SIZE_F ldNewSize;
	//////////////////////////////////////////////////////////////////////////
	// 下面5句是精华代码，统一计算等比缩放后的变化量，兼顾各种情况
	//////////////////////////////////////////////////////////////////////////
	{

		if (lfCurLineSlope >= lfBaseLineSlope)
		{
			// 取Y轴的缩放比例
			lfSizeY = (fabs(lfSizeY + ndSizeSrc.height) - ndSizeSrc.height);
			ldNewSize = D2D1::SizeF(lfSizeY*lfBaseLineSlope, lfSizeY);
		}
		else
		{
			// 取X轴的缩放比例，保持与X轴对齐
			lfSizeX = (fabs(lfSizeX + ndSizeSrc.width) - ndSizeSrc.width);
			ldNewSize = D2D1::SizeF(lfSizeX, lfSizeX / lfBaseLineSlope);
		}
	}

	ldRetRect.left = 0;
	ldRetRect.top = 0;
	ldRetRect.right = ldNewSize.width;
	ldRetRect.bottom = ldNewSize.height;

	//左右居中显示
	if (ldRetRect.right < (ndSizeSrc.width - 1.0f))
	{
		float lfOffset = (ndSizeSrc.width - ldRetRect.right) / 2.0f;
		ldRetRect.left += lfOffset;
		ldRetRect.right += lfOffset;
	}

	//上下居中显示
	if (ldRetRect.bottom < (ndSizeSrc.height - 1.0f))
	{
		float lfOffset = (ndSizeSrc.height - ldRetRect.bottom) / 2.0f;
		ldRetRect.top += lfOffset;
		ldRetRect.bottom += lfOffset;
	}

	return ldRetRect;
}

//调整图片大小和位置
void CThumbnailListItem::RelocationItem(void)
{
	if (mpIterBtClick != NULL)
	{
		mpIterPageNumber->SetPosition(mpIterator->GetSizeX() - mpIterPageNumber->GetSizeX() - 20.0f, mpIterator->GetSizeY() - mpIterPageNumber->GetSizeY() - 20.0f);

		D2D1_RECT_F ldRect = GetProportionalScaling(D2D1::SizeF(mpIterator->GetSizeX() - 4.0f, mpIterator->GetSizeY() - 4.0f),
			D2D1::SizeF(mpIterPicture->GetSizeX(), mpIterPicture->GetSizeY()));;

		mpIterPicture->SetPosition(ldRect.left + 2.0f, ldRect.top + 2.0f);
		mpIterPicture->SetSize(ldRect.right - ldRect.left, ldRect.bottom - ldRect.top);
	}
}

//元素参考尺寸发生变化
ERESULT CThumbnailListItem::OnElementResized(D2D1_SIZE_F nNewSize)
{
	if (mpIterBtClick != NULL)
	{
		CExMessage::SendMessage(mpIterBtClick, mpIterator, EACT_BUTTON_SET_ACTION_RECT, nNewSize);

		
		RelocationItem();
	}
	
	////mpIterLineOne->SetSize(nNewSize.width, mpIterLineOne->GetSize().height);

	//

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CThumbnailListItem::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}