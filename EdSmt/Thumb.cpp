/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#include "windows.h"
#include "thumb.h"
#include <Wincrypt.h>

#define BUFSIZE 1024
//
//bool HashBuffer(cmmStringW& input,cmmStringW& output)
//{
//	HCRYPTPROV hProv = 0;
//	HCRYPTHASH hHash = 0;
//	BYTE resultBuf[128];
//	DWORD cbHash = 0;
//
//	// Get handle to the crypto provider
//	if (!CryptAcquireContext(&hProv,
//		NULL,
//		NULL,
//		PROV_RSA_FULL,
//		CRYPT_VERIFYCONTEXT))
//	{
//		return false;
//	}
//
//	if (!CryptCreateHash(hProv, CALG_MD5/*CALG_SSL3_SHAMD5*/, 0, 0, &hHash))
//	{
//		return false;
//	}
//
//	if (!CryptHashData(hHash,(BYTE*)input.ptr(), input.size()*sizeof(wchar_t), 0))
//	{
//		return false;
//	}
//
//	cbHash = 128;
//	if (!CryptGetHashParam(hHash, HP_HASHVAL, resultBuf, &cbHash, 0))
//		return false;
//
//	if (output.DumpBinBuf(resultBuf, cbHash) == false)
//		return false;
//
//	CryptDestroyHash(hHash);
//	CryptReleaseContext(hProv, 0);
//
//	return true;
//}
//
bool CSmtThumb::SetDocument(const wchar_t* nphumbnailPathName)
{
	mCachePath.SetPathName(nphumbnailPathName);
	//cmmStringW trail;
	//cmmStringW hashName;
	//FILETIME createTime, accessTime, modifyTime;
	//HANDLE fileHandle;

	//fileHandle = CreateFile(docPathName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (fileHandle == INVALID_HANDLE_VALUE || GetFileTime(fileHandle,&createTime,&accessTime,&modifyTime) == FALSE)
	//{
	//	modifyTime.dwLowDateTime = modifyTime.dwHighDateTime = 0;
	//}
	//if (fileHandle != INVALID_HANDLE_VALUE)
	//	CloseHandle(fileHandle);

	//trail.format(L"%s:%X%X", docPathName,modifyTime.dwLowDateTime, modifyTime.dwHighDateTime);

	//if (HashBuffer(trail,hashName) == false)
	//	return false;

	//mCachePath.SetByUserAppData();
	//mCachePath.AssurePath();
	//mCachePath += L"EinkReader\\";
	//mCachePath += hashName.ptr();
	//mCachePath.AssurePath();

	////如果没有这个目录，则建立这个目录
	//mCachePath.CreatePath();

	return true;
}

const wchar_t* CSmtThumb::GetCachePath(void)
{
	return mCachePath.GetPathName();
}


void CSmtThumb::GenerateThumbFilePathName(PPAGE_PDF_CONTEXT p1, PPAGE_PDF_CONTEXT p2, cmmStringW& thumbPath)
{
	auto len = p2->pageContext - p1->pageContext;

	thumbPath.format(L"%s%06X-%04X.png", mCachePath.GetPathName(), p1->pageContext, len);
}

bool CSmtThumb::CheckThumbnailFile(cmmStringW& thumbPath)
{
	if (thumbPath.size() > 0 && GetFileAttributes(thumbPath.ptr()) != INVALID_FILE_ATTRIBUTES)
		return true;

	return false;
}

