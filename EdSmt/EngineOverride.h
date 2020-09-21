/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


/* Copied from EngineManager.h*/

/* 
modify:
add the loading progress callback function 
*/
#pragma once
#include "EdDoc.h"

enum EngineType {
    Engine_None = 0,
    // the EngineManager tries to create a new engine
    // in the following order (types on the same line
    // share common code and reside in the same file)
    Engine_PDF, Engine_XPS,
    Engine_DjVu,
    Engine_Image, Engine_ImageDir, Engine_ComicBook,
    Engine_PS,
    Engine_Epub, Engine_Fb2, Engine_Mobi, Engine_Pdb,
        Engine_Chm, Engine_Html, Engine_Txt,
};

typedef bool(__stdcall *PEDSMT_THREAD_CALLBACK)(uint32 loadingStep, void* context);

namespace EngineLoad {

	//bool IsSupportedFile(const WCHAR *filePath, bool sniff = false);
	//bool CreateEngine(const WCHAR *filePath, BaseEngine ** engineObj,  EngineType *typeOut = nullptr, PasswordUI *pwdUI = nullptr,PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL, PPAGE_PDF_CONTEXT pageContext=NULL);
	//bool GetPageContext(BaseEngine* baseEngine, int pageNo, PPAGE_PDF_CONTEXT pageContext);
	//bool ReQueryPageIndex(BaseEngine* baseEngine, PPAGE_PDF_CONTEXT pageContext);

class CEngineHandle 
{
public:
	CEngineHandle() {
		mRawEngine = NULL;
		mEngineType = Engine_None;
	}
	~CEngineHandle() {
		if (mRawEngine != NULL)
			delete mRawEngine;
	}

	static bool IsSupportedFile(const WCHAR *filePath, bool sniff = false);
	EngineType GetEngineType(void) {
		return mEngineType;
	}
	bool CreateEngine(const WCHAR *filePath,PasswordUI *pwdUI = nullptr, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL, PPAGE_PDF_CONTEXT pageContext = NULL);
	BaseEngine* GetRawEngineObj(void) {
		return mRawEngine;
	}
	bool GetPageContext(int pageNo, PPAGE_PDF_CONTEXT pageContext);
	bool ReQueryPageIndex(PPAGE_PDF_CONTEXT pageContext);

	int PageCount() const;

	bool RenderThumbnail(const WCHAR *filePath,int pageNo,float scalRatio);

protected:
	BaseEngine* mRawEngine;
	EngineType mEngineType;
};

}


