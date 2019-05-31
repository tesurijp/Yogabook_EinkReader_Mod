/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "HtmlFormatter.cpp"

void DrawHtmlPageNoLink(Graphics *g, mui::ITextRender *textDraw, Vec<DrawInstr> *drawInstructions, REAL offX, REAL offY, bool showBbox, Color textColor, bool *abortCookie)
{
	Pen debugPen(Color(255, 0, 0), 1);
	//Pen linePen(Color(0, 0, 0), 2.f);
	Pen linePen(Color(0x5F, 0x4B, 0x32), 2.f);

	WCHAR buf[512];

	// GDI text rendering suffers terribly if we call GetHDC()/ReleaseHDC() around every
	// draw, so first draw text and then paint everything else
	textDraw->SetTextColor(textColor);
	Status status = Ok;
	Timer t;
	textDraw->Lock();
	for (DrawInstr& i : *drawInstructions) {
		RectF bbox = i.bbox;
		bbox.X += offX;
		bbox.Y += offY;
		if (InstrString == i.type || InstrRtlString == i.type) {
			size_t strLen = str::Utf8ToWcharBuf(i.str.s, i.str.len, buf, dimof(buf));
			// soft hyphens should not be displayed
			strLen -= str::RemoveChars(buf, L"\xad");
			textDraw->Draw(buf, strLen, bbox, InstrRtlString == i.type);
		}
		else if (InstrSetFont == i.type) {
			textDraw->SetFont(i.font);
		}
		if (abortCookie && *abortCookie)
			break;
	}
	textDraw->Unlock();
	double dur = t.Stop();
	lf("DrawHtmlPage: textDraw %.2f ms", dur);

	for (DrawInstr& i : *drawInstructions) {
		RectF bbox = i.bbox;
		bbox.X += offX;
		bbox.Y += offY;
		if (InstrLine == i.type) {
			// hr is a line drawn in the middle of bounding box
			REAL y = floorf(bbox.Y + bbox.Height / 2.f + 0.5f);
			PointF p1(bbox.X, y);
			PointF p2(bbox.X + bbox.Width, y);
			if (showBbox) {
				status = g->DrawRectangle(&debugPen, bbox);
				CrashIf(status != Ok);
			}
			status = g->DrawLine(&linePen, p1, p2);
			CrashIf(status != Ok);
		}
		else if (InstrImage == i.type) {
			// TODO: cache the bitmap somewhere (?)
			Bitmap *bmp = BitmapFromData(i.img.data, i.img.len);
			if (bmp) {
				status = g->DrawImage(bmp, bbox, 0, 0, (REAL)bmp->GetWidth(), (REAL)bmp->GetHeight(), UnitPixel);
				// GDI+ sometimes seems to succeed in loading an image because it lazily decodes it
				CrashIf(status != Ok && status != Win32Error);
			}
			delete bmp;
		}
		else if (InstrLinkStart == i.type) {
			//// TODO: set text color to blue
			//REAL y = floorf(bbox.Y + bbox.Height + 0.5f); // $ax$
			//PointF p1(bbox.X, y);
			//PointF p2(bbox.X + bbox.Width, y);
			//Pen linkPen(textColor);
			//status = g->DrawLine(&linkPen, p1, p2);
			//CrashIf(status != Ok);
		}
		else if (InstrString == i.type || InstrRtlString == i.type) {
			if (showBbox) {
				status = g->DrawRectangle(&debugPen, bbox);
				CrashIf(status != Ok);
			}
		}
		else if (InstrLinkEnd == i.type) {
			// TODO: set text color back again
		}
		else if ((InstrElasticSpace == i.type) ||
			(InstrFixedSpace == i.type) ||
			(InstrString == i.type) ||
			(InstrRtlString == i.type) ||
			(InstrSetFont == i.type) ||
			(InstrAnchor == i.type)) {
			// ignore
		}
		else {
			CrashIf(true);
		}
		if (abortCookie && *abortCookie)
			break;
	}
}
