/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


/* Copyright 2015 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

/*
modify:
add the loading progress callback function
*/

#include "EbookEngine.cpp"	// 由于这个库将类定义放在了cpp中，为了重载部分类，只好加在这里了
// utils
#include "BaseUtil.h"
// rendering engines
//#include "BaseEngine.h"
#include "DjVuEngine.h"
//#include "EbookEngine.h"
#include "ImagesEngine.h"
#include "PdfEngine.h"
#include "PsEngine.h"
#include "EngineOverride.h"

/*
	Ax Jun.6,2019
	支持异步装入同时打开的解决思路（尚未实现）：
		EpubFormatOverride::FormatAllPages函数直接访问EpubOverride类的pages对象，添加页进入其中。主要做好对page对象访问时的数据并发访问冲突
*/


void DrawHtmlPageNoLink(Graphics *g, mui::ITextRender *textDraw, Vec<DrawInstr> *drawInstructions, REAL offX, REAL offY, bool showBbox, Color textColor, bool *abortCookie);
BaseEngine *PdfCreateFromFile(const wchar_t *fileName, PasswordUI *pwdUI, BaseEngine ** engineObj, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL);


namespace EngineLoad {

	static const WCHAR *GetDefaultFontName()
	{
		return gDefaultFontName ? gDefaultFontName : L"微软雅黑";
	}


	class EpubFormatOverride : public EpubFormatter
	{
	public:
		EpubFormatOverride(HtmlFormatterArgs *args, EpubDoc *doc, PEDSMT_THREAD_CALLBACK callFun=NULL,void* callContext=NULL) :
			EpubFormatter(args,doc) { 
			mCallBackContext = callContext;
			mCallBackFun = callFun;
		}

		Vec<HtmlPage*> *FormatAllPages(bool skipEmptyPages)	// copy from HtmlFormatter::FormatAllPages(bool skipEmptyPages)
		{
			Vec<HtmlPage *> *pages = new Vec<HtmlPage *>();

			int stepLoad = 0;
			InvokeCallbackFunction(stepLoad++);

			for (HtmlPage *pd = Next(skipEmptyPages); pd; pd = Next(skipEmptyPages)) {
				pages->Append(pd);

				//if(stepLoad <= 1) //为了效率，暂时只通知一次 ????niu 为什么调用一次程序会出错？
					if(InvokeCallbackFunction(stepLoad++)==false)
						break;//exit

			}

			InvokeCallbackFunction(0xFFFFFFFF);

			return pages;
		}
	protected:
		PEDSMT_THREAD_CALLBACK mCallBackFun;
		void* mCallBackContext;

		bool InvokeCallbackFunction(uint32 loadingStep) {
			bool rev = false;
			if (mCallBackFun != NULL)
			{
				try {
					rev = mCallBackFun(loadingStep, mCallBackContext);
				}
				catch(...){}
			}
			return rev;
		}
	};

class EpubOverride : public EpubEngineImpl
{
protected:
	PEDSMT_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;

public:
	EpubOverride(PEDSMT_THREAD_CALLBACK callFun, void* callContext):EpubEngineImpl(){
		mCallBackContext = callContext;
		mCallBackFun = callFun;
	};

	static BaseEngine *CreateFromFile(const WCHAR *fileName, BaseEngine ** engineObj, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL) {
		EpubOverride *engine = new EpubOverride(callFun,callContext);
		InterlockedExchangePointer((void**)engineObj, engine);
		if (!engine->Load(fileName)) {
			delete engine;
			InterlockedExchangePointer((void**)engineObj, NULL);
			return nullptr;
		}
		return engine;
	}

	bool Load(const WCHAR *fileName)	// copy from EpubEngineImpl::Load(const WCHAR *fileName)
	{
		this->fileName = str::Dup(fileName);
		if (dir::Exists(fileName)) {
			// load uncompressed documents as a recompressed ZIP stream
			ScopedComPtr<IStream> zipStream(OpenDirAsZipStream(fileName, true));
			if (!zipStream)
				return false;
			return EpubEngineImpl::Load(zipStream);
		}
		doc = EpubDoc::CreateFromFile(fileName);
		if (doc == NULL)
			return false;
		return FinishLoading();
	}

	bool FinishLoading()		// copy from EpubEngineImpl::FinishLoading()
	{
		if (!doc)
			return false;

		HtmlFormatterArgs args;
		args.htmlStr = doc->GetHtmlData(&args.htmlStrLen);
		args.pageDx = (float)pageRect.dx - 2 * pageBorder;
		args.pageDy = (float)pageRect.dy - 2 * pageBorder;
		args.SetFontName(EngineLoad::GetDefaultFontName());
		args.fontSize = GetDefaultFontSize();
		args.textAllocator = &allocator;
		args.textRenderMethod = mui::TextRenderMethodGdiplusQuick;

		pages = EpubFormatOverride(&args, doc,mCallBackFun,mCallBackContext).FormatAllPages(false);
		if (!ExtractPageAnchors())
			return false;

		return pages->Count() > 0;
	}

	RenderedBitmap *RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut) // copy from EbookEngine::RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut)
	{
		UNUSED(target);
		RectD pageRc = pageRect ? *pageRect : PageMediabox(pageNo);
		RectI screen = Transform(pageRc, pageNo, zoom, rotation).Round();
		PointI screenTL = screen.TL();
		screen.Offset(-screen.x, -screen.y);

		HANDLE hMap = nullptr;
		HBITMAP hbmp = CreateMemoryBitmap(screen.Size(), &hMap);
		HDC hDC = CreateCompatibleDC(nullptr);
		DeleteObject(SelectObject(hDC, hbmp));

		Graphics g(hDC);
		mui::InitGraphicsMode(&g);

		Color white(0xFF, 0xFF, 0xFF);
		SolidBrush tmpBrush(white);
		Rect screenR(screen.ToGdipRect());
		screenR.Inflate(1, 1);
		g.FillRectangle(&tmpBrush, screenR);

		Matrix m;
		GetTransform(m, zoom, rotation);
		m.Translate((REAL)-screenTL.x, (REAL)-screenTL.y, MatrixOrderAppend);
		g.SetTransform(&m);

		EbookAbortCookie *cookie = nullptr;
		if (cookieOut)
			*cookieOut = cookie = new EbookAbortCookie();

		ScopedCritSec scope(&pagesAccess);

		mui::ITextRender *textDraw = mui::TextRenderGdiplus::Create(&g);
		DrawHtmlPageNoLink(&g, textDraw, GetHtmlPage(pageNo), pageBorder, pageBorder, false, Color((ARGB)Color::Black), cookie ? &cookie->abort : nullptr);
		DrawAnnotations(g, userAnnots, pageNo);
		delete textDraw;
		DeleteDC(hDC);

		if (cookie && cookie->abort) {
			DeleteObject(hbmp);
			CloseHandle(hMap);
			return nullptr;
		}

		return new RenderedBitmap(hbmp, screen.Size(), hMap);
	}

};

class MobiFormatOverride : public MobiFormatter
{
public:
	MobiFormatOverride(HtmlFormatterArgs *args, MobiDoc *doc,PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL) :
		MobiFormatter(args, doc) {
		mCallBackContext = callContext;
		mCallBackFun = callFun;
	}

	Vec<HtmlPage*> *FormatAllPages(bool skipEmptyPages)	// copy from HtmlFormatter::FormatAllPages(bool skipEmptyPages)
	{
		Vec<HtmlPage *> *pages = new Vec<HtmlPage *>();

		int stepLoad = 0;
		InvokeCallbackFunction(stepLoad++);

		for (HtmlPage *pd = Next(skipEmptyPages); pd; pd = Next(skipEmptyPages)) {
			pages->Append(pd);

			if (InvokeCallbackFunction(stepLoad++) == false)
				break;//exit
		}

		InvokeCallbackFunction(0xFFFFFFFF);

		return pages;
	}
protected:
	PEDSMT_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;

	bool InvokeCallbackFunction(uint32 loadingStep) {
		bool rev = false;
		if (mCallBackFun != NULL)
		{
			try {
				rev = mCallBackFun(loadingStep, mCallBackContext);
			}
			catch (...) {}
		}
		return rev;
	}
};


class MobiOverride : public MobiEngineImpl
{
protected:
	PEDSMT_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;

public:
	MobiOverride(PEDSMT_THREAD_CALLBACK callFun, void* callContext) :MobiEngineImpl() {
		mCallBackContext = callContext;
		mCallBackFun = callFun;
	};

	static BaseEngine *CreateFromFile(const WCHAR *fileName, BaseEngine ** engineObj, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL) {
		MobiOverride *engine = new MobiOverride(callFun, callContext);
		InterlockedExchangePointer((void**)engineObj, engine);
		if (!engine->Load(fileName)) {
			delete engine;
			InterlockedExchangePointer((void**)engineObj, NULL);
			return nullptr;
		}
		return engine;
	}

	bool Load(const WCHAR *fileName)	// copy from EpubEngineImpl::Load(const WCHAR *fileName)
	{
		this->fileName = str::Dup(fileName);
		doc = MobiDoc::CreateFromFile(fileName);
		if (doc == NULL)
			return false;
		return FinishLoading();
	}

	bool FinishLoading()		// copy from EpubEngineImpl::FinishLoading()
	{
		HtmlFormatterArgs args;
		args.htmlStr = doc->GetHtmlData(args.htmlStrLen);
		args.pageDx = (float)pageRect.dx - 2 * pageBorder;
		args.pageDy = (float)pageRect.dy - 2 * pageBorder;
		args.SetFontName(EngineLoad::GetDefaultFontName());
		args.fontSize = GetDefaultFontSize();
		args.textAllocator = &allocator;
		args.textRenderMethod = mui::TextRenderMethodGdiplusQuick;

		pages = MobiFormatOverride(&args, doc, mCallBackFun, mCallBackContext).FormatAllPages(FALSE);
		if (!ExtractPageAnchors())
			return false;

		return pages->Count() > 0;
	}

	RenderedBitmap *RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut) // copy from EbookEngine::RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut)
	{
		UNUSED(target);
		RectD pageRc = pageRect ? *pageRect : PageMediabox(pageNo);
		RectI screen = Transform(pageRc, pageNo, zoom, rotation).Round();
		PointI screenTL = screen.TL();
		screen.Offset(-screen.x, -screen.y);

		HANDLE hMap = nullptr;
		HBITMAP hbmp = CreateMemoryBitmap(screen.Size(), &hMap);
		HDC hDC = CreateCompatibleDC(nullptr);
		DeleteObject(SelectObject(hDC, hbmp));

		Graphics g(hDC);
		mui::InitGraphicsMode(&g);

		Color white(0xFF, 0xFF, 0xFF);
		SolidBrush tmpBrush(white);
		Rect screenR(screen.ToGdipRect());
		screenR.Inflate(1, 1);
		g.FillRectangle(&tmpBrush, screenR);

		Matrix m;
		GetTransform(m, zoom, rotation);
		m.Translate((REAL)-screenTL.x, (REAL)-screenTL.y, MatrixOrderAppend);
		g.SetTransform(&m);

		EbookAbortCookie *cookie = nullptr;
		if (cookieOut)
			*cookieOut = cookie = new EbookAbortCookie();

		ScopedCritSec scope(&pagesAccess);

		mui::ITextRender *textDraw = mui::TextRenderGdiplus::Create(&g);
		DrawHtmlPageNoLink(&g, textDraw, GetHtmlPage(pageNo), pageBorder, pageBorder, false, Color((ARGB)Color::Black), cookie ? &cookie->abort : nullptr);
		DrawAnnotations(g, userAnnots, pageNo);
		delete textDraw;
		DeleteDC(hDC);

		if (cookie && cookie->abort) {
			DeleteObject(hbmp);
			CloseHandle(hMap);
			return nullptr;
		}

		return new RenderedBitmap(hbmp, screen.Size(), hMap);
	}

};

bool IsSupportedFile(const WCHAR *filePath, bool sniff)
{
	return PdfEngine::IsSupportedFile(filePath, sniff) ||
		//XpsEngine::IsSupportedFile(filePath, sniff) ||
		//DjVuEngine::IsSupportedFile(filePath, sniff) ||
		//ImageEngine::IsSupportedFile(filePath, sniff) ||
		//ImageDirEngine::IsSupportedFile(filePath, sniff) ||
		//CbxEngine::IsSupportedFile(filePath, sniff) ||
		//PsEngine::IsSupportedFile(filePath, sniff) ||
		//ChmEngine::IsSupportedFile(filePath, sniff) ||
		EpubEngine::IsSupportedFile(filePath, sniff) ||
		//Fb2Engine::IsSupportedFile(filePath, sniff) ||
		MobiEngine::IsSupportedFile(filePath, sniff);
		//PdbEngine::IsSupportedFile(filePath, sniff) ||
		//HtmlEngine::IsSupportedFile(filePath, sniff) ||
		//TxtEngine::IsSupportedFile(filePath, sniff)
}

bool CreateEngine(const WCHAR *filePath, BaseEngine ** engineObj, PasswordUI *pwdUI, EngineType *typeOut , PEDSMT_THREAD_CALLBACK callFun , void* callContext)
{
    CrashIf(!filePath);

    BaseEngine *engine = nullptr;
	EngineType engineType = Engine_None;

    if (PdfEngine::IsSupportedFile(filePath, false) && engineType != Engine_PDF) {
        engine = PdfCreateFromFile(filePath, pwdUI, engineObj, callFun, callContext);
        engineType = Engine_PDF;
    }
 //   else if (ImageEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Image) {
 //       engine = ImageEngine::CreateFromFile(filePath);
 //       engineType = Engine_Image;
 //   } 
	//else if (ChmEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Chm) {
 //       engine = ChmEngine::CreateFromFile(filePath);
 //       engineType = Engine_Chm;
 //   } 
	else if (EpubEngine::IsSupportedFile(filePath, false) && engineType != Engine_Epub) {
        engine = EpubOverride::CreateFromFile(filePath,engineObj,callFun,callContext);
        engineType = Engine_Epub;
    } else if (MobiEngine::IsSupportedFile(filePath, false) && engineType != Engine_Mobi) {
        engine = MobiOverride::CreateFromFile(filePath, engineObj, callFun, callContext);
        engineType = Engine_Mobi;
    } 
	//else if (HtmlEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Html) {
 //       engine = HtmlEngine::CreateFromFile(filePath);
 //       engineType = Engine_Html;
 //   } else if (TxtEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Txt) {
 //       engine = TxtEngine::CreateFromFile(filePath);
 //       engineType = Engine_Txt;
 //   }

    CrashIf(engine && !IsSupportedFile(filePath,false));

    if (typeOut)
        *typeOut = engine ? engineType : Engine_None;

	InterlockedExchangePointer((void**)engineObj, engine);
    return true;
}

} // namespace EngineLoad

