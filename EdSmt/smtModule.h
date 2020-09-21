/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EDPDFIMP_H_
#define _EDPDFIMP_H_
#include "smtmImp.h"

#include "BaseUtil.h"
#include <wininet.h>
#include "WinDynCalls.h"
#include "CryptoUtil.h"
#include "DirIter.h"
#include "Dpi.h"
#include "FileUtil.h"
#include "FileWatcher.h"
//#include "FrameRateWnd.h"
#include "GdiPlusUtil.h"
//#include "LabelWithCloseWnd.h"
#include "HtmlParserLookup.h"
#include "HttpUtil.h"
//#include "Mui.h"
//#include "SplitterWnd.h"
#include "SquareTreeParser.h"
#include "ThreadUtil.h"
#include "UITask.h"
#include "WinUtil.h"
// rendering engines
#include "BaseEngine.h"
#include "PsEngine.h"
#include "Engineoverride.h"
#include "Doc.h"
#include "FileModifications.h"
#include "PdfCreator.h"

#include "GdipStart.h"

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



DECLARE_BUILTIN_NAME(CSmtModule)
class CSmtModule : public cmmBaseObject<CSmtModule, IEdModule, GET_BUILTIN_NAME(CSmtModule)>
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
	int32Eink GetTypeCount(void);
	// 获取支持的文件格式的扩展名
	const char16_ptr GetTypeName(int32Eink index);

	// 打开文档
	ED_ERR OpenDocument(
		IN char16_ptr pathName,
		OUT IEdDocument_ptr* documentPtrPtr
	);

	// 获得唯一对象
	static CSmtModule* GetUniqueObject(void);

public:
	// 唯一实例
	static CSmtModule* glModule;

protected:
	// 内部变量
	CGdipStart mGidpStart;

	CSmtModule();
	~CSmtModule();

	DEFINE_CUMSTOMIZE_CREATE(CSmtModule, (), ())

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate();

};



#endif//_EDPDFIMP_H_