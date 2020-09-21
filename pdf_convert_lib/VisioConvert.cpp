#include "pch.h"
#include "ConvertTask.h"
#include "ConvertUtil.h"
#include <windows.h>
#include <winuser.h>

#import "./import/MSO.dll" \
	rename_namespace("MSO"), \
	auto_rename

#import "./import/VISLIB.DLL" \
	raw_interfaces_only, \
    rename("ExitWindows", "VisioExitWindows"), \
    rename("ReplaceText", "VisioReplaceText"), \
    rename("FindText", "VisioFindText"), \
    rename("RGB", "VisioRGB"), \
	rename_namespace("Visio")


namespace PDFConvert
{
	extern const char* g_randomInvalidPasswordA;
	extern const wchar_t* g_randomInvalidPasswordW;

	bool IsFileLocked(const wchar_t* filename);
	int callCsApp(wstring strType, wstring strFile);
	bool CloseOfficeDefaultWarning(wstring strApp);//Word/PowerPoint/Excel/Visio

	void DoVisioConvert_cs(wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock)
	{
		if (!ConvertUtil::VisoInstalled())
		{
			*convertResult = ConvertResult::OfficeComponentNotInstalled;
			*convertCompleted = true;
			return;
		}

		if (IsFileLocked(sourceFilePath.c_str()))
		{
			*convertResult = ConvertResult::FileLocked;
			*convertCompleted = true;
			return;
		}

		if (IsFileLocked(targetFilePath.c_str()))
		{
			*convertResult = ConvertResult::PDFFileLocked;
			*convertCompleted = true;
			return;
		}
		CloseOfficeDefaultWarning(L"Visio");//Word/PowerPoint//Visio

		ON_SCOPE_EXIT([=] {
			std::unique_lock<std::mutex> lock(*setFinishFlagLock);
			*convertCompleted = true;
			lock.unlock();
		});

		wstring strType = L"visio";
		int cvtRet = callCsApp(strType, sourceFilePath) - 100;
		*convertResult = ConvertResult(cvtRet);
		return;
	}

}
