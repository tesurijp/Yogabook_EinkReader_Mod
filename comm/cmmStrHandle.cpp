/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "cmmStrHandle.h"
#include <fstream>
#include <iostream>
#include <Strsafe.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

namespace cmmStrHandle{


//////////////////////////////////////////////////////////////////////////
// 字符串编码转换

// wide characters to multi bytes characters by specified code page
bool RealWideCharToMultibyte(IN const UINT nuiCodePage, IN const wstring& roWStrToConvert, OUT string& roStrConverted)
{
	bool lbResult = false;
	char* lscTempConvertedBuffer = NULL;
	do 
	{
		if(false != roWStrToConvert.empty())
			break;

		// 获取实际需要的输出缓冲字节数（输入缓冲的字符数大小包含了末尾结束符在内）
		int liCbOutputNeed = WideCharToMultiByte(nuiCodePage, NULL, roWStrToConvert.c_str(), -1, NULL, 0, NULL, NULL);

		lscTempConvertedBuffer = new char[liCbOutputNeed];
		ZeroMemory(lscTempConvertedBuffer, liCbOutputNeed * sizeof(char));

		int liCbRealConverted = WideCharToMultiByte(nuiCodePage, NULL, roWStrToConvert.c_str(), -1, lscTempConvertedBuffer, liCbOutputNeed, NULL, NULL);
		if(0 == liCbRealConverted || liCbOutputNeed != liCbRealConverted)
			break;

		roStrConverted = lscTempConvertedBuffer;

		lbResult = true;
	} while (false);

	if(NULL != lscTempConvertedBuffer)
	{
		delete lscTempConvertedBuffer;
		lscTempConvertedBuffer = NULL;
	}
	return lbResult;
}

// multi bytes characters to wide characters by specified code page
bool RealMultibyteToWideChar(IN const UINT nuiCodePage, IN const string& roStrToConvert, OUT wstring& roWStrConverted)
{
	bool lbResult = false;
	wchar_t* lswTempConvertedBuffer = NULL;
	do 
	{
		if(false != roStrToConvert.empty())
			break;

		// 获取实际需要的输出缓冲字节数（输入缓冲的字符数大小包含了末尾结束符在内）
		int liCchOutputNeed = MultiByteToWideChar(nuiCodePage, NULL, roStrToConvert.c_str(), -1, NULL, 0);

		lswTempConvertedBuffer = new wchar_t[liCchOutputNeed];
		ZeroMemory(lswTempConvertedBuffer, liCchOutputNeed * sizeof(wchar_t));

		int liCchRealConverted = MultiByteToWideChar(nuiCodePage, NULL, roStrToConvert.c_str(), -1, lswTempConvertedBuffer, liCchOutputNeed);
		if(0 == liCchRealConverted || liCchOutputNeed != liCchRealConverted)
			break;

		roWStrConverted = lswTempConvertedBuffer;

		lbResult = true;
	} while (false);

	if(NULL != lswTempConvertedBuffer)
	{
		delete lswTempConvertedBuffer;
		lswTempConvertedBuffer = NULL;
	}
	return lbResult;
}

// unicode to ansi
bool UnicodeToAnsi(IN const wstring& roWStrToConvert, OUT string& roStrConverted)
{
	return RealWideCharToMultibyte(CP_ACP, roWStrToConvert, roStrConverted);
}

// unicode to utf-8
bool UnicodeToUtf8(IN const wstring& roWStrToConvert, OUT string& roStrConverted)
{
	return RealWideCharToMultibyte(CP_UTF8, roWStrToConvert, roStrConverted);
}

// ansi to unicode
bool AnsiToUnicode(IN const string& roStrToConvert, OUT wstring& roWStrConverted)
{
	return RealMultibyteToWideChar(CP_ACP, roStrToConvert, roWStrConverted);
}

// ansi to utf-8
bool AnsiToUtf8(IN const string& roStrToConvert, OUT string& roStrConverted)
{
	// convert ansi to unicode first
	wstring loWStrTempConverted;
	if(false == AnsiToUnicode(roStrToConvert, loWStrTempConverted))
		return false;
	else
		return UnicodeToUtf8(loWStrTempConverted, roStrConverted);
}

// utf-8 to unicode
bool Utf8ToUnicode(IN const string& roStrToConvert, OUT wstring& roWStrConverted)
{
	return RealMultibyteToWideChar(CP_UTF8, roStrToConvert, roWStrConverted);
}

// utf-8 to ansi
bool Utf8ToAnsi(IN const string& roStrToConvert, OUT string& roStrConverted)
{
	// convert utf8 to unicode first
	wstring loWStrTempConverted;
	if(false == Utf8ToUnicode(roStrToConvert, loWStrTempConverted))
		return false;
	else
		return UnicodeToAnsi(loWStrTempConverted, roStrConverted);
}

//////////////////////////////////////////////////////////////////////////
// 编码辅助
FileCodeType GetTextFileCodeTypeA(IN const string& roStrFileName)
{
	FileCodeType leFileCodeType = FCT_ERROR;  
	do 
	{
		if(false != roStrFileName.empty())
			break;

		// TODO: 判断文件类型对不对

		std::ifstream file(roStrFileName.c_str());  
		if(false == file.good())
			break;

		// read three bytes from file head
		char lscFlag[3] = {0};
		file.read(lscFlag, sizeof(char) * 3);

		if((unsigned char)lscFlag[0] == 0xFF
			&& (unsigned char)lscFlag[1] == 0xFE)
		{
			leFileCodeType = cmmStrHandle::FCT_UNICODE;
		}
		else if ((unsigned char)lscFlag[0] == 0xEF   
			&& (unsigned char)lscFlag[1] == 0xBB   
			&& (unsigned char)lscFlag[2] == 0xBF)  
		{  
			leFileCodeType = cmmStrHandle::FCT_UTF8;  
		} 
		else if((unsigned char)lscFlag[1] == 0xFF
			&& (unsigned char)lscFlag[0] == 0xFE)
		{
			leFileCodeType = cmmStrHandle::FCT_UNICODE_BIG_ENDIAN;
		}
		else
			leFileCodeType = cmmStrHandle::FCT_ANSI;

	} while (false);

	return leFileCodeType;  
}

FileCodeType GetTextFileCodeTypeW(IN const wstring& roWStrFileName)
{
	string loStrFileName;
	if(false == UnicodeToAnsi(roWStrFileName, loStrFileName))
		return cmmStrHandle::FCT_ERROR;
	else
		return GetTextFileCodeTypeA(loStrFileName);
}

//////////////////////////////////////////////////////////////////////////
// 字符转转换
bool Int64ToStr(IN const LONGLONG nllIntegerToConvert, OUT string& roStrConverted)
{
	bool lbResult = false;
	char lscTempConverted[64] = {0};
	do 
	{
		if(0 != _i64toa_s(nllIntegerToConvert, lscTempConverted, sizeof(lscTempConverted), 10))
			break;

		roStrConverted = lscTempConverted;
		
		lbResult = true;
	} while (false);

	return lbResult;
}


bool Int64ToWStr(IN const LONGLONG nllIntegerToConvert, OUT wstring& roWStrConverted)
{
	bool lbResult = false;
	wchar_t lswTempConverted[64] = {0};
	do 
	{
 		if(0 != _i64tow_s(nllIntegerToConvert, lswTempConverted, sizeof(lswTempConverted) / sizeof(wchar_t), 10))
 			break;

		roWStrConverted = lswTempConverted;

		lbResult = true;
	} while (false);

	return lbResult;
}

bool UInt64ToStr(IN const ULONGLONG nullIntegerToConvert, OUT string& roStrConverted)
{
	bool lbResult = false;
	char lscTempConverted[64] = {0};
	do 
	{
		if(0 != _ui64toa_s(nullIntegerToConvert, lscTempConverted, sizeof(lscTempConverted), 10))
			break;

		roStrConverted = lscTempConverted;

		lbResult = true;
	} while (false);

	return lbResult;
}

bool UInt64ToWStr(IN const ULONGLONG nullIntegerToConvert, OUT wstring& roWStrConverted)
{
	bool lbResult = false;
	wchar_t lswTempConverted[64] = {0};
	do 
	{
		if(0 != _ui64tow_s(nullIntegerToConvert, lswTempConverted, sizeof(lswTempConverted) / sizeof(wchar_t), 10))
			break;

		roWStrConverted = lswTempConverted;

		lbResult = true;
	} while (false);

	return lbResult;
}

//////////////////////////////////////////////////////////////////////////
// 文件路径操作

// 描述：
//		获取给定的文件路径存放时最终的唯一路径（不与已经存在冲突）（通过在文件名后面累加数字)
bool GetUniqueFilePath(IN const wchar_t* nswFilePath, OUT wchar_t* nswUniquePath, IN int niCchBuffer)
{
	bool lbResult = false;
	do 
	{
		if(NULL == nswFilePath || UNICODE_NULL == nswFilePath[0]
		|| NULL == nswUniquePath || niCchBuffer < (int)wcslen(nswFilePath))
			break;
		StringCchCopyW(nswUniquePath, niCchBuffer, nswFilePath);
		wchar_t lswPathNoExt[MAX_PATH] = {0};
		StringCchCopyW(lswPathNoExt, MAX_PATH, nswFilePath);
		PathRemoveExtensionW(lswPathNoExt);

		const wchar_t* lswExtName = PathFindExtensionW(nswFilePath);
		BREAK_ON_NULL(lswExtName);
		int liIndex = 0;
		bool lbIsOk = true;
		while(FALSE != PathFileExistsW(nswUniquePath))			// 已经存在
		{
			if(FAILED(StringCchPrintfW(nswUniquePath, niCchBuffer, L"%s%d%s", lswPathNoExt, liIndex++, lswExtName)))
			{
				lbIsOk = false;
				break;
			}
		}

		BREAK_ON_FALSE(lbIsOk);

		lbResult = true;
	} while (false);
	return lbResult;
}

}