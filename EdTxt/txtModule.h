/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EDPDFIMP_H_
#define _EDPDFIMP_H_
#include "txtmImp.h"
#include "GdipStart.h"


DECLARE_BUILTIN_NAME(CEdtxtModule)
class CEdtxtModule : public cmmBaseObject<CEdtxtModule, IEdModule, GET_BUILTIN_NAME(CEdtxtModule)>
{
	friend 	IEdModule_ptr __stdcall EdGetModule(void);

public:
	//enum ERENDER_STEP {
	//	eRenderBegin = 1,
	//	eRenderParepare = 2,
	//	eRenderRender = 3,
	//	eRenderEnd = 4,
	//	eRenderStop = 5
	//};

	// 获取支持的文件格式数量
	int32 GetTypeCount(void);
	// 获取支持的文件格式的扩展名
	const char16_ptr GetTypeName(int32 index);

	// 打开文档
	ED_ERR OpenDocument(
		IN char16_ptr pathName,
		OUT IEdDocument_ptr* documentPtrPtr,
		IN int32 asType // -1 not indicated
	);


	// 获得唯一对象
	static CEdtxtModule* GetUniqueObject(void);

public:
	// 唯一实例
	static CEdtxtModule* glModule;

protected:
	// 内部变量
	//CGdipStart mGdipStart;

	CEdtxtModule();
	~CEdtxtModule();

	DEFINE_CUMSTOMIZE_CREATE(CEdtxtModule, (), ())

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate();

};



#endif//_EDPDFIMP_H_