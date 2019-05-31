/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */



#pragma once

#ifdef UNICODE

#define GetTextFileCodeType	GetTextFileCodeTypeW

#else
#define GetTextFileCodeType	GetTextFileCodeTypeA

#endif

#include "cmmBaseDef.h"
#include <windows.h>
#include <string>	// for string
using std::string;
using std::wstring;

namespace cmmStrHandle{

//////////////////////////////////////////////////////////////////////////
// 字符串编码转换

// wide characters to multi bytes characters by specified code page
bool RealWideCharToMultibyte(IN const UINT nuiCodePage, IN const wstring& roWStrToConvert, OUT string& roStrConverted);

// multi bytes characters to wide characters by specified code page
bool RealMultibyteToWideChar(IN const UINT nuiCodePage, IN const string& roStrToConvert, OUT wstring& roWStrConverted);

// unicode to ansi
bool UnicodeToAnsi(IN const wstring& roWStrToConvert, OUT string& roStrConverted);

// unicode to utf-8
bool UnicodeToUtf8(IN const wstring& roWStrToConvert, OUT string& roStrConverted);

// ansi to unicode
bool AnsiToUnicode(IN const string& roStrToConvert, OUT wstring& roWStrConverted);

// ansi to utf-8
bool AnsiToUtf8(IN const string& roStrToConvert, OUT string& roStrConverted);

// utf-8 to unicode
bool Utf8ToUnicode(IN const string& roStrToConvert, OUT wstring& roWStrConverted);

// utf-8 to ansi
bool Utf8ToAnsi(IN const string& roStrToConvert, OUT string& roStrConverted);


//////////////////////////////////////////////////////////////////////////
// 编码辅助

// get file code type
enum FileCodeType
{
	FCT_ERROR = 0,
	FCT_ANSI,
	FCT_UNICODE,
	FCT_UTF8,
	FCT_UNICODE_BIG_ENDIAN
};
FileCodeType GetTextFileCodeTypeA(IN const string& roStrFileName);
FileCodeType GetTextFileCodeTypeW(IN const wstring& roWStrFileName);


//////////////////////////////////////////////////////////////////////////
// 字符转转换

// number converts to characters
bool Int64ToStr(IN const LONGLONG nllIntegerToConvert, OUT string& roStrConverted);

bool Int64ToWStr(IN const LONGLONG nllIntegerToConvert, OUT wstring& roWStrConverted);

bool UInt64ToStr(IN const ULONGLONG nullIntegerToConvert, OUT string& roStrConverted);

bool UInt64ToWStr(IN const ULONGLONG nullIntegerToConvert, OUT wstring& roWStrConverted);



//////////////////////////////////////////////////////////////////////////
// 文件路径操作

// 描述：
//		获取给定的文件路径存放时最终的唯一路径（不与已经存在冲突）（通过在文件名后面累加数字)
bool GetUniqueFilePath(IN const wchar_t* nswFilePath, OUT wchar_t* nswUniquePath, IN int niCchBuffer);
}