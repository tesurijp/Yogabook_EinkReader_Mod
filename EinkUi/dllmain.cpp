/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "CommonHeader.h"
#include "EinkIteAPI.h"


// {EA3270AA-3D8B-4246-AE7F-0D65F3D7BD60}
static const GUID EGUID_EMGR_DROP_TEST = 
{ 0xea3270aa, 0x3d8b, 0x4246, { 0xae, 0x7f, 0xd, 0x65, 0xf3, 0xd7, 0xbd, 0x60 } };



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
		}
		break;
	case DLL_PROCESS_DETACH:
		{
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;;
	}
	return TRUE;
}

// XUI启动函数，如果希望修改DPI，请务必在调用此函数前调用Win API SetProcessDPIAware
int __stdcall EinkuiStart(
	IN const wchar_t* nswModulePath,	// System Widget的实现模块的文件路径名
	IN const wchar_t* nswHomeTempleteName,	// System Widget的Home Page的templete key的名字
	IN const wchar_t* nswClassesRegPath,	// Xui注册类所在的注册表路径，如:Software\\Lenovo\\Veriface5\\PublicClasses
	IN ULONG nuAutoRotate,				// 非零支持自动旋转
	IN PEINKUI_CUSTOMDRAW_SETTINGS npCustomDraw,	// 自绘Eink
	IN const wchar_t* nswWndTittle		// 主窗口标题
	)
{
	int liResult = -1;
	static bool glbStarted=false;
	STES_START_UP ldStart;

	wchar_t lszTemp[MAX_PATH] = {0};
	GetTempPath(MAX_PATH,lszTemp);
	wcscat_s(lszTemp,MAX_PATH,L".\\Xui.trf");
	//Trace_Init(lszTemp,L"XUI.DLL",2,200,L"Xui",L"35EEB40700AF554DB8CCEF09984873EF",0x402CE600,0x1CF8FC4);

	HRESULT hr;
	ID2D1Factory* lpFactory = NULL;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &lpFactory);
	if(FAILED(hr))
		return -1;

	lpFactory->GetDesktopDpi(&ldStart.Dpi.width,&ldStart.Dpi.height);

	lpFactory->Release();

	if(glbStarted != false)
	{
		//Trace_Point(22233);// 重复启动，目前已经启动了
		return -1;
	}

	CoInitialize(NULL);

	CEinkuiSystem* lpXuiSystem = CEinkuiSystem::GetUniqueObject();

	//Trace_Time(29955);//启动XUI系统
	if(lpXuiSystem != NULL)
	{
		ldStart.HomeTempleteName = nswHomeTempleteName;
		ldStart.ModulePathName = nswModulePath;
		ldStart.ClassesRegPath = nswClassesRegPath;
		ldStart.WindowTitle = nswWndTittle;
		ldStart.CustomDraw = npCustomDraw;
		ldStart.WaitingCaller = NULL;
		ldStart.AutoRotate = nuAutoRotate;

		liResult = lpXuiSystem->Startup(ldStart);
		lpXuiSystem->Release();
	}

	CoUninitialize();
	
	//Trace_Exit();

	return liResult;
}

// 获得XUI系统接口
IEinkuiSystem* __stdcall EinkuiGetSystem(void)
{
	return CEinkuiSystem::gpXuiSystem;
}

