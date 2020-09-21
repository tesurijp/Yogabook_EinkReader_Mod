/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "txtDocument.h"
#include "txtModule.h"


DEFINE_BUILTIN_NAME(CEdtxtModule)
CEdtxtModule* CEdtxtModule::glModule = NULL;



// 获取模块对象
IEdModule_ptr __stdcall EdGetModule(void)
{
	CEdtxtModule* uniqueObj = CEdtxtModule::GetUniqueObject();

	return uniqueObj;
}



CEdtxtModule::CEdtxtModule()
{
}

CEdtxtModule::~CEdtxtModule()
{
	//mGdipStart.UnInit();
}

// 获得唯一对象
CEdtxtModule* CEdtxtModule::GetUniqueObject(void)
{
	if (glModule == NULL)
		CEdtxtModule::CreateInstance();

	CMMASSERT(glModule != NULL);

	return glModule;
}

ULONG CEdtxtModule::InitOnCreate(void) {
	if (glModule != NULL)
		return EDERR_OBJECT_EXISTED;

	glModule = this;

	return EDERR_SUCCESS;
}

// 获取支持的文件格式数量
int32 CEdtxtModule::GetTypeCount(void)
{
	return 1;
}

// 获取支持的文件格式的扩展名
const char16_ptr CEdtxtModule::GetTypeName(int32 index)
{
	if(index == 0)
		return L"TXT";

	return L"";
}


// 打开文档
ED_ERR CEdtxtModule::OpenDocument(
	IN char16* pathName,
	OUT IEdDocument_ptr* documentPtrPtr
)
{
	if (pathName == NULL || pathName[0] == 0 || documentPtrPtr == NULL)
		return EDERR_WRONG_PARAMETERS;

	//mGdipStart.Init();

	if (_wcsicmp(wcsrchr(pathName, L'.'), L".txt") != 0)
		return EDERR_UNKOWN_DOCTYPE;

	auto docObj = CtxtDocument::CreateInstance(pathName);
	if(docObj == NULL)
		return EDERR_UNSUCCESSFUL;

	*documentPtrPtr = docObj;
	return EDERR_SUCCESS;
}
