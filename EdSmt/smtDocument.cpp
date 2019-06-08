/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "smtPage.h"
#include "smtDocument.h"
#include "GdipStart.h"


//////////////////////////////////////////////////////////////////////////
DEFINE_BUILTIN_NAME(CSmtDocument)


CSmtDocument::CSmtDocument()
{
	documentEngine = NULL;
	mPageLoadCallbackFun = NULL;
	mThreadHandle = NULL;
	mExitThread = 0;
}

CSmtDocument::~CSmtDocument()
{
	if (documentEngine != NULL)
		delete documentEngine;

	if (mThreadHandle != NULL)
	{
		mExitThread = 1;

		if (WaitForSingleObject(mThreadHandle, 30 * 1000) == WAIT_TIMEOUT)
			TerminateThread(mThreadHandle, 1);

		CloseHandle(mThreadHandle);
	}
	if (mPageLoadEvent != NULL)
		CloseHandle(mPageLoadEvent);
}

ULONG CSmtDocument::InitOnCreate(const char16_ptr pathName)
{
	mThreadData.pathName = pathName;
	mThreadData.docObject = (void*)this;

	mPageLoadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	return EDERR_SUCCESS;
}

ULONG WINAPI CSmtDocument::LoadingThread(PST_DOC_LOADING_THREAD npContext)
{
	CSmtDocument* lpThis = (CSmtDocument*)(npContext->docObject);
	CGdipStart gdipStart;

	//Sleep(10000);
	//return 0;

//	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
	gdipStart.Init();

	do {

		EngineLoad::CreateEngine(npContext->pathName,(BaseEngine**)&lpThis->documentEngine, NULL, &lpThis->engineType,(PEDSMT_THREAD_CALLBACK)CSmtDocument::PageLoadCallBack,(void*)lpThis);

		if (lpThis->documentEngine == NULL)
			break;

		switch (lpThis->engineType)
		{
		case Engine_PDF:
			lpThis->documentType = 0;
			break;
		case Engine_Epub:
			lpThis->documentType = 1;
			break;
		case Engine_Mobi:
			lpThis->documentType = 2;
			break;
		case Engine_Txt:
			lpThis->documentType = 3;
			break;
			//SetThreadPriority
		case Engine_None:
			/*break*/;
		case Engine_XPS:
			/*break*/;
		case Engine_DjVu:
			/*break*/;
		case Engine_Image:
			/*break*/;
		case Engine_ImageDir:
			/*break*/;
		case Engine_ComicBook:
			/*break*/;
		case Engine_PS:
			/*break*/;
		case Engine_Fb2:
			/*break*/;
		case Engine_Pdb:
			/*break*/;
		case Engine_Chm:
			/*break*/;
		case Engine_Html:
			/*break*/;
		default:
			lpThis->documentType = -1;
		}

	} while (false);

	if (lpThis->documentEngine != NULL && lpThis->documentType >= 0)
		lpThis->pageCount = lpThis->documentEngine->PageCount();

	gdipStart.UnInit();

	return 0;
}

bool32 CSmtDocument::LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr)
{
	mPageLoadCallbackFun = callBackFun;
	mPageLoadCallbackContext = contextPtr;
	HANDLE Events[2];

	ULONG luThreadID;

	mThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSmtDocument::LoadingThread, (void*)&mThreadData, 0 /*CREATE_SUSPENDED*/, &luThreadID);
	Events[0] = mThreadHandle;
	Events[1] = mPageLoadEvent;		// 只要装入一页了，就返回
	if (mThreadHandle == NULL || WaitForMultipleObjects(2,Events,FALSE,INFINITE) == WAIT_TIMEOUT)
	{
		return false;
	}

	if (documentEngine == nullptr)
	{
		return false;
	}

	pageCount = documentEngine->PageCount();

	return true;
}

int32Eink CSmtDocument::GetMetaData(char* keyName, char* dataBuf, int32Eink bufSize)
{
	if (_stricmp(keyName, "format") == 0)
	{
		strcpy_s(dataBuf, bufSize, "Text files");
		return (int32Eink)strlen("Text files");
	}

	return 0;
}

int32Eink CSmtDocument::GetDocType(void)
{
	return documentType;
}

int32Eink CSmtDocument::GetPageCount(void)
{
	return pageCount;
}

ED_ERR CSmtDocument::LoadPage(CSmtPage* pageObj)
{
	ED_ERR errBack = EDERR_UNSUCCESSFUL;

	errBack = EDERR_SUCCESS;

	return errBack;
}

IEdPage_ptr CSmtDocument::GetPage(int32Eink pageIndex)
{
	auto pageObj = CSmtPage::CreateInstance(pageIndex,this);
	if (pageObj == NULL)
		return NULL;

	if (ERR_FAILED(LoadPage(pageObj)))
	{
		pageObj->Release();
		return NULL;
	}

	return pageObj;
}

IEdPage_ptr CSmtDocument::GetPage(PPAGE_PDF_CONTEXT contextPtr)
{
	return GetPage(contextPtr->pageIndex);
}

IEdPage_ptr CSmtDocument::GetPage(IEdPage_ptr currentPage, int32Eink pagesOff)
{
	auto index = currentPage->GetPageIndex() + pagesOff;

	if(index >=0 && index < pageCount)
		return GetPage(index);

	return NULL;
}

IEdPage_ptr CSmtDocument::Rearrange(PPAGE_PDF_CONTEXT contextPtr)
{
	PAGE_PDF_CONTEXT context = { 0,0,0 };

	if (contextPtr == NULL)
		contextPtr = &context;

	// unsupport
	return GetPage(contextPtr);
}

IEdPage_ptr CSmtDocument::Rearrange(IEdPage_ptr currentPage)
{
	currentPage->AddRefer();
	// unsupport
	return currentPage;
}

void CSmtDocument::SetFont(const char16_ptr fontName, int32Eink fontSize)
{
	// unsupport
}

void CSmtDocument::SetViewPort(int32Eink viewWidth, int32Eink viewHeight)
{
	// unsupport
}

bool __stdcall CSmtDocument::PageLoadCallBack(uint32 loadingStep, CSmtDocument* thisDoc)
{

	if (thisDoc == NULL)
	{
		if (thisDoc->mExitThread != 0)
			return false;

		if (thisDoc->mPageLoadCallbackFun == NULL)
			return true;
	}

	if (loadingStep != MAXULONG32)
		thisDoc->pageCount = (loadingStep&(~0xFF000000));

	if ((loadingStep&(~0xFF000000)) == 1 || loadingStep == MAXULONG32)
		SetEvent(thisDoc->mPageLoadEvent);

	if (loadingStep != MAXULONG32 && loadingStep != 0 && (loadingStep%9)!=0 )
		return true;

	// 调用
	thisDoc->mPageLoadCallbackFun(loadingStep, thisDoc->pageCount,thisDoc->mPageLoadCallbackContext);

	return true;
}
