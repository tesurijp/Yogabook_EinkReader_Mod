#pragma once

#include <cstdint>
#include <vector>
#include <string>

using std::wstring;
using std::vector;

namespace EInkReaderUtil
{
	enum class NameCaseOption : int
	{
		KeepAsIs,
		ToUpper,
		ToLower,
	};
	extern const vector<wstring> kOfficeExtentions;
	bool IsPathDirectory(const wchar_t* path);
	bool IsPathDirectory(const wstring& path);
	wstring GetFileExtention(const wchar_t* path, NameCaseOption caseOption = NameCaseOption::KeepAsIs);
	wstring GetFileExtention(const wstring& path, NameCaseOption caseOption = NameCaseOption::KeepAsIs);
	bool IsOfficeFileName(const wchar_t* path);
	bool IsOfficeFileName(const wstring& path);
	bool IsFileExists(const wchar_t* path);
	bool IsFileExists(const wstring& path);
	int64_t GetFileSize(const wchar_t* path);
	int64_t GetFileSize(const wstring& path);
	bool IsFileLocked(const wchar_t* filename);
	bool IsFileLocked(const wstring& filename);
}
