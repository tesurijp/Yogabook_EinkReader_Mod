#include "pch.h"
#include "ConvertTask.h"
#include "ScopeGuard.h"
#include <Windows.h>

namespace PDFConvert
{
	ConvertResult g_convertResult = ConvertResult::OK;
	bool g_convertingCompleted = false;
	std::mutex g_setFinishFlagLock;

	// We use a random string as an expected invalid password
	// to test office files with password protection.
	// Office API will 
	//  * ignore password parameter 
	//    if opening file is not password protected (behavior we want)
	//  * report an error silently in API return value
	//    if opening file is password protected and password given is invalid (behavior we want)
	//  * show office application's interface and ask for password 
	//    if file is password protected and no password given. (behavior we don't want).
	const char* g_randomInvalidPasswordA = R"###(4StRcEBOi9WiRzRa3n80)###";
	const wchar_t* g_randomInvalidPasswordW = LR"###(4StRcEBOi9WiRzRa3n80)###";

	bool IsFileLocked(const wchar_t* filename);
}

using PDFConvert::ConvertResult;


namespace
{
	wstring g_targetFileName;

	enum class DocumentType : int
	{
		Unknwon = 0,
		WordFileGeneric = 1,
		ExcelFileGeneric = 2,
		PowerPointFileGeneric = 3,
		VisioGeneric = 4,
	};

	std::unordered_map<wstring, DocumentType> g_docTypeMap = {
		{L"doc", DocumentType::WordFileGeneric},
		{L"docx", DocumentType::WordFileGeneric},
		{L"rtf", DocumentType::WordFileGeneric},
		{L"xls", DocumentType::ExcelFileGeneric},
		{L"xlsx", DocumentType::ExcelFileGeneric},
		{L"csv", DocumentType::ExcelFileGeneric},
		{L"ppt", DocumentType::PowerPointFileGeneric},
		{L"pptx", DocumentType::PowerPointFileGeneric},
		{L"vsd", DocumentType::VisioGeneric},
		{L"vsdx", DocumentType::VisioGeneric},
	};

	std::unordered_map<
		DocumentType, 
		std::function<void(wstring, wstring, ConvertResult*, bool*, std::mutex*)>
	> g_taskMap = {
		{ DocumentType::WordFileGeneric, PDFConvert::DoWordConvert_cs },
		{ DocumentType::ExcelFileGeneric, PDFConvert::DoExcelConvert_cs },
		{ DocumentType::PowerPointFileGeneric, PDFConvert::DoPowerPointConvert_cs },
		{ DocumentType::VisioGeneric, PDFConvert::DoVisioConvert_cs },
	};

	ConvertResult g_convertResult = ConvertResult::OK;
	std::mutex g_setFinishFlagLock;

	wstring LowerString(const wstring& source)
	{
		wstring result;

		for (auto c : source)
		{
			if (c >= 'A' && c <= 'Z')
				result.push_back(c + 'a' - 'A');
			else
				result.push_back(c);
		}

		return result;
	}

	//通过源文件，获取文件类型 & 转换后的pdf文件名
	std::tuple<wstring, DocumentType> ConstructConvertParams(const wstring& sourceFileName)
	{
		size_t dotPosition = sourceFileName.rfind(L".");

		if (dotPosition == sourceFileName.npos)
		{
			g_convertResult = ConvertResult::UnsupportedFileType;
			return std::make_tuple(L"", DocumentType::Unknwon);
		}

		wstring extName = LowerString(sourceFileName.substr(dotPosition + 1));

		auto queryResult = g_docTypeMap.find(extName);
		if (queryResult == g_docTypeMap.end())
		{
			g_convertResult = ConvertResult::UnsupportedFileType;
			return std::make_tuple(L"", DocumentType::Unknwon);
		}

		return std::make_tuple(wstring(sourceFileName + L".pdf"), queryResult->second);
	}
}

namespace PDFConvert
{
	bool IsConvertingCompleted()
	{
		std::unique_lock<std::mutex> lock(g_setFinishFlagLock);
		ON_SCOPE_EXIT([&] {lock.unlock(); });
		return g_convertingCompleted;
	}

	ConvertResult GetConvertResult()
	{
		std::unique_lock<std::mutex> lock(g_setFinishFlagLock);
		ON_SCOPE_EXIT([&] {lock.unlock(); });
		return g_convertResult;
	}

	void SubmitConvertTask(const wstring& sourceFileName)
	{
		g_convertingCompleted = false;

		DocumentType documentType;
		wstring targetFileName;
		std::tie(targetFileName, documentType) = ConstructConvertParams(sourceFileName);

		g_targetFileName = targetFileName;

		if (g_convertResult == ConvertResult::UnsupportedFileType)//在ConstructConvertParams中，如果文件类型不支持，会修改这个值
		{
			g_convertingCompleted = true;
			return;
		}

		auto func = g_taskMap.find(documentType);
		if (func == g_taskMap.end())
		{
			g_convertingCompleted = true;
			return;
		}
		std::thread t1(std::bind(
			func->second,
			sourceFileName,
			targetFileName,
			&g_convertResult,
			&g_convertingCompleted,
			&g_setFinishFlagLock
		));
		t1.detach();
	}

	wstring GetPDFFileFullPath()
	{
		return g_targetFileName;
	}

	bool IsFileLocked(const wchar_t* filename)
	{
		HANDLE fh = NULL;
		fh = CreateFileW(filename, GENERIC_READ, 0 /* no sharing! exclusive */, NULL, OPEN_EXISTING, 0, NULL);
		if (fh == NULL || fh == INVALID_HANDLE_VALUE)
		{
			auto err = GetLastError();
			if (err == ERROR_SHARING_VIOLATION)
				return true;
		}

		CloseHandle(fh);
		return false;
	}

	int callCsApp(wstring strType, wstring strFile)
	{
		STARTUPINFOW sui = { 0 };
		sui.cb = sizeof(sui);
		sui.dwFlags = STARTF_USESHOWWINDOW;
		sui.wShowWindow = SW_HIDE;
		PROCESS_INFORMATION pi = { 0 };

		wstring strApp; //L"C:\\Program Files\\Lenovo\\EinkSDK\\UwpAgent.exe";
		wchar_t lszFilePath[MAX_PATH] = {0};
		if (GetModuleFileName(GetModuleHandle(NULL), lszFilePath, MAX_PATH) == 0)//hy 20190723
			return 107;
		*(wcsrchr(lszFilePath, L'\\') + 1) = UNICODE_NULL;
		wcscat_s(lszFilePath, MAX_PATH, L"UwpAgent.exe");
		strApp = lszFilePath;

		wstring strParam = L" ConvertPDF " + strType + L" \"" + strFile + L"\"";
		BOOL bRet = CreateProcessW(strApp.c_str(), (LPWSTR)strParam.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sui, &pi);
		DWORD dd = 0;

		DWORD uid = WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD dwRet = 107;
		GetExitCodeProcess(pi.hProcess, &dwRet);
		return dwRet;
	}
	
	//set registry value
	bool CloseOfficeDefaultWarning(wstring strApp)//Word/PowerPoint/Excel/Visio
	{
		DWORD ldwRet = -1;
		HKEY lhKey = NULL; 
		bool lbRet = false;

		wstring wszPath = L"SOFTWARE\\Microsoft\\Office\\16.0\\";
		wszPath = wszPath + strApp + L"\\Options";
		do
		{
			//打开注册表
			ldwRet = RegOpenKeyEx(HKEY_CURRENT_USER, wszPath.c_str(), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
			if (ldwRet != ERROR_SUCCESS)//如果还没有就创建
			{
				ldwRet = RegCreateKey(HKEY_CURRENT_USER, wszPath.c_str(), &lhKey);
			}
			DWORD dwValue = 0;
			ldwRet = RegSetValueEx(lhKey, L"AlertIfNotDefault", NULL, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
			lbRet = true;
		} while (false);

		if (lhKey != NULL)
			RegCloseKey(lhKey);

		return lbRet;
	}
}
