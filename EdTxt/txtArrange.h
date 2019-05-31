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


typedef void(__stdcall *PEDDOC_THREAD_CALLBACK)(LONG threadNumber, uint32 loadingStep,PST_PAGE_CONTROL pageCtl,void* context);


// 排版线程类
class CTxdArrangeThread {
public:
	LONG mThreadNumber;
	ED_ERR mResult;

	CTxdArrangeThread(const wchar_t* charBuffer,uint32 charStart,uint32 charEnd,const wchar_t* fontName, float fontSize, Gdiplus::RectF* viewPort, PEDDOC_THREAD_CALLBACK callBackFun, void* callBackContext)
	{
		mThreadNumber = InterlockedIncrement(&mGlobalThreadNumber);
		mResult = EDERR_UNSUCCESSFUL;

		mCharBuffer = charBuffer;
		mCharStart = charStart;
		mCharEnd = charEnd;

		mViewPortExt = *viewPort;
		mViewPortExt.Height *= 2.0f;
		mViewHeight = viewPort->Height;

		mCallBackContext = callBackContext;
		mCallBackFun = callBackFun;
		mExitFlag = 0;
		mPageLoadStep = 0;
		mThread2 = NULL;

		wcscpy_s(mFontName, 100, fontName);
		mFontSize = fontSize;

		if (mCallBackFun != NULL)
			mFocusPageLoaded = CreateEvent(NULL, true, false, NULL);
		else
			mFocusPageLoaded = NULL;

		mFontObj = NULL;
		mGdiObj = NULL;

		mThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CTxdArrangeThread::ThreadBridge, this, CREATE_SUSPENDED, &mThreadID);
		if (mThreadHandle == NULL)
			return;

		mResult = EDERR_SUCCESS;
	}
	~CTxdArrangeThread() {

		TerminateThread();

		if (mThread2 != NULL)
			delete mThread2;

		if(mFocusPageLoaded != NULL)
			CloseHandle(mFocusPageLoaded);

		if (mThreadHandle != NULL)
			CloseHandle(mThreadHandle);
	}

	void Start(void) {
		ResumeThread(mThreadHandle);

		if (mFocusPageLoaded != NULL)
		{
			if (IsDebuggerPresent() != FALSE)
				WaitForSingleObject(mFocusPageLoaded,INFINITE);
			else
				WaitForSingleObject(mFocusPageLoaded, 10 * 1000);	// 10秒超时
		}
	}

	void TerminateThread() {	// will wait the thread exit and set a timeout as 10 seconds
		if (mThread2 != NULL)
			mThread2->TerminateThread();

		InterlockedExchange(&mExitFlag, 1);
		if (WaitForSingleObject(mThreadHandle, 10 * 1000) == WAIT_TIMEOUT)
			::TerminateThread(mThreadHandle, -1);	// 直接中止
	}

private:
	static volatile LONG mGlobalThreadNumber;
	CGdipStart mGdiStart;
	LONG mCloned;
	PEDDOC_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;
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

	static ULONG WINAPI ThreadBridge(CTxdArrangeThread* thisPointer) {

		if (thisPointer->InitGdip() == false)
			return -1;

		try {
			if(thisPointer->mCallBackFun != NULL)
				thisPointer->ArrangeThreadRoutine();
			else
				thisPointer->ArrangeThreadRoutine2();
		}
		catch(...)
		{ }

		thisPointer->UninitGdip();

		return 0;
	}

	bool InitGdip(void) {
		mGdiStart.Init();

		mGdiObj = new Gdiplus::Graphics(GetDC(NULL));
		if (mGdiObj == NULL)
			return false;

		mFontObj = new Gdiplus::Font(mFontName, mFontSize);

		return mFontObj != NULL;
	}

	void UninitGdip(void) {
		if (mFontObj != NULL)
			delete mFontObj;

		if (mGdiObj != NULL)
			delete mGdiObj;
		mGdiStart.UnInit();
	}

	void ArrangeThreadRoutine();
	void ArrangeThreadRoutine2();

	ED_ERR ArrangePages(int32 pageRequired,uint32 stringBase, uint32 stringEnd);

	uint32 FillCharsToPage(uint32 stringBase, uint32 stringEnd, uint32 clusterBase, uint32 clusterSize);// clusterSize ： 32/1

};




#endif//_TXTARRANGE_H_