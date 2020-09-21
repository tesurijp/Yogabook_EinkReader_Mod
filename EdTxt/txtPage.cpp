/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "txtPage.h"
#include "txtDocument.h"
#include "gdiplus.h"


using namespace Gdiplus;
//////////////////////////////////////////////////////////////////////////
// Bitmap类
DEFINE_BUILTIN_NAME(CtxtBitmap)


CtxtBitmap::CtxtBitmap()
{
}

CtxtBitmap::~CtxtBitmap()
{
	if(mImageBuffer != NULL)
		HeapFree(GetProcessHeap(), 0, mImageBuffer);
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CtxtBitmap::InitOnCreate(int32 w, int32 h)
{
	mWidth =  w;
	mHeight = h;
	if (w != 0 && h != 0)
		mImageBuffer = (bin_ptr)HeapAlloc(GetProcessHeap(), 0, w*h * 4);
	else
		mImageBuffer = NULL;

	return EDERR_SUCCESS;
}

bin_ptr CtxtBitmap::GetBuffer()
{
	return mImageBuffer;
}

int32 CtxtBitmap::GetWidth()
{
	return mWidth;
}

int32 CtxtBitmap::GetHeight()
{
	return mHeight;
}

//////////////////////////////////////////////////////////////////////////
// Page类
DEFINE_BUILTIN_NAME(CtxtPage)

CtxtPage::CtxtPage()
{
	mDocObj = NULL;
}

CtxtPage::~CtxtPage()
{
	CMM_SAFE_RELEASE(mDocObj);
}


// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CtxtPage::InitOnCreate(int32 pageNo, wchar_t* charPtr, uint32 charLength, CtxtDocument* docObj)
{
	//mPageIndexOpenning = pageNo;
	mCharPtr = charPtr;
	mCharLength = charLength;
	mDocObj = docObj;
	mDocObj->AddRefer();

	mPageWidth = (uint32)mDocObj->GetViewPort()->X;
	mPageHeight = (uint32)mDocObj->GetViewPort()->Y;

	return EDERR_SUCCESS;
}

bool32 CtxtPage::GetMediaBox(
	OUT ED_RECTF_PTR mediaBox
)
{
	mediaBox->left = 0.0f;
	mediaBox->right = (float)mPageWidth;
	mediaBox->top = 0.0f;
	mediaBox->bottom = (float)mPageHeight;
	return true;
}

bool32 CtxtPage::GetCropBox(
	OUT ED_RECTF_PTR cropBox
)
{
	return false;//GetBox(PDF_NAME_CropBox, *cropBox);
}

bool32 CtxtPage::GetBleedBox(
	OUT ED_RECTF_PTR bleedBox
)
{
	return false;//GetBox(PDF_NAME_BleedBox,*bleedBox);
}

IEdBitmap_ptr CtxtPage::Render(
	IN float32 scalRatio,
	IN bool32 cleanUp
)
{
	bin_ptr imageBuf;
	Bitmap* gdipImage = NULL;
	Graphics* gdipGraphic = NULL;
	CGdipStart gdiStart;

	if (mDocObj == NULL)
		return NULL;

	CtxtBitmap* txtBitmap = CtxtBitmap::CreateInstance(mPageWidth,mPageHeight);

	if (txtBitmap == NULL)
		return NULL;

	try
	{
		gdiStart.Init();

		imageBuf = txtBitmap->GetBuffer();
		THROW_ON_NULL(imageBuf);

		//RtlZeroMemory(imageBuf, mPageWidth*mPageHeight*4);
		memset(imageBuf, 0xFF, mPageWidth*mPageHeight * 4);

		gdipImage = new Bitmap(mPageWidth,mPageHeight,mPageWidth* 4, PixelFormat32bppARGB, imageBuf);
		THROW_ON_NULL(gdipImage);

		gdipGraphic = new Graphics(gdipImage);
		THROW_ON_NULL(gdipGraphic);

		gdipGraphic->SetSmoothingMode(SmoothingModeAntiAlias);
		gdipGraphic->SetInterpolationMode(InterpolationModeHighQualityBicubic);
		//lpBigGps->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

		StringFormat strFormat;
		strFormat.SetAlignment(StringAlignmentNear);

		RectF boundRect(0.0f,0.0f,(float)mPageWidth,(float)mPageHeight);
		Gdiplus::Font fontObj(mDocObj->GetFontName(), mDocObj->GetFontSize());


		gdipGraphic->DrawString(mCharPtr,mCharLength,&fontObj, boundRect, &strFormat, &SolidBrush(Color::Black));

	}
	catch (...)
	{
	}

	CMM_SAFE_DELETE(gdipGraphic);
	CMM_SAFE_DELETE(gdipImage);

	gdiStart.UnInit();

	return txtBitmap;
}

int32 CtxtPage::GetPageIndex(void)
{
	return mDocObj->GetPageIndex(this);
}

bool32 CtxtPage::GetPageContext(PPAGE_PDF_CONTEXT contextPtr)
{
	contextPtr->pageIndex = GetPageIndex();
	contextPtr->pageContext = (uint32)(mCharPtr - mDocObj->GetDocBuffer());
	contextPtr->pageContext2 = 0;

	return true;
}

bool32 CtxtPage::GetSelectedText(IN ED_RECTF_PTR selBox, OUT char16_ptr textBuf, IN int32Eink bufSize)
{
	return true;
}

IEdStructuredTextPage_ptr CtxtPage::GetStructuredTextPage(void)
{
	return NULL;
}
