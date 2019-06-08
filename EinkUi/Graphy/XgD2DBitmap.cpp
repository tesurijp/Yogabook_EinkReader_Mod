/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include <math.h>
#include "shlwapi.h"
#include <string>
#include "CommonHeader.h"

#include "XgD2DBitmap.h"


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
	{16, 16} 
};

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



DEFINE_BUILTIN_NAME(CXD2dBitmap)

CXD2dBitmap::CXD2dBitmap()
{
	m_hBitmap = NULL;
	m_pIDecoder = NULL;
	m_pWICBitmap = NULL;
	m_pIDecoder = NULL;
	m_pIDecoderFrame = NULL;
	m_pBmpScaler = NULL;
	mpTransparentInfo = NULL;
	m_lExtendLineX = m_lExtendLineY = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nScaleHeight = 0;
	m_nScaleWidth = 0;

	mpBitmap2D = NULL;
	meBitmapType = None;
	mcIsInit = false;

	// 位图数据初始化
	memset(mswFileName, 0, MAX_PATH*sizeof(wchar_t));

	//mpText = NULL;
	//mpFont = NULL;

}

CXD2dBitmap::~CXD2dBitmap()
{
	CMM_SAFE_RELEASE(mpBitmap2D);
	CMM_SAFE_RELEASE(m_pIDecoder);
	CMM_SAFE_RELEASE(m_pIDecoderFrame);
	CMM_SAFE_RELEASE(m_pWICBitmap);
	CMM_SAFE_RELEASE(m_pBmpScaler);
	
	if(mpTransparentInfo)
		delete mpTransparentInfo;
	
	//if (mpText != NULL)
	//	delete mpText;
	//if (mpFont != NULL)
	//	delete mpFont;

}


ERESULT CXD2dBitmap::InitBitmap(IWICImagingFactory* npWICFactory, ID2D1Factory* npD2D1Factory, ID2D1RenderTarget* npRenderTarget)
{

	ERESULT luStatus = ERESULT_UNSUCCESSFUL;
	
	// 如果已经初始化，则直接返回成功
	if (mcIsInit == true)
		return ERESULT_SUCCESS;

	// 暂存渲染对象
	mpTempRenderTarget = npRenderTarget;

	UNREFERENCED_PARAMETER(npWICFactory);
	UNREFERENCED_PARAMETER(npD2D1Factory);
	UNREFERENCED_PARAMETER(npRenderTarget);

	switch(meBitmapType)
	{
	case CommonBitmap:
		{
//			luStatus = InitCommonBitmap(npWICFactory);
			luStatus = ERESULT_SUCCESS;
			break;
		}
	case IconBitmap:
		{
//			luStatus = InitIconBitmap(npWICFactory);
			luStatus = ERESULT_SUCCESS;
			break;
		}
	//case TextBitmap:
	//	{
	//		// 通过文字实例化位图，需要采用后加载模型
	//		luStatus = InitTextBitmap(npWICFactory, npD2D1Factory);
	//		break;
	//	}
	case MemoryBitmap:
		{
//			luStatus = InitMemoryBitmap(npRenderTarget);
			luStatus = ERESULT_SUCCESS;
			break;
		}
	case None:
		{
			luStatus = ERESULT_SUCCESS;
			break;
		}
	}
	
	// 如果初始化成功，则置位
	if (ERESULT_SUCCEEDED(luStatus))
		mcIsInit = true;

	return luStatus;

}

// 复制出一个新的对象,复制出来的对象，需要使用者自己释放
IEinkuiBitmap* __stdcall CXD2dBitmap::DuplicateBitmap(void)
{

	IEinkuiBitmap* lpNewBitmap = NULL;
	void*	lpBitmapBuffer = NULL;
	bool	lbStatus = false;

	// 不管是哪种类型的位图，都统一转换成内存型位图处理
	do 
	{
		lpBitmapBuffer = new char[m_nWidth * m_nHeight*4+4];
		BREAK_ON_NULL(lpBitmapBuffer);

		lbStatus = GetBitmapBuffer(D2D1::RectF(0.0f, 0.0f, (float)m_nWidth, (float)m_nHeight), lpBitmapBuffer);
		BREAK_ON_FALSE(lbStatus);

		lpNewBitmap =  CXD2dBitmap::CreateInstance(m_nWidth, m_nHeight,4,m_nWidth*4,lpBitmapBuffer);

	} while (false);

	CMM_SAFE_DELETE(lpBitmapBuffer);

	return lpNewBitmap;

}


// 指定位图大小，以及位图数据，创建一个位图对象
ULONG CXD2dBitmap::InitOnCreate(LONG nuWidth, LONG nuHeight,LONG PixelSize, LONG Stride, void* npRawData)
{
	ERESULT luStatus = ERESULT_UNSUCCESSFUL;
	meBitmapType = MemoryBitmap;
	IWICImagingFactory* lpWicFactory = NULL; 

	do 
	{
		// 向系统注册这个对象，用于统一释放设备相关资源
		CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(this);

		BREAK_ON_NULL(npRawData);

		if (PixelSize < 3 || PixelSize > 4)
		{
			luStatus = ERESULT_BMP_IVLDFORMAT;
			break;
		}

		// 获取WIC工厂类
		lpWicFactory = EinkuiGetSystem()->GetWICFactory();
		BREAK_ON_NULL(lpWicFactory);

		// 用WIC接口从内存数据直接生成位图
		luStatus = EinkuiGetSystem()->GetWICFactory()->CreateBitmapFromMemory(
			static_cast<unsigned int>(nuWidth), static_cast<unsigned int>(nuHeight),
			(PixelSize==3?GUID_WICPixelFormat24bppBGR:GUID_WICPixelFormat32bppPBGRA),
			Stride, nuWidth*nuHeight*PixelSize, (PBYTE)npRawData,
			&m_pWICBitmap);

		BREAK_ON_FAILED(luStatus);
		BREAK_ON_NULL(m_pWICBitmap);

		// 利用WIC接口获取图片宽高
		GetBitmapDimension();
		GenTransparentInfo();

		luStatus = ERESULT_SUCCESS;

	} while (false);


	return luStatus;
}

ERESULT CXD2dBitmap::InitMemoryBitmap(ID2D1RenderTarget* npRenderTarget)
{

	ERESULT luStatus = ERESULT_UNSUCCESSFUL;

	// 内存型位图，没必要采用后加载模型
	do 
	{
		UNREFERENCED_PARAMETER(npRenderTarget);

		luStatus = ERESULT_SUCCESS;
	} while (false);

	return luStatus;
}

// 通过Icon创建位图对象
ULONG CXD2dBitmap::InitOnCreate(wchar_t *npPeFileName, int niIconIdx, int niXSize, int niYSize)
{

	HRESULT luResult;
	CHICon oIcon;

	do 
	{
		// 向系统注册这个对象，用于统一释放设备相关资源
		CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(this);

		BREAK_ON_FALSE(oIcon.GetSizedIcon(npPeFileName, niIconIdx, niXSize, niYSize));
		
		// 通过WIC接口，从ICON图标创建位图
		luResult = EinkuiGetSystem()->GetWICFactory()->CreateBitmapFromHICON(oIcon.GetIconHandle(), &m_pWICBitmap);
		BREAK_ON_FAILED(luResult);

		luResult = m_pWICBitmap->GetSize(&m_nWidth, &m_nHeight);
		BREAK_ON_FAILED(luResult);

		luResult = GenTransparentInfo();

	} while (false);

	return luResult;

}

// 初始化ICON型位图对象
ERESULT CXD2dBitmap::InitIconBitmap(IWICImagingFactory* npWICFactory)
{

	ERESULT luStatus = ERESULT_UNSUCCESSFUL;

	// ICON型位图，没必要采用后加载模型
	do 
	{

		luStatus = ERESULT_SUCCESS;
	} while (false);

	return luStatus;

}

// 通过位图文件实例化位图对象
ULONG CXD2dBitmap::InitOnCreate(IN const wchar_t *npFileName) 
{
	// 向系统注册这个对象，用于统一释放设备相关资源
	CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(this);

	meBitmapType = CommonBitmap;
	memcpy(mswFileName, npFileName, (int)wcslen(npFileName)*sizeof(wchar_t));

	return InitCommonBitmap(EinkuiGetSystem()->GetWICFactory());

}

// 初始化普通位图对象
ERESULT CXD2dBitmap::InitCommonBitmap(IWICImagingFactory* npWICFactory)
{

	m_nScaleHeight = 0;
	m_nScaleWidth = 0;

	ULONG ulResult = ERESULT_UNSUCCESSFUL;
	//////////////////////////////////////////////////////////////////////////
	// 从文件创建IWICBitmapDecoder接口
	SafeRelease(m_pIDecoderFrame);
	m_hResult = npWICFactory->CreateDecoderFromFilename(
		mswFileName, 
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
			m_hResult = npWICFactory->CreateBitmapFromSource(
				m_pIDecoderFrame,
				WICBitmapCacheOnDemand,
				&m_pWICBitmap); 
			if(SUCCEEDED(m_hResult)) 
			{
				//////////////////////////////////////////////////////////////////////////
				// 获取IWICBitmapLock对象，取位图宽高信息
				m_hResult = m_pWICBitmap->GetSize(&m_nWidth, &m_nHeight);
				if(SUCCEEDED(m_hResult)) 
				{
					ulResult = GenTransparentInfo();
				}
				else
					ulResult = ERESULT_BMP_ERRGETSIZE;
			} else
				ulResult = ERESULT_BMP_CREATEWICBMP;
		} else 
		{
			ulResult = ERESULT_BMP_GETFRAME;
		}
	} else {
		ulResult = ERESULT_BMP_CREATEDECODER;
	}

	return ulResult;

}

//	好像从来没有用过，屏蔽了 ax nov.28,2017
//ULONG CXD2dBitmap::InitOnCreate(IN const wchar_t *npText, IN DWORD dwTextColor, IN const wchar_t *npFont, IN DWORD dwFontSize) 
//{
//	// 向系统注册这个对象，用于统一释放设备相关资源
//	CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(this);
//
//	if(npText == NULL || npFont == NULL)
//		return ERESULT_BMP_CREATEDWRITEFAC;
//
//	meBitmapType = TextBitmap;
//
//	// 分配文字内容所占用的内存
//	mpText = new wchar_t[(int)wcslen(npText)];
//	memcpy(mpText, npText, (int)wcslen(npText)*sizeof(wchar_t));
//
//	mpFont = new wchar_t[(int)wcslen(npFont)];
//	memcpy(mpFont, npFont, (int)wcslen(npFont)*sizeof(wchar_t));
//
//	mdwTextColor = dwTextColor;
//	mdwFontSize = dwFontSize;
//		
//	return 0;
////	return InitTextBitmap(EinkuiGetSystem()->GetWICFactory(), EinkuiGetSystem()->GetD2dFactory());
//
//}

//// 初始化文字型位图对象			好像从来没有用过，屏蔽了 ax nov.28,2017
//ERESULT CXD2dBitmap::InitTextBitmap(IWICImagingFactory* npWICFactory, ID2D1Factory* npD2D1Factory)
//{
//	ID2D1RenderTarget* lpBitmapRT = NULL;
//	IDWriteTextFormat* lpTextFormat = NULL;
//	IDWriteTextLayout* lpTextLayout = NULL;
//	IDWriteFactory* lpDWriteFactory = NULL;
//	ID2D1SolidColorBrush *lpBlackBrush = NULL;
//
//
//	__try {
//		// Create a DirectWrite factory.
//		m_hResult = DWriteCreateFactory(
//			DWRITE_FACTORY_TYPE_SHARED,
//			__uuidof(lpDWriteFactory),
//			reinterpret_cast<IUnknown **>(&lpDWriteFactory)
//			);
//		if(FAILED(m_hResult))
//			return ERESULT_BMP_CREATEDWRITEFAC;
//		
//		FLOAT lfDIPs = 0.0f;
//		FLOAT dpiX = 96.0f;
//		FLOAT dpiY = 96.0F;
//
//		//EinkuiGetSystem()->GetD2dFactory()->GetDesktopDpi(&dpiX, &dpiY); Modified by Ax
//		lfDIPs = mdwFontSize / (dpiX / 96.0f);
//		m_hResult = lpDWriteFactory->CreateTextFormat(
//			mpFont,
//			NULL,
//			DWRITE_FONT_WEIGHT_NORMAL,
//			DWRITE_FONT_STYLE_NORMAL,
//			DWRITE_FONT_STRETCH_NORMAL,
//			lfDIPs,
//			L"", //locale
//			&lpTextFormat);
//		if(FAILED(m_hResult))
//			return ERESULT_BMP_CREATETEXTFMT;
//		lpTextFormat->SetTextAlignment(/*DWRITE_TEXT_ALIGNMENT_CENTER*/DWRITE_TEXT_ALIGNMENT_LEADING);
//		lpTextFormat->SetParagraphAlignment(/*DWRITE_PARAGRAPH_ALIGNMENT_CENTER*/DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
//
//		m_hResult = lpDWriteFactory->CreateTextLayout(mpText, (UINT32)wcslen(mpText),
//			lpTextFormat, 500, 500, &lpTextLayout);
//		if(FAILED(m_hResult))
//			return ERESULT_BMP_CREATETEXTLAYOUT;
//
//		DWRITE_TEXT_METRICS dtm;
//		m_hResult = lpTextLayout->GetMetrics(&dtm);
//		if(FAILED(m_hResult))
//			return ERESULT_BMP_GETMETRICS;
//
//		m_hResult = npWICFactory->CreateBitmap(static_cast<UINT>(dtm.width+1),
//			static_cast<UINT>(dtm.height + 1), // 取整
//			GUID_WICPixelFormat32bppPBGRA,
//			WICBitmapCacheOnLoad,
//			&m_pWICBitmap);
//		if(FAILED(m_hResult)) {
//			return ERESULT_BMP_CREATEWICBMP;
//		}
//
//		D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
//			DXGI_FORMAT_B8G8R8A8_UNORM,
//			D2D1_ALPHA_MODE_PREMULTIPLIED);
//		m_hResult = npD2D1Factory->CreateWicBitmapRenderTarget(
//			m_pWICBitmap,
//			D2D1::RenderTargetProperties(),
//			&lpBitmapRT);
//		if(FAILED(m_hResult))
//			return ERESULT_BMP_CREATEWICBMPRT;
//
//		m_hResult = lpBitmapRT->CreateSolidColorBrush(
//			D2D1::ColorF(/*D2D1::ColorF::Red*/mdwTextColor),
//			&lpBlackBrush
//			);
//
//		//lpBitmapRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE); ??? 是否需要添加，这个类是给谁用的？Jun.14,2012
//
//		D2D1_SIZE_F renderTargetSize = lpBitmapRT->GetSize();
//		lpBitmapRT->BeginDraw();
//		lpBitmapRT->DrawText(mpText, 
//			(UINT32)wcslen(mpText),
//			lpTextFormat,
//			D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
//			lpBlackBrush);
//		lpBitmapRT->EndDraw();
//
//		GetBitmapDimension();
//
//		GenTransparentInfo();
//	}
//	__finally {
//		CMM_SAFE_RELEASE(lpBitmapRT);
//		CMM_SAFE_RELEASE(lpTextFormat);
//		CMM_SAFE_RELEASE(lpTextLayout);
//		CMM_SAFE_RELEASE(lpDWriteFactory);
//		CMM_SAFE_RELEASE(lpBlackBrush);
//	}
//
//	return ERESULT_SUCCESS;
//
//}

int __stdcall CXD2dBitmap::Release(void)
{

	int liCount = cmmBaseObject<CXD2dBitmap, IEinkuiBitmap, GET_BUILTIN_NAME(CXD2dBitmap)>::Release();
	// 如果返回值是0，表示需要被删除
	if(liCount == 0)
	{
		CEinkuiSystem::gpXuiSystem->GetBitmapList().UnregisteBitmap(this);
	}

	return liCount;

}


ERESULT CXD2dBitmap::GenTransparentInfo() {
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

	WICRect rcLock = { 0, 0, (INT)m_nWidth, (INT)m_nHeight };
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

ULONG CXD2dBitmap::GetBitmapDimension() {
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



UINT __stdcall CXD2dBitmap::GetWidth() 
{
	return m_nScaleWidth ? m_nScaleWidth : m_nWidth;
}

UINT __stdcall CXD2dBitmap::GetHeight() 
{
	return m_nScaleHeight ? m_nScaleHeight : m_nHeight;
}


ERESULT __stdcall CXD2dBitmap::GetWICObject(OUT IWICBitmap** ppWicBitmap)
{
	*ppWicBitmap = m_pWICBitmap;
	return ERESULT_SUCCESS;	
}

ERESULT __stdcall CXD2dBitmap::GetD2DObject(IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap)
{

	ERESULT luResult;

	if(mpBitmap2D == NULL)
	{
		luResult = CreateD2DObjFromBmpSource(npRT, m_pWICBitmap/*m_pIDecoderFrame*/,&mpBitmap2D);
		if(ERESULT_FAILED(luResult))
			return luResult;
	}

	if(mpBitmap2D == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	mpBitmap2D->AddRef();

	*nppParentBitmap = mpBitmap2D;

	return ERESULT_SUCCESS;
}

ERESULT __stdcall CXD2dBitmap::GetPixel(IN DWORD x, IN DWORD y, DWORD &nPixel)
{
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	if(mpTransparentInfo) {
		bool lfIsTrans = mpTransparentInfo->GetBit(y*m_nWidth + x);
		nPixel = lfIsTrans;
		ulResult = ERESULT_SUCCESS;
	}
	return ulResult;
}


ERESULT __stdcall CXD2dBitmap::GetBmpInfo(OUT BITMAP* npBmpInfo)
{
	return ERESULT_UNSUCCESSFUL; // Not implemented yet.

	if(NULL == m_hBitmap) {
		return -1;
	}
	::GetObject(m_hBitmap, sizeof(BITMAP), npBmpInfo);
	return 0;
}


ERESULT CXD2dBitmap::CreateD2DObjFromBmpSource(IN ID2D1RenderTarget *npRT, IN IWICBitmapSource *npSourceBmp, OUT ID2D1Bitmap **nppParentBitmap) {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	IWICBitmapSource* pWicBmpToGet = npSourceBmp;
	if(pWicBmpToGet == NULL)
		pWicBmpToGet = m_pWICBitmap;

	if(m_nScaleWidth && m_nScaleHeight && (NULL != m_pBmpScaler))
	{
		pWicBmpToGet = m_pBmpScaler;
	}
	// 添加异常捕获

	try
	{
		IWICFormatConverter *lpFormatConverter = NULL;

		do 
		{
			m_hResult = EinkuiGetSystem()->GetWICFactory()->CreateFormatConverter(&lpFormatConverter);
			BREAK_ON_FAILED(m_hResult);

			m_hResult = lpFormatConverter->Initialize(
				pWicBmpToGet,				    // Input bitmap to convert
				GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
				WICBitmapDitherTypeNone,         // Specified dither pattern
				NULL,                            // Specify a particular palette 
				0.0f,                             // Alpha threshold
				WICBitmapPaletteTypeCustom       // Palette translation type
				);
			BREAK_ON_FAILED(m_hResult);

			m_hResult = npRT->CreateBitmapFromWicBitmap(lpFormatConverter, NULL, nppParentBitmap);
			BREAK_ON_FAILED(m_hResult);


			ulResult = ERESULT_SUCCESS;

		} while (false);

		SafeRelease(lpFormatConverter);
		
	}
	catch (...)
	{
		*nppParentBitmap = NULL;
	}

	return ulResult;
}


// 供系统内部调用，废弃Direct2D的设备相关资源
void __stdcall CXD2dBitmap::DiscardsBitmapResource(void)
{
	CMM_SAFE_RELEASE(mpBitmap2D);
	CMM_SAFE_RELEASE(m_pBmpScaler);
	CMM_SAFE_RELEASE(m_pIDecoder);
	CMM_SAFE_RELEASE(m_pIDecoderFrame);

	mcIsInit = false;
}


ERESULT __stdcall CXD2dBitmap::Scale(UINT npWidth, UINT npHeight) {
	ULONG ulResult = ERESULT_UNSUCCESSFUL;

	m_nScaleWidth = npWidth;
	m_nScaleHeight = npHeight;

	DiscardsBitmapResource();

	IWICBitmapSource* pWicBmpToGet = m_pIDecoderFrame;
	if(pWicBmpToGet == NULL)
		pWicBmpToGet = m_pWICBitmap;

	HRESULT m_hResult = EinkuiGetSystem()->GetWICFactory()->CreateBitmapScaler(&m_pBmpScaler);
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
void __stdcall CXD2dBitmap::SetExtendLine(IN LONG nlX,IN LONG nlY)
{
	m_lExtendLineX = nlX;
	m_lExtendLineY = nlY;
}

//获取横向延展线
LONG __stdcall CXD2dBitmap::GetExtnedLineX(void)
{
	return m_lExtendLineX;
}

//获取纵向延展线
LONG __stdcall CXD2dBitmap::GetExtnedLineY(void)
{
	return m_lExtendLineY;
}


// 获取指定区域的位图数据
bool __stdcall CXD2dBitmap::GetBitmapBuffer(IN D2D1_RECT_F ndSrcRect, OUT void* npRawData)
{
	bool lbStatus = false;
	HRESULT	hr;
	IWICFormatConverter *lpFormatConverter = NULL;

	do 
	{
		// 如果图形对象没有初始化，则直接返回失败
		//BREAK_ON_FALSE(mcIsInit);
		BREAK_ON_NULL(m_pWICBitmap);
		BREAK_ON_NULL(npRawData);

		WICRect ldSrcWicRect;
		ldSrcWicRect.X = (INT)ndSrcRect.left;
		ldSrcWicRect.Y = (INT)ndSrcRect.top;
		ldSrcWicRect.Width = (INT)(ndSrcRect.right - ndSrcRect.left);
		ldSrcWicRect.Height = (INT)(ndSrcRect.bottom - ndSrcRect.top);

		// 裁剪区域不能大于原始区域
		BREAK_ON_FALSE(m_nWidth >= (UINT)ldSrcWicRect.Width);
		BREAK_ON_FALSE(m_nHeight >= (UINT)ldSrcWicRect.Height);

		// 首先解码，输出时转成BGRA的像素格式
		hr = EinkuiGetSystem()->GetWICFactory()->CreateFormatConverter(&lpFormatConverter);
		BREAK_ON_FAILED(hr);

		hr = lpFormatConverter->Initialize(
			m_pWICBitmap,					 // Input bitmap to convert
			GUID_WICPixelFormat32bppPBGRA,   // Destination pixel format
			WICBitmapDitherTypeNone,         // Specified dither pattern
			NULL,                            // Specify a particular palette 
			0.f,                             // Alpha threshold
			WICBitmapPaletteTypeCustom       // Palette translation type
			);
		BREAK_ON_FAILED(hr);

		// 第二个参数是stride,步幅（跨距)，表示位图中一行的像素数是多少，用于区分行与行的分割
		hr = lpFormatConverter->CopyPixels(
			&ldSrcWicRect, 
			ldSrcWicRect.Width*4,		
			ldSrcWicRect.Width*ldSrcWicRect.Height*4, 
			(BYTE*)npRawData
			);
		BREAK_ON_FAILED(hr);
		
		lbStatus = true;

	} while (false);


	CMM_SAFE_RELEASE(lpFormatConverter);

	return lbStatus;
}


bool __stdcall CXD2dBitmap::SaveAsThumbnail(IN LONG nuWidth, IN LONG nuHeight,  IN D2D1_RECT_F ndSampleRect, const wchar_t* nswOutputFilePath)
{
	bool lbStatus = false;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT hr = S_OK;
		
	IWICBitmap *lpWICBitmap = NULL;
	ID2D1RenderTarget *lpRT = NULL;
	IWICBitmapEncoder *lpEncoder = NULL;
	IWICBitmapFrameEncode *lpFrameEncode = NULL;
	IWICStream *lpStream = NULL;
	ID2D1Bitmap* lpBmp2d = NULL;	

	const UINT sc_bitmapWidth = (ULONG)nuWidth;
	const UINT sc_bitmapHeight = (ULONG)nuHeight;

	do 
	{
		// 创建WIC对象和RT对象
		hr = EinkuiGetSystem()->GetWICFactory()->CreateBitmap(
			sc_bitmapWidth,
			sc_bitmapHeight,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnLoad,
			&lpWICBitmap
			);
		BREAK_ON_FAILED(hr);

		hr = EinkuiGetSystem()->GetD2dFactory()->CreateWicBitmapRenderTarget(
			lpWICBitmap,
			D2D1::RenderTargetProperties(),
			&lpRT
			);
		BREAK_ON_FAILED(hr);

		// 根据原始图像的WIC对象，创建D2D位图对象
		luResult = CreateD2DObjFromBmpSource(lpRT, m_pWICBitmap,&lpBmp2d);
		BREAK_ON_FAILED(luResult);

		// 将原始位图绘制到RenderTarget,绘制后的区域和缩略图大小一样
		{
			lpRT->BeginDraw();

			// 必须要清屏，如果想创建带透明度信息的图，必须要把alpha全部清0
			lpRT->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));

			lpRT->DrawBitmap(
				lpBmp2d,
				D2D1::RectF(0.0f,0.0f,(FLOAT)nuWidth,(FLOAT)nuHeight),		// 目标区域大小
				1.0f,
				(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,
				ndSampleRect		// 原始区域大小
				);

			lpRT->EndDraw();
		}

		// 保存到文件，现在，RenderTarget上的内容就是缩略图
		hr = EinkuiGetSystem()->GetWICFactory()->CreateStream(&lpStream);
		BREAK_ON_FAILED(hr);

		//WICPixelFormatGUID format = GUID_WICPixelFormat32bppPBGRA;
		GUID containerFormat = GUID_ContainerFormatBmp;
		WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

		hr = lpStream->InitializeFromFilename(nswOutputFilePath, GENERIC_WRITE);
		BREAK_ON_FAILED(hr);

		std::wstring lpFileExtention = PathFindExtension(nswOutputFilePath);
		
		// 根据文件后缀，判断编码格式
		if (lpFileExtention.compare(L".jpg") == 0 ||
			lpFileExtention.compare(L".jpeg") == 0 ||
			lpFileExtention.compare(L".jpe") == 0 ||
			lpFileExtention.compare(L".jfif") == 0)
		{
			containerFormat = GUID_ContainerFormatJpeg;
		}
		else if (lpFileExtention.compare(L".tif") == 0 ||
			lpFileExtention.compare(L".tiff") == 0)
		{
			containerFormat = GUID_ContainerFormatTiff;
		}
		else if (lpFileExtention.compare(L".gif") == 0)
		{
			containerFormat = GUID_ContainerFormatGif;
		}
		else if (lpFileExtention.compare(L".png") == 0)
		{
			containerFormat = GUID_ContainerFormatPng;
		}
		else if (lpFileExtention.compare(L".wmp") == 0)
		{
			containerFormat = GUID_ContainerFormatWmp;
		}
		else if(lpFileExtention.compare(L".bmp") == 0)
		{
			containerFormat = GUID_ContainerFormatBmp;
		}

		hr = EinkuiGetSystem()->GetWICFactory()->CreateEncoder(containerFormat, NULL, &lpEncoder);
		BREAK_ON_FAILED(hr);

		hr = lpEncoder->Initialize(lpStream, WICBitmapEncoderNoCache);
		BREAK_ON_FAILED(hr);		

		hr = lpEncoder->CreateNewFrame(&lpFrameEncode, NULL);
		BREAK_ON_FAILED(hr);

		hr = lpFrameEncode->Initialize(NULL);
		BREAK_ON_FAILED(hr);		

		hr = lpFrameEncode->SetSize(sc_bitmapWidth, sc_bitmapHeight);
		BREAK_ON_FAILED(hr);	

		hr = lpFrameEncode->SetPixelFormat(&format);
		BREAK_ON_FAILED(hr);

		hr = lpFrameEncode->WriteSource(lpWICBitmap, NULL);
		BREAK_ON_FAILED(hr);

		hr = lpFrameEncode->Commit();
		BREAK_ON_FAILED(hr);	

		hr = lpEncoder->Commit();
		BREAK_ON_FAILED(hr);		
		
		lbStatus = true;

	} while (false);


	// 释放资源
	CMM_SAFE_RELEASE(lpWICBitmap);
	CMM_SAFE_RELEASE(lpRT);
	CMM_SAFE_RELEASE(lpEncoder);
	CMM_SAFE_RELEASE(lpFrameEncode);
	CMM_SAFE_RELEASE(lpStream);
	CMM_SAFE_RELEASE(lpBmp2d);

	return lbStatus;
}
