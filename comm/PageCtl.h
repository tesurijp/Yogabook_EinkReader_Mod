/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#pragma once
#include "eddoc.h"
#include "map"
#include "vector"
#include "tchar.h"
#include "cmmString.h"
#include "cmmBaseObj.h"
#include "cmmPath.h"


#define ED_PAGECTL_FLAG_THUMB	1
#define ED_PAGECTL_FLAG_ANNOT	2
#define ED_PAGECTL_FLAG(_THUMB,_ANNOT) ((_THUMB?ED_PAGECTL_FLAG_THUMB:0)|(_ANNOT?ED_PAGECTL_FLAG_ANNOT:0))

class CEdPageControl {
public:
	CEdPageControl() {}
	~CEdPageControl() {}

	bool SetDocument(const wchar_t* nphumbnailPathName);

	void Clear(void);

	void AddPage(int pageNumber, bool hasThumb = false,bool hasAnnot=false);
	bool MarkThumbnail(int pageNumber);
	bool HasThumbnail(int pageNumber,bool& hasAnnot);
	bool MarkAnnot(int pageNumber, bool mark);
	bool HasAnnot(int pageNumber);
	const wchar_t* GetCachePath(void); // 无需释放
	void GenerateThumbPath(int pageNumber, bool annot, cmmStringW& thumbPath);

	// 从记录中查看缩略图情况
	bool GetThumbInfor(int pageNumber,bool& hasAnnot, cmmStringW& thumbPath);

	bool CheckThumbnailFile(int pageNumber, bool& hasAnnot);

	int GetAnnotPageCount(void);

protected:
	CExclusiveAccess mThreadLock;
	std::map<int, UCHAR> mPages;
	CFilePathName mCachePath;

	UCHAR GetPageCtl(int pageNumber);

	bool ReadOpenningInfor(const wchar_t* path, FILETIME& createTime, ULONG& fileSize,bool& updateOpenInfor);
	bool SaveOpenningInfor(const wchar_t* path, const FILETIME& createTime, ULONG fileSize);
};
