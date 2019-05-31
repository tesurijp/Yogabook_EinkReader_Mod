/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once


#define MAX_SUPPORT_TEXT_LEN		(1 * 1024 * 1024) // 最大支持长度为1M的字符串

DECLARE_BUILTIN_NAME(CXuiTextBitmap)

class CXuiTextBitmap :
	public CXD2dBitmap
{
public:
	CXuiTextBitmap(void);
	~CXuiTextBitmap(void);

	ULONG InitOnCreate(STETXT_BMP_INIT& rdBmpInit);

	// 通过文字实例化位图对象
	DEFINE_CUMSTOMIZE_CREATE(CXuiTextBitmap,(STETXT_BMP_INIT& rdBmpInit), (rdBmpInit));

	ERESULT __stdcall GetD2DObject(IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap);
private:
	VOID MapAlignMode(STETXT_BMP_INIT::eTEXTALIGN t, STETXT_BMP_INIT::ePARAALIGN p);
	FLOAT CalcDIPFromPixel(ID2D1RenderTarget* npRt, DWORD ndwPixel);
	VOID BackOneChar();

	DWRITE_TEXT_ALIGNMENT meTextAlignMode;
	DWRITE_PARAGRAPH_ALIGNMENT meParaAlignMode;

	DWORD mdwFontSizePixel;	// 物理像素指定的字体大小
	wchar_t *mpaFontName; // 字体名字
	wchar_t *mpText;
	DWORD mdwTextLen;
	DWORD mdwFontColor;
	DWORD mdwWidthLimit;
	DWORD mdwHeightLimit;
	STETXT_BMP_INIT::eSIZELIMIT meSizeLimit;
};

