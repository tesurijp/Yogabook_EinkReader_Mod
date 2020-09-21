// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "cmmstruct.h"
#include "cmmBaseObj.h"

#include "Einkui.h"

#include "ElementImp.h"
#include "mapDefine.h"
#include "mapList.h"

#include "FactoryImp.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


// 实例化工厂类，传出接口指针
IElementFactory* XuiGetFactory()
{
	return CFactoryImp::GetUniqueObject();
}