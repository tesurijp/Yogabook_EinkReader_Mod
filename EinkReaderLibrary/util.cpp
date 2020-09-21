#include "stdafx.h"
#include "util.h"
#include <cstring>
#include <algorithm>
#include <memory>
#include <cwctype>
#include <Windows.h>

using std::transform;
using std::find;

namespace EInkReaderUtil
{
	const vector<wstring> kOfficeExtentions{
		L".docx",
		L".doc",
		L".xlsx",
		L".xls",
		L".pptx",
		L".ppt",
		L".vsdx",
		L".vsd",
	};

	bool IsPathDirectory(const wchar_t * path)
	{
		return (GetFileAttributesW(path) & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	bool IsPathDirectory(const wstring & path)
	{
		return IsPathDirectory(path.c_str());
	}

	wstring GetFileExtention(const wchar_t * path, NameCaseOption caseOption)
	{
		return GetFileExtention(wstring(path), caseOption);
	}

	wstring GetFileExtention(const wstring & path, NameCaseOption caseOption)
	{
		std::wstring fileExtention;
		size_t pos = path.rfind(L'.');
		if (pos == std::wstring::npos)
			return wstring();
		fileExtention = path.substr(pos, std::wstring::npos);

		switch (caseOption)
		{
		case NameCaseOption::ToUpper:
			std::transform(fileExtention.begin(), fileExtention.end(), fileExtention.begin(), [](wchar_t c) {
				return std::towupper(c);
			});
			break;

		case NameCaseOption::ToLower:
			std::transform(fileExtention.begin(), fileExtention.end(), fileExtention.begin(), [](wchar_t c) {
				return std::towlower(c);
			});
			break;

		}
		return fileExtention;
	}

	bool IsOfficeFileName(const wchar_t * path)
	{
		return IsOfficeFileName(wstring(path));
	}

	bool IsOfficeFileName(const wstring & path)
	{
		wstring extName = GetFileExtention(path);
		transform(extName.begin(), extName.end(), extName.begin(), tolower);

		return find(kOfficeExtentions.begin(), kOfficeExtentions.end(), extName) != kOfficeExtentions.end();
	}

	bool IsFileExists(const wchar_t * path)
	{
		auto result = GetFileAttributesW(path);
		return result != INVALID_FILE_ATTRIBUTES && !(result & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool IsFileExists(const wstring & path)
	{
		return IsFileExists(path.c_str());
	}

	int64_t GetFileSize(const wchar_t * path)
	{
		FILE* f = nullptr;
		_wfopen_s(&f, path, L"rb");
		if (f == nullptr) return 0;
		fseek(f, 0, SEEK_END);
		auto result = _ftelli64(f);
		fclose(f);
		return result;
	}

	int64_t GetFileSize(const wstring & path)
	{
		return GetFileSize(path.c_str());
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
			else
				return false;
		}

		CloseHandle(fh);
		return false;
	}
	bool IsFileLocked(const wstring & filename)
	{
		return IsFileLocked(filename.c_str());
	}
}
