/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once
#ifndef _EDPDFDOC_H_
#define _EDPDFDOC_H_
#include "PdfmImp.h"
#include "cmmstring.h"
#include "PageCtl.h"

typedef cmmVector<CpdfPage*, 16, 16> CpdfPage_vector;

//class CEsThreadNode {
//public:
//	ULONG muThreadID;
//	IXsWidget* mpOwnerWidget;
//	void operator=(const class CEsThreadNode& src) {
//		muThreadID = src.muThreadID;
//		mpOwnerWidget = src.mpOwnerWidget;
//	}
//};
//
//class CEsThreadNodeCriterion	// 默认的判断准则
//{
//public:
//	bool operator () (const CEsThreadNode& Obj1, const CEsThreadNode& Obj2)const // 一定要用内联函数
//	{
//		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
//		return (Obj1.muThreadID < Obj2.muThreadID);
//	}
//};
//
//// 按照ID排序的线程队列
//typedef cmmSequence<CEsThreadNode, CEsThreadNodeCriterion> TEsThreadSequence;



DECLARE_BUILTIN_NAME(CpdfDocument)
class CpdfDocument : public cmmBaseObject<CpdfDocument, IEdDocument, GET_BUILTIN_NAME(CpdfDocument)>
{
public:
	//enum ERENDER_STEP {
	//	eRenderBegin = 1,
	//	eRenderParepare = 2,
	//	eRenderRender = 3,
	//	eRenderEnd = 4,
	//	eRenderStop = 5
	//};

	DEFINE_CUMSTOMIZE_CREATE(CpdfDocument, (const char16_ptr pathName), (pathName))

	bool32 LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr, PPAGE_PDF_CONTEXT initPage);
	int32 GetMetaData(char* keyName, char* dataBuf, int32 bufSize);
	int32 GetDocType(void);
	int32 GetPageCount(void);
	IEdPage_ptr GetPage(int32Eink pageIndex);
	IEdPage_ptr GetPage(PPAGE_PDF_CONTEXT contextPtr);
	IEdPage_ptr GetPage(IEdPage_ptr currentPage, int32Eink pagesOff);
	bool32 LoadAllThumbnails(PEDDOC_THUMBNAILS_CALLBACK callBack, void_ptr contextPtr, const wchar_t* nphumbnailPathName);
	bool32 GetThumbnailPathName(int32Eink pageIndex, char16 pathName[256], bool* hasAnnot);
	bool32 GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen);
	int32Eink GetAnnotPageCount(void);

	bool32 SaveAllChanges(const char16_ptr pathName = NULL);

	pdf_document* GetPdfObj(void);
	bool SetDirty(bool dirty);

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

	//void TestHighLight();
public:

protected:
	// 内部变量
	fz_document* fzDoc;
	int32 pageCount;
	int miObjCount; //上次保存时的对象总数
	cmmStringW mPathName;
	bool mDirty;
	CEdPageControl mPageControl;
	CExclusiveAccess mDocAccLock;	// fz_doc对象的访问，必须互斥
	volatile bool mLoadThumb;
	HANDLE mThumbThreadExit;

	// 当前页
	int32 incomplete;

	CpdfDocument();
	~CpdfDocument();
	ED_ERR Open_progressive(const char16_ptr fileName);


	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(const char16_ptr pathName);

	ED_ERR LoadPage(CpdfPage* pageObj);
};



#endif//_EDPDFDOC_H_