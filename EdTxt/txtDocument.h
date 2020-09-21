/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#ifndef _TXTDOC_H_
#define _TXTDOC_H_
#include "TxtmImp.h"
#include "txtArrange.h"


DECLARE_BUILTIN_NAME(CtxtDocument)
class CtxtDocument : public cmmBaseObject<CtxtDocument, IEdDocument, GET_BUILTIN_NAME(CtxtDocument)>
{
public:
	enum BOM {
		un = 0,
		ansi = 1,
		u16 = 2,
		u16b = 3,
		u7 = 4,
		u8 = 5,
		u8NoBOM=6
	};

	DEFINE_CUMSTOMIZE_CREATE(CtxtDocument, (const char16_ptr pathName), (pathName))

	bool32 LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr, PPAGE_PDF_CONTEXT initPage);
	int32 GetMetaData(char* keyName, char* dataBuf, int32 bufSize);
	int32 GetDocType(void);
	int32 GetPageCount(void);
	IEdPage_ptr GetPage(int32 pageIndex);
	IEdPage_ptr GetPage(PPAGE_PDF_CONTEXT contextPtr);	// contextPtr==NULL for the first page of this doc
	IEdPage_ptr GetPage(IEdPage_ptr currentPage,int32 pagesOff);
	bool32 LoadAllThumbnails(PEDDOC_THUMBNAILS_CALLBACK callBack, void_ptr contextPtr, const wchar_t* nphumbnailPathName);
	bool32 GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen);
	bool32 GetThumbnailPathName(int32Eink pageIndex, char16 pathName[256], bool* hasAnnot);
	int32Eink GetAnnotPageCount(void) {
		return 0;
	}
	bool32 SaveAllChanges(const char16_ptr pathName) {
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// following functions just work on a txt document.

	// This method to update the layout of all pages.
	IEdPage_ptr Rearrange(PPAGE_PDF_CONTEXT contextPtr);

	// This method to update the layout of all pages.
	IEdPage_ptr Rearrange(IEdPage_ptr currentPage);

	// This method to change the font used to render this document. 
	void SetFont(const char16_ptr fontName, int32Eink fontSize);

	// this method to set the view port to render txt document. 
	void SetViewPort(int32Eink viewWidth, int32Eink viewHeight);

	wchar_t* GetDocBuffer(void) {
		return mpTextBuffer;
	}

	int32Eink GetPageIndex(CtxtPage* pageObj);

	//void RegisterPage(CtxtPage* pageObj);
	//void UnregisterPage(CtxtPage* pageObj);

public:
	char16_ptr GetFontName(void) {
		return mFontName;
	}
	float GetFontSize(void) {
		return mFontSize;
	}

	Gdiplus::PointF* GetViewPort(void) {
		return &mViewPort;
	}

protected:
	// 内部变量
	HANDLE mhTxtFile;
	DWORD muTxtFileLength;
	HANDLE mhTxtFileMap;
	bin_ptr mpTxtMappedBase;
	LARGE_INTEGER mFileSize;
	BOM mBom;
	wchar_t*  mpTextBuffer;
	uint32 mTextCharCount;
	PEDDOC_CALLBACK mPageLoadCallbackFun;
	void_ptr mPageLoadCallbackContext;

	// 当前页
	volatile int32 mUpdateForSize;
	volatile int32 mUpdateForFont;
	PageControl_Vector mAllPages;
	uint32 mAheadPageLoaded;
	char16	mFontName[256];
	float mFontSize;
	Gdiplus::PointF mViewPort;
	CExclusiveAccess mThreadDataLock;
	volatile int32 mArranging; // 排版中
	uint32 mCrtPageBegin;	// 仅在排版时，用于判断当前页码
	uint32 mCrtPageNumber;	// 仅在排版时，用于判断当前页码

	volatile CTxdArrangeThread* mpArrangeThread;
	volatile LONG mThreadNumber;

	// 排版信息
	ST_PAGE_CONTROL mRearrangePage;
	int32 mRearrangePageIndex;


	// 用于缩放页后，保持焦点页
	//FocusControl_Stack mFocusCtl;

	CtxtDocument();
	~CtxtDocument();


	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(const char16_ptr pathName);

	ED_ERR UnpackU8(void);
	ED_ERR IsU8File(void);
	//判断文件是否是utf8文件
	bool IsU8File(const void* npBuffer, long nlSize);

	ED_ERR UnpackAnsi(void);

	ED_ERR UnpackBigEnding16(void);

	ED_ERR LoadPage(CtxtPage* pageObj);

	BOM GetBomInfor();

	static void __stdcall PageArrangedCallBack(LONG threadNumber, uint32 loadingStep, PST_PAGE_CONTROL pageCtl, CtxtDocument* thisDoc);

};



#endif//_TXTDOC_H_