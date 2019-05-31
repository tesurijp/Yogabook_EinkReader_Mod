/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "txtPage.h"
#include "txtDocument.h"
#include "gdiplus.h"


using namespace Gdiplus;

//////////////////////////////////////////////////////////////////////////
// class CTxdArrangeThread
//////////////////////////////////////////////////////////////////////////


volatile LONG CTxdArrangeThread::mGlobalThreadNumber = 0;


void CTxdArrangeThread::ArrangeThreadRoutine()
{

	bool eventReleased = false;

	// 开始排版

	try
	{
		// 调用一次表示开始
		mCallBackFun(mThreadNumber, 0, 0, mCallBackContext);

		//////////////////////////////////////////////////////////////////////////
		// 首先排布前两页
		if (mCharStart > mCharEnd)
			mCharStart = 0;

		ArrangePages(2,mCharStart,mCharEnd);

		//////////////////////////////////////////////////////////////////////////
		// 通知外部可以异步执行了
		SetEvent(mFocusPageLoaded);
		eventReleased = true;

		if (mExitFlag != 0)
			THROW_FALSE;

		//////////////////////////////////////////////////////////////////////////
		// 启动对当前页之前页排版的第二线程
		if (mCharStart > 0)
		{
			Gdiplus::RectF rectViewPort;
			rectViewPort = mViewPortExt;
			rectViewPort.Height = mViewHeight;
			mThread2 = new CTxdArrangeThread(mCharBuffer,0,mCharStart,mFontName,mFontSize,&rectViewPort,NULL,NULL);

			mThread2->Start();
		}

		//////////////////////////////////////////////////////////////////////////
		// 继续排布当前页的后续页
		ArrangePages(0,mPageCtl.charBegin, mCharEnd);

		if (mExitFlag != 0)
			THROW_FALSE;

		//////////////////////////////////////////////////////////////////////////
		// 等待第二线程返回，最多等待30分钟
		if (mThread2 != NULL)
		{
			if (WaitForSingleObject(mThread2->mThreadHandle, 30 * 60 * 1000) == WAIT_TIMEOUT)
				THROW_FALSE;	// 超时的，则视作失败，所有数据都不使用

			//////////////////////////////////////////////////////////////////////////////
			//// 处理所有焦点前页
			//for (int i = 0; i < mThread2->mPages.Size() && mExitFlag == 0; i++)
			//{
			//	try {
			//			mCallBackFun(mThreadNumber,((uint32)i|0xC0000000),&(mThread2->mPages[i]),mCallBackContext);
			//	}
			//	catch (...)
			//	{
			//	}

			//}
			// 改为一次性传过去
			mCallBackFun(mThreadNumber, 0xCFFFFFFF,(PST_PAGE_CONTROL)&mThread2->mPages, mCallBackContext);

			//delete mThread2;
		}

	}
	catch (...)
	{
	}

	if (eventReleased == false)
	{
		SetEvent(mFocusPageLoaded);
		eventReleased = true;
	}

	// 调用一次表示结束
	mCallBackFun(mThreadNumber,MAXULONG32, 0, mCallBackContext);

}

void CTxdArrangeThread::ArrangeThreadRoutine2()
{	// 排版在视觉焦点页之前的页面

	// 首先检查自己所要排版的数据量是否过大，如果太大，则再次分身

	// 现在开始排布当前页之前的页面
	ArrangePages(0,mCharStart,mCharEnd);

	// 检查是否存在之前的倒数第一页内容过少的情况
	if (mPages.Size() >= 2 && mExitFlag==0)
	{
		Gdiplus::RectF boundRect;
		Gdiplus::PointF leftTop;

		leftTop.X = leftTop.Y = 0.0f;
		if (mGdiObj->MeasureString(mPages.Back().charBegin + mCharBuffer, mPages.Back().charCount, mFontObj,mViewPortExt, &boundRect) == Ok && boundRect.Height < mViewHeight / 2.0f)
		{
			// 将倒数第二页缩小1/4，多余的内容挤到下一页即倒数第一页
			auto x1 = mPages[mPages.Size() - 2].charCount;
			auto x2 = mPages[mPages.Size() - 1].charCount;

			uint32 charCount2Page = mPages[mPages.Size() - 2].charCount + mPages[mPages.Size() - 1].charCount;

			mViewHeight = (mViewHeight * 3) / 4;

			uint32 charCountNewPage1 = FillCharsToPage(mPages[mPages.Size() - 2].charBegin, mCharEnd, 0, 32);
			if (charCountNewPage1 < MAXULONG32)
			{
				mPages[mPages.Size() - 2].charCount = charCountNewPage1;
				mPages[mPages.Size() - 1].charBegin = mPages[mPages.Size() - 2].charBegin + mPages[mPages.Size() - 2].charCount;
				mPages[mPages.Size() - 1].charCount = charCount2Page - charCountNewPage1;
			}
		}
	}
	// 退出第二线程
}

ED_ERR CTxdArrangeThread::ArrangePages(int32 pageRequired, uint32 stringBase, uint32 stringEnd)
{
	try {

		mPageCtl.charBegin = stringBase;
		mPageCtl.charCount = 0;

		uint32 charCounted = 0;
		uint32 countTimes = 0;
		uint32 searchBase = 0;

		int32 pageLoaded = 0;
		//int32 pageIndex;
		//int32 pageCount;


		while (mPageCtl.charBegin < stringEnd && mExitFlag == 0)
		{
			mPageCtl.charCount = FillCharsToPage(mPageCtl.charBegin, stringEnd, searchBase, 32);
			if (mPageCtl.charCount == searchBase)	// 说明尝试的基地址已经跨过了本页的大小，再次向前退1024个单位（有可能直接退到0起始，并不会出错）
			{
				if (searchBase > 1024)	// 确保向前跳跃1024后，还存在内容
					mPageCtl.charCount = FillCharsToPage(mPageCtl.charBegin, stringEnd, searchBase - 1024, 32);

				if (mPageCtl.charCount == searchBase)	// 还是越界了，则从头开始查找
					mPageCtl.charCount = FillCharsToPage(mPageCtl.charBegin, stringEnd, 0, 32);

			}

			charCounted += mPageCtl.charCount;

			if (mCallBackFun != NULL)
			{
				mCallBackFun(mThreadNumber, ++mPageLoadStep, &mPageCtl, mCallBackContext);
			}
			else
			{
				mPages.Insert(-1, mPageCtl);
			}

			mPageCtl.charBegin += mPageCtl.charCount;

			// 使用平均页面数的1024下对齐，作为基数，来尝试查找
			searchBase = ((charCounted / (++countTimes)) / 1024) * 1024;

			if (pageRequired > 0 && ++pageLoaded >= pageRequired)
				break;
		}

	}
	catch (...)
	{
	}

	return EDERR_SUCCESS;
}

#define LOW_GRAIN(_X) (_X/32)//(_X>32?32:1)

uint32 CTxdArrangeThread::FillCharsToPage(uint32 stringBase, uint32 stringEnd, uint32 clusterBase, uint32 clusterSize)
{
	CharacterRange clusterCharRanges[32];
	Region clusterRegions[32];
	uint32 crtBoxBase = clusterBase;
	RectF boundOfCluster;
	int32 clusterCount = 0;

	if (mExitFlag != 0)
		return 0;	// 退出线程标志被设置

	// 如果剩余数据小于一个探测粒度，则降低粒度去探测
	if ((ULONG)stringBase + clusterBase + clusterSize >= stringEnd)
	{
		if (clusterSize > 1)
			return FillCharsToPage(stringBase, stringEnd, clusterBase, LOW_GRAIN(clusterSize));

		// 没有进一步的空间了，直接把当前的字符都返回吧
		return stringEnd - stringBase;
	}

	//////////////////////////////////////////////////////////////////////////
	//执行向后探测任务
	//////////////////////////////////////////////////////////////////////////

	// 从clusterBase开始设置32个探测cluster
	for (auto &i : clusterCharRanges)   
	{
		if (stringBase + crtBoxBase + clusterSize > stringEnd)
		{
			// 到了文章结尾了
			if (stringBase + crtBoxBase >= stringEnd)
				break;
			crtBoxBase = stringEnd - stringBase - clusterSize;	// 从文章结尾处倒退一个单元，确保这个单元也是可以被10整除。
																// 虽然这会导致此单元和前一个单元有重合，但算法上不会出错。
		}

		i.First = crtBoxBase;
		i.Length = clusterSize;
		crtBoxBase += clusterSize;

		clusterCount++;	// 计算有效的单元数
	}

	StringFormat strFormat;
	strFormat.SetAlignment(StringAlignmentNear);
	if (strFormat.SetMeasurableCharacterRanges(clusterCount, clusterCharRanges) != Ok)
		THROW_FAILED;

	if (mGdiObj->MeasureCharacterRanges(mCharBuffer + stringBase, crtBoxBase,mFontObj, mViewPortExt, &strFormat, clusterCount, clusterRegions) != Ok)
		THROW_FAILED;

	for (int j = 0; j < clusterCount; j++)
	{
		//// 过滤不可见字符
		//if (clusterSize == 1 && iswgraph(*(mCharBuffer + stringBase + clusterCharRanges[j].First)) == false)
		//	continue;

		if (clusterRegions[j].GetBounds(&boundOfCluster,mGdiObj) != Ok)
			THROW_FAILED;

		// 遇到尺寸为零的情况，先判断是不是遇到了不可见字符序列
		if (boundOfCluster.Height == 0)
		{
			// 因为测试设定的排版区域很高，如当前设定的簇大小为1，绝对不会出现可见字符超出排版区域的情况，这必然是不可见字符，则将不可见字符放到计算到前页
			if(clusterSize == 1)
				continue;

			// 如果簇大于1，一般是32个字符，那么进入分析这个簇内部的情况，也许整个簇都是不可见字符; 降低粒度，继续查找
			return FillCharsToPage(stringBase, stringEnd, clusterCharRanges[j].First, LOW_GRAIN(clusterSize));
		}

		// 如果本测试单元超出视口大小
		if (/*boundOfCluster.Height == 0 || */boundOfCluster.Y + boundOfCluster.Height > mViewHeight)
		{
			if (clusterSize == 1)// 是否粒度等于1了
				return clusterCharRanges[j].First;// 就是你了

			// 降低粒度，进一步查找这个单元
			return FillCharsToPage(stringBase, stringEnd, clusterCharRanges[j].First, LOW_GRAIN(clusterSize));
		}
	}

	// 是否不存在后续字符了
	if (stringBase + crtBoxBase >= stringEnd)
		return crtBoxBase;

	// 整个覆盖面都没有超出视口，那么向后查找，保持粒度不变
	return FillCharsToPage(stringBase, stringEnd, crtBoxBase, clusterSize);
}

// 下面的代码更加高效，但是存在一些bug，不清楚情况，先留在这
//uint32 CTxdArrangeThread::FillCharsToPage(uint32 stringBase, uint32 stringEnd, uint32 clusterBase, uint32 clusterSize)
//{
//	CharacterRange clusterCharRanges[32];
//	Region clusterRegions[32];
//	uint32 crtBoxBase = clusterBase;
//	RectF boundOfCluster[32];
//	int32 clusterCount = 0;
//
//	if (mExitFlag != 0)
//		return 0;	// 退出线程标志被设置
//
//					// 如果剩余数据小于一个探测粒度，则降低粒度去探测
//	if ((ULONG)stringBase + clusterBase + clusterSize >= stringEnd)
//	{
//		if (clusterSize > 1)
//			return FillCharsToPage(stringBase, stringEnd, clusterBase, LOW_GRAIN(clusterSize));
//
//		// 没有进一步的空间了，直接把当前的字符都返回吧
//		return stringEnd - stringBase;
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	//执行向后探测任务
//	//////////////////////////////////////////////////////////////////////////
//
//	// 从clusterBase开始设置32个探测cluster
//	for (auto &i : clusterCharRanges)
//	{
//		if (stringBase + crtBoxBase + clusterSize > stringEnd)
//		{
//			// 到了文章结尾了
//			if (stringBase + crtBoxBase >= stringEnd)
//				break;
//			crtBoxBase = stringEnd - stringBase - clusterSize;	// 从文章结尾处倒退一个单元，确保这个单元也是可以被10整除。
//																// 虽然这会导致此单元和前一个单元有重合，但算法上不会出错。
//		}
//
//		i.First = crtBoxBase;
//		i.Length = 1;// clusterSize;
//		crtBoxBase += clusterSize;
//
//		clusterCount++;	// 计算有效的单元数
//	}
//
//	StringFormat strFormat;
//	strFormat.SetAlignment(StringAlignmentNear);
//	if (strFormat.SetMeasurableCharacterRanges(clusterCount, clusterCharRanges) != Ok)
//		THROW_FAILED;
//
//	if (mGdiObj->MeasureCharacterRanges(mCharBuffer + stringBase, crtBoxBase, mFontObj, mViewPortExt, &strFormat, clusterCount, clusterRegions) != Ok)
//		THROW_FAILED;
//
//	for (int j = 0; j < clusterCount; j++)
//	{
//		if (clusterRegions[j].GetBounds(&boundOfCluster[j], mGdiObj) != Ok)
//			THROW_FAILED;
//	}
//	for (int j = 0; j < clusterCount; j++)
//	{
//		if (boundOfCluster[j].Height == 0) // 可能是不可见字符
//			for (int k = j + 1; k < clusterCount; k++)
//				if (boundOfCluster[k].Height != 0)
//				{
//					boundOfCluster[j] = boundOfCluster[k];
//					break;
//				}
//	}
//
//	for (int j = 0; j < clusterCount; j++)
//	{
//		//if (clusterRegions[j].GetBounds(&boundOfCluster,mGdiObj) != Ok)
//		//	THROW_FAILED;
//
//		// 如果本测试单元超出视口大小
//		if (boundOfCluster[j].Height == 0 || boundOfCluster[j].Y /*+ boundOfCluster.Height*/ > mViewHeight)
//		{
//			if (clusterSize == 1 || j == 0)// 是否粒度等于1了，或者没有前一个单元
//				return clusterCharRanges[j].First;// 就是你了，外部会判断两种情况，一是找到了分割字符，二是整体测试单元都超出了可视区域
//
//												  // 降低粒度，进一步查找前一个单元
//			return FillCharsToPage(stringBase, stringEnd, clusterCharRanges[j - 1].First, LOW_GRAIN(clusterSize));
//		}
//	}
//
//	// 整个覆盖面都没有超出视口，那么向后查找，保持粒度不变
//	return FillCharsToPage(stringBase, stringEnd, crtBoxBase, clusterSize);
//}
