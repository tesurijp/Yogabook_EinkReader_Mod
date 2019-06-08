/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "EinkReader.h"
#include "Einkui.h"
#include "cmmPath.h"
#include <Shellapi.h>
//#include "HellpFun.h"
#include <strsafe.h>
#include <crtdbg.h>
#include "Compile.h"



BOOL MutexPass(void)
{
	BOOL lbReval;

	HANDLE lhEvent = CreateEvent(NULL, FALSE, FALSE, MUTEX_EVENT_SETTINGS);
	if (lhEvent != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			lbReval = FALSE;
			CloseHandle(lhEvent);
		}
		else
			lbReval = TRUE;
	}
	else
		lbReval = TRUE;
	return lbReval;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _AX_TEST_MEMLEAK
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF); // ???ax
	_CrtSetBreakAlloc(1400);
#endif//_AX_TEST_MEMLEAK

	//if (MutexPass() == false)
	//{
	//	//µ¥ÊµÀý¿ØÖÆ
	//	return FALSE;
	//}

	CFilePathName loPath;
	RECT rcWorkArea;

	//Sleep(1000 * 20);

	SetProcessDPIAware();

	SystemParametersInfo(SPI_GETWORKAREA, sizeof(rcWorkArea), &rcWorkArea, 0);

	loPath.SetByModulePathName();
	loPath.Transform(L".\\EinkReader\\EinkReaderLibrary.dll");

	if((rcWorkArea.right-rcWorkArea.left)%2 !=0)
		rcWorkArea.right--;
	if((rcWorkArea.bottom-rcWorkArea.top)%2 != 0)
		rcWorkArea.bottom--;

	try
	{
		//Sleep(1000 * 30);
		EinkuiStart(loPath.GetPathName(),L"homepage",L"Software\\Lenovo\\EinkXctl\\PublicClasses", 1,NULL,L"Eink Reader");
	}
	catch(...) 
	{

	}

	//_CrtDumpMemoryLeaks();

	return 0;
}