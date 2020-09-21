/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _PDFSTEXTLIST_H_
#define _PDFSTEXTLIST_H_
#include "PdfmImp.h"
#include "cmmstruct.h"



DECLARE_BUILTIN_NAME(CpdfStextQuadsList)
class CpdfStextQuadsList : public cmmBaseObject<CpdfStextQuadsList, IEdStextQuadList, GET_BUILTIN_NAME(CpdfStextQuadsList)>
{
protected:
	// 内部变量
	cmmVector<ED_RECTF,16,16> mQuads;
	//ED_POINTF mAPoint, mBPoint;


	CpdfStextQuadsList();

	bool HasIntersection(const ED_RECTF& a, const ED_RECTF& b);
	bool IsSameLine(const ED_RECTF& a, const ED_RECTF& b);

public:
	~CpdfStextQuadsList();
	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	DEFINE_CUMSTOMIZE_CREATE(CpdfStextQuadsList, (void), ())

	void AddQuad(const ED_RECTF& quad);
	void AddQuad(const fz_quad& quad);
	bool Combination(IEdStextQuadList_ptr addition);

	// 获得A点（上止点）
	virtual void GetAPoint(
		OUT ED_POINTF* pt
	);

	// 获得B点（下止点）
	virtual void GetBPoint(
		OUT ED_POINTF* pt
	);

	// 获得分块总数
	virtual int32Eink GetQuadCount(void);

	// 获得一个分块的位置
	virtual void GetQuadBound(
		IN int32Eink index,
		OUT ED_RECTF_PTR quad
	);

	virtual const ED_RECTF* GetQuad(IN int32Eink index);

	int32 DetectIntersection(IEdStextQuadList_ptr checkWith); // 返回和本实例的第几个Quad相交，返回-1表示不相交
	int32 DetectIntersection(IEdStextQuadList_ptr quadList, const ED_RECTF& quad);

};



#endif//_PDFSTEXTLIST_H_