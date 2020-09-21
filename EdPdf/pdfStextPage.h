/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _EDPDFSTEXTPAGEIMP_H_
#define _EDPDFSTEXTPAGEIMP_H_
#include "PdfmImp.h"
#include "pdfImg.h"
#include "pdfAnnot.h"
#include "inkAnnot.h"
#include "vector"

using namespace std;

class CpdfPage;

DECLARE_BUILTIN_NAME(CpdfStextPage)
class CpdfStextPage : public cmmBaseObject<CpdfStextPage, IEdStructuredTextPage, GET_BUILTIN_NAME(CpdfStextPage)>
{
public:
	// 探测文本选中情况
	virtual bool DetectSelectedText(
		IN const ED_POINTF* aPoint,
		IN const ED_POINTF* bPoint,
		OUT IEdStextQuadList_ptr* stext,	// 返回结构化文本对象，需要释放
		OUT IEdAnnotList_ptr* impactedAnnot,	// 返回碰撞的已有Annot对象列表，需要释放
		IN bool32 includeImpacted	// 将碰撞的annot区域也加入选区
	);	// 返回的对象需要释放

	// 复制目标区域文本，返回值是复制的字符数，不包含结尾0
	virtual int32Eink CopyText(
		IN ED_RECTF_PTR selBox,
		OUT char16_ptr textBuf,	// 传入NULL，函数返回实际的字符数，不含结尾0
		IN int32Eink bufSize
	);

	DEFINE_CUMSTOMIZE_CREATE(CpdfStextPage, (CpdfPage* page), (page))

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(CpdfPage* page);
protected:
	// 内部变量
	CpdfPage* mNotStextPage;
	//fz_page *mFzPage;
	fz_stext_page* mStextPage;
	
	CpdfStextPage();
	~CpdfStextPage();


};



#endif//_EDPDFSTEXTPAGEIMP_H_