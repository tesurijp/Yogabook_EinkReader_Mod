/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "EdViewCtl.h"

#ifdef _MUPDF_READER_TESTER
#include "FloatH.h"

#endif//_MUPDF_READER_TESTER



CEdViewCtl::CEdViewCtl()
{
	ED_SIZE_CLEAN(mViewPort);
	ED_SIZE_CLEAN(mImageInit);
	ED_SIZE_CLEAN(mImageReal);
	mUserRatio = 1.0f;
	mGap = 0.0f;

	mViewPortModified = true;
	mUserRatioModified = true;
	mImageInitModified = true;
}


CEdViewCtl::~CEdViewCtl()
{
}

void CEdViewCtl::SetViewPort(const ED_SIZE& viewPort)
{
	mViewPort.x = viewPort.x;
	mViewPort.y = viewPort.y;

	mViewPortModified = true;
}

void CEdViewCtl::SetUserRatio(float32 userRatio)
{
	mUserRatio = userRatio;
	ED_SIZE_CLEAN(mImageReal);

	mUserRatioModified = true;
}

void CEdViewCtl::SetImageInit(const ED_SIZE& imageInit)
{
	mImageInit.x = imageInit.x;
	mImageInit.y = imageInit.y;
	//mFocusOn.x = 0.5f;
	//mFocusOn.y = 0.5f;
	ED_SIZE_CLEAN(mImageReal);

	mImageInitModified = true;
}

void CEdViewCtl::Move(const ED_POINT& offset)
{
	// 计算目标图像大小
	ED_SIZEF imageExtSize;

	if (mImageReal.x > 0 && mImageReal.y > 0)
	{
		imageExtSize.x = (float)mImageReal.x;
		imageExtSize.y = (float)mImageReal.y;
	}
	else
	{
		imageExtSize.x = mImageInit.x*mRealRatio + mGap;
		imageExtSize.y = mImageInit.y*mRealRatio;
	}

	if((float32)mViewPort.x < imageExtSize.x)
		mFocusOn.x -= offset.x / (imageExtSize.x);	// 焦点和页面的移动方向是相反的
	if((float32)mViewPort.y < imageExtSize.y)
		mFocusOn.y -= offset.y / (imageExtSize.y);
}

void CEdViewCtl::MoveTo(const ED_POINT& postion)
{
	// 计算目标图像大小
	ED_SIZEF imageExtSize;

	if (mImageReal.x > 0 && mImageReal.y > 0)
	{
		imageExtSize.x = (float)mImageReal.x;
		imageExtSize.y = (float)mImageReal.y;
	}
	else
	{
		imageExtSize.x = mImageInit.x*mRealRatio + mGap;
		imageExtSize.y = mImageInit.y*mRealRatio;
	}

	if ((float32)mViewPort.x < imageExtSize.x)
		mFocusOn.x = (float32)postion.x / imageExtSize.x;
	if ((float32)mViewPort.y < imageExtSize.y)
		mFocusOn.y = (float32)postion.y / imageExtSize.y;
}

float32 CEdViewCtl::GetRealRatio()
{
	if (Calculate() == false)
		return 0.0f;

	return mRealRatio;
}

float32 CEdViewCtl::GetBaseRatio()
{
	if (Calculate() == false)
		return 0.0f;

	return mBaseRatio;
}

float32 CEdViewCtl::GetFatRatio()
{
	if (Calculate() == false)
		return 0.0f;

	return mFatRatio;
}

//srcPageSize整个页面实际大小
bool CEdViewCtl::GetViewMapArea(ED_RECT& destArea, ED_RECT& srcArea, ED_SIZEF& imageExtSize, UCHAR* edgeImpact)
{
	if (Calculate() == false)
		return false;

	// 计算目标图像大小
	//ED_SIZEF imageExtSize;

	if (mImageReal.x > 0 && mImageReal.y > 0)
	{
		imageExtSize.x = (float)mImageReal.x;
		imageExtSize.y = (float)mImageReal.y;
	}
	else
	{
		imageExtSize.x = mImageInit.x*mRealRatio + mGap;
		imageExtSize.y = mImageInit.y*mRealRatio;
	}

	// 如果某个方向小于视口，则将焦点移到图像中间
	if (imageExtSize.x <= mViewPort.x && mFocusOn.x != 0.5f)
		mFocusOn.x = 0.5f;
	if (imageExtSize.y <= mViewPort.y && mFocusOn.y != 0.5f)
		mFocusOn.y = 0.5f;

	// 将目标图像加上偏移
	ED_RECTF destImage;
	destImage.left = mViewPort.x / 2.0f - (imageExtSize.x * mFocusOn.x);
	destImage.right = destImage.left + (imageExtSize.x);
	destImage.top = mViewPort.y / 2.0f - (imageExtSize.y * mFocusOn.y);
	destImage.bottom = destImage.top + (imageExtSize.y);

	// 检查是否有边缩入视口，如有则调整
	bool32 moveFocus = false;
	if (destImage.left > 0.0f && destImage.right > (float32)mViewPort.x)
	{
		destImage.left = 0.0f;
		destImage.right = imageExtSize.x;
		moveFocus = true;
	}
	if (destImage.right < (float32)mViewPort.x && destImage.left < 0.0f)
	{
		destImage.right = (float32)mViewPort.x;
		destImage.left = destImage.right - imageExtSize.x;
		moveFocus = true;
	}
	if (destImage.top > 0.0f && destImage.bottom > (float32)mViewPort.y)
	{
		destImage.top = 0.0f;
		destImage.bottom = imageExtSize.y;
		moveFocus = true;
	}
	if (destImage.bottom < (float32)mViewPort.y && destImage.top < 0.0f)
	{
		destImage.bottom = (float32)mViewPort.y;
		destImage.top = destImage.bottom - imageExtSize.y;
		moveFocus = true;
	}

	//destArea.left = CExFloat::ToLong(destImage.left);
	//destArea.right = CExFloat::ToLong(destImage.right);
	//destArea.top = CExFloat::ToLong(destImage.top);
	//destArea.bottom = CExFloat::ToLong(destImage.bottom);

	if (moveFocus)
	{
		mFocusOn.x = ((float32)mViewPort.x / 2.0f - destImage.left) / imageExtSize.x;
		mFocusOn.y = ((float32)mViewPort.y / 2.0f - destImage.top) / imageExtSize.y;
	}


	if (imageExtSize.x < mViewPort.x)
	{
		destArea.left = CExFloat::ToLong(destImage.left);
		destArea.right = CExFloat::ToLong(destImage.right);

		srcArea.left = 0;
		srcArea.right = CExFloat::ToLong(imageExtSize.x);
	}
	else
	{
		destArea.left = 0;
		destArea.right = mViewPort.x;

		srcArea.left = 0 - CExFloat::ToLong(destImage.left);
		srcArea.right = srcArea.left + mViewPort.x;
	}

	if (imageExtSize.y <= mViewPort.y)
	{
		destArea.top = CExFloat::ToLong(destImage.top);
		destArea.bottom = CExFloat::ToLong(destImage.bottom);

		srcArea.top = 0;
		srcArea.bottom = CExFloat::ToLong(imageExtSize.y);
	}
	else
	{
		destArea.top = 0;
		destArea.bottom = mViewPort.y;

		srcArea.top = 0 - CExFloat::ToLong(destImage.top);
		srcArea.bottom = srcArea.top + mViewPort.y;
	}

	mVisibleRectOnSource = srcArea;
	mVisibleRectOnView = destArea;

	if (edgeImpact != NULL)
	{
		*edgeImpact = 0;

		if (srcArea.left > 1)
			*edgeImpact |= 1;
		if (srcArea.right < CExFloat::ToLong(imageExtSize.x) - 1)
			*edgeImpact |= 2;
		if (srcArea.top > 1)
			*edgeImpact |= 4;
		if (srcArea.bottom < CExFloat::ToLong(imageExtSize.y) - 1)
			*edgeImpact |= 8;
	}

	return true;
}

bool32 CEdViewCtl::Calculate(void)
{
	bool32 focusOnNewPage = false;
	if (mViewPort.x == 0 || mViewPort.y == 0 || mImageInit.x == 0 || mImageInit.y == 0)
		return false;

	if (mViewPortModified)
	{
		mImageInitModified = true;
		mUserRatioModified = true;

		mViewPortModified = false;
	}

	if (mImageInitModified)
	{
		if (mViewPort.x == 0 || mViewPort.y == 0)
			return false;

		// 计算基础放大倍数
		float32 imageSlope = (float32)mImageInit.x / (float32)mImageInit.y;
		float32 viewPortSlope = (float32)mViewPort.x / (float32)mViewPort.y;

		if (imageSlope > viewPortSlope)
		{
			auto baseWidth = (float32)mViewPort.x;
			mBaseRatio = baseWidth / (float32)mImageInit.x;
			//baseHeight = CExFloat::Round(baseWidth / imageSlope);
			mFatRatio = (float32)mViewPort.y / (float32)mImageInit.y;
		}
		else
		{
			auto baseHeight = (float32)mViewPort.y;
			mBaseRatio = baseHeight / (float32)mImageInit.y;
			//baseWidth = CExFloat::Round(baseHeight*imageSlope);
			mFatRatio = (float32)mViewPort.x / (float32)mImageInit.x;
		}

		mUserRatioModified = true;
		focusOnNewPage = true;
		mImageInitModified = false;
	}

	if (mUserRatioModified)
	{
		mRealRatio = mBaseRatio*mUserRatio; 

		mUserRatioModified = false;
	}
	if (focusOnNewPage)
	{
		mFocusOn.x = 0.5f;	// 横轴定位到中间

		// 纵轴定位到页面上部
		if (mImageInit.y*mRealRatio > (float32)mViewPort.y)
			mFocusOn.y = (float32)mViewPort.y / (mImageInit.y*mRealRatio*2.0f);
		else
			mFocusOn.y = 0.5f;
	}

	return true;
}

//显示坐标转换为文档坐标
bool CEdViewCtl::ViewToImageInit(int viewX, int viewY, ED_POINT& ptInInit)
{
	if (viewX < mVisibleRectOnView.left || viewX >= mVisibleRectOnView.right || viewY < mVisibleRectOnView.top || viewY >= mVisibleRectOnView.bottom)
		return false;

	//float32 xOff = (float)(viewX - mVisibleRectOnView.left) / mRealRatio + mVisibleRectOnSource.left;
	//float32 yOff = (float)(viewY - mVisibleRectOnView.top) / mRealRatio + mVisibleRectOnSource.top;

	//ptInInit.x = CExFloat::ToLong(xOff);
	//ptInInit.y = CExFloat::ToLong(yOff);

	float32 xOff = (float)(viewX - mVisibleRectOnView.left + mVisibleRectOnSource.left);
	float32 yOff = (float)(viewY - mVisibleRectOnView.top + mVisibleRectOnSource.top);

	ptInInit.x = CExFloat::ToLong(xOff / mRealRatio);
	ptInInit.y = CExFloat::ToLong(yOff / mRealRatio);

	return true;
}

//显示坐标转换为文档坐标
bool CEdViewCtl::ViewToImageInit(int viewX, int viewY, ED_POINTF& ptInInit)
{
	if (viewX < mVisibleRectOnView.left || viewX >= mVisibleRectOnView.right || viewY < mVisibleRectOnView.top || viewY >= mVisibleRectOnView.bottom)
		return false;

	//float32 xOff = (float)(viewX - mVisibleRectOnView.left) / mRealRatio + mVisibleRectOnSource.left;
	//float32 yOff = (float)(viewY - mVisibleRectOnView.top) / mRealRatio + mVisibleRectOnSource.top;

	//ptInInit.x = CExFloat::ToLong(xOff);
	//ptInInit.y = CExFloat::ToLong(yOff);

	float32 xOff = (float)(viewX - mVisibleRectOnView.left + mVisibleRectOnSource.left);
	float32 yOff = (float)(viewY - mVisibleRectOnView.top + mVisibleRectOnSource.top);

	ptInInit.x = round(xOff / mRealRatio);
	ptInInit.y = round(yOff / mRealRatio);

	return true;
}

void CEdViewCtl::ImageInitToView(IN ED_RECTF& rectInit, OUT ED_RECTF& rectView)
{
	ED_RECTF off;

	off.left = rectInit.left * mRealRatio;
	off.right = rectInit.right * mRealRatio;
	off.top = rectInit.top * mRealRatio;
	off.bottom = rectInit.bottom * mRealRatio;

	rectView.left = off.left + mVisibleRectOnView.left - mVisibleRectOnSource.left;
	rectView.right = off.right + mVisibleRectOnView.left - mVisibleRectOnSource.left;
	rectView.top = off.top + mVisibleRectOnView.top - mVisibleRectOnSource.top;
	rectView.bottom = off.bottom + mVisibleRectOnView.top - mVisibleRectOnSource.top;
}

//文档坐标转换为显示坐标
bool CEdViewCtl::ImageToViewInit(int imageX, int imageY, ED_POINT& ptInInit)
{
	//if (imageX < mVisibleRectOnSource.left || imageX >= mVisibleRectOnSource.right || imageY < mVisibleRectOnSource.top || imageY >= mVisibleRectOnSource.bottom)
	//	return false;

	//float32 xOff = (float)(imageX - mVisibleRectOnSource.left) * mRealRatio + mVisibleRectOnView.left;
	//float32 yOff = (float)(imageY - mVisibleRectOnSource.top) * mRealRatio + mVisibleRectOnView.top;

	//ptInInit.x = CExFloat::ToLong(xOff);
	//ptInInit.y = CExFloat::ToLong(yOff);

	float32 xOff = imageX * mRealRatio + mVisibleRectOnView.left - mVisibleRectOnSource.left;
	float32 yOff = imageY * mRealRatio + mVisibleRectOnView.top - mVisibleRectOnSource.top;

	ptInInit.x = CExFloat::ToLong(xOff);
	ptInInit.y = CExFloat::ToLong(yOff);

	return true;
}

//文档坐标转换为显示坐标
bool CEdViewCtl::ImageToViewInit(int imageX, int imageY, ED_POINTF& ptInInit)
{
	//if (imageX < mVisibleRectOnSource.left || imageX >= mVisibleRectOnSource.right || imageY < mVisibleRectOnSource.top || imageY >= mVisibleRectOnSource.bottom)
	//	return false;

	//float32 xOff = (float)(imageX - mVisibleRectOnSource.left) * mRealRatio + mVisibleRectOnView.left;
	//float32 yOff = (float)(imageY - mVisibleRectOnSource.top) * mRealRatio + mVisibleRectOnView.top;

	//ptInInit.x = CExFloat::ToLong(xOff);
	//ptInInit.y = CExFloat::ToLong(yOff);

	float32 xOff = imageX * mRealRatio + mVisibleRectOnView.left - mVisibleRectOnSource.left;
	float32 yOff = imageY * mRealRatio + mVisibleRectOnView.top - mVisibleRectOnSource.top;

	ptInInit.x = xOff;
	ptInInit.y = yOff;

	return true;
}

bool CEdViewCtl::IsInPage2(ED_POINT& ptInInit, ED_POINT& ptInPage, bool nbView)
{
	ptInPage = ptInInit;

	//这里计算的是双页模式下中线的绝对位置，而不是相对屏幕的位置，所以需要使用真实大小计算
	int32 centerX = CExFloat::ToLong((mImageReal.x / 2) / mRealRatio);
	// 此处没有判断mGap的影响，当mGap不等于0时，需要重新调整代码
	if (nbView != false)
		ptInPage.x += centerX;
	else if (ptInInit.x >= centerX)
	{
		ptInPage.x -= centerX;
		return true;
	}

	ptInPage.x = ptInInit.x;
	return false;
}

bool CEdViewCtl::IsInPage2(ED_POINTF& ptInInit, ED_POINTF& ptInPage, bool nbView)
{
	ptInPage = ptInInit;

	//这里计算的是双页模式下中线的绝对位置，而不是相对屏幕的位置，所以需要使用真实大小计算
	int32 centerX = CExFloat::ToLong((mImageReal.x / 2) / mRealRatio);
	// 此处没有判断mGap的影响，当mGap不等于0时，需要重新调整代码

	if(nbView != false)
		ptInPage.x += centerX;
	else if (ptInInit.x >= centerX)
	{
		ptInPage.x -= centerX;
		return true;
	}

	ptInPage.x = ptInInit.x;
	return false;
}
