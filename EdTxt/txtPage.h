/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EDPDFPAGEIMP_H_
#define _EDPDFPAGEIMP_H_
#include "txtmImp.h"


//typedef cmmVector<CXsWidget*, 64, 64> TEsWidgetVector;

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

DECLARE_BUILTIN_NAME(CtxtBitmap)
class CtxtBitmap : public cmmBaseObject<CtxtBitmap, IEdBitmap, GET_BUILTIN_NAME(CtxtBitmap)>
{
	friend 	class CtxtPage;

public:
	bin_ptr GetBuffer();
	int32 GetWidth();
	int32 GetHeight();

protected:
	// 内部变量
	int32 mWidth;
	int32 mHeight;
	bin_ptr mImageBuffer;

	CtxtBitmap();
	~CtxtBitmap();

	//DEFINE_CUMSTOMIZE_CREATE(CtxtBitmap, (fz_pixmap* fzImage), (fzImage))
	DEFINE_CUMSTOMIZE_CREATE(CtxtBitmap, (int32 w,int32 h), (w,h))

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(int32 w, int32 h);

};




DECLARE_BUILTIN_NAME(CtxtPage)
class CtxtPage : public cmmBaseObject<CtxtPage, IEdPage, GET_BUILTIN_NAME(CtxtPage)>
{
	friend 	class CtxtDocument;

public:
	//enum ERENDER_STEP {
	//	eRenderBegin = 1,
	//	eRenderParepare = 2,
	//	eRenderRender = 3,
	//	eRenderEnd = 4,
	//	eRenderStop = 5
	//};

	bool32 GetMediaBox(
		OUT ED_RECTF_PTR mediaBox
	);
	bool32 GetCropBox(
		OUT ED_RECTF_PTR cropBox
	);
	bool32 GetBleedBox(
		OUT ED_RECTF_PTR bleedBox
	);
	IEdBitmap_ptr Render(
		IN float32 scalRatio,
		IN bool32 cleanUp
	);
	int32 GetPageIndex(void);

	bool32 GetPageContext(
		PPAGE_PDF_CONTEXT contextPtr
	);

protected:
	// 内部变量
	int32 mPageIndexOpenning;
	int32 incomplete;
	wchar_t* mCharPtr;
	uint32 mCharLength;
	uint32 mPageWidth,mPageHeight;
	CtxtDocument* mDocObj;

	CtxtPage();
	~CtxtPage();
//	fz_pixmap* DrawBitmap(float32 scalRatio, bool32 cleanUp);
	//bool32 GetBox(pdf_obj *boxName, ED_RECTF& boxRect);//PDF_NAME_MediaBox,PDF_NAME_CropBox,PDF_NAME_ArtBox,PDF_NAME_BleedBox,PDF_NAME_TrimBox
	//void MakeupMatrix(int32 w,int32 h,fz_matrix& matrix);


	DEFINE_CUMSTOMIZE_CREATE(CtxtPage, (int32 pageNo,wchar_t* charPtr,uint32 charLength, CtxtDocument* docObj), (pageNo,charPtr,charLength,docObj))

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(int32 pageNo, wchar_t* charPtr, uint32 charLength, CtxtDocument* docObj);
};



#endif//_EDPDFPAGEIMP_H_