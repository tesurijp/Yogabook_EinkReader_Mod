/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "cmmstruct.h"
#include "cmmBaseObj.h"
#include "FactoryImp.h"








//月try让他


//更符合规划
//BOOL APIENTRY DllMain( HMODULE hModule,
//                       DWORD  ul_reason_for_call,
//                       LPVOID lpReserved
//					 )
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//	case DLL_THREAD_ATTACH:
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		break;
//	}
//	return TRUE;
//}


// 实例化工厂类，传出接口指针
IElementFactory* XuiGetFactory()
{
	return CFactoryImp::GetUniqueObject();

}

void GetWidgetInformation(OUT wchar_t* npIconName,				//在添加页中显示的图标名称
	OUT wchar_t* npPreviewPicName,		//添加的时候显示的预览图名称
	OUT LONG& rlWidth,					//页宽
	OUT LONG& rlHeight					//页高
	)
{

}