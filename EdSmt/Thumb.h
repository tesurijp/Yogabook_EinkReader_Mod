/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#pragma once
#include "eddoc.h"
#include "vector"
#include "tchar.h"
#include "cmmString.h"
#include "cmmBaseObj.h"
#include "cmmPath.h"



class CSmtThumb {
public:
	CSmtThumb() {}
	~CSmtThumb() {}

	bool SetDocument(const wchar_t* nphumbnailPathName);

	void GenerateThumbFilePathName(PPAGE_PDF_CONTEXT p1, PPAGE_PDF_CONTEXT p2, cmmStringW& thumbPath);

	bool CheckThumbnailFile(cmmStringW& thumbPath);

	const wchar_t* GetCachePath(void);

protected:
	CFilePathName mCachePath;
};
