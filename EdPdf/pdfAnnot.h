/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _PDFANNOT_H_
#define _PDFANNOT_H_
#include "PdfmImp.h"
#include "vector"

using namespace std;

DECLARE_BUILTIN_NAME(CpdfdAnnot)
class CpdfdAnnot : public cmmBaseObject<CpdfdAnnot, IEdAnnot, GET_BUILTIN_NAME(CpdfdAnnot)>
{
	friend 	class CpdfPage;


protected:
	// 内部变量

	CpdfdAnnot();
	~CpdfdAnnot();

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	DEFINE_CUMSTOMIZE_CREATE(CpdfdAnnot, (void), ())

public:
	virtual uint32 GetType() {
		return EDPDF_ANNOT_NONE;
	}
	virtual char* GetTypeName() {
		return "identity";
	}

	IEdTextAnnot_ptr ConvertToTextAnnot(void) { return NULL; }
	IEdInkAnnot_ptr ConvertToInkAnnot(void) { return NULL; }
	IEdStextAnnot_ptr ConvertToStextAnnot(void) { return NULL; }

	virtual uint32 SaveToAchive(buf_ptr buf, uint32 bufSize);	// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)
	virtual void Remove(void) {};		// 从文件中删除这个对象，删除后，本接口页还需要调用Release释放

};

class CpdfStextPage;
class CpdfPage;

DECLARE_BUILTIN_NAME(CpdfdAnnotList)
class CpdfdAnnotList : public cmmBaseObject<CpdfdAnnotList, IEdAnnotList, GET_BUILTIN_NAME(CpdfdAnnotList)>
{
	friend CpdfPage;
	friend CpdfStextPage;

protected:
	// 内部变量
	vector<IEdAnnot_ptr> mAnnots;

	void AddAnnot(IEdAnnot_ptr annot);
	void AddAnnots(vector<IEdAnnot_ptr> annots);

	CpdfdAnnotList();

public:
	~CpdfdAnnotList();

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void);

	DEFINE_CUMSTOMIZE_CREATE(CpdfdAnnotList, (void), ())

	virtual int32Eink GetCount(void);
	virtual IEdAnnot_ptr GetAnnot(int32Eink index);	// 返回的对象需要释放

};



#endif//_PDFANNOT_H_