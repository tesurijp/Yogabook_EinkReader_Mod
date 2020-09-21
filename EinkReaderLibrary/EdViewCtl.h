/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "EdDoc.h"


class CEdViewCtl
{
public:
	CEdViewCtl();
	~CEdViewCtl();

	void SetViewPort(const ED_SIZE& viewPort);
	void SetUserRatio(float32 userRatio);
	void SetImageInit(const ED_SIZE& imageInit);
	void SetCenterGap(float32 gap)	{
		mGap = gap;
	}
	void Move(const ED_POINT& offset);
	void MoveTo(const ED_POINT& postion);
	void AdjustImageRealSize(const ED_SIZE& imageReal) {
		mImageReal.x = imageReal.x;
		mImageReal.y = imageReal.y;
	}

	const ED_SIZE& GetViewPort(void) {
		return mViewPort;
	}
	float32 GetUserRatio(void) {
		return mUserRatio;
	}
	const ED_SIZE& GetImageInit(void) {
		return mImageInit;
	}

	float32 GetRealRatio();
	float32 GetBaseRatio();
	float32 GetFatRatio();
	bool GetViewMapArea(ED_RECT& destArea, ED_RECT& srcArea, ED_SIZEF& imageExtSize,UCHAR* edgeImpact=NULL); // 如果图像显示完整，返回true；否则返回false
	

	void ImageInitToView(IN ED_RECTF& rectInit, OUT ED_RECTF& rectView);
	bool ViewToImageInit(int viewX, int viewY, ED_POINTF& ptInInit);
	//显示坐标转换为文档坐标
	bool ViewToImageInit(int viewX, int viewY, ED_POINT& ptInInit);
	//文档坐标转换为显示坐标
	bool ImageToViewInit(int imageX, int imageY, ED_POINT& ptInInit);
	bool ImageToViewInit(int imageX, int imageY, ED_POINTF& ptInInit);

	bool IsInPage2(ED_POINT& ptInInit, ED_POINT& ptInPage,bool nbView = false);	// 当页面处于双页显示时调用，判断点是否处于第二页中，返回ture处于第二页，否则仍然在第一页，ptInPage返回页内的坐标值
	bool IsInPage2(ED_POINTF& ptInInit, ED_POINTF& ptInPage, bool nbView = false);	// 当页面处于双页显示时调用，判断点是否处于第二页中，返回ture处于第二页，否则仍然在第一页，ptInPage返回页内的坐标值

protected:
	ED_SIZE mViewPort;	// 视口大小
	ED_POINTF mFocusOn;	// 焦点
	ED_SIZE mImageInit; // 图像初始大小
	float32 mGap;		// 双页显示模式下的中间的缝隙的宽度
	ED_SIZE mImageReal;	// 图像的实际大小
	float32 mBaseRatio;	// 基础放大比例，这个比例被视作用户放大比例的1.0
	float32 mUserRatio;	// 用户设定的放大倍数
	float32 mRealRatio;	// 基础和用户放大倍数合并后
	float32 mFatRatio;	// 占满屏幕需要的最小放大比例
	bool32 mViewPortModified;
	bool32 mUserRatioModified;
	bool32 mImageInitModified;

	ED_RECT mVisibleRectOnView;
	ED_RECT mVisibleRectOnSource;

	bool32 Calculate(void);
};

