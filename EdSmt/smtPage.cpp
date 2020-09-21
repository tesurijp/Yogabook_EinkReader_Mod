/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "smtPage.h"
#include "smtDocument.h"
#include "EngineOverride.h"

//////////////////////////////////////////////////////////////////////////
// Bitmap类
DEFINE_BUILTIN_NAME(CSmtBitmap)


CSmtBitmap::CSmtBitmap()
{
	mEngineBmp = NULL;
	mDIBSection = NULL;
}

CSmtBitmap::~CSmtBitmap()
{
	if (mEngineBmp != NULL)
		delete mEngineBmp;
	if (mDIBSection != NULL)
		DeleteObject(mDIBSection);
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CSmtBitmap::InitOnCreate(RenderedBitmap* engineBmp)
{
	mEngineBmp = engineBmp;

	return EDERR_SUCCESS;
}

bin_ptr CSmtBitmap::GetBuffer()
{
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biHeight = -mEngineBmp->Size().dy;
	bmi.bmiHeader.biWidth = mEngineBmp->Size().dx;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	unsigned char *bmpData = nullptr;

	if (mDIBSection != NULL)
	{
		DeleteObject(mDIBSection);
		mDIBSection = NULL;
	}

	mDIBSection = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, (void **)&bmpData, nullptr, 0);
	if (mDIBSection == NULL)
		return NULL;

	HDC hdc = GetDC(nullptr);
	int dx = mEngineBmp->Size().dx;
	auto dy = mEngineBmp->Size().dy;
	if (mEngineBmp && GetDIBits(hdc, mEngineBmp->GetBitmap(), 0, mEngineBmp->Size().dy, bmpData, &bmi, DIB_RGB_COLORS)) {
		for (int i = 0; i < dx * dy; i++)
			bmpData[4 * i + 3] = 0xFF;
	}
	//else {
	//	DeleteObject(hthumb);
	//	hthumb = nullptr;
	//}

	ReleaseDC(nullptr, hdc);

	return bmpData;
}

int32Eink CSmtBitmap::GetWidth()
{
	if (mEngineBmp == NULL)
		return 0;
	return mEngineBmp->Size().dx;
}

int32Eink CSmtBitmap::GetHeight()
{
	if (mEngineBmp == NULL)
		return 0;
	return mEngineBmp->Size().dy;
}

//////////////////////////////////////////////////////////////////////////
// Page类
DEFINE_BUILTIN_NAME(CSmtPage)

CSmtPage::CSmtPage()
{
	documentObject = NULL;
}

CSmtPage::~CSmtPage()
{
	CMM_SAFE_RELEASE(documentObject);
}


// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CSmtPage::InitOnCreate(int32Eink pageNo, CSmtDocument* docObj)
{
	rawContext.pageIndex = pageNo;
	documentObject = docObj;
	documentObject->AddRefer();

	if(documentObject != NULL && rawContext.pageIndex < documentObject->GetPageCount())
		return EDERR_SUCCESS;

	return EDERR_FIALED_LOAD_PAGE;
}

bool32 CSmtPage::GetMediaBox(
	OUT ED_RECTF_PTR mediaBox
)
{

	auto engine = documentObject->GetEngine();
	if (!engine || mediaBox == NULL)
		return false;

	RectD mediaBoxF = engine->GetRawEngineObj()->Transform(engine->GetRawEngineObj()->PageMediabox(ENGINE_INDEX(rawContext.pageIndex)), ENGINE_INDEX(rawContext.pageIndex), 1.0, 0);
	RectI mediaBoxI = mediaBoxF.Round();

	mediaBox->left =	(float)mediaBoxI.x;
	mediaBox->right =	(float)mediaBoxI.x + mediaBoxI.dx;
	mediaBox->top =		(float)mediaBoxI.y;
	mediaBox->bottom =	(float)mediaBoxI.y + mediaBoxI.dy;

	return true;
}

bool32 CSmtPage::GetCropBox(
	OUT ED_RECTF_PTR cropBox
)
{
	return false;//GetBox(PDF_NAME_CropBox, *cropBox);
}

bool32 CSmtPage::GetBleedBox(
	OUT ED_RECTF_PTR bleedBox
)
{
	return false;//GetBox(PDF_NAME_BleedBox,*bleedBox);
}

IEdBitmap_ptr CSmtPage::Render(
	IN float32 scalRatio,
	IN bool32 cleanUp
)
{
	auto engine = documentObject->GetEngine();
	if (!engine)
		return NULL;

	RenderedBitmap* engineBitmap = engine->GetRawEngineObj()->RenderBitmap(ENGINE_INDEX(rawContext.pageIndex), scalRatio, 0,NULL);
	if (engineBitmap == NULL)
		return NULL;

	return CSmtBitmap::CreateInstance(engineBitmap);
}

int32Eink CSmtPage::GetPageIndex(void)
{
	auto engine = documentObject->GetEngine();
	if (!engine)
		return NULL;

	engine->ReQueryPageIndex(&rawContext);

	return rawContext.pageIndex;
}

bool32 CSmtPage::GetPageContext(PPAGE_PDF_CONTEXT contextPtr)
{
	contextPtr->pageIndex = rawContext.pageIndex;
	contextPtr->pageContext = rawContext.pageContext;
	contextPtr->pageContext2 = rawContext.pageContext2;

	return true;
}

bool32 CSmtPage::GetSelectedText(IN ED_RECTF_PTR selBox, OUT char16_ptr textBuf, IN int32Eink bufSize)
{
	return false;
}

IEdStructuredTextPage_ptr CSmtPage::GetStructuredTextPage(void)
{
	return NULL;
}

