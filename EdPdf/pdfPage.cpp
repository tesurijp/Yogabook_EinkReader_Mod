/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "pdfPage.h"
#include "pdfDocument.h"
#include "cmmPtr.h"
#include "stextAnnot.h"
#include "mupdf/pdf/annot.h"

//////////////////////////////////////////////////////////////////////////
// Page类
DEFINE_BUILTIN_NAME(CpdfPage)

CpdfPage::CpdfPage()
{
	page_list = NULL;
	mPage = NULL;
}

CpdfPage::~CpdfPage()
{
	if(page_list != NULL)
		fz_drop_display_list(CEdpdfModule::GetUniqueObject()->fzContext, page_list);
	if(mPage != NULL)
		fz_drop_page(CEdpdfModule::GetUniqueObject()->fzContext, mPage);

	for (auto i : mAnnots)
		i->Release();
}


// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfPage::InitOnCreate(int32 pageNo)
{
	mPageNo = pageNo;

	return EDERR_SUCCESS;
}

bool32 CpdfPage::GetMediaBox(
	OUT ED_RECTF_PTR mediaBox
)
{
	mediaBox->left = page_bbox.x0;
	mediaBox->right = page_bbox.x1;
	mediaBox->top = page_bbox.y0;
	mediaBox->bottom = page_bbox.y1;
	return true;//GetBox(PDF_NAME_MediaBox,*mediaBox);
}

bool32 CpdfPage::GetCropBox(
	OUT ED_RECTF_PTR cropBox
)
{
	return false;//GetBox(PDF_NAME_CropBox, *cropBox);
}

bool32 CpdfPage::GetBleedBox(
	OUT ED_RECTF_PTR bleedBox
)
{
	return false;//GetBox(PDF_NAME_BleedBox,*bleedBox);
}

IEdBitmap_ptr CpdfPage::Render(
	IN float32 scalRatio,
	IN bool32 cleanUp
)
{
	fz_pixmap* fzImage = DrawBitmap(scalRatio, cleanUp);
	if (fzImage == NULL)
		return NULL;

	return CpdfBitmap::CreateInstance(fzImage);
}

int32 CpdfPage::GetPageIndex(void)
{
	return mPageNo;
}


IEdAnnotManager_ptr CpdfPage::GetAnnotManager(void)
{
	return dynamic_cast<IEdAnnotManager*>(this);
}

IEdAnnot_ptr CpdfPage::LoadAnnotFromArchive(buf_ptr buf, uint32 bufSize)
{
	uint32* head = (uint32*)buf;

	switch (head[0])
	{
	case 'inka':	// ink
	{
		cmmReleasePtr<CpdfInkAnnot> annotObj;

		annotObj = CpdfInkAnnot::CreateInstance();
		if (annotObj == NULL)
			break;

		if (annotObj->LoadFromAchive(GetPdfObj(), buf, bufSize) == false)
			break;

		mDocObj->SetDirty(true);
		mAnnots.push_back(annotObj.ptr());

		return annotObj.notfree();
	}
	break;
	case 'stxt': // structured text annotation
	{
		ED_POINTF a, b;
		ULONG type;
		ED_COLOR clr;
		int32 clrN;
		if (CpdfStextAnnot::UnpackFromAchive(buf, bufSize,a,b,type,clr,clrN) == false)
			break;

		cmmReleasePtr<IEdStructuredTextPage> stextPage;
		stextPage = GetStructuredTextPage();
		if(stextPage == NULL)
			break;

		cmmReleasePtr<IEdStextQuadList> selQuads;
		cmmReleasePtr<IEdAnnotList> annots;

		auto rev = stextPage->DetectSelectedText(&a, &b, selQuads.addressOfPtr(), annots.addressOfPtr(), true);
		if (rev == false)
			break;

		IEdAnnot_ptr annot = NULL;
		switch (type)
		{
		case EDPDF_ANNOT_UNDERL:
			annot = AddUnderLineAnnot(selQuads.ptr(),&clr,clrN);
			break;
		case EDPDF_ANNOT_DELETE:
			annot = AddDeleteLineAnnot(selQuads.ptr(), &clr, clrN);
			break;
		case EDPDF_ANNOT_HIGHLIGHT:
			annot = AddHighLightAnnot(selQuads.ptr(), &clr, clrN);
			break;
		default:
			break;
		}

		return annot;
	}
	break;
	default:
		break;
	}

	return NULL;
}

IEdAnnot_ptr CpdfPage::AddTextAnnot(IN ED_POINT position, IN char16_ptr text)
{
	return NULL;
}

IEdAnnot_ptr CpdfPage::AddInkAnnot(
	IN ED_POINTF* stroke,
	IN int32 pointCount,
	IN ED_COLOR* color,
	IN float32 border
	)
{
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	pdf_page* pdfPage = GetPdfObj();
	if (pdfPage == NULL)
		return NULL;

	cmmReleasePtr<CpdfInkAnnot> annotObj;

	annotObj = CpdfInkAnnot::CreateInstance();
	if (annotObj == NULL)
		return NULL;

	CpdfInkMultiList multiList;
	CpdfInkStrokeList listOne;
	CpdfInkStroke strokeOne;

	for (int32 i = 0; i < pointCount; i++)
	{
		strokeOne.x = (float)stroke[i].x;
		strokeOne.y = (float)stroke[i].y;

		listOne.Insert(-1, strokeOne);
	}


	CpdfSmoothMethod::SmoothStroke(listOne, 3, border);
	//CpdfSmoothMethod::SmoothStrokeNew(listOne); //niu 调用新算法试试看

	if (listOne.Size() <= 0)
		return NULL;

	multiList.Insert(-1, &listOne);

	float clr[4] = { 0, 0,0 };
	if (color != NULL)
	{
		clr[0] = (float)color->r / 255.0f;
		clr[1] = (float)color->g / 255.0f;
		clr[2] = (float)color->b / 255.0f;
	}

	if(annotObj->PdfCreateAnnot(pdfPage, multiList,clr,3, border) == false)
		return NULL;

	mDocObj->SetDirty(true);
	mAnnots.push_back(annotObj.ptr());

	return annotObj.notfree();
}

IEdAnnot_ptr CpdfPage::AddStextAnnot(IN IEdStextQuadList_ptr stext, IN enum pdf_annot_type type,IN ED_COLOR* color, IN int32Eink clrN /*= 3 */)
{
	cmmReleasePtr<CpdfStextAnnot> annotObj = CpdfStextAnnot::CreateInstance();
	if (annotObj == NULL)
		return NULL;

	float clr[4] = { 0, 0,0,1.0f};
	if (color != NULL)
	{
		clr[0] = (float)color->r / 255.0f;
		clr[1] = (float)color->g / 255.0f;
		clr[2] = (float)color->b / 255.0f;
	}
	if (clrN >= 4)
		clr[3] = (float)color->a / 255.0f;

	if (annotObj->PdfCreateAnnot(GetPdfObj(),stext,type,clr,clrN,1) == false)
		return NULL;

	mDocObj->SetDirty(true);
	mAnnots.push_back(annotObj.ptr());

	return annotObj.notfree();
}

IEdAnnot_ptr CpdfPage::AddHighLightAnnot(IN IEdStextQuadList_ptr stext, IN ED_COLOR* color /*= NULL*/, IN int32Eink clrN /*= 3 */)
{
	//ED_COLOR defClr = {0,0,255,50};	// HighLight类型必须是半透明叠加的
	ED_COLOR defClr = { 255,229,0,255 };	// HighLight类型必须是半透明叠加的
	if (color == NULL)
	{
		color = &defClr;
		clrN = 3;
	}

	return AddStextAnnot(stext, PDF_ANNOT_HIGHLIGHT,color,clrN);
}

IEdAnnot_ptr CpdfPage::AddDeleteLineAnnot(IN IEdStextQuadList_ptr stext, IN ED_COLOR* color /*= NULL*/, IN int32Eink clrN /*= 3 */)
{
	return AddStextAnnot(stext, PDF_ANNOT_STRIKE_OUT, color, clrN);
}

IEdAnnot_ptr CpdfPage::AddUnderLineAnnot(IN IEdStextQuadList_ptr stext, IN ED_COLOR* color /*= NULL*/, IN int32Eink clrN /*= 3 */)
{
	return AddStextAnnot(stext, PDF_ANNOT_UNDERLINE, color, clrN);
}

// 获取所有对象的列表，当annotList==NULL时，返回需要的List单元数,当缓冲区不足时返回-1；成功返回获得的IEdAnnot_ptr对象数
int32 CpdfPage::GetAllAnnot(IEdAnnot_ptr* annotList, int bufSize)
{
	int32 liRet = 0;

	if (annotList == NULL)
	{
		liRet = (int32)mAnnots.size();
	}
	else
	{
		for (auto j : mAnnots)
		{
			j->AddRefer();
			annotList[liRet++] = j;

			if(liRet >= bufSize)
				break;
		}
	}

	return liRet;
}

// 检测一笔画接触到的一系列Ink笔记对象（相交），当annotList==NULL时，返回需要的缓冲区长度,当缓冲区不足时返回-1
int32 CpdfPage::DetectInkAnnot(
	IN ED_POINTF* stroke,
	IN int32Eink pointCount,
	OUT	IEdAnnot_ptr* annotList,	// 用于返回所有相交对象的缓冲区，建议一次性分配大的缓冲区，比如 IEdAnnot_ptr buf[256];
	IN int32Eink bufSize					// 上述缓冲区的单元数，不是字节数
)
{
	int32 hitCount = 0;
	vector<IEdAnnot_ptr> inkAnnotList;

	for (auto i:mAnnots)
	{
		IEdInkAnnot_ptr inkAnnot = i->ConvertToInkAnnot();
		if (inkAnnot == NULL)
			continue;

		CpdfInkAnnot* obj = dynamic_cast<CpdfInkAnnot*>(inkAnnot);
		if (obj == NULL)
			continue;

		if(obj->DetectIntersection(stroke,pointCount)<0)
			continue;

		inkAnnot->AddRefer();
		inkAnnotList.push_back(inkAnnot);
	}

	if (annotList == NULL)
		return (int32)inkAnnotList.size();

	if (inkAnnotList.size() > bufSize)
		return -1;

	for (auto j : inkAnnotList)
	{
		annotList[hitCount++] = j;
	}

	return hitCount;
}

bool32 CpdfPage::GetSelectedText(IN ED_RECTF_PTR selBox, OUT char16_ptr textBuf, IN int32Eink bufSize)
{
	fz_cookie cookie = { 0 };
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	fz_stext_page *text = fz_new_stext_page(ctx, fz_bound_page(ctx,mPage));
	fz_device *dev = fz_new_stext_device(ctx, text, NULL);
	fz_run_page(ctx, mPage, dev, fz_identity, &cookie);
	fz_close_device(ctx, dev);
	fz_drop_device(ctx, dev);

	fz_point a, b;

	a.x = selBox->left;
	a.y = selBox->top;
	b.x = selBox->right;
	b.y = selBox->bottom;

	char * stringSel = fz_copy_selection(ctx, text, a, b, 0);
	bool rev = (stringSel != NULL);

	cmmStringW ss;

	ss = stringSel;

	wcscpy_s(textBuf, bufSize, ss.ptr());

	fz_drop_stext_page(ctx,text);

	return rev;
}

IEdStructuredTextPage_ptr CpdfPage::GetStructuredTextPage(void)
{
	CpdfStextPage* spage = CpdfStextPage::CreateInstance(this);

	return spage;
}

IEdAnnot_ptr CpdfPage::GetAnnotBySignature(ULONGLONG signature)
{
	UCHAR type;
	USHORT altData = 0;
	ULONG checkSum = 0;

	if (mPageNo != ANN_SIGN_PAGE(signature))
		return NULL;

	for (auto i : mAnnots)
	{
		type = (UCHAR)i->GetType();

		if(type != ANN_SIGN_TYPE(signature))
			continue;

		switch (type)
		{
		case EDPDF_ANNOT_INK:
		{
			IEdInkAnnot_ptr inkAnnot = i->ConvertToInkAnnot();
			if (inkAnnot == NULL)
				break;

			CpdfInkAnnot* obj = dynamic_cast<CpdfInkAnnot*>(inkAnnot);
			if (obj == NULL)
				break;

			obj->GetSignature(checkSum, altData);
		}
		case EDPDF_ANNOT_HIGHLIGHT:
		case EDPDF_ANNOT_DELETE:
		case EDPDF_ANNOT_UNDERL:
		{
			IEdStextAnnot_ptr lStextAnnot = i->ConvertToStextAnnot();
			if (lStextAnnot == NULL)
				break;

			CpdfStextAnnot* obj = dynamic_cast<CpdfStextAnnot*>(lStextAnnot);
			if (obj == NULL)
				break;

			obj->GetSignature(checkSum, altData);
		}
		break;
		default:
			//later: 补充其他类别
			altData = 0;
			checkSum = 0;
			break;
		}

		if(altData != ANN_SIGN_ALT(signature))
			continue;

		if(checkSum != ANN_SIGN_CHECKSUM(signature))
			continue;

		i->AddRefer();
		return i;
	}

	return NULL;
}

ULONGLONG CpdfPage::GetSignature(IEdAnnot_ptr annot)
{
	UCHAR type;
	USHORT pageNum = 0;
	USHORT altData = 0;
	ULONG checkSum = 0;

	for (auto i : mAnnots)
	{
		if (i == annot)
		{
			pageNum = (USHORT)mPageNo;
			type = (UCHAR)i->GetType();

			switch (type)
			{
			case EDPDF_ANNOT_INK:
			{
				IEdInkAnnot_ptr inkAnnot = i->ConvertToInkAnnot();
				if (inkAnnot == NULL)
					break;

				CpdfInkAnnot* obj = dynamic_cast<CpdfInkAnnot*>(inkAnnot);
				if (obj == NULL)
					break;

				obj->GetSignature(checkSum, altData);
			}
			break;
			case EDPDF_ANNOT_HIGHLIGHT:
			case EDPDF_ANNOT_DELETE:
			case EDPDF_ANNOT_UNDERL:
			{
				IEdStextAnnot_ptr liHighlightAnnot = i->ConvertToStextAnnot();
				if (liHighlightAnnot == NULL)
					break;

				CpdfStextAnnot* obj = dynamic_cast<CpdfStextAnnot*>(liHighlightAnnot);
				if (obj == NULL)
					break;

				obj->GetSignature(checkSum, altData);
			}
			break;
			default:
				//later: 补充其他类别
				altData = 0;
				checkSum = 0;
				break;
			}

			ULONGLONG rev = ANN_MAKE(type, pageNum,altData, checkSum);
			return rev;
		}
	}

	return 0;
}

IEdAnnot_ptr CpdfPage::GetFirstAnnot(void)
{
	if (mAnnots.size() == 0)
		return NULL;
	auto rev = mAnnots.front();
	rev->AddRefer();

	return rev;
}

IEdAnnot_ptr CpdfPage::GetNextAnnot(IEdAnnot_ptr crt)
{
	for (int i = 0; i < (mAnnots.size()-1); i++)
	{
		if (mAnnots[i] == crt)
		{
			mAnnots[i + 1]->AddRefer();

			return mAnnots[i + 1];
		}
	}
	return NULL;
}

void CpdfPage::RemoveAnnot(IEdAnnot_ptr annot)
{
	for (auto i=mAnnots.begin();i!=mAnnots.end();i++)
	{
		pdf_annot* rawObj = NULL;

		if(*i != annot)
			continue;

		switch ((*i)->GetType())
		{
		case EDPDF_ANNOT_INK:
		{
			IEdInkAnnot_ptr inkAnnot = (*i)->ConvertToInkAnnot();
			if (inkAnnot == NULL)
				break;

			CpdfInkAnnot* obj = dynamic_cast<CpdfInkAnnot*>(inkAnnot);
			if (obj == NULL)
				break;

			rawObj = obj->GetPdfObj();

			pdf_delete_annot(CEdpdfModule::GetUniqueObject()->fzContext, GetPdfObj(), rawObj);

			mAnnots.erase(i);

			inkAnnot->Release();

			mDocObj->SetDirty(true);

			return;
		}
		case EDPDF_ANNOT_HIGHLIGHT:
		case EDPDF_ANNOT_DELETE:
		case EDPDF_ANNOT_UNDERL:
		{
			IEdStextAnnot_ptr lStextAnnot = (*i)->ConvertToStextAnnot();
			if (lStextAnnot == NULL)
				break;

			CpdfStextAnnot* obj = dynamic_cast<CpdfStextAnnot*>(lStextAnnot);
			if (obj == NULL)
				break;

			rawObj = obj->GetPdfObj();

			pdf_delete_annot(CEdpdfModule::GetUniqueObject()->fzContext, GetPdfObj(), rawObj);

			mAnnots.erase(i);

			lStextAnnot->Release();

			mDocObj->SetDirty(true);

			return;
		}
		break;
		default:
			break;
		}
	}
}

pdf_page* CpdfPage::GetPdfObj(void)
{
	return pdf_page_from_fz_page(CEdpdfModule::GetUniqueObject()->fzContext, mPage);
}

bool32 CpdfPage::Load(CpdfDocument* doc, fz_page *page)
{
	mDocObj = doc;
	mPage = page;

	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	// 读取注释
	pdf_page* pdfPage = GetPdfObj();
	if (pdfPage == NULL)
		return false;

	enum pdf_annot_type unknownIs = PDF_ANNOT_UNKNOWN;
	pdf_annot* annot = pdf_first_annot(ctx, pdfPage);
	while (annot != NULL)
	{
		switch (pdf_annot_type(ctx, annot))
		{
		case PDF_ANNOT_INK:
		{
			cmmReleasePtr<CpdfInkAnnot> annotObj;
			annotObj = CpdfInkAnnot::CreateInstance();
			if (annotObj == NULL)
				return false;

			if (annotObj->Load(annot) == false)
				return false;

			mAnnots.push_back(annotObj.ptr());
			annotObj.notfree();
		}
			break;
		case PDF_ANNOT_UNDERLINE://later:下划线类型
		case PDF_ANNOT_HIGHLIGHT://later:高亮类型
		case PDF_ANNOT_STRIKE_OUT://later:删除线
		{
			cmmReleasePtr<CpdfStextAnnot> annotObj;
			annotObj = CpdfStextAnnot::CreateInstance();
			if (annotObj == NULL)
				return false;

			if (annotObj->Load(annot) == false)
				return false;

			mAnnots.push_back(annotObj.ptr());
			annotObj.notfree();
		}
			break;
		default:
			unknownIs = pdf_annot_type(ctx, annot);	// ???
			break;
		}

		annot = pdf_next_annot(ctx, annot);
	}

	return true;
}

fz_pixmap* CpdfPage::DrawBitmap(float32 scalRatio,bool32 cleanUp)
{
	fz_pixmap* fzImage = NULL;
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;
	fz_colorspace *colorspace;
	fz_device *idev = NULL;
	fz_matrix ctm;
	fz_rect bounds;
	fz_irect ibounds;
	fz_cookie cookie = { 0 };

//	fz_pre_rotate(fz_scale(mat, app->resolution*2.0f / 72.0f, app->resolution / 72.0f), app->rotate);
	ctm = fz_scale(scalRatio, scalRatio);

	bounds = page_bbox;
	ibounds = fz_round_rect(fz_transform_rect(bounds, ctm));
	bounds= fz_rect_from_irect(ibounds);

	/* Draw */
	//if (app->grayscale)
	//	colorspace = fz_device_gray(ctx);
	//else
	colorspace = CEdpdfModule::GetUniqueObject()->colorSpace;

	fz_var(fzImage);
	fz_var(idev);

	fz_try(ctx)
	{
		fzImage = fz_new_pixmap_with_bbox(ctx, colorspace, ibounds, NULL, 1);
		if(cleanUp!=NULL)
			fz_clear_pixmap_with_value(ctx, fzImage, 255);
		if (page_list /*|| app->annotations_list*/)
		{
			idev = fz_new_draw_device(ctx, fz_identity,fzImage);
			//pdfapp_runpage(app, idev, &ctm, &bounds, &cookie);
			fz_run_display_list(ctx, page_list, idev,ctm, bounds,&cookie);

			//more:渲染annot
			for (auto i : mAnnots)
			{
				pdf_annot* annot = NULL;
				switch (i->GetType())
				{
				case EDPDF_ANNOT_INK:
				{
					IEdInkAnnot_ptr inkAnnot = i->ConvertToInkAnnot();
					if(inkAnnot == NULL)
						break;

					CpdfInkAnnot* obj = dynamic_cast<CpdfInkAnnot*>(inkAnnot);
					if(obj == NULL)
						break;

					annot = obj->GetPdfObj();
				}
					break;
				case EDPDF_ANNOT_UNDERL:
				case EDPDF_ANNOT_DELETE:
				case EDPDF_ANNOT_HIGHLIGHT:
				{
					IEdStextAnnot_ptr stextAnnot = i->ConvertToStextAnnot();
					if(stextAnnot == NULL)
						break;

					CpdfStextAnnot* obj = dynamic_cast<CpdfStextAnnot*>(stextAnnot);
					if(obj == NULL)
						break;

					annot = obj->GetPdfObj();
				}
					break;
				default:
					break;
				}

				if(annot != NULL)
					pdf_run_annot(ctx, annot, idev, ctm, &cookie);
			}

			fz_close_device(ctx, idev);
		}
		//if (app->invert)
		//	fz_invert_pixmap(ctx, app->image);
		//if (app->tint)
		//	fz_tint_pixmap(ctx, app->image, app->tint_r, app->tint_g, app->tint_b);
	}
	fz_always(ctx)
		fz_drop_device(ctx, idev);
	fz_catch(ctx)
		cookie.errors++;

	return fzImage;
}

bool32 CpdfPage::GetPageContext(PPAGE_PDF_CONTEXT contextPtr)
{
	contextPtr->pageIndex = mPageNo;
	contextPtr->pageContext = contextPtr->pageContext2 = 0;

	return true;
}