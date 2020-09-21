/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_
#include "PdfmImp.h"
#include "cmmstruct.h"
#include "cmmGeometry.h"
#include "cmmptr.h"
#include "pdfStextQuadList.h"


DECLARE_BUILTIN_NAME(CpdfStextAnnot)
class CpdfStextAnnot : public cmmBaseObject<CpdfStextAnnot, IEdStextAnnot, GET_BUILTIN_NAME(CpdfStextAnnot)>
{
	friend 	class CpdfPage;

protected:
	// 内部变量
	uint32 mAnnotType;
	pdf_annot* mPdfRawObj;
	float mColor[4];
	int mColorN;
	float mBorder;

	cmmReleasePtr<CpdfStextQuadsList> mQuadListObj;

	CpdfStextAnnot();

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	DEFINE_CUMSTOMIZE_CREATE(CpdfStextAnnot, (void), ())


	// 向页面添加INK Annotation，数据来自Stroke List，并装入本对象
	bool PdfCreateAnnot(
		IN pdf_page* pageObj, 
		IN IEdStextQuadList_ptr stext,
		IN enum pdf_annot_type type,
		IN float* clr, 
		IN int clrN = 3, 
		IN float32 border = 1
		);

	// 从存档解码StextAnnot存档数据
	static bool UnpackFromAchive(char* archive, int32 bufSize,OUT ED_POINTF& a,OUT ED_POINTF& b,OUT uint32& type,OUT ED_COLOR& clr,OUT int32Eink& clrN);
	// 将pdf_annotation对象装入到本对象
	bool Load(pdf_annot* annot);

	// 获取签名的内部值
	void GetSignature(ULONG& checksum, USHORT& altData);

	pdf_annot* GetPdfObj() {
		return mPdfRawObj;
	}

	//int32 DetectIntersection(CpdfStextQuadsList& checkWith); // 返回和本实例的第几个Quad相交，返回-1表示不相交

public:
	~CpdfStextAnnot();


	virtual uint32 GetType();	// EDPDF_ANNOT_INK ,EDPDF_ANNOT_UNDERL, EDPDF_ANNOT_DELETE,EDPDF_ANNOT_HIGHLIGHT, EDPDF_ANNOT_TEXT
	virtual char* GetTypeName();		// "ink" "text" "highlight" "underline" "deleteline" or "Identity"

	IEdTextAnnot_ptr ConvertToTextAnnot(void) { return NULL; }
	IEdInkAnnot_ptr ConvertToInkAnnot(void) { return NULL; }
	IEdStextAnnot_ptr ConvertToStextAnnot(void) { return this; }

	uint32 SaveToAchive(buf_ptr buf, uint32 bufSize);	// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)

	void SetColor(ED_COLOR clr);		// 设置线条颜色
	ED_COLOR GetColor(void);

	void SetBorder(int32 border);	// 设置线条宽度 1 ~ N
	int32 GetBorder(void);

	virtual CpdfStextQuadsList* GetQuadsList(void); // 获得分块列表
};



#endif//_HIGHLIGHT_H_