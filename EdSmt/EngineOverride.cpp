/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */



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
#include "cmmBaseObj.h"
#include "cmmString.h"

/*
	Ax Apr.11,2020
	修改异步装入的错误，直接在构造Formatter对象时，传入ReparseIdx
			args.reparseIdx = mInitPage.pageContext;
	去掉重载格式化函数中的
			htmlParser->SetCurrPosOff(pageContext->pageContext);	// 将当前解析位置设置为目标页起始位置（0开始）

	Ax Jun.6,2019
	支持异步装入同时打开的解决思路（尚未实现）：
		EpubFormatOverride::FormatAllPages函数直接访问EpubOverride类的pages对象，添加页进入其中。主要做好对page对象访问时的数据并发访问冲突
	
	Ax Nov.27,2019
	将原来Epub和Mobi分别实现的FormatAll,Load,Create等功能，用模板方法实现为一套；
	增加对BaseEngine访问的互斥保护

*/


void DrawHtmlPageNoLink(Graphics *g, mui::ITextRender *textDraw, Vec<DrawInstr> *drawInstructions, REAL offX, REAL offY, bool showBbox, Color textColor, bool *abortCookie);
//BaseEngine *PdfCreateFromFile(const wchar_t *fileName, PasswordUI *pwdUI, BaseEngine ** engineObj, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL);


namespace EngineLoad {

	static const WCHAR *GetDefaultFontName()
	{
		return gDefaultFontName ? gDefaultFontName : L"微软雅黑";
	}

	template<class FormatterClass,class DocClass>
	class EngineFormatter:public FormatterClass{
	public:
		EngineFormatter(HtmlFormatterArgs *args, DocClass *doc, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL):FormatterClass(args,doc){
			mCallBackContext = callContext;
			mCallBackFun = callFun;
		}
		~EngineFormatter() {}

		void FormatAllPages(Vec<HtmlPage *>* pages, volatile LONG* baseNoInPages,CExclusiveAccess& lock, bool skipEmptyPages, PPAGE_PDF_CONTEXT pageContext)
		{
			// 如果不在首页，则从指定页开始装入全部页
			if (pageContext != NULL/* && pageContext->pageIndex > 0 */&& pageContext->pageContext > 0)
			{
				CSectionAutoLock lock(lock);

				InterlockedExchange(baseNoInPages, pageContext->pageIndex);

				// 装入首页，用来初始化部分数据结构，开源代码有些问题，通过这个规避
				htmlParser->SetCurrPosOff(0);
				HtmlPage *pd = Next(skipEmptyPages);

				htmlParser->SetCurrPosOff(pageContext->pageContext);	// 将当前解析位置设置为目标页起始位置（0开始，函数内会加上内存基址）
				currPage->reparseIdx = pageContext->pageContext;		// 将已分配待用的page对象的ReparseIndex设置到当前解析位置
			}

			for (HtmlPage *pd = Next(skipEmptyPages); pd; pd = Next(skipEmptyPages)) {
				{
					CSectionAutoLock lock(lock);
					pages->Append(pd);
				}

				if (InvokeCallbackFunction(mStepLoad++) == false)
					break;//exit

			}

			// 从首页装入剩下还没有装入的头部页
			if (pageContext != NULL/* && pageContext->pageIndex > 0 */&& pageContext->pageContext > 0)
			{
				Vec<HtmlPage *> pagesAhead;

				htmlParser->SetCurrPosOff(0);	// 设定当前解析位置为内容区开始位置（0开始）
				finishedParsing = false;		// 清楚上一次解析循环留下的结束标志
				currReparseIdx = 0;				// 将当前重解析位置设置为0，下面的EmitNewPage生成的page对象的reparseIdx由此设置

				EmitNewPage();					// 分配一个新的page对象待用

				HtmlPage* pd = Next(false);// skipEmptyPages); modified by Ax. 我也不知道我之前写的代码这个地方要单独调用一次，不过这个地方如果设置为true会导致图片首页被抛弃，所以，还是不要开启skip空白页

				for (pd = Next(skipEmptyPages); pd; pd = Next(skipEmptyPages)) {
					{
						CSectionAutoLock lock(lock);

						if ((ULONG)abs(pd->reparseIdx) >= pageContext->pageContext)	// 这个对象是否是已经装入的解析开始页
						{
							delete pd;	// 删除掉这个页对象，并不会影响真是的页内容，已装入的页对象不会受到影响
							break;
						}

						pagesAhead.InsertAt(0,pd);
					}

					if (InvokeCallbackFunction(mStepLoad++) == false)
						break;//exit

				}

				// 合并数据
				{
					CSectionAutoLock lock(lock);

					for (int i = 0; i < pagesAhead.Size(); i++)
					{
						pages->InsertAt(0, pagesAhead.At(i));
					}

				}
			}
		}

		void InitCallBack() {
			mStepLoad = 0;
			InvokeCallbackFunction(mStepLoad++);
		}

		void EndCallBack() {
			InvokeCallbackFunction(0xFFFFFFFF);
		}

	protected:
		PEDSMT_THREAD_CALLBACK mCallBackFun;
		void* mCallBackContext;
		int mStepLoad;

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

	class EpubOverride: public EpubEngineImpl
	{
	public:
		const char * GetHtmlData(size_t& lenOut) {
			return doc->GetHtmlData(&lenOut);
		}
		virtual bool FinishLoading()
		{
			return false;
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
		const WCHAR *GetDefaultFileExt() const {// 为了用BaseEngine*返回我最终的模板类型，只好重载这个不重要的函数
			return L"epub";
		}

		EngineType GetType() {
			return Engine_Epub;
		}
	};

	class MobiOverride : public MobiEngineImpl
	{
	public:
		const char * GetHtmlData(size_t& lenOut) {
			return doc->GetHtmlData(lenOut);
		}
		virtual bool FinishLoading()
		{
			return false;
		}
		bool Load(const WCHAR *fileName)	// copy from EpubEngineImpl::Load(const WCHAR *fileName)
		{
			this->fileName = str::Dup(fileName);
			doc = MobiDoc::CreateFromFile(fileName);
			if (doc == NULL)
				return false;
			return FinishLoading();
		}
		const WCHAR *GetDefaultFileExt() const {// 为了用BaseEngine*返回我最终的模板类型，只好重载这个不重要的函数
			return L"mobi";
		}
		EngineType GetType() {
			return Engine_Mobi;
		}
	};

	template<class EngineClass,class FormatterClass, class DocClass>
	class EngineTemplate:public EngineClass
	{
	protected:
		PEDSMT_THREAD_CALLBACK mCallBackFun;
		void* mCallBackContext;
		CExclusiveAccess mAccLock;
		PAGE_PDF_CONTEXT mInitPage;
		volatile LONG mBaseNoInPages;
		volatile LONG mLoadInPages;

	public:
		EngineTemplate(PEDSMT_THREAD_CALLBACK callFun, void* callContext){
			mCallBackContext = callContext;
			mCallBackFun = callFun;
		};

		static bool CreateFromFile(const WCHAR *fileName, BaseEngine ** engineObj, EngineType& eType,PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL, PPAGE_PDF_CONTEXT pageContext = NULL) {
			EngineTemplate<EngineClass,FormatterClass,DocClass> *engine = new EngineTemplate<EngineClass, FormatterClass, DocClass>(callFun, callContext);
			InterlockedExchangePointer((void**)engineObj, engine);
			if (pageContext != NULL)
			{
				engine->mInitPage.pageIndex = pageContext->pageIndex;
				engine->mInitPage.pageContext = pageContext->pageContext;
				engine->mInitPage.pageContext2 = pageContext->pageContext2;
			}
			else
			{
				engine->mInitPage.pageIndex = 0;
				engine->mInitPage.pageContext = 0;
 				//engine->mInitPage.pageIndex = 29;// 0;	???
 				//engine->mInitPage.pageContext = 0x12677;// 0;
				engine->mInitPage.pageContext2 = 0;
			}

			bool rev = true;
			InterlockedExchange(&engine->mLoadInPages, 1);

			eType = engine->GetType();
			if (!engine->Load(fileName)) {
				delete engine;
				engine = NULL;
				InterlockedExchangePointer((void**)engineObj, NULL);
				rev = false;
			}

			if(engine != NULL)
				InterlockedExchange(&engine->mLoadInPages, 0);

			return rev;
		}

		bool GetPageContext(int pageNo, PPAGE_PDF_CONTEXT pageContext) {
			CSectionAutoLock lock(mAccLock);

			if (pageNo < 0)
			{
				// 更新pageContext->pageContext(reparse point)对应的页编号
				// 如果当前还在全部页装入状态，无需更新
				if (mLoadInPages != 0)
					return true;

				for (int i = 0; i < pages->Size(); i++)
				{
					auto reparse = ((HtmlPage*)(pages->At(i)))->reparseIdx;
					if (reparse == (int)pageContext->pageContext)
					{
						pageContext->pageIndex = (ULONG)i;
						pageContext->pageContext2 = 0;	// 清除标志，下次就不再需要更新了
						return true;
					}
				}

				return false;
			}
			else
			{
 				pageContext->pageIndex = pageNo;
				pageContext->pageContext = (ULONG)(((HtmlPage*)(pages->At(pageNo)))->reparseIdx);
				pageContext->pageContext2 = mLoadInPages;	// 非0代表正在装入
			}

			return true;
		}

		bool FinishLoading()		// copy from EpubEngineImpl::FinishLoading()
		{
			if (!doc)
				return false;

			HtmlFormatterArgs args;
			args.htmlStr = GetHtmlData(args.htmlStrLen);		// 在MobiEngineImp的地方是args.htmlStr = doc->GetHtmlData(args.htmlStrLen);
			args.pageDx = (float)pageRect.dx - 2 * pageBorder;
			args.pageDy = (float)pageRect.dy - 2 * pageBorder;
			args.SetFontName(EngineLoad::GetDefaultFontName());
			args.fontSize = GetDefaultFontSize();
			args.textAllocator = &allocator;
			args.textRenderMethod = mui::TextRenderMethodGdiplusQuick;
			args.reparseIdx = mInitPage.pageContext;

			pages = new Vec<HtmlPage *>();
			if (pages == NULL)
				return false;

			EngineFormatter<FormatterClass, DocClass> format(&args, doc, mCallBackFun, mCallBackContext);
			
			format.InitCallBack();

			//by Ax There will be some empty-pages while we reopen a document with a reparse point. The page-number would be different than the number your read the page before
			//format.FormatAllPages(pages, &mBaseNoInPages, mAccLock, false, &mInitPage); 
			format.FormatAllPages(pages, &mBaseNoInPages, mAccLock, true, &mInitPage);

			InterlockedExchange(&mLoadInPages, 0);	//CreateFromFile里面的调用时机晚了，所以提前调用以下

			format.EndCallBack();

			if (!ExtractPageAnchors())
				return false;


			return pages->Count() > 0;
		}

		RenderedBitmap *RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut) // copy from EbookEngine::RenderBitmap(int pageNo, float zoom, int rotation, RectD *pageRect, RenderTarget target, AbortCookie **cookieOut)
		{
			//CSectionAutoLock lock(mAccLock);
			mAccLock.Enter();
			RenderedBitmap* rev = NULL;

			__try {
				auto ff = [&](){
					RectD pageRc = pageRect ? *pageRect : PageMediabox(pageNo);
					RectI screen = EngineClass::Transform(pageRc, pageNo, zoom, rotation).Round();	// 直接调用基类的函数，省去重复进入互斥区
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
						return (RenderedBitmap*)nullptr;
					}
					else
					{
						return new RenderedBitmap(hbmp, screen.Size(), hMap);
					}
				};

				rev = ff();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				if(rev != nullptr)
					rev = nullptr;
			}

			mAccLock.Leave();
			return rev;
		}

		PointD Transform(PointD pt, int pageNo, float zoom, int rotation, bool inverse = false) {
			CSectionAutoLock lock(mAccLock);
			return EngineClass::Transform(pt, pageNo, zoom, rotation, inverse);
		}

		RectD Transform(RectD rect, int pageNo, float zoom, int rotation, bool inverse = false) {
			CSectionAutoLock lock(mAccLock);
			return EngineClass::Transform(rect, pageNo, zoom, rotation, inverse);
		}


	};


bool CEngineHandle::IsSupportedFile(const WCHAR *filePath, bool sniff /*= false*/)
{
	return //PdfEngine::IsSupportedFile(filePath, sniff) ||
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

bool CEngineHandle::CreateEngine(const WCHAR *filePath, PasswordUI *pwdUI /*= nullptr*/, PEDSMT_THREAD_CALLBACK callFun /*= NULL*/, void* callContext /*= NULL*/, PPAGE_PDF_CONTEXT pageContext /*= NULL*/)
{
	CrashIf(!filePath);

	bool rev = false;

	//EngineType engineType = Engine_None;

	//if (PdfEngine::IsSupportedFile(filePath, false) && engineType != Engine_PDF) {
	//    engine = PdfCreateFromFile(filePath, pwdUI, engineObj, callFun, callContext);
	//    engineType = Engine_PDF;
	//}
	//   else if (ImageEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Image) {
	//       engine = ImageEngine::CreateFromFile(filePath);
	//       engineType = Engine_Image;
	//   } 
	//else if (ChmEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Chm) {
	//       engine = ChmEngine::CreateFromFile(filePath);
	//       engineType = Engine_Chm;
	//   } 
	//else
	if (EpubEngine::IsSupportedFile(filePath, false)/* && engineType != Engine_Epub*/) {
		//engine = EpubOverride::CreateFromFile(filePath,engineObj,callFun,callContext);
		rev = EngineTemplate<EpubOverride, EpubFormatter, EpubDoc>::CreateFromFile(filePath,&mRawEngine,mEngineType,callFun, callContext, pageContext);
	}
	else if (MobiEngine::IsSupportedFile(filePath, false) /*&& engineType != Engine_Mobi*/) {
		//engine = MobiOverride::CreateFromFile(filePath, engineObj, callFun, callContext);
		rev = EngineTemplate<MobiOverride, MobiFormatter, MobiDoc>::CreateFromFile(filePath, &mRawEngine,mEngineType, callFun, callContext, pageContext);
	}
	//else if (HtmlEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Html) {
	//       engine = HtmlEngine::CreateFromFile(filePath);
	//       engineType = Engine_Html;
	//   } else if (TxtEngine::IsSupportedFile(filePath, sniff) && engineType != Engine_Txt) {
	//       engine = TxtEngine::CreateFromFile(filePath);
	//       engineType = Engine_Txt;
	//   }

	CrashIf(mRawEngine && !IsSupportedFile(filePath, false));

	if (rev == false)
		mEngineType = Engine_None;

	return rev;
}

bool CEngineHandle::GetPageContext(int pageNo, PPAGE_PDF_CONTEXT pageContext)
{
	if (mRawEngine == NULL)
		return false;

	if (mEngineType == Engine_Epub)
	{
		EngineTemplate<EpubOverride, EpubFormatter, EpubDoc>* engine = dynamic_cast<EngineTemplate<EpubOverride, EpubFormatter, EpubDoc>*>(mRawEngine);
		return engine->GetPageContext(pageNo, pageContext);
	}
	else
	if (mEngineType == Engine_Mobi)
	{
		EngineTemplate<MobiOverride, MobiFormatter, MobiDoc>* engine = dynamic_cast<EngineTemplate<MobiOverride, MobiFormatter, MobiDoc>*>(mRawEngine);
		return engine->GetPageContext(pageNo, pageContext);
	}

	return false;
}

bool CEngineHandle::ReQueryPageIndex(PPAGE_PDF_CONTEXT pageContext)
{
	if (pageContext->pageContext2 == 0)	// 说明这个页是全部页装载完毕才获取的，无需更新这个页编号
		return true;
	
	return GetPageContext(-1,pageContext);
}


int CEngineHandle::PageCount() const
{
	return mRawEngine->PageCount();
}

bool CEngineHandle::RenderThumbnail(const WCHAR *filePath, int pageNo, float scalRatio)
{
	RenderedBitmap* engineBitmap = mRawEngine->RenderBitmap(pageNo+1, scalRatio, 0, NULL);
	if (engineBitmap == NULL)
		return false;

	Bitmap bmp(engineBitmap->GetBitmap(), nullptr);
	CLSID tmpClsid = GetEncoderClsid(L"image/png");
	bmp.Save(filePath, &tmpClsid, nullptr);

	delete engineBitmap;
	return true;
}

} // namespace EngineLoad

