/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include <d2d1.h>
#include <dwrite.h>
#include <Wincodec.h>
#include <d2d1helper.h>
#include <ShellAPI.h>

#include "cmmstruct.h"
#include "Eui.h"
#include "cmmPath.h"
#include "EsWgtContext.h"
#include "EsBmpList.h"
#include "euiimp.h"
#include "ElementImp.h"
#include "EleIteratorImp.h"
#include "ElMsgImp.h"
#include "EsMsgQueue.h"
#include "EleMgrImp.h"
#include "ElBitmapImp.h"

#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "Windowscodecs.lib")

template <typename T>
inline void SafeRelease(T *&pI)
{
	if (NULL != pI)
	{
		pI->Release();
		pI = NULL;
	}
}


class CHICon {
	HICON mhIcon;
	int mnSize;
	static SIZE naPredefinedSize[6];

	BOOL GetSpecifiedSizedIcon(HINSTANCE hMod, WORD nwIconIdx, int niSize);
	BOOL GetIconByResId(wchar_t *npPeFile, int nwResId);
	BOOL GetIconByIndex(wchar_t *npPeFile, int nwResIdx);

public:
	CHICon();
	~CHICon();

	HICON GetIconHandle();
	int GetIconSize();

	// nwIconIdx >= 0，说明为图标索引
	// nwIconIdx <0， 说明其绝对值为图标资源ID
	BOOL GetSizedIcon(wchar_t *npPeFile, int nwIconIdx, int cx, int cy);

};


SIZE CHICon::naPredefinedSize[6] = {
	{0, 0},
	{256, 256},
	{128, 128},
	{64, 64},
	{32, 32},
	{16, 16} };

CHICon::CHICon()
{
	mhIcon = NULL;
	mnSize = 0;
}

CHICon::~CHICon()
{
	if(mhIcon) {
		DestroyIcon(mhIcon);
		mhIcon = NULL;
	}
}

HICON CHICon::GetIconHandle()
{
	return mhIcon;
}

int CHICon::GetIconSize()
{
	return mnSize;
}

BOOL CHICon::GetSizedIcon(wchar_t *npPeFile, int niIconIdx, int niXSize, int niYSize)
{
	BOOL fResult = FALSE;

	wchar_t wsRealFilePath[MAX_PATH+1] = L"";
	ExpandEnvironmentStrings(npPeFile, wsRealFilePath, MAX_PATH);

	naPredefinedSize[0].cx = niXSize;
	naPredefinedSize[0].cy = niYSize;

	if(niIconIdx < 0) {
		fResult = GetIconByResId(wsRealFilePath, -niIconIdx);
	} else {
		fResult = GetIconByIndex(wsRealFilePath, niIconIdx);
	}

	return fResult;
}

BOOL CHICon::GetIconByIndex(wchar_t *npPeFile, int nwResIdx)
{
	BOOL fReturn = FALSE;
	UINT nResId = 0;
	UINT uiCount;

	for(int i= 0; i< _countof(naPredefinedSize); i++) {
		uiCount = PrivateExtractIcons(npPeFile, nwResIdx, naPredefinedSize[i].cx, naPredefinedSize[i].cy, &mhIcon, &nResId, 1, LR_SHARED);
		if(uiCount == 1 && mhIcon != NULL) {
			fReturn = TRUE;
			break;
		}
	}

	return fReturn;
}

BOOL CHICon::GetIconByResId(wchar_t *npPeFile, int nwResId)
{
	BOOL fResult = FALSE;

	HINSTANCE hMod = LoadLibraryEx(npPeFile, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	if(hMod != NULL) {
		for(int i = 0; i< _countof(naPredefinedSize); i++) {
			if(GetSpecifiedSizedIcon(hMod, nwResId, naPredefinedSize[i].cx)) {
				mnSize = naPredefinedSize[i].cx;
				fResult = TRUE;
				break;
			}
		}
		FreeLibrary(hMod);
	}

	return fResult;
}

BOOL CHICon::GetSpecifiedSizedIcon(HINSTANCE hMod, WORD nwIconResId, int niSize)
{
	UINT nIconToExtract = 1;
	UINT nIconId = 0;

	mhIcon = (HICON)LoadImage(hMod, MAKEINTRESOURCE(nwIconResId), IMAGE_ICON, niSize, niSize, LR_SHARED);
	if(mhIcon)
		return TRUE;
	return FALSE;
}

DEFINE_BUILTIN_NAME(CElBitmap)

CElBitmap::CElBitmap()
{
	m_hBitmap = NULL;
	m_pIDecoder = NULL;
	m_pWICBitmap = NULL;
	m_pIDecoder = NULL;
	m_pIDecoderFrame = NULL;
	m_pBmpScaler = NULL;
	mpTransparentInfo = NULL;
	mpDxgiSurface = NULL;
	m_lExtendLineX = m_lExtendLineY = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nScaleHeight = 0;
	m_nScaleWidth = 0;

	mpBitmap2D = NULL;

}

CElBitmap::~CElBitmap()
{
	CMM_SAFE_RELEASE(mpBitmap2D);
	CMM_SAFE_RELEASE(m_pIDecoder);
	CMM_SAFE_RELEASE(m_pIDecoderFrame);
	CMM_SAFE_RELEASE(m_pWICBitmap);
	CMM_SAFE_RELEASE(m_pBmpScaler);
	if(mpTransparentInfo)
		delete mpTransparentInfo;
}

ULONG CElBitmap::InitOnCreate(ID2D1RenderTarget* npRt, FLOAT nfWidth, FLOAT nfHeight)
{
	D2D1_SIZE_F lstBmpSrcSize = npRt->GetSize();

	return 0;

}

ULONG CElBitmap::InitOnCreate(wchar_t *npPeFileName, int niIconIdx, int niXSize, int niYSize)
{
	ULONG ulResult;

	HICON lhIconLarge = NULL, lhIconSmall = NULL;

	CHICon oIcon;

	if(!oIcon.GetSizedIcon(npPeFileName, niIconIdx, niXSize, niYSize))
		return ERESULT_BMP_EXSTRACTICON;

	m_hResult = EuiGetSystem()->GetWICFactory()->CreateBitmapFromHICON(oIcon.GetIconHandle(), &m_pWICBitmap);
	if(SUCCEEDED(m_hResult))
	{
		m_hResult = m_pWICBitmap->GetSize(&m_nWidth, &m_nHeight);
		if(SUCCEEDED(m_hResult)) {
			ulResult = GenTransparentInfo();
		} else {
			ulResult = ERESULT_BMP_ERRGETSIZE;
		}
	}
	else
	{
		ulResult = ERESULT_BMP_CREATEWICBMP;
	}

	DestroyIcon(lhIconLarge);
	DestroyIcon(lhIconSmall);

	return ulResult;
}

ULONG CElBitmap::InitOnCreate(IN const wchar_t *npFileName) {
	return Load(npFileName);
}

ULONG CElBitmap::InitOnCreate(IN const wchar_t *npText, IN DWORD dwTextColor, IN const wchar_t *npFont, IN DWORD dwFontSize) {
	ID2D1RenderTarget* lpBitmapRT = NULL;
	IDWriteTextFormat* lpTextFormat = NULL;
	IDWriteTextLayout* lpTextLayout = NULL;
	IDWriteFactory* lpDWriteFactory = NULL;
	ID2D1SolidColorBrush *lpBlackBrush = NULL;

	if(npText == NULL || npFont == NULL)
		return ERESULT_BMP_CREATEDWRITEFAC;

	__try {
		// Create a DirectWrite factory.
		m_hResult = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(lpDWriteFactory),
			reinterpret_cast<IUnknown **>(&lpDWriteFactory)
			);
		if(FAILED(m_hResult))
			return ERESULT_BMP_CREATEDWRITEFAC;
		FLOAT lfDIPs = 0.0f;
		FLOAT dpiX = 96.0f;
		FLOAT dpiY = 96.0F;
		//EuiGetSystem()->GetD2dFactory()->GetDesktopDpi(&dpiX, &dpiY); Modified by Ax
		lfDIPs = dwFontSize / (dpiX / 96.0f);
		m_hResult = lpDWriteFactory->CreateTextFormat(
			npFont,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			lfDIPs,
			L"", //locale
			&lpTextFormat);
		if(FAILED(m_hResult))
			return ERESULT_BMP_CREATETEXTFMT;
		lpTextFormat->SetTextAlignment(/*DWRITE_TEXT_ALIGNMENT_CENTER*/DWRITE_TEXT_ALIGNMENT_LEADING);
		lpTextFormat->SetParagraphAlignment(/*DWRITE_PARAGRAPH_ALIGNMENT_CENTER*/DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		m_hResult = lpDWriteFactory->CreateTextLayout(npText, wcslen(npText),
			lpTextFormat, 500, 500, &lpTextLayout);
		if(FAILED(m_hResult))
			return ERESULT_BMP_CREATETEXTLAYOUT;

		DWRITE_TEXT_METRICS dtm;
		m_hResult = lpTextLayout->GetMetrics(&dtm);
		if(FAILED(m_hResult))
			return ERESULT_BMP_GETMETRICS;

		m_hResult = EuiGetSystem()->GetWICFactory()->CreateBitmap(static_cast<UINT>(dtm.width+1),
			static_cast<UINT>(dtm.height + 1), // 取整
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&m_pWICBitmap);
		if(FAILED(m_hResult)) {
			return ERESULT_BMP_CREATEWICBMP;
		}

		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
			DXGI_FORMAT_B8G8R8A8_UNORM,
			D2D1_ALPHA_MODE_PREMULTIPLIED);
		m_hResult = EuiGetSystem()->GetD2dFactory()->CreateWicBitmapRenderTarget(
			m_pWICBitmap,
			D2D1::RenderTargetProperties(),
			&lpBitmapRT);
		if(FAILED(m_hResult))
			return ERESULT_BMP_CREATEWICBMPRT;

		m_hResult = lpBitmapRT->CreateSolidColorBrush(
			D2D1::ColorF(/*D2D1::ColorF::Red*/dwTextColor),
			&lpBlackBrush
			);

		D2D1_SIZE_F renderTargetSize = lpBitmapRT->GetSize();
		lpBitmapRT->BeginDraw();
		// 这行代码用于测试生成图片的尺寸是否正确
		// lpBitmapRT->DrawRectangle(D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), lpBlackBrush);
		lpBitmapRT->DrawText(npText, 
			wcslen(npText),
			lpTextFormat,
			D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
			lpBlackBrush);
		lpBitmapRT->EndDraw();

		GetBitmapDimension();

		GenTransparentInfo();
	}
	__finally {
		CMM_SAFE_RELEASE(lpBitmapRT);
		CMM_SAFE_RELEASE(lpTextFormat);
		CMM_SAFE_RELEASE(lpTextLayout);
		CMM_SAFE_RELEASE(lpDWriteFactory);
		CMM_SAFE_RELEASE(lpBlackBrush);
	}


	return ERESULT_SUCCESS;
}

int __stdcall CElBitmap::Release(void)
{

	int liCount = cmmBaseObject<CElBitmap, ILwBitmap, GET_BUILTIN_NAME(CElBitmap)>::Release();
	// 如果返回值是0，表示需要被删除
	if(liCount == 0)
	{
		CLwuiSystem::gpEuiSystem->GetBitmapList().UnregisteBitmap(this);
	}

	return liCount;
}

ULONG CElBitmap::Load(IN const wchar_t* npFileName) {

	m_nScaleHeight = 0;
	m_nScaleWidth = 0;

	ULONG ulResult = ERESULT_UNSUCCESSFUL;
	//////////////////////////////////////////////////////////////////////////
	// 从文件创建IWICBitmapDecoder接口
	SafeRelease(m_pIDecoderFrame);
	m_hResult = EuiGetSystem()->GetWICFactory()->CreateDecoderFromFilename(
		npFileName, 
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&m_pIDecoder);

	if(SUCCEEDED(m_hResult)) {
		//////////////////////////////////////////////////////////////////////////
		// 从IWICBitmapDecoder中获取第一帧
		SafeRelease(m_pIDecoderFrame);
		m_hResult = m_pIDecoder->GetFrame(0, &m_pIDecoderFrame);
		if(SUCCEEDED(m_hResult)) {
			//////////////////////////////////////////////////////////////////////////
			// 将IWICBitmapDecoder转换为IWICBitmap接口
			SafeRelease(m_pWICBitmap);
			m_hResult = EuiGetSystem()->GetWICFactory()->CreateBitmapFromSource(
				m_pIDecoderFrame,
				WICBitmapCacheOnDemand,
				&m_pWICBitmap);
			if(SUCCEEDED(m_hResult)) {
				//////////////////////////////////////////////////////////////////////////
				// 获取IWICBitmapLock对象，取位图宽高信息
				m_hResult = m_pWICBitmap->GetSize(&m_nWidth, &m_nHeight);
				if(SUCCEEDED(m_hResult)) {
					ulResult = GenTransparentInfo();
				}
				else
					ulResult = ERESULT_BMP_ERRGETSIZE;
			} else
				ulResult = ERESULT_BMP_CREATEWICBMP;
		} else {
			ulResult = ERESULT_BMP_GETFRAME;
		}
	} else {
		ulResult = ERESULT_BMP_CREATEDECODER;
	}
	return ulResult;
}

ERESULT CElBitmap::GenTransparentInfo() {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	if(mpTransparentInfo) {
		delete mpTransparentInfo;
		mpTransparentInfo = NULL;
		return ulResult;
	}
	mpTransparentInfo = new CBitArray(m_nHeight * m_nWidth);
	if(mpTransparentInfo == NULL) {
		return ERESULT_INSUFFICIENT_RESOURCES;
	}

	WICRect rcLock = { 0, 0, m_nWidth, m_nHeight };
	IWICBitmapLock *pLocker = NULL;
	m_hResult = m_pWICBitmap->Lock(
		&rcLock, WICBitmapLockRead, 
		&pLocker);
	if(SUCCEEDED(m_hResult)) {
		WICPixelFormatGUID pf;
		m_hResult = pLocker->GetPixelFormat(&pf);
		if(SUCCEEDED(m_hResult)) {
			UINT nSize = 0;
			BYTE *pBuf = NULL;
			m_hResult = pLocker->GetDataPointer(&nSize, &pBuf);
			if(SUCCEEDED(m_hResult)) {
				// 使用下面方法判断每个点的透明度
				// 避免每次在循环内判断GUID影响图形加载效率
				// 但代码看起来比较冗余
				if(IsEqualGUID(GUID_WICPixelFormat32bppBGRA, pf)) {
					for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
						mpTransparentInfo->SetBit(n, (pBuf[n*4 + 3] > TRANSPARENT_THRESHOLD_VALUE) ? true : false);
					}
				} else if(IsEqualGUID(GUID_WICPixelFormat32bppPBGRA, pf)) {
					for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
						mpTransparentInfo->SetBit(n, (pBuf[n*4 + 3] > TRANSPARENT_THRESHOLD_VALUE) ? true : false);
					}
				} else if(IsEqualGUID(GUID_WICPixelFormat32bppRGBA, pf)) {
					for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
						mpTransparentInfo->SetBit(n, (pBuf[n*4 + 3] > TRANSPARENT_THRESHOLD_VALUE) ? true : false);
					}
				} else if(IsEqualGUID(GUID_WICPixelFormat32bppPRGBA, pf)) {
					for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
						mpTransparentInfo->SetBit(n, (pBuf[n*4 + 3] > TRANSPARENT_THRESHOLD_VALUE) ? true : false);
					}
				} else {
					for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
						mpTransparentInfo->SetBit(n, true);
					}
				}
				ulResult = ERESULT_SUCCESS;
			} else {
				ulResult = ERESULT_BMP_GETBMPBUF;
			}
		} else {
			for(UINT n=0; n< m_nWidth*m_nHeight; n++) {
				mpTransparentInfo->SetBit(n, true);
			}
		}
		pLocker->Release();
	} else {
		ulResult = ERESULT_BMP_WICLOCKBMP;
	}
	return ulResult;

}

ULONG CElBitmap::GetBitmapDimension() {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;
	IWICBitmapLock *lpLock = NULL;
	m_hResult = m_pWICBitmap->Lock(NULL, WICBitmapLockRead, &lpLock);
	if(SUCCEEDED(m_hResult)) {
		m_hResult = lpLock->GetSize(&m_nWidth, &m_nHeight);
		if(SUCCEEDED(m_hResult)) {
			ulResult = ERESULT_SUCCESS;
		} else {
			m_nHeight = 0;
			m_nWidth = 0;
			ulResult = ERESULT_BMP_ERRGETSIZE;
		}
		lpLock->Release();
	} else
		ulResult = ERESULT_BMP_WICLOCKBMP;
	return ulResult;
}

ERESULT __stdcall CElBitmap::GetWICObject(OUT IWICBitmap** ppWicBitmap) {
	*ppWicBitmap = m_pWICBitmap;
	return ERESULT_SUCCESS;
}

UINT __stdcall CElBitmap::GetWidth() 
{
	return m_nScaleWidth ? m_nScaleWidth : m_nWidth;
}

UINT __stdcall CElBitmap::GetHeight() 
{
	return m_nScaleHeight ? m_nScaleHeight : m_nHeight;
}

ERESULT __stdcall CElBitmap::GetD2DObject(IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap)
{
	//if(m_nScaleWidth && m_nScaleHeight) {
	//	return GetScaledD2DObject(m_nScaleWidth, m_nScaleHeight, npRT, nppParentBitmap);
	//}
	ERESULT luResult;

	if(mpBitmap2D == NULL)
	{
		luResult = CreateD2DObjFromBmpSource(npRT, m_pIDecoderFrame,&mpBitmap2D);
		if(ERESULT_FAILED(luResult))
			return luResult;
	}

	if(mpBitmap2D == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	mpBitmap2D->AddRef();

	*nppParentBitmap = mpBitmap2D;

	return ERESULT_SUCCESS;
}

ERESULT __stdcall CElBitmap::GetPixel(IN DWORD x, IN DWORD y, DWORD &nPixel)
{
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	if(mpTransparentInfo) {
		bool lfIsTrans = mpTransparentInfo->GetBit(y*m_nWidth + x);
		nPixel = lfIsTrans;
		ulResult = ERESULT_SUCCESS;
	}
	return ulResult;
}

//ERESULT CElBitmap::GetScaledD2DObject(
//	IN UINT npWidth, IN UINT npHeight,
//	IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap) {
//		ULONG ulResult = ERESULT_UNSUCCESSFUL;
//
//		IWICBitmapScaler *lpIScaler = NULL;
//		m_hResult = m_pWICFactory->CreateBitmapScaler(&lpIScaler);
//		if(SUCCEEDED(m_hResult)) {
//			if(m_pIDecoderFrame)
//				m_hResult = lpIScaler->Initialize(
//				m_pIDecoderFrame,
//				npWidth,
//				npHeight,
//				WICBitmapInterpolationModeFant);
//			else
//				m_hResult = lpIScaler->Initialize(
//				m_pWICBitmap,
//				npWidth,
//				npHeight,
//				WICBitmapInterpolationModeFant);
//
//			if(SUCCEEDED(m_hResult)) {
//				ulResult = CreateD2DObjFromBmpSource(npRT, lpIScaler, nppParentBitmap);
//			} else {
//				ulResult = ERESULT_BMP_INITSCALER;
//			}
//			SafeRelease(lpIScaler);
//		} else {
//			ulResult = ERESULT_BMP_CREATESCALER;
//		}
//
//
//		return ulResult;
//	
//}

ERESULT __stdcall CElBitmap::GetBmpInfo(OUT BITMAP* npBmpInfo)
{
	return ERESULT_UNSUCCESSFUL; // Not implemented yet.

	if(NULL == m_hBitmap) {
		return -1;
	}
	::GetObject(m_hBitmap, sizeof(BITMAP), npBmpInfo);
	return 0;
}

ERESULT CElBitmap::CreateD2DObjFromBmpSource(IN ID2D1RenderTarget *npRT, IN IWICBitmapSource *npSourceBmp, OUT ID2D1Bitmap **nppParentBitmap) {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	IWICBitmapSource* pWicBmpToGet = npSourceBmp;
	if(pWicBmpToGet == NULL)
		pWicBmpToGet = m_pWICBitmap;

	if(m_nScaleWidth && m_nScaleHeight && (NULL != m_pBmpScaler))
	{
		pWicBmpToGet = m_pBmpScaler;
	}

	IWICFormatConverter *lpFormatConverter = NULL;
	m_hResult = EuiGetSystem()->GetWICFactory()->CreateFormatConverter(&lpFormatConverter);
	if(SUCCEEDED(m_hResult)) {
		m_hResult = lpFormatConverter->Initialize(
			pWicBmpToGet,				    // Input bitmap to convert
			GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
			WICBitmapDitherTypeNone,         // Specified dither pattern
			NULL,                            // Specify a particular palette 
			0.f,                             // Alpha threshold
			WICBitmapPaletteTypeCustom       // Palette translation type
			);
		if(SUCCEEDED(m_hResult)) {
			m_hResult = npRT->CreateBitmapFromWicBitmap(lpFormatConverter, NULL, nppParentBitmap);
			if(SUCCEEDED(m_hResult)) {
				ulResult = ERESULT_SUCCESS;
			} else {
				ulResult = ERESULT_BMP_CREATED2DBMP;
			}
		} else {
			ulResult = ERESULT_BMP_INITCVTER;
		}
		SafeRelease(lpFormatConverter);
	} else {
		ulResult = ERESULT_BMP_CREATECVTER;
	}

	return ulResult;
}

// 供系统内部调用，废弃Direct2D的设备相关资源
void __stdcall CElBitmap::Discards2DResource(void)
{
	CMM_SAFE_RELEASE(mpBitmap2D);
	CMM_SAFE_RELEASE(m_pBmpScaler);
}


ERESULT __stdcall CElBitmap::Scale(UINT npWidth, UINT npHeight) {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	m_nScaleWidth = npWidth;
	m_nScaleHeight = npHeight;

	Discards2DResource();

	IWICBitmapSource* pWicBmpToGet = m_pIDecoderFrame;
	if(pWicBmpToGet == NULL)
		pWicBmpToGet = m_pWICBitmap;

	HRESULT m_hResult = EuiGetSystem()->GetWICFactory()->CreateBitmapScaler(&m_pBmpScaler);
	if(SUCCEEDED(m_hResult)) {
		m_hResult = m_pBmpScaler->Initialize(
			pWicBmpToGet,
			m_nScaleWidth,
			m_nScaleHeight,
			WICBitmapInterpolationModeFant);
		if(SUCCEEDED(m_hResult)) {
			pWicBmpToGet = m_pBmpScaler;
			ulResult = ERESULT_SUCCESS;
		}
		else
			ulResult = ERESULT_BMP_INITSCALER;
	} else {
		ulResult = ERESULT_BMP_CREATESCALER;
	}
	GetBitmapDimension();
	GenTransparentInfo();

	return ulResult;
}

//设置延展线
void __stdcall CElBitmap::SetExtendLine(IN LONG nlX,IN LONG nlY)
{
	m_lExtendLineX = nlX;
	m_lExtendLineY = nlY;
}

//获取横向延展线
LONG __stdcall CElBitmap::GetExtnedLineX(void)
{
	return m_lExtendLineX;
}
//获取纵向延展线
LONG __stdcall CElBitmap::GetExtnedLineY(void)
{
	return m_lExtendLineY;
}