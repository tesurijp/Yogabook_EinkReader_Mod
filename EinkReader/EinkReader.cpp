/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "EinkReader.h"
#include "Einkui.h"
#include "cmmPath.h"
#include <Shellapi.h>
//#include "HellpFun.h"
#include <strsafe.h>
#include <crtdbg.h>
#include "Compile.h"

HANDLE lhEvent = NULL;  //change by xingej1

using std::wstring;
using std::vector;
using std::unique_ptr;
using std::make_unique;
const wchar_t* kReaderTempPath = L"EInkReader";
const wchar_t* kCommandLineParamFile = L"cmdparam";

BOOL MutexPass(void)
{
	BOOL lbReval;

	lhEvent = CreateEvent(NULL, FALSE, FALSE, MUTEX_EVENT_SETTINGS);
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

std::vector<std::wstring> SplitString(const std::wstring& s, const std::wstring& c)
{
	std::vector<std::wstring> result;

	std::wstring::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::wstring::npos != pos2)
	{
		result.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		result.push_back(s.substr(pos1));
	return result;
}

bool DirectoryExists(const wchar_t* szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void ProcessCommandLineParam(const wchar_t* commandLine)
{
	std::vector<wstring> paramList = SplitString(commandLine, L" ");
	if (paramList.size() < 2) return;
	if (paramList[0] != L"-open") return;

	const int bufferSize = 1024;
	unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(bufferSize);
	GetTempPathW(bufferSize, buffer.get());

	std::wostringstream stream;
	stream << buffer.get() << L"\\" << kReaderTempPath;
	wstring tempPath = stream.str();
	if (!DirectoryExists(tempPath.c_str()))
	{
		CreateDirectoryW(tempPath.c_str(), nullptr);
	}
	stream << L"\\" << kCommandLineParamFile;
	FILE* f = nullptr;
	_wfopen_s(&f, stream.str().c_str(), L"wt,ccs=utf-8");
	if (f == nullptr) return;
	fwprintf_s(f, L"%s", commandLine);
	fclose(f);
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

	ProcessCommandLineParam(lpCmdLine);

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
	CloseHandle(lhEvent);//change by xingej1
	lhEvent = NULL;

	return 0;
}