/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "smtPage.h"
#include "smtDocument.h"
#include "GdipStart.h"
#include "thread"


//////////////////////////////////////////////////////////////////////////
DEFINE_BUILTIN_NAME(CSmtDocument)


CSmtDocument::CSmtDocument()
{
	mPageLoadCallbackFun = NULL;
	mThreadHandle = NULL;
	mExitThread = 0;
	mLoadThumb = false;
}

CSmtDocument::~CSmtDocument()
{

	if (mThreadHandle != NULL)
	{
		mExitThread = 1;

		if (WaitForSingleObject(mThreadHandle, 30 * 1000) == WAIT_TIMEOUT)
			TerminateThread(mThreadHandle, 1);

		CloseHandle(mThreadHandle);
	}

	if (mThumbThreadExit != NULL)
	{
		mLoadThumb = false;
		// 等它5秒钟退出
		WaitForSingleObject(mThumbThreadExit, 5000);
		CloseHandle(mThumbThreadExit);
		mThumbThreadExit = NULL;
	}


	if (mPageNo1LoadedEvent != NULL)
		CloseHandle(mPageNo1LoadedEvent);
	if (mOverallLoadedEvent != NULL)
		CloseHandle(mOverallLoadedEvent);
}

ULONG CSmtDocument::InitOnCreate(const char16_ptr pathName)
{
	mThreadData.pathName = pathName;
	mThreadData.docObject = (void*)this;

	mPageNo1LoadedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	mOverallLoadedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	mThumbThreadExit = CreateEvent(NULL, TRUE,TRUE, NULL);

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
		if (lpThis->documentEngine.CreateEngine(npContext->pathName, NULL, (PEDSMT_THREAD_CALLBACK)CSmtDocument::PageLoadCallBack, (void*)lpThis, &npContext->pageContext) != false)
			switch (lpThis->documentEngine.GetEngineType())
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
		else
			lpThis->documentType = -1;

	} while (false);

	if (lpThis->documentType >= 0)
	{
		lpThis->pageCount = lpThis->documentEngine.PageCount();
		SetEvent(lpThis->mOverallLoadedEvent);
	}

	gdipStart.UnInit();

	return 0;
}

bool32 CSmtDocument::LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr, PPAGE_PDF_CONTEXT initPage)
{
	mPageLoadCallbackFun = callBackFun;
	mPageLoadCallbackContext = contextPtr;
	mThreadData.pageContext.pageIndex = initPage->pageIndex;
	mThreadData.pageContext.pageContext = initPage->pageContext;
	mThreadData.pageContext.pageContext2 = initPage->pageContext2;

	HANDLE Events[2];

	ULONG luThreadID;

	mThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSmtDocument::LoadingThread, (void*)&mThreadData, 0 /*CREATE_SUSPENDED*/, &luThreadID);
	Events[0] = mThreadHandle;
	Events[1] = mPageNo1LoadedEvent;		// 只要装入一页了，就返回
	DWORD threadRev;
	if (mThreadHandle == NULL || (threadRev = WaitForMultipleObjects(2,Events,FALSE,INFINITE)) == WAIT_TIMEOUT)
	{
		return false;
	}

	if (documentEngine.GetEngineType() == Engine_None)
	{
		return false;
	}

	pageCount = documentEngine.PageCount();

	//auto x = documentEngine->SupportsAnnotation(true);

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
	PAGE_PDF_CONTEXT context;

	if (documentEngine.GetPageContext(pageObj->rawContext.pageIndex, &context) == false)
		return errBack;

	pageObj->SavePageContext(&context);

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

bool32 CSmtDocument::LoadAllThumbnails(PEDDOC_THUMBNAILS_CALLBACK callBack, void_ptr contextPtr, const wchar_t* nphumbnailPathName)
{
	//需要先算出路径
	mThumbCtl.SetDocument(nphumbnailPathName);

	std::thread td([this, callBack, contextPtr, nphumbnailPathName] {

		// 判断是否已经开始生成Thumbnails
		{// 进入互斥区
			CSectionAutoLock lock(this->mDocAccLock);
			if (this->mLoadThumb != false)	// 已经启动装载
				return false;

			mLoadThumb = true;

		}// 离开互斥区

		// 等待页面全部装入
		while(mLoadThumb != false)
		{
			if (WaitForSingleObject(this->mOverallLoadedEvent, 1000) != WAIT_TIMEOUT)
				break;
		}

		if (this->mLoadThumb == false)
			return false;	// 需要退出，放弃继续加载

		if (this->pageCount == 0)
			return false;

		ResetEvent(mThumbThreadExit);

		// 计算缩放比
		RectD mediaBoxF = documentEngine.GetRawEngineObj()->Transform(documentEngine.GetRawEngineObj()->PageMediabox(1), ENGINE_INDEX(1), 1.0, 0);

		float sw = (float)mediaBoxF.dx;
		float sh = (float)mediaBoxF.dy;
		float dw, dh;

		if (sw > sh)
		{
			dw = 480.0f;
			dh = (dw*sh) / sw;
		}
		else
		{
			dh = 480.0f;
			dw = (dh*sw) / sh;
		}

		float scalRatio = dw / sw;

		// 获取首页的信息
		PAGE_PDF_CONTEXT p1,p2;
		ZeroMemory(&p1, sizeof(p1));
		ZeroMemory(&p2, sizeof(p2));

		if (documentEngine.GetPageContext(0, &p1) == false)
			return false;

		CGdipStart gdiStart;
		gdiStart.Init();

		for (int i = 1; i <= pageCount; i++)	// i==pageCount时，处理第pageCount-1页
		{
			cmmStringW fileName;
			if (this->mLoadThumb == false)
				break;	// 需要退出，放弃继续加载

			if (i>=pageCount || documentEngine.GetPageContext(i, &p2) == false)
			{
				p2.pageContext = p1.pageContext;
			}

			// 组装文件名
			mThumbCtl.GenerateThumbFilePathName(&p1,&p2, fileName);

			// 查看是否已经存在thunbnails
			if (mThumbCtl.CheckThumbnailFile(fileName) == true)
			{
				// 调用回调函数
				if (callBack != NULL)
					callBack((uint32Eink)i, contextPtr);
			}
			else
			{
				// 不存在，则生成这张缩略图
				if(documentEngine.RenderThumbnail(fileName.ptr(),(int)p1.pageIndex,scalRatio) != false)
				{
					// 调用回调函数
					if (callBack != NULL)
						callBack((uint32Eink)i, contextPtr);
				}
				else
				{
					
				}
			}

			if (GetFileAttributes(fileName.ptr()) == INVALID_FILE_ATTRIBUTES)
			{
				//文件不存在了
				int i = 0;
				i++;
			}

			p1.pageIndex = p2.pageIndex;
			p1.pageContext = p2.pageContext;
		}

		gdiStart.UnInit();

		SetEvent(this->mThumbThreadExit);

		// 调用回调函数
		if (callBack != NULL)
			callBack((uint32Eink)MAXULONG32, contextPtr);

		return true;
	});

	HANDLE handle = td.native_handle();
	SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);

	td.detach();
	//td.join();

	return true;
}

bool32 CSmtDocument::GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen)
{

	bool32 lbRet = false;

	do
	{
		BREAK_ON_NULL(npszPathBuffer);
		wcscpy_s(npszPathBuffer, niLen, mThumbCtl.GetCachePath());

		lbRet = true;

	} while (false);

	return lbRet;
}

bool32 CSmtDocument::GetThumbnailPathName(int32Eink pageIndex, char16 pathName[256], bool* /*hasAnnot*/)
{
	// 获取首页的信息
	cmmStringW fileName;
	PAGE_PDF_CONTEXT p1, p2;

	if (documentEngine.GetPageContext(pageIndex, &p1) == false)
		return false;

	if (pageIndex+1 >= pageCount || documentEngine.GetPageContext(pageIndex+1, &p2) == false)
	{
		p2.pageContext = p1.pageContext;
	}

	// 组装文件名
	mThumbCtl.GenerateThumbFilePathName(&p1, &p2, fileName);

	wcscpy_s(pathName, 256, fileName.ptr());

	return true;
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
		return true;

	if (thisDoc->mExitThread != 0)
		return false;

	if (thisDoc->mPageLoadCallbackFun == NULL)
		return true;

	if (loadingStep != MAXULONG32)
		thisDoc->pageCount = (loadingStep&(~0xFF000000));

	if ((loadingStep&(~0xFF000000)) == 1 || loadingStep == MAXULONG32)
		SetEvent(thisDoc->mPageNo1LoadedEvent);

	if (loadingStep != MAXULONG32 && loadingStep != 0 && (loadingStep%9)!=0 )
		return true;

	// 调用
	thisDoc->mPageLoadCallbackFun(loadingStep, thisDoc->pageCount,thisDoc->mPageLoadCallbackContext);

	return true;
}
