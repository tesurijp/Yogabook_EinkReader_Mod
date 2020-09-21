/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "pdfPage.h"
#include "pdfDocument.h"
#include "thread"

//////////////////////////////////////////////////////////////////////////
DEFINE_BUILTIN_NAME(CpdfDocument)

CpdfDocument::CpdfDocument()
{
	fzDoc = NULL;
	mDirty = false;
	miObjCount = 0;
	mLoadThumb = false;
	mThumbThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CpdfDocument::~CpdfDocument()
{
	//CSectionAutoLock lock(mDocAccLock);
	if(fzDoc != NULL)
		fz_drop_document(CEdpdfModule::GetUniqueObject()->fzContext,fzDoc);
	if (mThumbThreadExit != NULL)
	{
		mLoadThumb = false;
		// 等它5秒钟退出
		if(pageCount > 0)
			WaitForSingleObject(mThumbThreadExit, 5000);
		CloseHandle(mThumbThreadExit);
		mThumbThreadExit = NULL;
	}
}

ED_ERR CpdfDocument::Open_progressive(const char16_ptr fileName)
{
	enum Throw_Err
	{
		HasPass = 100,
		Empty = 101
	};

	CSectionAutoLock lock(mDocAccLock);
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;
	//char *password = "";
	ED_ERR errorBack = EDERR_SUCCESS;
	char multiCharName[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0,fileName, -1, multiCharName,MAX_PATH, 0, FALSE);

	mPathName = fileName;

	fz_try(ctx)
	{
		fz_register_document_handlers(ctx);

		fz_set_use_document_css(ctx, 1);

		fzDoc = fz_open_document(ctx, mPathName.CopyToMultiByteBuffer(NULL,CP_UTF8));

		miObjCount = GetPdfObj()->xref_sections->num_objects;
	}
	fz_catch(ctx)
	{
		return EDERR_FIALED_OPENFILE;
	}

	//idoc = pdf_specifics(ctx, app->doc); 禁用JS，参考event_cb函数实现，可以知道JS的主要目的是点击超链接、弹出对话框、等功能；这儿不使用这些功能 ax Apr.12
	//if (idoc)
	//{
	//	fz_try(ctx)
	//	{
	//		pdf_enable_js(ctx, idoc);
	//		pdf_set_doc_event_callback(ctx, idoc, event_cb, app);
	//	}
	//	fz_catch(ctx)
	//	{
	//		pdfapp_error(app, "cannot load javascript embedded in document");
	//	}
	//}

	fz_try(ctx)
	{

		if (fz_needs_password(ctx, fzDoc))
		{
			fz_throw(ctx, Throw_Err::HasPass, "EDERR_HAS_PASSWORD");
			//int okay = fz_authenticate_password(app->ctx, app->doc, password);
			//while (!okay)
			//{
			//	password = winpassword(app, filename);
			//	if (!password)
			//		fz_throw(ctx, FZ_ERROR_GENERIC, "Needs a password");
			//	okay = fz_authenticate_password(app->ctx, app->doc, password);
			//	if (!okay)
			//		pdfapp_warn(app, "Invalid password.");
			//}
		}

		//fz_layout_document(app->ctx, app->doc, app->layout_w, app->layout_h, app->layout_em);

		while (1)
		{
			fz_try(ctx)
			{
				pageCount = fz_count_pages(ctx, fzDoc);
				if (pageCount <= 0)
					fz_throw(ctx, Throw_Err::Empty, "EDERR_EMPTY_CONTENT");
			}
			fz_catch(ctx)
			{
				if (fz_caught(ctx) == FZ_ERROR_TRYLATER)
					continue;

				//fz_rethrow(ctx);
			}
			break;
		}
		//while (1) 暂时不处理大纲 ax Apr.13
		//{
		//	fz_try(ctx)
		//	{
		//		app->outline = fz_load_outline(app->ctx, app->doc);
		//	}
		//	fz_catch(ctx)
		//	{
		//		app->outline = NULL;
		//		if (fz_caught(ctx) == FZ_ERROR_TRYLATER)
		//			app->outline_deferred = PDFAPP_OUTLINE_DEFERRED;
		//		else
		//			pdfapp_warn(app, "failed to load outline");
		//	}
		//	break;
		//}
	}
	fz_catch(ctx)
	{
		switch (fz_caught(ctx))
		{
		case Throw_Err::HasPass:
			errorBack = EDERR_HAS_PASSWORD;
			break;
		case Throw_Err::Empty:
			errorBack = EDERR_EMPTY_CONTENT;
			break;
		default:
			errorBack = EDERR_UNSUCCESSFUL;
		}
	}

	return errorBack;
}


ULONG CpdfDocument::InitOnCreate(const char16_ptr pathName)
{
	try
	{
		ED_ERR errCode = Open_progressive(pathName);
		if (ERR_FAILED(errCode))
			return EDERR_UNSUCCESSFUL;
	}
	catch (...)
	{
		OutputDebugString(L"oepn file error");
	}


	//LoadPage(1);
	return EDERR_SUCCESS;
}

bool32 CpdfDocument::LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr, PPAGE_PDF_CONTEXT initPage)
{
	//later: 参考下面的smtPage类的实现
	//mPageLoadCallbackFun = callBackFun;
	//mPageLoadCallbackContext = contextPtr;
	//HANDLE Events[2];

	//ULONG luThreadID;

	//mThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CSmtDocument::LoadingThread, (void*)&mThreadData, 0 /*CREATE_SUSPENDED*/, &luThreadID);
	//Events[0] = mThreadHandle;
	//Events[1] = mPageLoadEvent;		// 只要装入一页了，就返回
	//if (mThreadHandle == NULL || WaitForMultipleObjects(2, Events, FALSE, INFINITE) == WAIT_TIMEOUT)
	//{
	//	return false;
	//}

	//if (documentEngine == nullptr)
	//{
	//	return false;
	//}

	//pageCount = documentEngine->PageCount();

	callBackFun(0xFFFFFFFF, pageCount, contextPtr);

	return true;
}



int32 CpdfDocument::GetMetaData(char* keyName, char* dataBuf, int32 bufSize)
{
	CSectionAutoLock lock(mDocAccLock);

	return fz_lookup_metadata(CEdpdfModule::GetUniqueObject()->fzContext, fzDoc, keyName,dataBuf, bufSize);
}

int32 CpdfDocument::GetDocType(void)
{
	return 0;
}

int32 CpdfDocument::GetPageCount(void)
{
	return pageCount;
}

//void CpdfDocument::TestHighLight()
//{
//	fz_cookie cookie = { 0 };
//	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;
//	fz_page *page = fz_load_page(ctx, fzDoc, 0);
//	//fz_rect mediabox;
//	fz_stext_page *text = fz_new_stext_page(ctx, fz_bound_page(ctx, page));
//	fz_device *dev = fz_new_stext_device(ctx,text, NULL);
//	fz_run_page(ctx, page, dev, fz_identity,&cookie);
//	fz_close_device(ctx, dev);
//	fz_drop_device(ctx, dev);
//
//	cmmDeletePtr<fz_quad> quads;
//
//	quads.allocate(100);
//	fz_point a, b;
//
//	a.x = 140.0f;
//	a.y = 100.0f;
//	b.x = 120.0f;
//	b.y = 150.0f;
//
//	//char * xxx = fz_copy_selection(ctx, text, a,b, 0);
//
//	int n = fz_highlight_selection(ctx, text, a, b, quads.ptr(), 100);
//	if (n > 0)
//	{
//		pdf_annot *annot = pdf_create_annot(ctx, pdf_page_from_fz_page(ctx, page), PDF_ANNOT_HIGHLIGHT);
//		for (int i = 0; i < n; ++i)
//		{
//			//char * stringSel = fz_copy_selection(ctx, text, quads()[i].ul, quads()[i].lr, 0);
//			pdf_add_annot_quad_point(ctx, annot, quads()[i]);
//		}
//		pdf_update_annot(ctx, annot);
//	}
//
//	//hit_count = fz_search_stext_page(ctx, text, needle, hit_bbox, nelem(hit_bbox));
//
//	//fz_drop_stext_page(ctx, text);
//	//fz_drop_stext_sheet(ctx, sheet);
//	fz_drop_page(ctx, page);
//
//	SetDirty(true);
//
//	SaveAllChanges();
//}

ED_ERR CpdfDocument::LoadPage(CpdfPage* pageObj)
{
	CSectionAutoLock lock(mDocAccLock);
	fz_device *dev = NULL;
	int errored = 0;
	fz_cookie cookie = { 0 };
	ED_ERR errBack = EDERR_SUCCESS;

	fz_var(dev);

	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	//if (ctx == NULL)
	//	TestHighLight();

	//fz_drop_display_list(ctx, page_list);
	//fz_drop_page(ctx, page);

	pageObj->page_list = NULL;
	pageObj->mPage = NULL;
	pageObj->page_bbox.x0 = 0;
	pageObj->page_bbox.y0 = 0;
	pageObj->page_bbox.x1 = 100;
	pageObj->page_bbox.y1 = 100;

	incomplete = 0;

	fz_try(ctx)
	{
		fz_page* page = fz_load_page(ctx, fzDoc, pageObj->mPageNo);
		if (page == NULL)
			fz_throw(ctx, -1, "fz_load_page failed");

		pageObj->Load(this, page);

		pageObj->page_bbox = fz_bound_page(ctx, pageObj->mPage);

		// * Create display lists * /
		pageObj->page_list = fz_new_display_list(ctx, fz_bound_page(ctx, pageObj->mPage));
		dev = fz_new_list_device(ctx, pageObj->page_list);

		//if (no_cache)
		//	fz_enable_device_hints(ctx, dev, FZ_NO_CACHE);
		cookie.incomplete = 1;
		fz_run_page_contents(ctx, pageObj->mPage, dev, fz_identity, &cookie);

		if (cookie.incomplete)
		{
			//pdfapp_warn(app, "Incomplete page rendering");
			incomplete = 1;
		}
		else if (cookie.errors)
		{
			//pdfapp_warn(app, "Errors found on page");
			errBack = EDERR_FIALED_LOAD_PAGE;
		}
		//fz_close_device(ctx, dev);
	}
	fz_always(ctx)
	{
		if (dev != NULL)
		{
			fz_close_device(ctx, dev);
			fz_drop_device(ctx, dev);
		}
	}
	fz_catch(ctx)
	{
		if (fz_caught(ctx) == FZ_ERROR_TRYLATER)
			incomplete = 1;
		else
		{
			//pdfapp_warn(app, "Cannot load page");
			errBack = EDERR_FIALED_LOAD_PAGE;
		}
	}

	//fz_try(ctx)
	//{
	//	app->page_links = fz_load_links(ctx, app->mPage);
	//}
	//fz_catch(ctx)
	//{
	//	if (fz_caught(ctx) == FZ_ERROR_TRYLATER)
	//		app->incomplete = 1;
	//	else if (!errored)
	//		pdfapp_warn(app, "Cannot load mPage");
	//}
	return errBack;
}

IEdPage_ptr CpdfDocument::GetPage(int32Eink pageIndex)
{
	auto pageObj = CpdfPage::CreateInstance(pageIndex);
	if (pageObj == NULL)
		return NULL;

	if (ERR_FAILED(LoadPage(pageObj)))
	{
		pageObj->Release();
		return NULL;
	}

	return pageObj;
}

IEdPage_ptr CpdfDocument::GetPage(PPAGE_PDF_CONTEXT contextPtr)
{
	return GetPage(contextPtr->pageIndex);
}

IEdPage_ptr CpdfDocument::GetPage(IEdPage_ptr currentPage, int32Eink pagesOff)
{
	auto index = currentPage->GetPageIndex() + pagesOff;

	if (index >= 0 && index < pageCount)
		return GetPage(index);

	return NULL;
}

bool32 CpdfDocument::LoadAllThumbnails(PEDDOC_THUMBNAILS_CALLBACK callBack, void_ptr contextPtr, const wchar_t* nphumbnailPathName)
{
	
	mPageControl.SetDocument(nphumbnailPathName);

	thread td([this,callBack,contextPtr, nphumbnailPathName] {
		fz_context* thumbCtx = CEdpdfModule::GetUniqueObject()->fzContext;
		fz_cookie cookie = { 0 };
		//auto tt = GetTickCount();

		// 判断是否已经开始生成Thumbnails
		{	//进入Fz_Doc互斥区
			CSectionAutoLock lock(mDocAccLock);
			if (mLoadThumb != false)	// 已经启动装载
				return false;

			mLoadThumb = true;

			// Clone一个上下文
			thumbCtx = fz_clone_context(CEdpdfModule::GetUniqueObject()->fzContext);
			if (thumbCtx == NULL)
			{
				mLoadThumb = true;
				return false;
			}
				

		}// 离开互斥区

		for (int i = 0; i < pageCount; i++)
		{
			cmmStringW fileName;
			fz_page* page = NULL;
			fz_pixmap* pix = NULL;
			fz_device* dev = NULL;
			bool done = false;
			bool hasAnnot = false;

			if(mLoadThumb == false)
				break;	// 需要退出，放弃继续加载

			// 查看是否已经存在thunbnails
			if (mPageControl.CheckThumbnailFile(i,hasAnnot) != false)
			{	// 此页已经存在thumbnail

				mPageControl.AddPage(i,true,hasAnnot);

				// 调用回调函数
				if (callBack != NULL)
					callBack((uint32Eink)i, contextPtr);
				continue;	//继续下一页
			}


			{	// 进入Fz_Doc互斥区
				CSectionAutoLock lock(mDocAccLock);

				fz_try(thumbCtx){

					page = fz_load_page(thumbCtx, fzDoc, i);
					if(page == NULL)
						fz_throw(thumbCtx, -1, "fz_load_page -> NULL");

					auto pdfPage = pdf_page_from_fz_page(thumbCtx,page);
					if(pdfPage == NULL)
						fz_throw(thumbCtx, -1, "pdf_page_from_fz_page -> NULL");

					auto annot = pdf_first_annot(thumbCtx,pdfPage);
					if (annot != NULL)
					{ 
						hasAnnot = true;
						//pdf_drop_annot(thumbCtx, annot);	// 不能drop，会在fz_run_page时崩溃
					}
					else
						hasAnnot = false;

					// 获取正确的thumbnail的名字
					mPageControl.GenerateThumbPath(i,hasAnnot, fileName);

					fz_rect bound;
					bound = fz_bound_page(thumbCtx, page);
					int sw = bound.x1 - bound.x0;
					int sh = bound.y1 - bound.y0;
					int dw, dh;

					if (sw > sh)
					{
						dw = 480;
						dh = (dw*sh) / sw;
					}
					else
					{
						dh = 480;
						dw = (dh*sw) / sh;
					}

					fz_matrix ctm = fz_scale((float)dw / sw, (float)dh / sh);

					//pix = fz_new_pixmap_from_page_number(thumbCtx,fzDoc,i, ctm, fz_device_rgb(thumbCtx),1);
					pix = fz_new_pixmap(thumbCtx, CEdpdfModule::GetUniqueObject()->colorSpace, dw, dh, NULL, 1);
					if(pix == NULL)
						fz_throw(thumbCtx, -1, "pix=NULL");

					fz_clear_pixmap_with_value(thumbCtx, pix, 255);

					dev = fz_new_draw_device(thumbCtx, fz_identity, pix);
					if(dev == NULL)
						fz_throw(thumbCtx, -1, "dev=NULL");

					//fz_enable_device_hints(thumbCtx,dev, FZ_DONT_INTERPOLATE_IMAGES| FZ_NO_CACHE); 这里加不加没有区别

					fz_run_page(thumbCtx, page, dev, ctm, &cookie);

					fz_save_pixmap_as_png(thumbCtx, pix, fileName.CopyToMultiByteBuffer(NULL, CP_UTF8));

					done = true;
				}
				fz_always(thumbCtx)
				{
					if (dev != NULL)
					{
						fz_close_device(thumbCtx, dev);
						fz_drop_device(thumbCtx,dev);
					}

					if (pix != NULL)
						fz_drop_pixmap(thumbCtx, pix);

					if (page != NULL)
						fz_drop_page(thumbCtx, page);
				}
				fz_catch(thumbCtx)
				{
					done = false;
				}
			}	// 离开互斥区


			if (done != false)
			{
				// 新的thumb记录
				mPageControl.AddPage(i,true,hasAnnot);

				// 调用回调函数
				if (callBack != NULL)
					callBack((uint32Eink)i, contextPtr);
			}
			
		}

		{	//进入Fz_Doc互斥区
			CSectionAutoLock lock(mDocAccLock);

			if (thumbCtx != NULL)
				fz_drop_context(thumbCtx);
		}// 离开互斥区

		SetEvent(mThumbThreadExit);

		//auto tt2 = GetTickCount();

		//tt2 -= tt;
		// 调用回调函数
		if (callBack != NULL)
			callBack((uint32Eink)MAXULONG32, contextPtr);

		mLoadThumb = false;
		return true;
	});

	HANDLE handle = td.native_handle();
	SetThreadPriority(handle, THREAD_PRIORITY_BELOW_NORMAL);

	td.detach();
	//td.join();

	return true;
}

bool32 CpdfDocument::GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen)
{
	
	bool32 lbRet = false;

	do 
	{
		BREAK_ON_NULL(npszPathBuffer);
		wcscpy_s(npszPathBuffer, niLen, mPageControl.GetCachePath());

		lbRet = true;

	} while (false);

	return lbRet;
}

bool32 CpdfDocument::GetThumbnailPathName(int32Eink pageIndex, char16 pathName[256], bool* hasAnnot)
{
//	CSectionAutoLock lock(mDocAccLock);

	cmmStringW nameStr;
	bool annot;

	if (mPageControl.GetThumbInfor(pageIndex, annot, nameStr) == false)
		return false;

	if (nameStr.size() + 1 > 256)
		return false;

	wcscpy_s(pathName, 256, nameStr.ptr());
	if (hasAnnot != NULL)
		*hasAnnot = annot;

	return true;
}

int32Eink CpdfDocument::GetAnnotPageCount(void)
{
	return mPageControl.GetAnnotPageCount();
}

bool32 CpdfDocument::SaveAllChanges(const char16_ptr pathName )
{
	if (mDirty == false)
		return true;

	bool32 lbRet = false;

	do
	{
		pdf_document* pdfDoc = GetPdfObj();
		if (pdfDoc == NULL)
			break;

		pdf_write_options_s wop = { 0 };

		cmmStringW saveTo;

		if (pathName != NULL)
			saveTo = pathName;
		else
			saveTo = mPathName;

		wop.do_incremental = 1;
		wop.do_compress = 1;
		//wop.do_appearance = 1;
		//wop.do_clean = 1;
		//wop.do_sanitize = 1;
		//wop.do_garbage = 3;
		pdfDoc->repair_attempted = 0;
		int liCan = 1;// pdf_can_be_saved_incrementally(CEdpdfModule::GetUniqueObject()->fzContext, pdfDoc);
					  /*if (liCan == 0)
					  {
					  pdf_repair_xref(CEdpdfModule::GetUniqueObject()->fzContext, pdfDoc);
					  liCan = pdf_can_be_saved_incrementally(CEdpdfModule::GetUniqueObject()->fzContext, pdfDoc);
					  }*/

		fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;
		fz_try(ctx)
		{

			if (liCan == 1) //不等于1调用save会引发异常
			{
				pdf_save_document(CEdpdfModule::GetUniqueObject()->fzContext, pdfDoc, saveTo.CopyToMultiByteBuffer(NULL, CP_UTF8), &wop);

				if (miObjCount > 0)
				{
					//把保存过的对象状态全部置0
					for (int i = miObjCount; i < pdfDoc->xref_sections->num_objects; i++)
					{
						pdfDoc->xref_sections->subsec->table[i].type = 0;
					}
				}

				miObjCount = pdfDoc->xref_sections->num_objects;
			}
		}
		fz_catch(ctx)
		{
			break;
		}

		mDirty = false;

		lbRet = true;

	} while (false);

	return lbRet;
}

pdf_document* CpdfDocument::GetPdfObj(void)
{
	return pdf_specifics(CEdpdfModule::GetUniqueObject()->fzContext, fzDoc);
}

bool CpdfDocument::SetDirty(bool dirty)
{
	auto crt = mDirty;
	mDirty = dirty;

	return crt;
}

IEdPage_ptr CpdfDocument::Rearrange(PPAGE_PDF_CONTEXT contextPtr)
{
	PAGE_PDF_CONTEXT context = { 0,0,0 };

	if (contextPtr == NULL)
		contextPtr = &context;

	// unsupport
	return GetPage(contextPtr);
}

IEdPage_ptr CpdfDocument::Rearrange(IEdPage_ptr currentPage)
{
	currentPage->AddRefer();
	// unsupport
	return currentPage;
}

void CpdfDocument::SetFont(const char16_ptr fontName, int32Eink fontSize)
{
	// unsupport
}

void CpdfDocument::SetViewPort(int32Eink viewWidth, int32Eink viewHeight)
{
	// unsupport
}



/*
main()
{
	std::string pathToOutputBook3 += "\\testPDF.pdf";
	std::string pathToImage3 = "Assets/hortitsa.jpg";

	fz_point pts[4] = { { 0, 0 },{ 0, 0 },{ 600, 0 },{ 600, 499 } };
	fz_rect rect = fz_empty_rect;
	pdf_document *pdfDoc = (pdf_document *)this->mu_doc;
	pdf_page *pdfPage = (pdf_page *)page;
	image_document *imgDoc = (image_document *)fz_open_document(this->mu_ctx, pathToImage3.c_str());
	fz_image *image = imgDoc->image;
	fz_matrix page_ctm = { 1, 0, 0, 1, 0, 0 };

	fz_include_point_in_rect(&rect, &pts[0]);
	fz_include_point_in_rect(&rect, &pts[1]);
	fz_include_point_in_rect(&rect, &pts[2]);
	fz_include_point_in_rect(&rect, &pts[3]);

	display_list = fz_new_display_list(this->mu_ctx);
	dev = fz_new_list_device(this->mu_ctx, display_list);

	auto annot = pdf_create_annot(pdfDoc, pdfPage, fz_annot_type::FZ_ANNOT_SQUARE);

	fz_fill_image(dev, image, &page_ctm, 1.0f);

	fz_transform_rect(&rect, &page_ctm);
	pdf_set_annot_appearance(pdfDoc, annot, &rect, display_list);

	fz_write_document(this->mu_doc, (char *)pathToOutputBook3.c_str(), nullptr);

}
//图片格式为png格式, 如果是图片中含有透明通道则必须添加smask字典
void pdfapp_add_image(pdfapp_t *app, const char * path, fz_point * pt)
{
	fz_image * image = NULL;
	fz_rect rect;
	fz_point p;
	fz_matrix ctm;
	fz_display_list * dlist;
	fz_device * dev;
	pdf_document * doc;
	pdf_page * page;
	pdf_annot * annot;
	fz_irect irect = { 0, 0, app->layout_w, app->layout_h };
	image = fz_new_image_from_file(app->ctx, path);
	p.x = pt->x - app->panx + irect.x0;
	p.y = pt->y - app->pany + irect.y0;

	pdfapp_viewctm(&ctm, app);
	fz_invert_matrix(&ctm, &ctm);
	fz_transform_point(&p, &ctm);

	rect.x0 = p.x;
	rect.x1 = rect.x0 + image->w;
	rect.y0 = p.y;
	rect.y1 = rect.y0 + image->h;

	doc = pdf_specifics(app->ctx, app->doc);
	page = pdf_load_page(app->ctx, doc, app->pageno - 1);
	annot = pdf_create_annot(app->ctx, page, PDF_ANNOT_SCREEN);
	pdf_set_annot_rect(app->ctx, annot, &rect);
	dlist = fz_new_display_list(app->ctx, NULL);
	dev = fz_new_list_device(app->ctx, dlist);
	ctm.a = image->w;
	ctm.b = 0;
	ctm.c = 0;
	ctm.d = image->h;
	ctm.e = p.x;
	ctm.f = p.y

		fz_fill_image(app->ctx, dev, image, &ctm, 1.0, NULL);
	pdf_set_annot_appearance(app->ctx, doc, annot, &rect, dlist);
	fz_drop_display_list(app->ctx, dlist);
	fz_drop_device(app->ctx, dev);
	fz_drop_image(app->ctx, image);
	pdfapp_showpage(app, 1, 1, 0, 0, 0);
}

*/