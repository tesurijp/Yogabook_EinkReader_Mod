/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#include "stdafx.h"

#include "CommonHeader.h"

#include "XelDataMgr.h"
#include "XelFactoryMgr.h"

DEFINE_BUILTIN_NAME(CXelFactoryMgr)
CXelFactoryMgr::CXelFactoryMgr()
{

}

CXelFactoryMgr::~CXelFactoryMgr()
{
	// 辅助卸载DLL
						
}



HMODULE __stdcall CXelFactoryMgr::LoadDll(const wchar_t* nswDllPath)
{

	HMODULE lpDllHandle;
	OutputDebugString(nswDllPath);
	lpDllHandle = LoadLibraryEx(nswDllPath, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
	if (lpDllHandle == NULL )
		return NULL;

	return lpDllHandle;

}

BOOL __stdcall CXelFactoryMgr::UnLoadDll(HMODULE nhDllHandle)
{

	if (nhDllHandle == NULL)
		return FALSE;

	if(NULL == FreeLibrary(nhDllHandle))
		return FALSE;

	//CXelDataMgr::SingleInstance()->ModLoadedList(NULL, nhDllHandle, NULL, FALSE); //niu 无用代码
	
	return TRUE;

}




IElementFactory* __stdcall CXelFactoryMgr::GetFactInstance(const wchar_t* nswDllPath)
{
	typedef IElementFactory* (*FXuiGetFactory)();
	// 定义DLL的引出函数指针变量
	FXuiGetFactory lPfnXuiGetFactory;
	HMODULE lpDllHandle = NULL;
	IElementFactory* lpELementFact = NULL;

	do 
	{
		lpDllHandle = LoadDll(nswDllPath);
		if (lpDllHandle == NULL)
			break;

		lPfnXuiGetFactory = (FXuiGetFactory)GetProcAddress((HMODULE)lpDllHandle, "XuiGetFactory");
		if (lPfnXuiGetFactory == NULL)
			break;

		// 通过引出函数实例化工厂类，返回指针
		lpELementFact = lPfnXuiGetFactory();
		if(lpELementFact == NULL)
			break;

		CXelDataMgr::SingleInstance()->ModLoadedList(nswDllPath, lpDllHandle, lpELementFact, TRUE);

	} while (false);

	if(lpELementFact == NULL && lpDllHandle!=NULL)
		FreeLibrary(lpDllHandle);

	return lpELementFact;

}