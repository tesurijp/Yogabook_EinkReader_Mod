/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _HIGHLIGHT_H_
#define _HIGHLIGHT_H_
#include "PdfmImp.h"
#include "cmmstruct.h"

class CpdfHighlightQuad {
public:
	float32 x1,y1,x2,y2;
	CpdfHighlightQuad() {
	}

	CpdfHighlightQuad(const CpdfHighlightQuad& src) {
		*this = src;
	}

	void operator = (const CpdfHighlightQuad& src) {
		x1 = src.x1;
		x2 = src.x2;
		y1 = src.y1;
		y2 = src.y2;
	}
	void operator =(const fz_quad& quad) {
		x1 = quad.ul.x;
		y1 = quad.ul.y;
		x2 = quad.lr.x;
		y2 = quad.lr.y;
	}

	// 判断线段是否与我相交
	bool IsLineCrossed(float p1x, float p1y, float p2x, float p2y)
	{
		// 首先判断线段的两个顶点是否有在矩形中的
		if ((p1x >= x1 && p1x < x2 && p1y >= y1 && p1y < y2) != false)
			return true;
		if ((p2x >= x1 && p2x < x2 && p2y >= y1 && p2y < y2) != false)
			return true;

		// 在判断这条线段是否和矩形的四条边中的某条相交
		if (CExLine::IsSegmentCross(p1x, p1y, p2x, p2y, x1, y1, x2, y1) != false)
			return true;
		if (CExLine::IsSegmentCross(p1x, p1y, p2x, p2y, x2, y1, x2, y2) != false)
			return true;
		if (CExLine::IsSegmentCross(p1x, p1y, p2x, p2y, x1, y2, x2, y2) != false)
			return true;
		if (CExLine::IsSegmentCross(p1x, p1y, p2x, p2y, x1, y1, x1, y2) != false)
			return true;

		return false;
	}
};

typedef cmmVector<CpdfHighlightQuad, 0, 128> CpdfHighlightQuadList;

DECLARE_BUILTIN_NAME(CpdfHlAnnot)
class CpdfHlAnnot : public cmmBaseObject<CpdfHlAnnot, IEdHighLightAnnot, GET_BUILTIN_NAME(CpdfHlAnnot)>
{
	friend 	class CpdfPage;


protected:
	// 内部变量
	pdf_annot* mPdfRawObj;
	//ED_POINT mPosition;
	float mColor[4];
	int mColorN;
	float mBorder;

	CpdfHighlightQuadList mPdfHlQuads;	// 存放pdf文件装入的Highlight quad数据

	CpdfHlAnnot();

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate();

	DEFINE_CUMSTOMIZE_CREATE(CpdfHlAnnot, (), ())

	// 向页面添加INK Annotation，数据来自Stroke List，并装入本对象
	bool AddInk(pdf_page* pageObj, CpdfInkMultiList& strokeList, float* clr, int clrN, float32 border);
	// 向页面添加INK Annotation，数据来自存档，并装入本对象
	bool AddInk(pdf_page* pageObj, char* archive, int32 bufSize);
	// 将pdf_annotation对象装入到本对象
	bool LoadInk(pdf_annot* annot);

	// 获取签名的内部值
	void GetSignature(ULONG& checksum, USHORT& altData);

	pdf_annot* GetPdfObj() {
		return mPdfRawObj;
	}

	int32 DetectIntersection(ED_POINT* strokes, int32 pointCount); // 返回和第几条轨迹相交

public:
	~CpdfHlAnnot();


	virtual uint32 GetType();	// EDPDF_ANNOT_INK ,EDPDF_ANNOT_UNDERL, EDPDF_ANNOT_DELETE,EDPDF_ANNOT_HIGHLIGHT, EDPDF_ANNOT_TEXT
	virtual char* GetTypeName();		// "ink" "text" "highlight" "underline" "deleteline" or "Identity"

	IEdTextAnnot_ptr ConvertToTextAnnot(void) { return NULL; }
	IEdInkAnnot_ptr ConvertToInkAnnot(void) { return NULL; }
	IEdHighLightAnnot_ptr ConvertToHighLightAnnot(void) { return this; }
	IEdDeleteLineAnnot_ptr ConvertToDeleteLineAnnot(void) { return NULL; }
	IEdUnderLineAnnot_ptr ConvertToUnderLineAnnot(void) { return NULL; }

	uint32 SaveToAchive(buf_ptr buf, uint32 bufSize);	// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)

	void SetColor(ED_COLOR clr);		// 设置线条颜色
	ED_COLOR GetColor(void);

	void SetBorder(int32 border);	// 设置线条宽度 1 ~ N
	int32 GetBorder(void);

};



#endif//_HIGHLIGHT_H_