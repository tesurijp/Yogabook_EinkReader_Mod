/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "inkAnnot.h"
#include "pdfPage.h"
#include "pdfDocument.h"
#include "cmmPtr.h"
#include<math.h>
#include "cmmGeometry.h"

//////////////////////////////////////////////////////////////////////////
// Annot类
DEFINE_BUILTIN_NAME(CpdfInkAnnot)



CpdfInkAnnot::CpdfInkAnnot()
{
	mPdfRawObj = NULL;
}

CpdfInkAnnot::~CpdfInkAnnot()
{
	//if (mfzImage != NULL)
	//	fz_drop_pixmap(CEdpdfModule::GetUniqueObject()->fzContext, mfzImage);

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		delete mPdfStrokeLists[i];
	}
	mPdfStrokeLists.Clear();

	//if (mPdfRawObj != NULL)	似乎不能drop,看CpdfDocument::LoadAllThumbnails处的注释
	//	pdf_drop_annot(CEdpdfModule::GetUniqueObject()->fzContext, mPdfRawObj);

}


ULONG CpdfInkAnnot::InitOnCreate()
{
	return EDERR_SUCCESS;
}

//int32 CpdfInkAnnot::GetPointCount()
//{
//	return mStrokePoints.Size();
//}
//
//const ED_POINT& CpdfInkAnnot::GetPoint(int32 index)
//{
//	return mStrokePoints[index];
//}

bool CpdfInkAnnot::PdfCreateAnnot(pdf_page* pageObj, CpdfInkMultiList& strokeList,float* clr,int clrN, float32 border)
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

		//fz_rect ldRect = { 1920,1920,0,0 };

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

		//pdf_set_annot_rect(ctx, annot, ldRect);
		//pdf_dirty_annot(ctx, annot);
		pdf_update_appearance(ctx, /*mDocObj->GetPdfObj(), */annot);

	}
	catch (...)
	{
		annot = NULL;
	}

	if (annot != NULL)
		return Load(annot);

	return false;
}

bool CpdfInkAnnot::LoadFromAchive(pdf_page* pageObj, char* archive, int32 bufSize)
{
	uint32* head = (uint32*)archive;

	// 'inka',size in uint32,list count, color32,color N, [{point count,[pt1,pt2]},{}],'inka'
	if (head[0] != 'inka' || head[1] > bufSize/sizeof(uint32) || head[head[1]-1] != 'inka')
		return false;

	CpdfInkMultiList multiList;
	CpdfInkStrokeList* listOne;

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

		listOne = new CpdfInkStrokeList;
		if (listOne == NULL)
			break;	// 出错

		for (uint32 j = 0; j < strokeCount; j++)
		{
			CpdfInkStroke stroke;

			stroke.x = *(float32*)nextVal++;
			stroke.y = *(float32*)nextVal++;

			listOne->Insert(-1, stroke);
		}

		multiList.Insert(-1, listOne);
	}

	if (multiList.Size() <= 0)
		return false;

	bool rev = PdfCreateAnnot(pageObj, multiList, clr, clrN, border);

	for (int z = 0; z < multiList.Size(); z++)
		delete multiList[z];

	return rev;
}

bool CpdfInkAnnot::Load(pdf_annot* annot)
{
	int32 listCount;
	int32 strokeCount;
	CpdfInkStrokeList* listOne;

	mPdfRawObj = annot;

	fz_context* ctx = CEdpdfModule::GetUniqueObject()->fzContext;

	listCount = pdf_annot_ink_list_count(ctx, mPdfRawObj);

	for (int32 i = 0; i < listCount; i++)
	{
		strokeCount = pdf_annot_ink_list_stroke_count(ctx, mPdfRawObj, i);
		if(strokeCount <= 0)
			continue;

		listOne = new CpdfInkStrokeList;
		if(listOne == NULL)
			break;	// 出错

		CpdfInkStroke stroke;
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


void CpdfInkAnnot::GetSignature(ULONG& checksum, USHORT& altData)
{
	int32 totalStrokes = 0;
	uint32 sum = 0;

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		CpdfInkStrokeList* lineOne = mPdfStrokeLists[i];

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

int32 CpdfInkAnnot::DetectIntersection(ED_POINTF* strokes, int32 pointCount)
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
			CpdfInkStrokeList* lineOne = mPdfStrokeLists[i];

			if(lineOne == NULL)
				continue;

			q11 = lineOne->GetEntry(0).x;
			q12 = lineOne->GetEntry(0).y;

			for (int j=1;j< lineOne->Size();j++)
			{
				q21 = lineOne->GetEntry(j).x;
				q22 = lineOne->GetEntry(j).y;

				if (CGmLine::IsSegmentCross(p11, p12, p21, p22, q11, q12, q21, q22) != false)
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

uint32 CpdfInkAnnot::GetType()
{
	return EDPDF_ANNOT_INK;
}

char* CpdfInkAnnot::GetTypeName()
{
	return "ink";
}

// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)
// 当buf==NULL时，返回需要的缓冲区字节数，缓冲区不足返回-1
uint32 CpdfInkAnnot::SaveToAchive(buf_ptr buf, uint32 bufSize)
{
	uint32 bufRequired = sizeof(uint32)*7;	// 'inka',size in uint32,list count, color32,color N,border, [{point count,[pt1,pt2]},{}],'inka'

	for (int i = 0; i < mPdfStrokeLists.Size(); i++)
	{
		CpdfInkStrokeList* lineOne = mPdfStrokeLists[i];
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
		CpdfInkStrokeList* lineOne = mPdfStrokeLists[i];

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

void CpdfInkAnnot::SetColor(ED_COLOR clr)
{

}

ED_COLOR CpdfInkAnnot::GetColor(void)
{
	ED_COLOR clr = { 0,0,0,0 };

	return clr;
}

void CpdfInkAnnot::SetBorder(int32 border)
{

}

int32 CpdfInkAnnot::GetBorder(void)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CpdfSmoothMethod

bool CpdfSmoothMethod::SmoothStroke(CpdfInkStrokeList& mStrokePoints, float threshold  /*= 5.0*/, float nfPenWidth)
{
	if (mStrokePoints.Size() <= 0)
		return false;


	CpdfSmoothMethod method(&mStrokePoints,threshold);

	//先把重复无用点清除一下
	int liIndex = 1;
	while (liIndex < mStrokePoints.Size())
	{
		int liRet = method.LineIntersectSide(mStrokePoints.GetEntry(liIndex - 1).x, mStrokePoints.GetEntry(liIndex - 1).y, mStrokePoints.GetEntry(liIndex).x, mStrokePoints.GetEntry(liIndex).y, 0.0f,0.0f);
		if (liRet == 1)
			mStrokePoints.RemoveByIndex(liIndex);
		else if (liRet == 2)
			mStrokePoints.RemoveByIndex(liIndex - 1);
		else
			liIndex++;
	}


	method.SmoothRecurs(1, mStrokePoints.Size() - 2);

	/*return mStrokePoints.Size() > 0;*/
	if (mStrokePoints.Size() <= 6)
	{
		CpdfInkStroke ldBegin = mStrokePoints.GetEntry(0);
		CpdfInkStroke ldEnd = mStrokePoints.GetEntry(mStrokePoints.Size() - 1);
		if (fabs(ldBegin.x - ldEnd.x) < nfPenWidth && fabs(ldBegin.y - ldEnd.y) < nfPenWidth)
		{
			//这种情况基本只是个小点
			CpdfInkStroke ldStroke;
			ldStroke.x = mStrokePoints.GetEntry(0).x - nfPenWidth / 2;
			ldStroke.y = mStrokePoints.GetEntry(0).y;

			mStrokePoints.Clear();

			mStrokePoints.Insert(0, ldStroke);

			ldStroke.x += nfPenWidth;
			//ldStroke.y = mStrokePoints.GetEntry(0).y;
			mStrokePoints.Insert(0, ldStroke);

			return false;
		}

	}
		

	int liPointCount = mStrokePoints.Size();
	if (false)
	{
		/*for (int n = 0; n < liPointCount; n++) {
		if (n <= liPointCount - 4) {
		for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
		var a1 = pow((1 - t), 3) / 6;
		var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
		var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
		var a4 = pow(t, 3) / 6;

		var x = a1*points[n].x + a2*points[n + 1].x + a3*points[n + 2].x + a4*points[n + 3].x;
		var y = a1*points[n].y + a2*points[n + 1].y + a3*points[n + 2].y + a4*points[n + 3].y;
		newPnts.push({ x: x, y : y });
		}
		}
		else if (n == liPointCount - 3) {
		for (var t = 0.0; t <= 1.0; t += 0.1) {
		var a1 = pow((1 - t), 3) / 6;
		var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
		var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
		var a4 = pow(t, 3) / 6;

		var x = a1*points[n].x + a2*points[n + 1].x + a3*points[n + 2].x + a4*points[0].x;
		var y = a1*points[n].y + a2*points[n + 1].y + a3*points[n + 2].y + a4*points[0].y;
		newPnts.push({ x: x, y : y });
		}
		}
		else if (n == liPointCount - 2) {
		for (var t = 0.0; t <= 1.0; t += 0.1) {
		var a1 = pow((1 - t), 3) / 6;
		var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
		var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
		var a4 = pow(t, 3) / 6;

		var x = a1*points[n].x + a2*points[n + 1].x + a3*points[0].x + a4*points[1].x;
		var y = a1*points[n].y + a2*points[n + 1].y + a3*points[0].y + a4*points[1].y;
		newPnts.push({ x: x, y : y });
		}
		}
		else if (n == liPointCount - 1) {
		for (var t = 0.0; t <= 1.0; t += 0.1) {
		var a1 = pow((1 - t), 3) / 6;
		var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
		var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
		var a4 = pow(t, 3) / 6;

		var x = a1*points[n].x + a2*points[0].x + a3*points[1].x + a4*points[2].x;
		var y = a1*points[n].y + a2*points[0].y + a3*points[1].y + a4*points[2].y;
		newPnts.push({ x: x, y : y });
		}
		}
		}*/
	}
	else
	{
		//newPnts.push({ x: points[0].x, y : points[0].y });
		CpdfInkStrokeList ldStrokePoints;
		CpdfInkStroke ldlastStroke;
		ldlastStroke.x = 0.0f;
		ldlastStroke.y = 0.0f;

		CpdfInkStroke ldBegin = mStrokePoints.GetEntry(0);
		ldBegin.y -= 1.0f;
		ldStrokePoints.Insert(-1, ldBegin);

		for (int n = 0; n < liPointCount; n++) {
			if (n <= liPointCount - 4) {
				int liIndex = 1;
				for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
					float a1 = pow((1.0f - t), 3.0f) / 6.0f;
					float a2 = (3.0f * pow(t, 3.0f) - 6.0f * pow(t, 2.0f) + 4.0f) / 6.0f;
					float a3 = (-3.0f * pow(t, 3.0f) + 3.0f * pow(t, 2.0f) + 3.0f * t + 1.0f) / 6.0f;
					float a4 = pow(t, 3.0f) / 6.0f;

					CpdfInkStroke ldStroke;
					ldStroke.x = a1*mStrokePoints.GetEntry(n).x + a2*mStrokePoints.GetEntry(n + 1).x + a3*mStrokePoints.GetEntry(n + 2).x + a4*mStrokePoints.GetEntry(n + 3).x;
					ldStroke.y = a1*mStrokePoints.GetEntry(n).y + a2*mStrokePoints.GetEntry(n + 1).y + a3*mStrokePoints.GetEntry(n + 2).y + a4*mStrokePoints.GetEntry(n + 3).y;

					if (ldStroke.x != ldlastStroke.x || ldStroke.y != ldlastStroke.y)
					{
						//相同点不处理
						ldlastStroke = ldStroke;
						ldStrokePoints.Insert(-1, ldStroke);
					}
					
				}
			}
		}

		CpdfInkStroke ldEnd = mStrokePoints.GetEntry(mStrokePoints.Size() - 1);
		ldEnd.y += 1.0f;
		ldStrokePoints.Insert(-1, ldEnd);

		mStrokePoints.Clear();
		for (int i = 0; i<ldStrokePoints.Size(); i++)
		{
			mStrokePoints.Insert(-1, ldStrokePoints.GetEntry(i));
		}

		//newPnts.push({ x: points[liPointCount - 1].x, y : points[liPointCount - 1].y });
	}

	return mStrokePoints.Size() > 0;
}

//三次B样条曲线近似拟合算法，处理曲线平滑,nbisClose为true表示是闭合曲线
//static bool SmoothStrokeNew(CpdfInkStrokeList& rStrokePoints, bool nbisClose = false)
//{
//	if (rStrokePoints.Size() <= 4)
//		return false;
//
//	int liPointCount = rStrokePoints.Size();
//	if (nbisClose != false) 
//	{
//		/*for (int n = 0; n < liPointCount; n++) {
//			if (n <= liPointCount - 4) {
//				for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
//					var a1 = pow((1 - t), 3) / 6;
//					var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
//					var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
//					var a4 = pow(t, 3) / 6;
//
//					var x = a1*points[n].x + a2*points[n + 1].x + a3*points[n + 2].x + a4*points[n + 3].x;
//					var y = a1*points[n].y + a2*points[n + 1].y + a3*points[n + 2].y + a4*points[n + 3].y;
//					newPnts.push({ x: x, y : y });
//				}
//			}
//			else if (n == liPointCount - 3) {
//				for (var t = 0.0; t <= 1.0; t += 0.1) {
//					var a1 = pow((1 - t), 3) / 6;
//					var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
//					var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
//					var a4 = pow(t, 3) / 6;
//
//					var x = a1*points[n].x + a2*points[n + 1].x + a3*points[n + 2].x + a4*points[0].x;
//					var y = a1*points[n].y + a2*points[n + 1].y + a3*points[n + 2].y + a4*points[0].y;
//					newPnts.push({ x: x, y : y });
//				}
//			}
//			else if (n == liPointCount - 2) {
//				for (var t = 0.0; t <= 1.0; t += 0.1) {
//					var a1 = pow((1 - t), 3) / 6;
//					var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
//					var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
//					var a4 = pow(t, 3) / 6;
//
//					var x = a1*points[n].x + a2*points[n + 1].x + a3*points[0].x + a4*points[1].x;
//					var y = a1*points[n].y + a2*points[n + 1].y + a3*points[0].y + a4*points[1].y;
//					newPnts.push({ x: x, y : y });
//				}
//			}
//			else if (n == liPointCount - 1) {
//				for (var t = 0.0; t <= 1.0; t += 0.1) {
//					var a1 = pow((1 - t), 3) / 6;
//					var a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
//					var a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
//					var a4 = pow(t, 3) / 6;
//
//					var x = a1*points[n].x + a2*points[0].x + a3*points[1].x + a4*points[2].x;
//					var y = a1*points[n].y + a2*points[0].y + a3*points[1].y + a4*points[2].y;
//					newPnts.push({ x: x, y : y });
//				}
//			}
//		}*/
//	}
//	else 
//	{
//		//newPnts.push({ x: points[0].x, y : points[0].y });
//		CpdfInkStrokeList ldStrokePoints;
//		for (int n = 0; n < liPointCount; n++) {
//			if (n <= liPointCount - 4) {
//				int liIndex = 1;
//				for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
//					float a1 = pow((1 - t), 3) / 6;
//					float a2 = (3 * pow(t, 3) - 6 * pow(t, 2) + 4) / 6;
//					float a3 = (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1) / 6;
//					float a4 = pow(t, 3) / 6;
//
//					CpdfInkStroke ldStroke;
//					ldStroke.x = a1*rStrokePoints.GetEntry(n).x + a2*rStrokePoints.GetEntry(n + 1).x + a3*rStrokePoints.GetEntry(n + 2).x + a4*rStrokePoints.GetEntry(n + 3).x;
//					ldStroke.y = a1*rStrokePoints.GetEntry(n).y + a2*rStrokePoints.GetEntry(n + 1).y + a3*rStrokePoints.GetEntry(n + 2).y + a4*rStrokePoints.GetEntry(n + 3).y;
//					ldStrokePoints.Insert(-1,ldStroke);
//				}
//			}
//		}
//
//		rStrokePoints.Clear();
//		for (int i=0;i<ldStrokePoints.Size();i++)
//		{
//			rStrokePoints.Insert(-1, ldStrokePoints.GetEntry(i));
//		}
//
//		//newPnts.push({ x: points[liPointCount - 1].x, y : points[liPointCount - 1].y });
//	}
//
//	return rStrokePoints.Size() > 0;
//}

CpdfSmoothMethod::CpdfSmoothMethod(CpdfInkStrokeList* list, float threshold)
{
	mSmoothThreshold = threshold;
	data = list;
}

void CpdfSmoothMethod::SmoothRecurs(int32 pb, int32 pe)
{
	if (pb + 1 >= pe)
		return;

	//float mainLine = sqrt(pow(data->GetEntry(pe).y - data->GetEntry(pb).y, 2) + pow(data->GetEntry(pe).x - data->GetEntry(pb).x, 2));

	int32 specialPt = -1;
	float largest = 0.0;

	for (int32 i = pb + 1; i < pe; i++)
	{
		float dis = DistancePointToLine(data->GetEntry(i).x, data->GetEntry(i).y, data->GetEntry(pb).x, data->GetEntry(pb).y, data->GetEntry(pe).x, data->GetEntry(pe).y);
		if (dis > largest)
		{
			specialPt = i;
			largest = dis;
		}
	}

	if (specialPt < 0)
		return;

	if (largest < mSmoothThreshold)
	{
		data->RemoveByIndex(specialPt);
		pe--;
	}

	if (specialPt < pe)
		SmoothRecurs(specialPt, pe);

	SmoothRecurs(pb, specialPt);
}

int CpdfSmoothMethod::LineIntersectSide(float x, float y, float x1, float y1, float x2, float y2)
{
	int liRet = 0;

	if (x == x1 && y == y1)
		liRet = 1;


	//float lfA = (y1 - y) * (x - x1) - (x1 - x) * (y - y1);
	//float lfB = (y2 - y) * (x - x1) - (x2 - x) * (y - y1);

	//if (fabs(lfA) > 0.0f || fabs(lfB) > 0.0f)
	//	return liRet;

	//lfA = (y - y1) * (x1 - x2) - (x - x1) * (y1 - y2);
	//lfB = (y1 - y1) * (x1 - x2) - (x1 - x1) * (y1 - y2);

	//if (fabs(lfA) > 0.0f || fabs(lfB) > 0.0f)
	//	return liRet;

	//if (y == y2 && y == y1)
	//{
	//	//横线
	//	//if (fabs(x - x2) > fabs(x - x1))
	//	if(x < x1 && x1 < x2)
	//		liRet = 1; //中间点是多余的
	//	else
	//		liRet = 2; //中间点是多余的
	//	//else if(x1 < x && x1 < x2)
	//	//	liRet = 2; //前一个点是多余的
	//	//else if (x1 == x && x1 == x2)
	//	//	liRet = 2; //前一个点是多余的
	//}
	//else if( x == x2 && x== x1)
	//{
	//	//坚线
	//	//if (fabs(y - y2) > fabs(y - y1))
	//	if(y < y1 && y1 < y2)
	//		liRet = 1; //中间点是多余的
	//	else
	//		liRet = 2; //前一个点是多余的
	//	//else if (y1 < y && y1 < y2)
	//	//	liRet = 2; //前一个点是多余的
	//	//else if (y1 == y && y1 == y2)
	//	//	liRet = 2; //前一个点是多余的
	//}

	return liRet;
}


float CpdfSmoothMethod::DistancePointToLine(float x, float y, float x1, float y1, float x2, float y2)
{
	float d = (fabs((y2 - y1) * x + (x1 - x2) * y + ((x2 * y1) - (x1 * y2)))) / (sqrt(pow(y2 - y1, 2) + pow(x1 - x2, 2)));
	return d;
}

