/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#include "windows.h"
#include "PageCtl.h"
#include <Wincrypt.h>
#include <fstream>
#include <iostream>

#define BUFSIZE 1024

bool HashBuffer(cmmStringW& input,cmmStringW& output)
{
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE resultBuf[128];
	DWORD cbHash = 0;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		return false;
	}

	if (!CryptCreateHash(hProv, CALG_MD5/*CALG_SSL3_SHAMD5*/, 0, 0, &hHash))
	{
		return false;
	}

	if (!CryptHashData(hHash,(BYTE*)input.ptr(), input.size()*sizeof(wchar_t), 0))
	{
		return false;
	}

	cbHash = 128;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, resultBuf, &cbHash, 0))
		return false;

	if (output.DumpBinBuf(resultBuf, cbHash) == false)
		return false;

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return true;
}

bool CEdPageControl::SetDocument(const wchar_t* nphumbnailPathName)
{
	//cmmStringW trail;
	//cmmStringW hashName;
	//FILETIME createTime, accessTime, modifyTime;
	//HANDLE fileHandle;

	//// 获取原始文件的修改时间和大小
	//fileHandle = CreateFile(docPathName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//if (fileHandle == INVALID_HANDLE_VALUE)
	//	return false;

	//if(GetFileTime(fileHandle, &createTime, &accessTime, &modifyTime) == FALSE)
	//{
	//	modifyTime.dwLowDateTime = modifyTime.dwHighDateTime = 0;
	//}

	//DWORD hiSize;
	//DWORD lowSize = GetFileSize(fileHandle, &hiSize);

	//CloseHandle(fileHandle);

//	trail.format(L"%s:%d%d", docPathName, modifyTime.dwLowDateTime, modifyTime.dwHighDateTime);

	//if (HashBuffer(trail,hashName) == false)
	//	return false;

	mPages.clear();
	mCachePath.SetPathName(nphumbnailPathName);

	// 建立缓存路径
	//trail = docPathName;
	//if (HashBuffer(trail, hashName) == false)
	//	return false;

	//mCachePath.SetByUserAppData();
	//mCachePath.AssurePath();
	//mCachePath += L"EinkReader\\";
	//mCachePath += hashName.ptr();
	//mCachePath.AssurePath();

	//如果没有这个目录，则建立这个目录
	//mCachePath.CreatePath();

	//bool updateOpenInfor;
	//// 从缓存路径中读取信息文件
	//if (ReadOpenningInfor(mCachePath.GetPathName(), createTime, lowSize,updateOpenInfor) == false)
	//{
	//	// 说明不相符，删除本目录下全部文件
	//	wchar_t lszTemp[MAX_PATH] = { 0 };
	//	wcscpy_s(lszTemp, MAX_PATH, mCachePath.GetPathName());
	//	wcscat_s(lszTemp, MAX_PATH, L"*.*");
	//	lszTemp[wcslen(lszTemp) + 1] = UNICODE_NULL; //双0结尾
	//	SHFILEOPSTRUCT ldShfile;
	//	memset(&ldShfile, 0, sizeof(ldShfile));
	//	ldShfile.pFrom = lszTemp;
	//	ldShfile.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_FILESONLY;
	//	ldShfile.wFunc = FO_DELETE;
	//	SHFileOperation(&ldShfile);
	//}

	//// 将新的信息写入
	//if(updateOpenInfor != false)
	//	SaveOpenningInfor(mCachePath.GetPathName(), createTime, lowSize);

	return true;
}

void CEdPageControl::Clear(void)
{
	CSectionAutoLock lock(mThreadLock);

	mPages.clear();
}

const wchar_t* CEdPageControl::GetCachePath(void)
{
	return mCachePath.GetPathName();
}

void CEdPageControl::GenerateThumbPath(int pageNumber,bool annot,cmmStringW& thumbPath)
{
	thumbPath.format(L"%s%06d%s.png", mCachePath.GetPathName(), pageNumber,annot?L"A":L"");
}

bool CEdPageControl::GetThumbInfor(int pageNumber, bool& hasAnnot, cmmStringW& thumbPath)
{
	if (HasThumbnail(pageNumber, hasAnnot) == false)
		return false;

	GenerateThumbPath(pageNumber, hasAnnot, thumbPath);

	return true;
}

bool CEdPageControl::CheckThumbnailFile(int pageNumber,bool& hasAnnot)
{
	cmmStringW fileName;

	GenerateThumbPath(pageNumber, false,fileName);

	if (fileName.size() > 0 && GetFileAttributes(fileName.ptr()) != INVALID_FILE_ATTRIBUTES)
	{
		hasAnnot = false;
		return true;
	}

	GenerateThumbPath(pageNumber, true, fileName);

	if (fileName.size() > 0 && GetFileAttributes(fileName.ptr()) != INVALID_FILE_ATTRIBUTES)
	{
		hasAnnot = true;
		return true;
	}

	return false;
}

int CEdPageControl::GetAnnotPageCount(void)
{
	CSectionAutoLock lock(mThreadLock);
	int count = 0;

	for (auto i : mPages)
	{
		if ((i.second&ED_PAGECTL_FLAG_ANNOT) != 0)
			count++;
	}

	return count;

}

void CEdPageControl::AddPage(int pageNumber, bool hasThumb, bool hasAnnot)
{
	CSectionAutoLock lock(mThreadLock);

	mPages[pageNumber] = ED_PAGECTL_FLAG(hasThumb, hasAnnot);
}

bool CEdPageControl::MarkThumbnail(int pageNumber)
{
	CSectionAutoLock lock(mThreadLock);

	std::map<int, UCHAR>::iterator itr;

	itr = mPages.find(pageNumber);

	if (itr == mPages.end())
		return false;

	itr->second |= ED_PAGECTL_FLAG_THUMB;

	return true;
}

bool CEdPageControl::HasThumbnail(int pageNumber, bool& hasAnnot)
{
	CSectionAutoLock lock(mThreadLock);

	std::map<int, UCHAR>::iterator itr;

	itr = mPages.find(pageNumber);

	if (itr == mPages.end())
		return false;

	hasAnnot = ((itr->second&ED_PAGECTL_FLAG_ANNOT) != 0);
	return (itr->second&ED_PAGECTL_FLAG_THUMB)!=0;
}

bool CEdPageControl::MarkAnnot(int pageNumber,bool mark)
{
	CSectionAutoLock lock(mThreadLock);
	std::map<int, UCHAR>::iterator itr;

	itr = mPages.find(pageNumber);

	if (itr == mPages.end())
		return false;

	if(mark != false)
		itr->second |= ED_PAGECTL_FLAG_ANNOT;
	else
		itr->second &= (~ED_PAGECTL_FLAG_ANNOT);

	return true;
}

bool CEdPageControl::HasAnnot(int pageNumber)
{
	CSectionAutoLock lock(mThreadLock);
	std::map<int, UCHAR>::iterator itr;

	itr = mPages.find(pageNumber);

	if (itr == mPages.end())
		return false;

	return (itr->second&ED_PAGECTL_FLAG_ANNOT) != 0;
}

UCHAR CEdPageControl::GetPageCtl(int pageNumber)
{
	CSectionAutoLock lock(mThreadLock);
	std::map<int, UCHAR>::iterator itr;

	itr = mPages.find(pageNumber);

	if (itr == mPages.end())
		return 0xFF;

	return itr->second;
}

bool CEdPageControl::ReadOpenningInfor(const wchar_t* path, FILETIME& createTime, ULONG& fileSize, bool& updateOpenInfor)
{
	/*std::ifstream inforFile;
	char buf[1024];
	char name[128];
	char value[128];
	cmmStringA nameStr,createStr, sizeStr;

	cmmStringW pathName = path;
	pathName += L"Openning.txt";

	inforFile.open(pathName.conPtr(),std::ios::in);
	if (inforFile.is_open() == false)
		return false;

	updateOpenInfor = false;
	while (inforFile.getline(buf, 1024))
	{
		cmmStringA readIn = buf;

		if(readIn.scanf("%[a-z]%*[:]%s", name,123, value,128)!=2)
			continue;

		nameStr = name;
		if (nameStr.compare("create") == 0)
		{
			createStr = value;
		}
		else
		if (nameStr.compare("size") == 0)
		{
			sizeStr = value;
		}
		else
		if(nameStr.compare("update") == 0)
		{
			cmmStringA vv = value;
			if (vv.compare("now") == 0)
			{
				updateOpenInfor = true;
				break;
			}
		}
	}
	inforFile.close();

	if (updateOpenInfor != false)
		return true;

	cmmStringA dump;

	dump.DumpBinBuf((unsigned char*)&createTime, sizeof(createTime));
	if (dump.compare(createStr.conPtr()) != 0)
		return false;

	dump.format("%d", fileSize);
	if (dump.compare(sizeStr.conPtr()) != 0)
		return false;*/

	return true;
}

bool CEdPageControl::SaveOpenningInfor(const wchar_t* path, const FILETIME& createTime, ULONG fileSize)
{
	/*std::ofstream inforFile;
	cmmStringW pathName = path;
	pathName += L"Openning.txt";

	inforFile.open(pathName.conPtr(), std::ios::out | std::ios::trunc);
	if (inforFile.is_open() == false)
		return  false;

	cmmStringA dump;

	dump.DumpBinBuf((unsigned char*)&createTime, sizeof(createTime));

	inforFile << "create:" << dump.conPtr() << std::endl;

	dump.format("%d", fileSize);

	inforFile << "size:" << dump.conPtr() << std::endl;

	inforFile.close();*/

	return true;
}
