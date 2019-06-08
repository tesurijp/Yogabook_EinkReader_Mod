/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "smtDocument.h"
#include "smtModule.h"




#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Mincore.lib")
#pragma comment(lib,"Windowscodecs.lib")

#ifdef _DEBUG
#pragma comment(lib,"..\\sumatrapdf\\dbg64\\utils.lib")
#pragma comment(lib,"..\\sumatrapdf\\dbg64\\libmupdf.lib")
#pragma comment(lib,"..\\sumatrapdf\\dbg64\\chm.lib")
#else
#pragma comment(lib,"..\\sumatrapdf\\rel64\\utils.lib")
#pragma comment(lib,"..\\sumatrapdf\\rel64\\libmupdf.lib")
#pragma comment(lib,"..\\sumatrapdf\\rel64\\chm.lib")
#endif

DEFINE_BUILTIN_NAME(CSmtModule)
CSmtModule* CSmtModule::glModule = NULL;



// 获取模块对象
IEdModule_ptr __stdcall EdGetModule(void)
{
	CSmtModule* uniqueObj = CSmtModule::GetUniqueObject();             

	return uniqueObj;
}



CSmtModule::CSmtModule()
{
}

CSmtModule::~CSmtModule()
{
	mGidpStart.UnInit();
}

// 获得唯一对象
CSmtModule* CSmtModule::GetUniqueObject(void)
{
	if (glModule == NULL)
		CSmtModule::CreateInstance();

	CMMASSERT(glModule != NULL);

	return glModule;
}

ULONG CSmtModule::InitOnCreate(void) {
	if (glModule != NULL)
		return EDERR_OBJECT_EXISTED;

	glModule = this;


	return EDERR_SUCCESS;
}

// 获取支持的文件格式数量
int32Eink CSmtModule::GetTypeCount(void)
{
	return 3;
}

// 获取支持的文件格式的扩展名
const char16_ptr CSmtModule::GetTypeName(int32Eink index)
{
	char16_ptr typeString;

	switch (index)
	{
	case 0:
		typeString = L"PDF";
		break;
	case 1:
		typeString = L"EPUB";
		break;
	case 2:
		typeString = L"MOBI";
		break;
	//case 3:
	//	typeString = L"TXT";
	//	break;
	default:
		typeString = L"";
	}

	return typeString;
}


// 打开文档
ED_ERR CSmtModule::OpenDocument(
	IN char16* pathName,
	OUT IEdDocument_ptr* documentPtrPtr,
	IN int32Eink asType // -1 not indicated
)
{
	if (pathName == NULL || pathName[0] == 0 || documentPtrPtr == NULL)
		return EDERR_WRONG_PARAMETERS;

	mGidpStart.Init();

	auto docObj = CSmtDocument::CreateInstance(pathName);
	if(docObj == NULL)
		return EDERR_UNSUCCESSFUL;

	*documentPtrPtr = docObj;
	return EDERR_SUCCESS;
}

