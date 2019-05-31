/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"

#include "CommonHeader.h"
#include "XgTextBitmapImp.h"


DEFINE_BUILTIN_NAME(CXuiTextBitmap)

CXuiTextBitmap::CXuiTextBitmap(void)
{
	mpaFontName = NULL;
	mpText = NULL;
}


CXuiTextBitmap::~CXuiTextBitmap(void)
{
	if(mpaFontName)
		delete []mpaFontName;
	if(mpText)
		delete []mpText;
}

ULONG CXuiTextBitmap::InitOnCreate(STETXT_BMP_INIT& rdBmpInit)
{

// 	IDWriteTextLayout* lpTextLayout = NULL;
// 	IDWriteTextFormat* lpTextFormat = NULL;
	CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(this);

	if(rdBmpInit.Text == NULL || rdBmpInit.FontName == NULL)
		return ERESULT_BMP_INVALIDBUF;

	DWORD ldwBufLen = 0;
	size_t lsStrLen;
	m_hResult = StringCchLength(rdBmpInit.FontName, MAX_PATH, &lsStrLen);
	if(FAILED(m_hResult))
		return ERESULT_BMP_INVALIDBUF;
	mpaFontName = new wchar_t[lsStrLen+1];
	StringCchCopy(mpaFontName, lsStrLen+1, rdBmpInit.FontName);

	m_hResult = StringCchLength(rdBmpInit.Text, MAX_SUPPORT_TEXT_LEN, &lsStrLen);
	if(FAILED(m_hResult))
		return ERESULT_BMP_INVALIDBUF;
	mdwTextLen = (DWORD)lsStrLen;
	mpText = new wchar_t[mdwTextLen+1];
	StringCchCopy(mpText, mdwTextLen+1, rdBmpInit.Text);

	mdwFontColor = rdBmpInit.TextColor;
	mdwHeightLimit = rdBmpInit.Height;
	mdwWidthLimit = rdBmpInit.Width;
	meSizeLimit = rdBmpInit.Limit;
	mdwFontSizePixel = rdBmpInit.FontSize;

	MapAlignMode(rdBmpInit.Talign, rdBmpInit.Palign);

	IDWriteTextFormat *lpTextFormat = NULL;
	IDWriteTextLayout *lpTextLayout = NULL;
	ID2D1RenderTarget *lpBitmapRT = NULL;
	ID2D1SolidColorBrush *lpFontBrush = NULL;
	if(NULL == mpBitmap2D)
	{
		__try
		{
			// 计算DIP单位参数
			FLOAT lfFontSize = CalcDIPFromPixel(NULL, mdwFontSizePixel);
			FLOAT lfWidthLimit = CalcDIPFromPixel(NULL, mdwWidthLimit);
			FLOAT lfHeightLimit = CalcDIPFromPixel(NULL, mdwHeightLimit);

			// 创建FontFormat
			m_hResult = EinkuiGetSystem()->GetDWriteFactory()->CreateTextFormat(
				mpaFontName,
				NULL,
				(rdBmpInit.FontWeight == STETXT_BMP_INIT::EL_FW_NORMAL) ? DWRITE_FONT_WEIGHT_NORMAL : DWRITE_FONT_WEIGHT_BOLD,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				lfFontSize,
				L"", //locale
				&lpTextFormat);
			if(FAILED(m_hResult))
				return ERESULT_BMP_CREATETEXTFMT;
			lpTextFormat->SetTextAlignment(meTextAlignMode);
			lpTextFormat->SetParagraphAlignment(meParaAlignMode);

			// 创建FontLayout
			if(STETXT_BMP_INIT::EL_FREESIZE == meSizeLimit)
			{
				lfWidthLimit = 60000.0f; // 设置为一个超大的值
			} else if(STETXT_BMP_INIT::EL_FIXEDWIDTH != meSizeLimit && STETXT_BMP_INIT::EL_FIXEDSIZE != meSizeLimit)
			{
				return ERESULT_NOT_SET; // 不支持之外的限制方式
			}


			while (TRUE)
			{
				m_hResult = EinkuiGetSystem()->GetDWriteFactory()->CreateTextLayout(
					mpText,
					mdwTextLen,
					lpTextFormat,
					lfWidthLimit,
					lfHeightLimit,
					&lpTextLayout
					);
				if(FAILED(m_hResult))
					return ERESULT_BMP_CREATETEXTLAYOUT;
				if(STETXT_BMP_INIT::EL_FIXEDSIZE != meSizeLimit)
					break;

				DWRITE_TEXT_METRICS dtm;
				m_hResult = lpTextLayout->GetMetrics(&dtm);
				if(FAILED(m_hResult))
					return ERESULT_BMP_GETMETRICS;
				if(dtm.height > dtm.layoutHeight) {
					if(mdwTextLen > 4) {
						BackOneChar();
						CMM_SAFE_RELEASE(lpTextLayout);
					}
					else
						break;
				} else {
					break;
				}
			}

			// 获取实际宽高
			DWRITE_TEXT_METRICS dtm;
			m_hResult = lpTextLayout->GetMetrics(&dtm);
			if(FAILED(m_hResult))
				return ERESULT_BMP_GETMETRICS;

			// 创建WICBitmap
			D2D1_SIZE_U lstLayoutSize = D2D1::SizeU(static_cast<UINT32>(dtm.width), static_cast<UINT32>(dtm.height));
			if(STETXT_BMP_INIT::EL_FIXEDWIDTH == meSizeLimit)
			{
				if(dtm.layoutWidth > dtm.width)
					lstLayoutSize.width = static_cast<UINT32>(dtm.layoutWidth);
			}
			if(STETXT_BMP_INIT::EL_FIXEDSIZE == meSizeLimit)
			{
				if(dtm.layoutWidth > dtm.width)
					lstLayoutSize.width = static_cast<UINT32>(dtm.layoutWidth);
				if(dtm.layoutHeight > dtm.height)
					lstLayoutSize.height = static_cast<UINT32>(dtm.layoutHeight);
			}

			m_hResult = EinkuiGetSystem()->GetWICFactory()->CreateBitmap(lstLayoutSize.width,
				lstLayoutSize.height,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapCacheOnLoad,
				&m_pWICBitmap);
			if(FAILED(m_hResult))
				return ERESULT_BMP_CREATEWICBMP;
			m_hResult = EinkuiGetSystem()->GetD2dFactory()->CreateWicBitmapRenderTarget(
				m_pWICBitmap,
				D2D1::RenderTargetProperties(),
				&lpBitmapRT);
			if(FAILED(m_hResult))
				return ERESULT_BMP_CREATEWICBMPRT;

			m_hResult = lpBitmapRT->CreateSolidColorBrush(
				D2D1::ColorF(mdwFontColor),
				&lpFontBrush);
			if(FAILED(m_hResult))
				return ERESULT_BMP_CREATEBMPBRUSH;

			lpBitmapRT->BeginDraw();
			lpBitmapRT->DrawTextLayout(D2D1::Point2F(), 
				lpTextLayout,
				lpFontBrush);
			m_hResult = lpBitmapRT->EndDraw();
			if(FAILED(m_hResult))
				return ERESULT_BMP_DRAWRT;

			GetBitmapDimension();
			GenTransparentInfo();
		}
		__finally
		{
			CMM_SAFE_RELEASE(lpTextFormat);
			CMM_SAFE_RELEASE(lpTextLayout);
			CMM_SAFE_RELEASE(lpBitmapRT);
			CMM_SAFE_RELEASE(lpFontBrush);
		}
	}

	return ERESULT_SUCCESS;
}

VOID CXuiTextBitmap::BackOneChar()
{
	mpText[mdwTextLen-2] = L'.';
	mpText[mdwTextLen-3] = L'.';
	mpText[mdwTextLen-4] = L'.';
	mpText[mdwTextLen-1] = L'\0';
	mdwTextLen -= 1;
}
VOID CXuiTextBitmap::MapAlignMode(STETXT_BMP_INIT::eTEXTALIGN t, STETXT_BMP_INIT::ePARAALIGN p)
{
	switch(t)
	{
	case STETXT_BMP_INIT::EL_TEXTALIGN_LEADING:
		meTextAlignMode = DWRITE_TEXT_ALIGNMENT_LEADING;
		break;
	case STETXT_BMP_INIT::EL_TEXTALIGN_TRAILING:
		meTextAlignMode = DWRITE_TEXT_ALIGNMENT_TRAILING;
		break;
	case STETXT_BMP_INIT::EL_TEXTALIGN_CENTER:
	default:
		meTextAlignMode = DWRITE_TEXT_ALIGNMENT_CENTER;
	}

	switch(p)
	{
	case STETXT_BMP_INIT::EL_PARAALIGN_FAR:
		meParaAlignMode = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
		break;
	case STETXT_BMP_INIT::EL_PARAALIGN_NEAR:
		meParaAlignMode = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
		break;
	case STETXT_BMP_INIT::EL_PARAALIGN_CENTER:
	default:
		meParaAlignMode = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	}
	if(meSizeLimit == STETXT_BMP_INIT::EL_FREESIZE)
	{
		meParaAlignMode = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
		meTextAlignMode = DWRITE_TEXT_ALIGNMENT_LEADING;
	}
}


ERESULT CXuiTextBitmap::GetD2DObject(IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap)
{
	return CXD2dBitmap::GetD2DObject(npRT, nppParentBitmap);
}

FLOAT CXuiTextBitmap::CalcDIPFromPixel(ID2D1RenderTarget* npRt, DWORD ndwPixel)
{
	// fixed dpi
	// npRt->GetDpi(&dpiX, &dpiY);
	return (FLOAT)(ndwPixel * 96) / 96.0f;
}