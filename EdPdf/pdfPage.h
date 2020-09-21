/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _EDPDFPAGEIMP_H_
#define _EDPDFPAGEIMP_H_
#include "PdfmImp.h"
#include "pdfImg.h"
#include "pdfAnnot.h"
#include "inkAnnot.h"
#include "vector"
#include "pdfStextPage.h"

using namespace std;

DECLARE_BUILTIN_NAME(CpdfPage)
class CpdfPage : public cmmBaseObject<CpdfPage, IEdPage, GET_BUILTIN_NAME(CpdfPage)>, public IEdAnnotManager
{
	friend 	class CpdfDocument;
	friend CpdfStextPage;

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
	
	bool32 GetPageContext(PPAGE_PDF_CONTEXT contextPtr);
		
	//bool32 AddTextAnnot(
	//	IN ED_POINT position,
	//	IN char16_ptr text
	//);

	//bool32 AddInkAnnot(
	//	IN int32 strokeCount,
	//	IN ED_POINT** strokeList,
	//	IN int32* pointCountList
	//);

	virtual IEdAnnotManager_ptr GetAnnotManager(void);

	// 从存档数据装入一个Annotation
	virtual IEdAnnot_ptr LoadAnnotFromArchive(buf_ptr buf, uint32 bufSize);

	// 新建文本笔记
	virtual IEdAnnot_ptr AddTextAnnot(
		IN ED_POINT position,
		IN char16_ptr text
	);

	// 新建墨水笔记
	virtual IEdAnnot_ptr AddInkAnnot(
		IN ED_POINTF* stroke,
		IN int32 pointCount,
		IN ED_COLOR* color = NULL,
		IN float32 border = 1.0f
	);

	// 新建Highlight笔记
	virtual IEdAnnot_ptr AddHighLightAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 新建删除现笔记
	virtual IEdAnnot_ptr AddDeleteLineAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 新建下划线笔记
	virtual IEdAnnot_ptr AddUnderLineAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 获取所有对象的列表，当annotList==NULL时，返回需要的List单元数,当缓冲区不足时返回-1；成功返回获得的IEdAnnot_ptr对象数
	virtual int32 GetAllAnnot(IEdAnnot_ptr* annotList, int bufSize);

	// 检测一笔画接触到的一系列Ink笔记对象（相交），当annotList==NULL时，返回需要的缓冲区长度,当缓冲区不足时返回-1
	virtual int32Eink DetectInkAnnot(
		IN ED_POINTF* stroke,
		IN int32Eink pointCount,
		OUT	IEdAnnot_ptr* annotList,	// 用于返回所有相交对象的缓冲区，建议一次性分配大的缓冲区，比如 IEdAnnot_ptr buf[256];
		IN int32Eink bufSize					// 上述缓冲区的单元数，不是字节数
	);

	bool32 GetSelectedText(
		IN ED_RECTF_PTR selBox,
		OUT char16_ptr textBuf,
		IN int32Eink bufSize
	);

	virtual IEdStructuredTextPage_ptr GetStructuredTextPage(void);	// 返回的对象需要调用release释放

	// 通过签名查找一个Annot
	IEdAnnot_ptr GetAnnotBySignature(ULONGLONG signature);
	ULONGLONG GetSignature(IEdAnnot_ptr annot);


	IEdAnnot_ptr GetFirstAnnot(void);

	IEdAnnot_ptr GetNextAnnot(IEdAnnot_ptr crt);

	// 删除一个Annotation，调用此函数后，IEdAnnot_ptr annot，仍然需要释放
	virtual void RemoveAnnot(
		IEdAnnot_ptr annot
	);

	pdf_page* GetPdfObj(void);
	fz_page* GetFzObj(void) {	return mPage; }

	bool32 Load(CpdfDocument* doc,fz_page *page);

	vector<pdf_annot*> mHighLights;

protected:
	// 内部变量
	int32 mPageNo;
	int32 incomplete;
	CpdfDocument* mDocObj;
	fz_page *mPage;
	fz_rect page_bbox;
	fz_display_list *page_list;
	vector<IEdAnnot_ptr> mAnnots;

	CpdfPage();
	~CpdfPage();
	fz_pixmap* DrawBitmap(float32 scalRatio, bool32 cleanUp);
	//bool32 GetBox(pdf_obj *boxName, ED_RECTF& boxRect);//PDF_NAME_MediaBox,PDF_NAME_CropBox,PDF_NAME_ArtBox,PDF_NAME_BleedBox,PDF_NAME_TrimBox
	//void MakeupMatrix(int32 w,int32 h,fz_matrix& matrix);


	DEFINE_CUMSTOMIZE_CREATE(CpdfPage, (int32 pageNo), (pageNo))

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(int32 pageNo);

	vector<IEdAnnot_ptr>& GetAnnotList(void) {
		return mAnnots;
	}

	// 新建Structured Text Annotation
	IEdAnnot_ptr AddStextAnnot(
		IN IEdStextQuadList_ptr stext,
		IN enum pdf_annot_type type,
		IN ED_COLOR* color,
		IN int32Eink clrN /*= 3 */
		);


};



#endif//_EDPDFPAGEIMP_H_