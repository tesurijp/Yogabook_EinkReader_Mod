#include "EdPdf.h"
#include "PdfCreator.h"

void __stdcall EdCreatePdf(const wchar_t* pngPath, const wchar_t* pdfPath)
{
	Yoga2::PdfCreator creator;
	creator.AddImageToPdf(pngPath);
	creator.SavePdf(pdfPath);
}
