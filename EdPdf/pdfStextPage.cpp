/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "pdfstextPage.h"
#include "pdfDocument.h"
#include "cmmPtr.h"
#include "pdfPage.h"
#include "pdfStextQuadList.h"
#include "pdfAnnot.h"
#include "stextAnnot.h"

//////////////////////////////////////////////////////////////////////////
// Structured Text Page类
DEFINE_BUILTIN_NAME(CpdfStextPage)

CpdfStextPage::CpdfStextPage()
{
	mStextPage = NULL;
	//mFzPage = NULL;
	mNotStextPage = NULL;
}

CpdfStextPage::~CpdfStextPage()
{
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	if (mStextPage != NULL)
		fz_drop_stext_page(ctx,mStextPage);

	if (mNotStextPage != NULL)
		mNotStextPage->Release();
}


// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfStextPage::InitOnCreate(CpdfPage* page)
{
	mNotStextPage = page;
	mNotStextPage->AddRefer();

	fz_cookie cookie = { 0 };
	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	mStextPage = fz_new_stext_page(ctx, fz_bound_page(ctx, mNotStextPage->GetFzObj()));
	if (mStextPage == NULL)
		return EDERR_3RDPART_FAILED;

	fz_device* device = fz_new_stext_device(ctx, mStextPage, NULL);
	if (device == NULL)
		return EDERR_3RDPART_FAILED;

	fz_run_page(ctx, mNotStextPage->GetFzObj(), device, fz_identity, &cookie);

	fz_close_device(ctx, device);
	fz_drop_device(ctx, device);

	return EDERR_SUCCESS;
}

bool CpdfStextPage::DetectSelectedText(IN const ED_POINTF* aPoint, IN const ED_POINTF* bPoint, OUT IEdStextQuadList_ptr* stext, /* 返回结构化文本对象，需要释放 */ OUT IEdAnnotList_ptr* impactedAnnot, /* 返回碰撞的已有Annot对象列表，需要释放 */IN bool32 includeImpacted /*将碰撞的annot区域也加入选区*/)
{
	fz_point a, b;

	a.x = aPoint->x;
	a.y = aPoint->y;
	b.x = bPoint->x;
	b.y = bPoint->y;

	//float zoom = 72.0 / 200;
	//mtx = fz_scale(a.x,a.y);
	//a = fz_transform_point(a, mtx);
	//b = fz_transform_point(b, mtx);
	//a.x = 140.0f;
	//a.y = 100.0f;
	//b.x = 120.0f;
	//b.y = 150.0f;

	cmmDeletePtr<fz_quad> quads;
	quads.allocate(1000);

	int n = fz_highlight_selection(CEdpdfModule::GetUniqueObject()->fzContext, mStextPage, a, b, quads.ptr(), 1000);
	if (n <= 0)
		return false;

	cmmReleasePtr<CpdfStextQuadsList> quadList = CpdfStextQuadsList::CreateInstance();
	if (quadList == NULL)
		return false;

	cmmReleasePtr<CpdfdAnnotList> impactList = CpdfdAnnotList::CreateInstance();
	if (quadList == NULL)
		return false;

	for (int i = 0; i < n; i++)
	{
		quadList->AddQuad(quads()[i]);
	}


	// 取page对象的全部annot，探测是否有碰撞
	auto annots = mNotStextPage->GetAnnotList();
	vector<IEdStextAnnot_ptr> impacked;
	if (annots.size() > 0)
	{
		for (auto i : annots)
		{
			CpdfStextQuadsList* quadlistFromPage = NULL;
			switch (i->GetType())
			{
			case EDPDF_ANNOT_UNDERL:
			case EDPDF_ANNOT_DELETE:
			case EDPDF_ANNOT_HIGHLIGHT:
			{
				IEdStextAnnot_ptr stextAnnot = i->ConvertToStextAnnot();
				if (stextAnnot == NULL)
					break;

				CpdfStextAnnot* obj = dynamic_cast<CpdfStextAnnot*>(stextAnnot);
				if (obj == NULL)
					break;

				quadlistFromPage = obj->GetQuadsList();

				if (quadlistFromPage != NULL && quadList->DetectIntersection(quadlistFromPage) >= 0)
				{
					// 发现碰撞，保存这个对象
					impactList->AddAnnot(i);
					i->AddRefer();
					impacked.push_back(obj);
				}
			}
			break;
			case EDPDF_ANNOT_INK:
			default:
				break;
			}

		}
	}

	// 将碰撞的区域纳入选择区
	if (includeImpacted != false && impacked.size() > 0)
	{
		for (auto i : impacked)
		{
			if (quadList->Combination(i->GetQuadsList()) == false)
				return false;
		}

		// 因为存在选取重叠，很多的quad会出现相互重叠，并且需要将一些quad做拼接，此处最简单的方法是直接调用本函数一次，让底层重新计算选取，未来不再将碰撞加入选取，因为碰撞区已经被第二次的选取包含
		ED_POINTF ptA, ptB;
		quadList->GetAPoint(&ptA);
		quadList->GetBPoint(&ptB);

		return DetectSelectedText(&ptA, &ptB, stext, impactedAnnot, false);
	}

	*stext = quadList.notfree();
	
	if(impactedAnnot != NULL)
		*impactedAnnot = impactList.notfree();

	return true;
}

int32Eink CpdfStextPage::CopyText(IN ED_RECTF_PTR selBox, OUT char16_ptr textBuf, /* 传入NULL，函数返回实际的字符数，不含结尾0 */ IN int32Eink bufSize)
{
	fz_point a, b;

	a.x = selBox->left;
	a.y = selBox->top;
	b.x = selBox->right;
	b.y = selBox->bottom;

	if (textBuf != NULL)
		*textBuf = UNICODE_NULL;

	char * stringSel = fz_copy_selection(CEdpdfModule::GetUniqueObject()->fzContext, mStextPage, a, b, 0);
	if (stringSel == NULL)
		return 0;

	cmmStringW ss;

	ss = stringSel;

	if (textBuf != NULL)
	{
		wcscpy_s(textBuf, bufSize, ss.ptr());
	}

	return ss.size();
}


