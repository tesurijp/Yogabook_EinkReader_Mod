#include "pch.h"
#include "ConvertTask.h"
#include "ConvertUtil.h"
#include <windows.h>
#include <winuser.h>

#import "./import/MSO.dll" \
	rename_namespace("MSO"), \
	auto_rename

#import "./import/MSWORD.OLB" \
	raw_interfaces_only, \
    rename("ExitWindows", "WordExitWindows"), \
    rename("ReplaceText", "WordReplaceText"), \
    rename("FindText", "WordFindText"), \
    rename("RGB", "WordRGB"), \
	rename_namespace("Word")


namespace PDFConvert
{
	extern const char* g_randomInvalidPasswordA;
	extern const wchar_t* g_randomInvalidPasswordW;

	bool IsFileLocked(const wchar_t* filename);
	int callCsApp(wstring strType, wstring strFile);
	bool CloseOfficeDefaultWarning(wstring strApp);//Word/PowerPoint/Excel/Visio

	void DoWordConvert_cs(wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock)
	{
		if (!ConvertUtil::WordInstalled())
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
		CloseOfficeDefaultWarning(L"Word");//Word/PowerPoint/Excel/Visio

		ON_SCOPE_EXIT([=] {
			std::unique_lock<std::mutex> lock(*setFinishFlagLock);
			*convertCompleted = true;
			lock.unlock();
		});

		wstring strType = L"word";
		wstring strFile = sourceFilePath;
		int cvtRet = callCsApp(strType, strFile) -100;
		*convertResult = ConvertResult(cvtRet);

		return;
	}


}
