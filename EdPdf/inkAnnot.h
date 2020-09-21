/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _INKANNOT_H_
#define _INKANNOT_H_
#include "PdfmImp.h"
#include "cmmstruct.h"

class CpdfInkStroke {
public:
	float32 x, y;
	CpdfInkStroke() {
	}

	CpdfInkStroke(const CpdfInkStroke& src) {
		*this = src;
	}

	void operator = (const CpdfInkStroke& src) {
		x = src.x;
		y = src.y;
	}
	void operator =(const fz_point& pt) {
		x = pt.x;
		y = pt.y;
	}
};

typedef cmmVector<CpdfInkStroke, 0, 128> CpdfInkStrokeList;
typedef cmmVector<CpdfInkStrokeList*, 0, 16> CpdfInkMultiList;

DECLARE_BUILTIN_NAME(CpdfInkAnnot)
class CpdfInkAnnot : public cmmBaseObject<CpdfInkAnnot, IEdInkAnnot, GET_BUILTIN_NAME(CpdfInkAnnot)>
{
	friend 	class CpdfPage;


protected:
	// 内部变量
	pdf_annot* mPdfRawObj;
	ED_POINT mPosition;
	float mColor[4];
	int mColorN;
	float mBorder;

	cmmVector<CpdfInkStrokeList*, 0, 16> mPdfStrokeLists;	// 存放pdf文件装入的笔迹数据

	CpdfInkAnnot();

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate();

	DEFINE_CUMSTOMIZE_CREATE(CpdfInkAnnot, (), ())

	// 向页面添加INK Annotation，数据来自Stroke List，并装入本对象
	bool PdfCreateAnnot(pdf_page* pageObj, CpdfInkMultiList& strokeList, float* clr, int clrN, float32 border);
	// 从存档装入INK Annotation，数据来自存档，并装入本对象
	bool LoadFromAchive(pdf_page* pageObj, char* archive, int32 bufSize);
	// 将pdf_annotation对象装入到本对象
	bool Load(pdf_annot* annot);

	// 获取签名的内部值
	void GetSignature(ULONG& checksum, USHORT& altData);

	pdf_annot* GetPdfObj() {
		return mPdfRawObj;
	}

	int32 DetectIntersection(ED_POINTF* strokes, int32 pointCount); // 返回和第几条轨迹相交

public:
	~CpdfInkAnnot();


	virtual uint32 GetType();	// EDPDF_ANNOT_INK ,EDPDF_ANNOT_UNDERL, EDPDF_ANNOT_DELETE,EDPDF_ANNOT_HIGHLIGHT, EDPDF_ANNOT_TEXT
	virtual char* GetTypeName();		// "ink" "text" "highlight" "underline" "deleteline" or "Identity"

	IEdTextAnnot_ptr ConvertToTextAnnot(void) { return NULL; }
	IEdInkAnnot_ptr ConvertToInkAnnot(void) { return this; }
	IEdStextAnnot_ptr ConvertToStextAnnot(void) { return NULL; }

	uint32 SaveToAchive(buf_ptr buf, uint32 bufSize);	// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)

	void SetColor(ED_COLOR clr);		// 设置线条颜色
	ED_COLOR GetColor(void);

	void SetBorder(int32 border);	// 设置线条宽度 1 ~ N
	int32 GetBorder(void);

};

class CpdfSmoothMethod {
public:
	CpdfSmoothMethod(CpdfInkStrokeList* list, float threshold);

	static bool SmoothStroke(CpdfInkStrokeList& mStrokePoints, float threshold = 5.0f, float nfPenWidth = 1.0f);	// 2 - 15比较合适

	//三次B样条曲线近似拟合算法，处理曲线平滑,nbisClose为true表示是闭合曲线
	//static bool SmoothStrokeNew(CpdfInkStrokeList& rStrokePoints, bool nbisClose = false);

	static float DistancePointToLine(float x, float y, float x1, float y1, float x2, float y2);
	//点1，点2，和点2，点3的线段是否重叠  
	static int LineIntersectSide(float x, float y, float x1, float y1, float x2, float y2);
private:
	CpdfInkStrokeList* data;
	float mSmoothThreshold;

	void SmoothRecurs(int32 pb, int32 pe);
};



#endif//_INKANNOT_H_