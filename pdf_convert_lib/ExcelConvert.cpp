#include "pch.h"
#include "ConvertTask.h"
#include "ConvertUtil.h"
#include <windows.h>
#include <winuser.h>
#include <string>

#import "./import/MSO.dll" \
	rename_namespace("MSO"), \
	auto_rename

#import "./import/EXCEL.EXE" \
	raw_interfaces_only, \
	auto_rename, \
    rename( "DialogBox", "ExcelDialogBox" ), \
    rename( "RGB", "ExcelRGB" ), \
    rename( "CopyFile", "ExcelCopyFile" ), \
    rename( "ReplaceText", "ExcelReplaceText" ), \
    exclude( "IFont", "IPicture" ), \
	rename_namespace("Excel")

namespace PDFConvert
{
	extern const char* g_randomInvalidPasswordA;
	extern const wchar_t* g_randomInvalidPasswordW;

	bool IsFileLocked(const wchar_t* filename);
	int callCsApp(wstring strParam, wstring strFile);
	bool CloseOfficeDefaultWarning(wstring strApp);//Word/PowerPoint/Excel/Visio

	void DoExcelConvert_cs(wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock)
	{
		if (!ConvertUtil::ExcelInstalled())
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
		CloseOfficeDefaultWarning(L"Excel");//Word/PowerPoint//Visio

		ON_SCOPE_EXIT([=] {
			std::unique_lock<std::mutex> lock(*setFinishFlagLock);
			*convertCompleted = true;
			lock.unlock();
		});

		wstring strType = L"excel";
		int cvtRet = callCsApp(strType, sourceFilePath) - 100;
		*convertResult = ConvertResult(cvtRet);
		return;
	}
}

