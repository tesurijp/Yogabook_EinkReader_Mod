/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "hlAnnot.h"
#include "pdfPage.h"
#include "pdfDocument.h"
#include "cmmPtr.h"
#include<math.h>
#include "FloatH.h"

//////////////////////////////////////////////////////////////////////////
// Annot类
DEFINE_BUILTIN_NAME(CpdfHlAnnot)



CpdfHlAnnot::CpdfHlAnnot()
{
}

CpdfHlAnnot::~CpdfHlAnnot()
{
	//if (mfzImage != NULL)
	//	fz_drop_pixmap(CEdpdfModule::GetUniqueObject()->fzContext, mfzImage);

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		delete mPdfStrokeLists[i];
	}
	mPdfStrokeLists.Clear();

}


ULONG CpdfHlAnnot::InitOnCreate()
{
	return EDERR_SUCCESS;
}

//int32 CpdfHlAnnot::GetPointCount()
//{
//	return mStrokePoints.Size();
//}
//
//const ED_POINT& CpdfHlAnnot::GetPoint(int32 index)
//{
//	return mStrokePoints[index];
//}

bool CpdfHlAnnot::AddInk(pdf_page* pageObj, CpdfInkMultiList& strokeList,float* clr,int clrN, float32 border)
{
	pdf_annot * annot;
	fz_context* ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	try
	{
		annot = pdf_create_annot(ctx, pageObj, PDF_ANNOT_INK);

		//pdf_set_annot_flags(PDF_ANNOT_IS_PRINT)
		pdf_set_annot_border(ctx, annot, border);

		if (clr != NULL)
		{
			pdf_set_annot_color(ctx, annot,clrN, clr);
		}

		for (int i=0;i<strokeList.Size();i++)
		{
			cmmDeletePtr<fz_point> pdfStroke;

			if(strokeList[i]->Size()<=0)
				continue;
			
			pdfStroke = new fz_point[strokeList[i]->Size()];
			for (int j = 0; j < strokeList[i]->Size(); j++)
			{
				pdfStroke.ptr()[j].x = (strokeList[i]->GetEntry(j).x);
				pdfStroke.ptr()[j].y = (strokeList[i]->GetEntry(j).y);
			}

			pdf_add_annot_ink_list(ctx, annot, strokeList[i]->Size(), pdfStroke.ptr());
		}

		   
		pdf_update_appearance(ctx, /*mDocObj->GetPdfObj(), */annot);

	}
	catch (...)
	{
		annot = NULL;
	}

	if (annot != NULL)
		return LoadInk(annot);

	return false;
}

bool CpdfHlAnnot::AddInk(pdf_page* pageObj, char* archive, int32 bufSize)
{
	uint32* head = (uint32*)archive;

	// 'inka',size in uint32,list count, color32,color N, [{point count,[pt1,pt2]},{}],'inka'
	if (head[0] != 'inka' || head[1] > bufSize/sizeof(uint32) || head[head[1]-1] != 'inka')
		return false;

	CpdfInkMultiList multiList;
	CpdfHighlightQuadList* listOne;

	float clr[4];
	int32 clrN;
	float border;
	uint32 clrCmpz = head[3];

	clr[0] = (clrCmpz & 0xFF)/255.0f;
	clrCmpz >>= 8;
	clr[1] = (clrCmpz & 0xFF) / 255.0f;
	clrCmpz >>= 8;
	clr[2] = (clrCmpz & 0xFF) / 255.0f;
	clrCmpz >>= 8;
	clr[3] = (clrCmpz & 0xFF) / 255.0f;

	clrN = (int32)head[4];
	border = (float)head[5];

	uint32* nextVal = &head[6];

	for (uint32 i = 0; i < head[2]; i++)
	{
		uint32 strokeCount = *nextVal++;
		if ((strokeCount & 0xFFF00000) != 0xFEF00000)
			return false;

		strokeCount &= 0xFFFFF;
		if(strokeCount == 0)
			continue;

		listOne = new CpdfHighlightQuadList;
		if (listOne == NULL)
			break;	// 出错

		for (uint32 j = 0; j < strokeCount; j++)
		{
			CpdfHighlightQuad stroke;

			stroke.x = *(float32*)nextVal++;
			stroke.y = *(float32*)nextVal++;

			listOne->Insert(-1, stroke);
		}

		multiList.Insert(-1, listOne);
	}

	if (multiList.Size() <= 0)
		return false;

	bool rev = AddInk(pageObj, multiList, clr, clrN, border);

	for (int z = 0; z < multiList.Size(); z++)
		delete multiList[z];

	return rev;
}

bool CpdfHlAnnot::LoadInk(pdf_annot* annot)
{
	int32 listCount;
	int32 strokeCount;
	CpdfHighlightQuadList* listOne;

	mPdfRawObj = annot;

	fz_context* ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	listCount = pdf_annot_ink_list_count(ctx, mPdfRawObj);

	for (int32 i = 0; i < listCount; i++)
	{
		strokeCount = pdf_annot_ink_list_stroke_count(ctx, mPdfRawObj, i);
		if(strokeCount <= 0)
			continue;

		listOne = new CpdfHighlightQuadList;
		if(listOne == NULL)
			break;	// 出错

		CpdfHighlightQuad stroke;
		for (int32 j=0;j<strokeCount;j++)
		{
			stroke = pdf_annot_ink_list_stroke_vertex(ctx, mPdfRawObj, i, j);
			listOne->Insert(-1, stroke);
			//CmmTrace::Trace(L"%08d,%08d\n", (INT)(stroke.x*10000), (INT)(stroke.y * 10000));
		}

		mPdfStrokeLists.Insert(-1, listOne);
	}

	pdf_annot_color(ctx,annot,&mColorN,mColor);
	mBorder = pdf_annot_border(ctx, annot);


	//auto bufSize = SaveToAchive(NULL, 0);
	//cmmDeletePtr<CHAR> buf;

	//buf.allocate(bufSize);

	//auto sizeSaved = SaveToAchive(buf.ptr(), bufSize);
	//sizeSaved -= bufSize;

	return mPdfStrokeLists.Size()>0;
}


void CpdfHlAnnot::GetSignature(ULONG& checksum, USHORT& altData)
{
	int32 totalStrokes = 0;
	uint32 sum = 0;

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		CpdfHighlightQuadList* lineOne = mPdfStrokeLists[i];

		if (lineOne == NULL)
			continue;

		for (int j = 1; j < lineOne->Size(); j++)
		{
			sum += (uint32)lineOne->GetEntry(j).x;
			sum += (uint32)lineOne->GetEntry(j).y;
		}
	}

	altData = (USHORT)totalStrokes;
	checksum = (ULONG)sum;
	checksum &= 0xFFFFFF;
}

int32 CpdfHlAnnot::DetectIntersection(ED_POINT* strokes, int32 pointCount)
{
	int32 crossAt = -1;
	float p11, p12, p21, p22;

	p11 = (float)strokes[0].x;
	p12 = (float)strokes[0].y;

	for (int p = 1; p < pointCount; p++)
	{
		p21 = (float)strokes[p].x;
		p22 = (float)strokes[p].y;

		for (int i = 0; i < mPdfStrokeLists.Size(); i++)
		{
			float q11, q12, q21, q22;
			CpdfHighlightQuadList* lineOne = mPdfStrokeLists[i];

			if(lineOne == NULL)
				continue;

			q11 = lineOne->GetEntry(0).x;
			q12 = lineOne->GetEntry(0).y;

			for (int j=1;j< lineOne->Size();j++)
			{
				q21 = lineOne->GetEntry(j).x;
				q22 = lineOne->GetEntry(j).y;

				if (IsSegmentCross(p11, p12, p21, p22, q11, q12, q21, q22) != false)
				{
					crossAt = i;
					break;
				}

				q11 = q21;
				q12 = q22;
			}

			if(crossAt >= 0)
				break;
		}
		if(crossAt >= 0)
			break;

		p11 = p21;
		p12 = p22;
	}

	return crossAt;
}

uint32 CpdfHlAnnot::GetType()
{
	return EDPDF_ANNOT_INK;
}

char* CpdfHlAnnot::GetTypeName()
{
	return "ink";
}

// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)
// 当buf==NULL时，返回需要的缓冲区字节数，缓冲区不足返回-1
uint32 CpdfHlAnnot::SaveToAchive(buf_ptr buf, uint32 bufSize)
{
	uint32 bufRequired = sizeof(uint32)*7;	// 'inka',size in uint32,list count, color32,color N,border, [{point count,[pt1,pt2]},{}],'inka'

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		CpdfHighlightQuadList* lineOne = mPdfStrokeLists[i];
		if (lineOne == NULL)
			continue;

		bufRequired += ((uint32)lineOne->Size() * sizeof(uint32) * 2) + sizeof(uint32);
	}

	if (buf == NULL)
		return bufRequired;

	if (bufSize < bufRequired)
		return -1;

	uint32* head = (uint32*)buf;
	uint32* end = head + bufSize / sizeof(uint32);

	head[0] = 'inka';
	head[1] = 0;
	head[2] = (uint32)mPdfStrokeLists.Size();
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

	for (uint32 i = 0; i < head[2]; i++)
	{
		CpdfHighlightQuadList* lineOne = mPdfStrokeLists[i];

		if (lineOne == NULL)
			continue;

		*nextVal++ = (0xFEF00000|(uint32)lineOne->Size());

		for (int j = 0; j < lineOne->Size(); j++)
		{
			*(float32*)nextVal++ = lineOne->GetEntry(j).x;
			*(float32*)nextVal++ = lineOne->GetEntry(j).y;

			if (nextVal >= end)
				return -1;
		}
	}

	*nextVal++ = 'inka';
	head[1] = (uint32)(nextVal - head);

	return head[1]*sizeof(uint32);
}

void CpdfHlAnnot::SetColor(ED_COLOR clr)
{

}

ED_COLOR CpdfHlAnnot::GetColor(void)
{
	ED_COLOR clr = { 0,0,0,0 };

	return clr;
}

void CpdfHlAnnot::SetBorder(int32 border)
{

}

int32 CpdfHlAnnot::GetBorder(void)
{
	return 0;
}

