/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#ifndef _TXTARRANGE_H_
#define _TXTARRANGE_H_
#include "TxtmImp.h"

#pragma pack(4)
typedef struct _ST_PAGE_CONTROL {
	uint32 charBegin;
	uint32 charCount;
}ST_PAGE_CONTROL,* PST_PAGE_CONTROL;

typedef cmmVector<ST_PAGE_CONTROL, 1, 1024> PageControl_Vector;
#pragma pack()


typedef void(__stdcall *PEDDOC_THREAD_CALLBACK)(LONG threadNumber, uint32 loadingStep, PST_PAGE_CONTROL pageCtl, void* context);

#define LOADING_STEP_COMPLETE 0xFFFFFFFF
#define LOADING_STEP_AHEAD_COMPLETE	  0xFFFFFFF0
#define LOADING_STEP_AHEAD(_X)	(_X&0x80000000)
#define LOADING_STEP_AHEAD_PREFIX(_X)	(_X|0x80000000)
#define LOADING_STEP_AHEAD_NUMBER(_X)	(_X&0x7FFFFFFF)

// 排版线程类
class CTxdArrangeThread {
public:
	LONG mThreadNumber;
	ED_ERR mResult;

	CTxdArrangeThread(const wchar_t* charBuffer, uint32 charStart, uint32 charEnd, bool aheadPages, const wchar_t* fontName, float fontSize, Gdiplus::RectF* viewPort, PEDDOC_THREAD_CALLBACK callBackFun, void* callBackContext);
	~CTxdArrangeThread();

	void Start(void);

	void TerminateThread();

private:
	static volatile LONG mGlobalThreadNumber;
	CGdipStart mGdiStart;
	LONG mCloned;
	PEDDOC_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;
	bool  mAheadArrange;	// 说明当前时在排版焦点之前页
	ULONG mThreadID;
	HANDLE mThreadHandle;
	HANDLE mFocusPageLoaded;
	ST_PAGE_CONTROL mPageCtl;

	Gdiplus::RectF mViewPortExt;
	FLOAT mViewHeight;
	Gdiplus::Graphics* mGdiObj;
	Gdiplus::Font* mFontObj;
	wchar_t mFontName[100];
	float mFontSize;

	PageControl_Vector mPages;
	const wchar_t* mCharBuffer;
	uint32 mCharStart;
	uint32 mCharEnd;
	uint32 mPageLoadStep;
	CTxdArrangeThread* mThread2;

	volatile LONG mExitFlag;

	static ULONG WINAPI ThreadBridge(CTxdArrangeThread* thisPointer);

	bool InitGdip(void);

	void UninitGdip(void);

	void ArrangeThreadRoutine();
	void ArrangeThreadRoutine4AheadPages();

	ED_ERR ArrangePages(int32 pageRequired,uint32 stringBase, uint32 stringEnd);

	uint32 FillCharsToPage(uint32 stringBase, uint32 stringEnd, uint32 clusterBase, uint32 clusterSize);// clusterSize ： 32/1

};




#endif//_TXTARRANGE_H_