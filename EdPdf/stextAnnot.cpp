/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "stextannot.h"
#include "pdfPage.h"
#include "pdfDocument.h"
#include "cmmPtr.h"
#include<math.h>

//////////////////////////////////////////////////////////////////////////
// Annot类
DEFINE_BUILTIN_NAME(CpdfStextAnnot)



CpdfStextAnnot::CpdfStextAnnot()
{
	mColorN = 3;
	mBorder = 1;
}

CpdfStextAnnot::~CpdfStextAnnot()
{
}


ULONG CpdfStextAnnot::InitOnCreate(void)
{
	return EDERR_SUCCESS;
}

bool CpdfStextAnnot::Load(pdf_annot* annot)
{
	fz_context* ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	switch (pdf_annot_type(ctx, annot))
	{
	case PDF_ANNOT_UNDERLINE:
		//later:增加下划线类型
		mAnnotType = EDPDF_ANNOT_UNDERL;
		break;
	case PDF_ANNOT_HIGHLIGHT:
		//later:增加高亮类型
		mAnnotType = EDPDF_ANNOT_HIGHLIGHT;
		break;
	case PDF_ANNOT_STRIKE_OUT:
		//later:增加删除线
		mAnnotType = EDPDF_ANNOT_DELETE;
		break;
	default:
		return false;
	}

	mPdfRawObj = annot;

	mQuadListObj = CpdfStextQuadsList::CreateInstance();
	if (mQuadListObj == NULL)
		return false;

	auto count = pdf_annot_quad_point_count(ctx, annot);
	float packedQuad[8];
	ED_RECTF quadRect;

	for (int i = 0; i < count; i++)
	{
		pdf_annot_quad_point(ctx, annot, i, packedQuad);
		quadRect.left = packedQuad[0];
		quadRect.top = packedQuad[1];
		quadRect.right = packedQuad[6];
		quadRect.bottom = packedQuad[7];

		mQuadListObj->AddQuad(quadRect);
	}

	pdf_annot_color(ctx, annot, &mColorN, mColor);
	mBorder = pdf_annot_border(ctx, annot);

	return true;
}

bool CpdfStextAnnot::UnpackFromAchive(char* archive, int32 bufSize, OUT ED_POINTF& a, OUT ED_POINTF& b, OUT uint32& type, OUT ED_COLOR& clr, OUT int32Eink& clrN)
{
	uint32* head = (uint32*)archive;

	// 'stxt',size in uint32,list count, color32,color N, A Point,B Point,type,'stxt'
	if (head[0] != 'stxt' || head[1] > bufSize / sizeof(uint32) || head[head[1] - 1] != 'stxt')
		return false;

	float border;
	uint32 clrCmpz = head[3];

	clr.r = (clrCmpz & 0xFF);
	clrCmpz >>= 8;
	clr.g = (clrCmpz & 0xFF);
	clrCmpz >>= 8;
	clr.b = (clrCmpz & 0xFF);
	clrCmpz >>= 8;
	clr.a = (clrCmpz & 0xFF);

	clrN = (int32)head[4];
	border = (float)head[5];

	if (clrN < 4)
		clr.a = 255;

	uint32* nextVal = &head[6];

	a.x = *(float32*)nextVal++;
	a.y = *(float32*)nextVal++;
	b.x = *(float32*)nextVal++;
	b.y = *(float32*)nextVal++;
	type = *nextVal++;

	return true;
}

void CpdfStextAnnot::GetSignature(ULONG& checksum, USHORT& altData)
{
	int32 totalQuads = mQuadListObj->GetQuadCount();
	uint32 sum = 0;
	ED_RECTF quad;

	for (int i = 0; i < totalQuads; i++)
	{
		mQuadListObj->GetQuadBound(i, &quad);
		sum += static_cast<LONG>(floor(quad.left + quad.top + 0.49999f));
	}

	altData = (USHORT)totalQuads;
	checksum = (ULONG)sum;
	checksum &= 0xFFFFFF;
}

//int32 CpdfStextAnnot::DetectIntersection(CpdfStextQuadsList& checkWith)
//{
//	return -1;
//}

uint32 CpdfStextAnnot::GetType()
{
	return mAnnotType;
}

char* CpdfStextAnnot::GetTypeName()
{
	char* rev;
	switch (mAnnotType)
	{
	case EDPDF_ANNOT_UNDERL:
		rev = "underline";
	case EDPDF_ANNOT_DELETE:
		rev = "underline";
	case EDPDF_ANNOT_HIGHLIGHT:
		rev = "underline";
		break;
	default:
		rev = "hello";
		break;
	}

	return rev;
}

// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)
// 当buf==NULL时，返回需要的缓冲区字节数，缓冲区不足返回-1
uint32 CpdfStextAnnot::SaveToAchive(buf_ptr buf, uint32 bufSize)
{
	uint32 bufRequired = sizeof(uint32)*8 + sizeof(float32)*4;	// 'stxt',size in uint32,quads count, color32,color N,border, A Point,B Point,type,'stxt'

	if (buf == NULL)
		return bufRequired;

	if (bufSize < bufRequired)
		return -1;

	uint32* head = (uint32*)buf;
	uint32* end = head + bufSize / sizeof(uint32);

	head[0] = 'stxt';
	head[1] = 0;
	head[2] = (uint32)mQuadListObj->GetQuadCount();
	head[3] = (unsigned char)(255.0f*mColor[3]);
	head[3] <<= 8;
	head[3] |= (unsigned char)(255.0f*mColor[2]);
	head[3] <<= 8;
	head[3] |= (unsigned char)(255.0f*mColor[1]);
	head[3] <<= 8;
	head[3] |= (unsigned char)(255.0f*mColor[0]);
	head[4] = (uint32)mColorN;
	head[5] = (uint32)mBorder;

	uint32* nextVal = &head[6];

	ED_POINTF a, b;
	mQuadListObj->GetAPoint(&a);
	mQuadListObj->GetBPoint(&b);

	*(float32*)nextVal++ = a.x;
	*(float32*)nextVal++ = a.y;
	*(float32*)nextVal++ = b.x;
	*(float32*)nextVal++ = b.y;

	*nextVal++ = mAnnotType;
	*nextVal++ = 'stxt';
	head[1] = (uint32)(nextVal - head);

	return head[1]*sizeof(uint32);
}

void CpdfStextAnnot::SetColor(ED_COLOR clr)
{

}

ED_COLOR CpdfStextAnnot::GetColor(void)
{
	ED_COLOR clr = { 0,0,0,0 };

	return clr;
}

void CpdfStextAnnot::SetBorder(int32 border)
{

}

int32 CpdfStextAnnot::GetBorder(void)
{
	return 0;
}

CpdfStextQuadsList* CpdfStextAnnot::GetQuadsList(void)
{
	return mQuadListObj.ptr();
}

bool CpdfStextAnnot::PdfCreateAnnot(IN pdf_page* pageObj, IN IEdStextQuadList_ptr stext, IN enum pdf_annot_type type, IN float* clr, IN int clrN, IN float32 border)
{
	if (stext->GetQuadCount() <= 0)
		return false;

	mColor[0] = clr[0];
	mColor[1] = clr[1];
	mColor[2] = clr[2];
	mColor[3] = clr[3];

	mColorN = clrN;
	mBorder = border;

	fz_context *ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	pdf_annot *annot = pdf_create_annot(ctx, pageObj, type);

	fz_quad quadPdf;
	ED_RECTF quadRect;
	ZeroMemory(&quadPdf, sizeof(quadPdf));
	ZeroMemory(&quadRect, sizeof(quadRect));

	if (clr != NULL)
	{
		pdf_set_annot_color(ctx, annot, clrN, clr);
	}

	for (int i = 0; i < stext->GetQuadCount(); ++i)
	{
		stext->GetQuadBound(i, &quadRect);

		if (quadPdf.ul.x == 0 && quadPdf.ur.x == 0)
		{
			//第一个区域
			quadPdf.ul.x = quadRect.left;
			quadPdf.ul.y = quadRect.top;
			quadPdf.ur.x = quadRect.right;
			quadPdf.ur.y = quadRect.top;
			quadPdf.ll.x = quadRect.left;
			quadPdf.ll.y = quadRect.bottom;
			quadPdf.lr.x = quadRect.right;
			quadPdf.lr.y = quadRect.bottom;
		}
		if (quadRect.left - quadPdf.ur.x < 20.0f && fabs(quadRect.top - quadPdf.ur.y) < 5.0f)
		{
			//可以拼合
			quadPdf.ur.x = quadRect.right;
			//quadPdf.ur.y = quadRect.top;
			quadPdf.lr.x = quadRect.right;
			//quadPdf.lr.y = quadRect.bottom;

		}
		else
		{
			pdf_add_annot_quad_point(ctx, annot, quadPdf);
			ZeroMemory(&quadPdf, sizeof(quadPdf));

			quadPdf.ul.x = quadRect.left;
			quadPdf.ul.y = quadRect.top;
			quadPdf.ur.x = quadRect.right;
			quadPdf.ur.y = quadRect.top;
			quadPdf.ll.x = quadRect.left;
			quadPdf.ll.y = quadRect.bottom;
			quadPdf.lr.x = quadRect.right;
			quadPdf.lr.y = quadRect.bottom;
		}

		if(i < stext->GetQuadCount()-1)
			continue;

		pdf_add_annot_quad_point(ctx, annot, quadPdf);
		ZeroMemory(&quadPdf, sizeof(quadPdf));
	}
	pdf_update_annot(ctx, annot);

	if (Load(annot) == false)
		return false;

	return true;
}


