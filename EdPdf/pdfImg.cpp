/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "pdfImg.h"
#include "pdfDocument.h"

//////////////////////////////////////////////////////////////////////////
// Bitmap类
DEFINE_BUILTIN_NAME(CpdfBitmap)


CpdfBitmap::CpdfBitmap()
{
	mfzImage = NULL;
}

CpdfBitmap::~CpdfBitmap()
{
	if(mfzImage != NULL)
		fz_drop_pixmap(CEdpdfModule::GetUniqueObject()->fzContext, mfzImage);
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfBitmap::InitOnCreate(fz_pixmap* fzImage)
{
	mfzImage = fzImage;
	mWidth = fz_pixmap_width(CEdpdfModule::GetUniqueObject()->fzContext,mfzImage);
	mHeight = fz_pixmap_height(CEdpdfModule::GetUniqueObject()->fzContext,mfzImage);
	fz_pixmap_components(CEdpdfModule::GetUniqueObject()->fzContext, mfzImage);

	return EDERR_SUCCESS;
}

bin_ptr CpdfBitmap::GetBuffer()
{
	return fz_pixmap_samples(CEdpdfModule::GetUniqueObject()->fzContext, mfzImage);
}

int32 CpdfBitmap::GetWidth()
{
	return mWidth;
}

int32 CpdfBitmap::GetHeight()
{
	return mHeight;
}

