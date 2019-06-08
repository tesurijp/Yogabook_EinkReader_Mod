/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "txtPage.h"
#include "txtDocument.h"
#include "gdiplus.h"

//////////////////////////////////////////////////////////////////////////
DEFINE_BUILTIN_NAME(CtxtDocument)

using namespace Gdiplus;

CtxtDocument::CtxtDocument()
{
	mpTxtMappedBase = NULL;
	mhTxtFileMap = INVALID_HANDLE_VALUE;
	mhTxtFile = INVALID_HANDLE_VALUE;
	mViewPort.X = 0.0f;// 250.0f;
	mViewPort.Y = 0.0f;// 400.0f;
	mpTextBuffer = NULL;
	//mIncomplete = 0;
	mUpdateForSize = false;
	mUpdateForFont = false;
	mPageLoadCallbackFun = NULL;
	//mArrageAgain = false;
	mThreadNumber = 0;
	mpArrangeThread = NULL;
	/*mPreFocusPage.charBegin = */mRearrangePage.charBegin = MAXULONG32;
	SetFont(L"Arial", 12);
}

CtxtDocument::~CtxtDocument()
{
	// 首先需要停掉工作线程
	if (mpArrangeThread != NULL)
		delete mpArrangeThread;

	if(mpTextBuffer != NULL && ((bin_ptr)mpTextBuffer < mpTxtMappedBase)|| (bin_ptr)mpTextBuffer > mpTxtMappedBase+mFileSize.LowPart)
		HeapFree(GetProcessHeap(), 0, mpTextBuffer);

	if(mpTxtMappedBase != NULL)
		UnmapViewOfFile(mpTxtMappedBase);

	if(mhTxtFile != INVALID_HANDLE_VALUE)
		CloseHandle(mhTxtFile);
	if (mhTxtFileMap != INVALID_HANDLE_VALUE)
		CloseHandle(mhTxtFileMap);
}

ULONG CtxtDocument::InitOnCreate(const char16_ptr pathName)
{
	ULONG errorCode = EDERR_UNSUCCESSFUL;

	try
	{
		//mpFont = new Gdiplus::Font(L"Arial", 12);
		//THROW_ON_NULL(mpFont);

		// 映射文件进内存
		mhTxtFile = CreateFile(pathName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		if (mhTxtFile == INVALID_HANDLE_VALUE)
			THROW_INVALID;

		if (GetFileSizeEx(mhTxtFile, &mFileSize) == FALSE)
			THROW_FAILED;

		if (mFileSize.HighPart != 0)
			THROW_OVERFLOW;

		if (mFileSize.LowPart == 0)
			THROW_EMPTY;

		//指定文件大小
		mhTxtFileMap = CreateFileMapping(mhTxtFile, NULL, PAGE_READONLY, 0,mFileSize.LowPart, NULL);
		THROW_ON_INVALID(mhTxtFileMap);

		mpTxtMappedBase = (bin_ptr)MapViewOfFile(mhTxtFileMap, FILE_MAP_READ, 0, 0, 0);
		THROW_ON_NULL(mpTxtMappedBase);

		if (GetBomInfor() == CtxtDocument::un)
			THROW_WRONG_FORMAT;

		// 如果是Utf8格式
		if ((mBom == u8 || mBom == u8NoBOM)&& UnpackU8()!= EDERR_SUCCESS)
			THROW_WRONG_FORMAT;

		// 如果是Ansi格式
		if (mBom == ansi && UnpackAnsi() != EDERR_SUCCESS)
			THROW_WRONG_FORMAT;

		// 如果是UNIDOE big ending格式
		if (mBom == u16b && UnpackBigEnding16() != EDERR_SUCCESS)
			THROW_WRONG_FORMAT;

		if (mBom == u16)
		{
			mpTextBuffer = (wchar_t*)(mpTxtMappedBase+2);	// skip BOM
			mTextCharCount = (mFileSize.LowPart / 2) - 1;
		}
		
		//mFirstPageLoaded = CreateEvent(NULL, true, false, NULL);

		//mThreadStopped = CreateEvent(NULL, true, true, NULL);

		errorCode = EDERR_SUCCESS;// UpdateLayoutForAll(true);
	}
	catch (...)
	{
	}

	return errorCode;
}

ED_ERR CtxtDocument::UnpackU8(void)
{
	//if (mpTextBuffer != NULL && (bin_ptr)mpTextBuffer != mpTxtMappedBase)
	//	HeapFree(GetProcessHeap(), 0, mpTextBuffer);

	// 计算一下尺寸
	auto nextChar = mpTxtMappedBase;
	auto endChar = nextChar + mFileSize.LowPart;
	uint32 wideCharCount = 0;

	// skip BOM
	if(mBom != u8NoBOM)
		nextChar += 3;

	while (nextChar < endChar)
	{
		try
		{
			while (nextChar < endChar)
			{
				if ((nextChar[0] & 0x80) == 0)
				{
					nextChar++;
				}
				else
				if ((nextChar[0] & 0xE0) == 0xC0)
				{
					// two bytes
					if (nextChar + 1 >= endChar || (nextChar[1] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据
					nextChar += 2;
				}
				else
				if ((nextChar[0] & 0xF0) == 0xE0)
				{
					// three bytes
					if (nextChar + 2 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据

					nextChar += 3;
				}
				else
				if ((nextChar[0] & 0xF8) == 0xF0)
				{
					// four bytes
					if (nextChar + 3 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80 || (nextChar[3] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据
					nextChar += 4;
				}
				else
				THROW_UNKNOWN;	// 错误数据

				wideCharCount++;
			}
		}
		catch (...)
		{
			while (++nextChar < endChar)
			{
				// 查找下一个有效字符
				if ((nextChar[0] & 0x80) == 0 || (nextChar[0] & 0xC0) == 0xC0)
					break;
			}
		}
	}
	wideCharCount += 10;	// 增加一点缓冲区


	// 分配内存用于解包
	mpTextBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(),0,wideCharCount * 2);
	if (mpTextBuffer == NULL)
		return EDERR_NOTENOUGH_MEMORY;

	auto crtWideChar = mpTextBuffer;
	auto endWideChar = crtWideChar + wideCharCount;

	// skip BOM
	nextChar = mpTxtMappedBase+3;

	while (nextChar < endChar)
	{
		try
		{
			while (nextChar < endChar && crtWideChar < endWideChar)
			{
				if ((nextChar[0] & 0x80) == 0)
				{
					*crtWideChar = *nextChar++;
				}
				else
				if ((nextChar[0] & 0xE0) == 0xC0)
				{
					// two bytes
					if(nextChar+1 >= endChar || (nextChar[1] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据

					*crtWideChar = (((wchar_t)(nextChar[0] & 0X1F)) << 6) | ((wchar_t)(nextChar[1] & 0X3F));
					nextChar += 2;
				}
				else
				if ((nextChar[0] & 0xF0) == 0xE0)
				{
					// three bytes
					if (nextChar + 2 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据

					*crtWideChar = (((wchar_t)(nextChar[0] & 0X1F)) << 12) | (((wchar_t)(nextChar[1] & 0X3F)) << 6) | ((wchar_t)(nextChar[2] & 0X3F));
					nextChar += 3;
				}
				else
				if ((nextChar[0] & 0xF8) == 0xF0)
				{
					// four bytes
					if (nextChar + 3 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80 || (nextChar[3] & 0xC0) != 0x80)
						THROW_UNKNOWN;	// 错误数据
					// 这个地方是有问题的，wide char只有16位，无法保存后面的21个bits，所以暂时不支持，跳过这个字符，$ax$
					//*crtWideChar = (((uint16)(nextChar[0] & 0X1F)) << 18) | (((uint16)(nextChar[1] & 0X3F)) << 12) | (((uint16)(nextChar[2] & 0X3F)) << 6) | ((uint16)(nextChar[3] & 0X3F));
					nextChar += 4;
				}
				else
					THROW_UNKNOWN;	// 错误数据

				crtWideChar++;
			}

		}
		catch (...)
		{
			while (++nextChar < endChar)
			{
				// 查找下一个有效字符
				if ((nextChar[0]&0x80)==0 || (nextChar[0] & 0xC0) == 0xC0)
					break;
			}
		}
	}

	mTextCharCount = (uint32)(crtWideChar - mpTextBuffer);
	*crtWideChar = UNICODE_NULL;

	return EDERR_SUCCESS;
}

//判断文件是否是utf8文件
bool CtxtDocument::IsU8File(const void* npBuffer, long nlSize)
{
	bool lbIsUTF8 = false;
	int liUtf8Count = 0;
	unsigned char* lpStart = (unsigned char*)npBuffer;
	unsigned char* lpEnd = (unsigned char*)npBuffer + nlSize;
	while (lpStart < lpEnd)
	{
		if (*lpStart < 0x80) // (10000000): 值小于0x80的为ASCII字符    
		{
			lpStart++;
		}
		else if (*lpStart < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符    
		{
			lbIsUTF8 = false;
			liUtf8Count = 0;
			break;
		}
		else if (*lpStart < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符    
		{
			if (lpStart >= lpEnd - 1)
			{
				break;
			}

			if ((lpStart[1] & (0xC0)) != 0x80)
			{
				lbIsUTF8 = false;
				liUtf8Count = 0;
				break;
			}

			liUtf8Count++;
			lpStart += 2;
		}
		else if (*lpStart < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符    
		{
			if (lpStart >= lpEnd - 2)
			{
				break;
			}

			if ((lpStart[1] & (0xC0)) != 0x80 || (lpStart[2] & (0xC0)) != 0x80)
			{
				lbIsUTF8 = false;
				liUtf8Count = 0;
				break;
			}

			liUtf8Count++;
			lpStart += 3;
		}
		else
		{
			lbIsUTF8 = false;
			liUtf8Count = 0;
			break;
		}
	}

	return liUtf8Count>0;
}

//这个函数有时会误判，会把ansi格式文件认为是utf8
ED_ERR CtxtDocument::IsU8File(void)
{
	auto nextChar = mpTxtMappedBase;
	auto endChar = nextChar + mFileSize.LowPart;
	uint32 wideCharCount = 0;
	uint32 u8FlagCount = 0;

	try
	{
		while (nextChar < endChar && u8FlagCount < 2)
		{
			if ((nextChar[0] & 0x80) == 0)
			{
				nextChar++;
			}
			else
			if ((nextChar[0] & 0xE0) == 0xC0)
			{
				// two bytes
				if (nextChar + 1 >= endChar || (nextChar[1] & 0xC0) != 0x80)
					THROW_UNKNOWN;	// 错误数据
				nextChar += 2;
				u8FlagCount++;
			}
			else
			if ((nextChar[0] & 0xF0) == 0xE0)
			{
				// three bytes
				if (nextChar + 2 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80)
					THROW_UNKNOWN;	// 错误数据

				nextChar += 3;
				u8FlagCount++;
			}
			else
			if ((nextChar[0] & 0xF8) == 0xF0)
			{
				// four bytes
				if (nextChar + 3 >= endChar || (nextChar[1] & 0xC0) != 0x80 || (nextChar[2] & 0xC0) != 0x80 || (nextChar[3] & 0xC0) != 0x80)
					THROW_UNKNOWN;	// 错误数据
				nextChar += 4;
				u8FlagCount++;
			}
			else
			THROW_UNKNOWN;	// 错误数据

			wideCharCount++;
		}
	}
	catch (...)
	{
		// 出错了，因为没有BOM头，那就不能被认作utf8，上面的正式的解压函数能够容错处理utf8中的错误
		nextChar = endChar;
	}

	return u8FlagCount>=2;	// 找到了u8的数据块多次才能认定是u8文件，否则就是ansi文件
}

ED_ERR CtxtDocument::UnpackAnsi(void)
{
	uint32 wideCharCount;

	wideCharCount = MultiByteToWideChar(CP_ACP, 0, (char*)mpTxtMappedBase, mFileSize.LowPart, NULL, 0);
	if (wideCharCount == 0)
		return EDERR_UNSUCCESSFUL;

	// 分配内存用于解包
	mpTextBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (wideCharCount+10) * 2);
	if (mpTextBuffer == NULL)
		return EDERR_NOTENOUGH_MEMORY;

	if (MultiByteToWideChar(CP_ACP,0, (char*)mpTxtMappedBase,mFileSize.LowPart,mpTextBuffer,wideCharCount) == 0)
		return EDERR_UNSUCCESSFUL;

	mpTextBuffer[wideCharCount] = UNICODE_NULL;

	mTextCharCount = wideCharCount;

	return EDERR_SUCCESS;
}

ED_ERR CtxtDocument::UnpackBigEnding16(void)
{
	uint32 wideCharCount = mFileSize.LowPart / 2;
	// 分配内存用于解包
	mpTextBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, (wideCharCount +10) * 2);
	if (mpTextBuffer == NULL)
		return EDERR_NOTENOUGH_MEMORY;

	auto crtWideChar = mpTextBuffer;
	auto endWideChar = crtWideChar + mTextCharCount;

	// skip BOM
	auto nextChar = mpTxtMappedBase + 2;
	auto endChar = mpTxtMappedBase + mFileSize.LowPart;

	while (nextChar < endChar)
	{
		*crtWideChar = ((((wchar_t)(nextChar[0])) << 8) | nextChar[1]);

		nextChar += 2;
		crtWideChar++;
	}

	mTextCharCount = (uint32)(crtWideChar - mpTextBuffer);
	*crtWideChar = UNICODE_NULL;

	return EDERR_SUCCESS;
}

bool32 CtxtDocument::LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr)
{
	mPageLoadCallbackFun = callBackFun;
	mPageLoadCallbackContext = contextPtr;
 
	return true;
}

int32 CtxtDocument::GetMetaData(char* keyName, char* dataBuf, int32 bufSize)
{
	if (_stricmp(keyName, "format") == 0)
	{
		strcpy_s(dataBuf, bufSize, "Text files");
		return (int32)strlen("Text files");
	}

	return 0;
}

int32 CtxtDocument::GetDocType(void)
{
	return 0;
}

int32 CtxtDocument::GetPageCount(void)
{
	int32 revDate;

	mThreadDataLock.Enter();

	revDate = mAllPages.Size();

	mThreadDataLock.Leave();

	return revDate;
}

ED_ERR CtxtDocument::LoadPage(CtxtPage* pageObj)
{
	ED_ERR errBack = EDERR_SUCCESS;


	return errBack;
}

IEdPage_ptr CtxtDocument::GetPage(PPAGE_PDF_CONTEXT contextPtr)
{
	CtxtPage* pageObj = NULL;
	int32 pageIndex;
	PAGE_PDF_CONTEXT defaultContext = { 0,0,0 };

	if (contextPtr != NULL)
	{
		if(defaultContext.pageContext < mTextCharCount)
			defaultContext = *contextPtr;	// 有效则使用它

		contextPtr = &defaultContext;
	}


	mThreadDataLock.Enter();

	pageIndex = contextPtr->pageIndex;
	if (contextPtr->pageIndex >= (uint32)mAllPages.Size() || mAllPages[contextPtr->pageIndex].charBegin != contextPtr->pageContext)
	{
		for (int i = 0; i < mAllPages.Size(); i++)
		{
			if (mAllPages[i].charBegin == contextPtr->pageContext)
			{
				pageIndex = i;
				break;
			}
		}
	}

	if (pageIndex < mAllPages.Size())
	{
		try
		{
			pageObj = CtxtPage::CreateInstance(pageIndex, mpTextBuffer + mAllPages[pageIndex].charBegin, mAllPages[pageIndex].charCount, this);
			if (pageObj != NULL && ERR_FAILED(LoadPage(pageObj)))
			{
				pageObj->Release();
				pageObj = NULL;
			}
		}
		catch (...)
		{
		}
	}

	mThreadDataLock.Leave();


	return pageObj;
}

IEdPage_ptr CtxtDocument::GetPage(int32 pageIndex)
{
	CtxtPage* pageObj = NULL;

	mThreadDataLock.Enter();

	if (pageIndex >= 0 && pageIndex < mAllPages.Size())
	{
		try
		{
			pageObj = CtxtPage::CreateInstance(pageIndex, mpTextBuffer + mAllPages[pageIndex].charBegin, mAllPages[pageIndex].charCount, this);
			if (pageObj != NULL && ERR_FAILED(LoadPage(pageObj)))
			{
				pageObj->Release();
				pageObj = NULL;
			}
		}
		catch (...)
		{
		}
	}

	mThreadDataLock.Leave();


	return pageObj;
}

IEdPage_ptr CtxtDocument::GetPage(IEdPage_ptr currentPage, int32 pagesOff)
{
	PAGE_PDF_CONTEXT pageContext;
	int32 pageIndex;

	currentPage->GetPageContext(&pageContext);

	mThreadDataLock.Enter();

	pageIndex = (int32)pageContext.pageIndex;
	if (pageIndex >= mAllPages.Size() || mAllPages[pageIndex].charBegin != pageContext.pageContext)
	{	// 如果index不对，则去数据中查询到更新的index
		for (int i = 0; i < mAllPages.Size(); i++)
		{
			if (mAllPages[i].charBegin == pageContext.pageContext)
			{
				pageIndex = i;
				break;
			}
		}
	}

	if (pageIndex < mAllPages.Size() && pageIndex + pagesOff >= 0 && pageIndex + pagesOff < mAllPages.Size())
	{
		pageContext.pageIndex = (uint32)(pageIndex+pagesOff);
		pageContext.pageContext = mAllPages[pageContext.pageIndex].charBegin;
		pageContext.pageContext2 = 0;
	}
	else
		pageContext.pageIndex = MAXULONG32;

	mThreadDataLock.Leave();

	if (pageContext.pageIndex == MAXULONG32)
		return NULL;

	return GetPage(&pageContext);
}


IEdPage_ptr CtxtDocument::Rearrange(IEdPage_ptr currentPage)
{
	PAGE_PDF_CONTEXT context = { 0,0,0 };

	if (currentPage != NULL)
		currentPage->GetPageContext(&context);

	return Rearrange(&context);
}

cmmVector<ULONG> threadArrived;
// This method to update the layout of all pages.
IEdPage_ptr CtxtDocument::Rearrange(PPAGE_PDF_CONTEXT contextPtr)
{
	IEdPage_ptr newPage = NULL;

	threadArrived.Insert(-1, GetCurrentThreadId());

	if (mUpdateForSize == false && mUpdateForFont == false)
		return NULL;

	Gdiplus::RectF viewPort;

	viewPort.X = 0;
	viewPort.Y = 0;
	viewPort.Width = mViewPort.X;
	viewPort.Height = mViewPort.Y;


	// 释放前一个线程对象
	CTxdArrangeThread* lpThreadObj = (CTxdArrangeThread*)InterlockedExchangePointer((void**)&mpArrangeThread, NULL);
	if (lpThreadObj != NULL)
		delete lpThreadObj;

	lpThreadObj = new CTxdArrangeThread(mpTextBuffer, contextPtr->pageContext, mTextCharCount, mFontName, (float)mFontSize, &viewPort, (PEDDOC_THREAD_CALLBACK)CtxtDocument::PageArrangedCallBack, (void*)this);
	if (lpThreadObj == NULL)
		return NULL;

	InterlockedExchangePointer((void**)&mpArrangeThread, lpThreadObj);

	mThreadNumber = lpThreadObj->mThreadNumber;

	//清空当前页面
	mThreadDataLock.Enter();
	mAllPages.Clear();
	mThreadDataLock.Leave();

	// 开始排版
	lpThreadObj->Start();

	if (lpThreadObj->mResult != EDERR_SUCCESS)
		return NULL;

	return GetPage(contextPtr);
}

void CtxtDocument::SetFont(const char16_ptr fontName, int32Eink fontSize)
{
	wcscpy_s(mFontName, 256, fontName);
	mFontSize = (float)fontSize;

	mUpdateForFont = true;
}

void CtxtDocument::SetViewPort(int32Eink viewWidth, int32Eink viewHeight)
{
	if (mViewPort.X != (float)viewWidth || mViewPort.Y != (float)viewHeight)
	{
		mViewPort.X = (float)viewWidth;
		mViewPort.Y = (float)viewHeight;
		mUpdateForSize = true;
	}
}

int32Eink CtxtDocument::GetPageIndex(CtxtPage* pageObj)
{
	uint32 pageBegin = (uint32)(pageObj->mCharPtr - mpTextBuffer);
	int32Eink pageIndex = -1;

	mThreadDataLock.Enter();

	if (pageObj->mPageIndexOpenning >= 0 && pageObj->mPageIndexOpenning < mAllPages.Size() \
		&& mAllPages[pageObj->mPageIndexOpenning].charBegin == pageBegin)
	{
		pageIndex = pageObj->mPageIndexOpenning;
	}
	else
	{
		for (int i = 0; i < mAllPages.Size(); i++)
		{
			if (mAllPages[i].charBegin == pageBegin)
			{
				pageIndex = i;
				pageObj->mPageIndexOpenning = i;
				break;
			}
		}
	}

	mThreadDataLock.Leave();

	return pageIndex;
}

CtxtDocument::BOM CtxtDocument::GetBomInfor()
{
	const unsigned char* const stringHead = (const unsigned char*)mpTxtMappedBase;
	mBom = CtxtDocument::ansi;
//	WCHAR string[] = L"私は4年前に東京に行ってきました。 私が到着したのは寒い秋です。 その気持ちはとても悪い国には決して住んでいないので、私の気持ちはとても難しいです。最初の1ヶ月は東京に住んでいました。";
	//WCHAR string[] = L"大大泡泡糖";
	//char ss[] = "大大AAA";

	do 
	{
		if (mFileSize.LowPart < 4)
			break;

		if (stringHead[0] == 0xFF && stringHead[1] == 0xFE)
		{
			mBom = u16;
			break;
		}

		if (stringHead[0] == 0xFE && stringHead[1] == 0xFF)
		{
			mBom = u16b;
			break;
		}

		if (stringHead[0] == 0xEF && stringHead[1] == 0xBB && stringHead[2] == 0xBF)
		{
			mBom = u8;
			break;
		}

		// 目前不支持Utf7格式
		//if (stringHead[0] == 0x2B && stringHead[1] == 0x2F && stringHead[2] == 0x76 )
		//{
		//	mBom = u7;
		//	break;
		//}

		// 尝试分析一下，这个目标是不是utf8
		if (IsU8File(mpTxtMappedBase,mFileSize.LowPart) != false)
		{
			mBom = u8NoBOM;
		}

	} while (false);
		
	return mBom;
}

void __stdcall CtxtDocument::PageArrangedCallBack(LONG threadNumber, uint32 loadingStep, PST_PAGE_CONTROL pageCtl, CtxtDocument* thisDoc)
{
	if (thisDoc->mThreadNumber != threadNumber)
		return;

	thisDoc->mThreadDataLock.Enter();

	if (loadingStep > 0 && loadingStep < MAXULONG32)
	{
		if (loadingStep == 0xCFFFFFFF)
		{
			// 特殊用法，直接复制整个向量
			PageControl_Vector* pageVector = (PageControl_Vector*)pageCtl;

			thisDoc->mAllPages.Insert(0, *pageVector);
		}
		else
		if ((loadingStep&0xC0000000)!=0)
			thisDoc->mAllPages.Insert((loadingStep&(~0xC0000000)), *pageCtl);
		else
			thisDoc->mAllPages.Insert(-1, *pageCtl);
	}

	uint32 pageCount = thisDoc->mAllPages.Size();

	thisDoc->mThreadDataLock.Leave();

	// 调用
	thisDoc->mPageLoadCallbackFun(loadingStep, pageCount, thisDoc->mPageLoadCallbackContext);
}


