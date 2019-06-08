/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"

#include "XgD2DEngine4Eink.h"
#include <dwmapi.h>
#include <ppl.h>  
#include "EiUpdate.h"

using namespace concurrency;


DEFINE_BUILTIN_NAME(CXD2dEngine)



CXD2dEngine::CXD2dEngine()
{
	// 初始化设备无关资源
	mpD2dFactory = NULL;
	mpWicFactory = NULL;
	mpWriteFactory = NULL;
	mpDbgTextFormat = NULL;

	mpEinkBuf = NULL;
	mpCustomDraw = NULL;

	// 初始化设备相关资源
	mpTarget2D = NULL;
	mpTargetBitmap = NULL;
	mpDbgTextBrush = NULL;
	mpFogBrush = NULL;

	// 绘图上下文
	mpElementManager = NULL;
	mpMessage = NULL;
	mfFpsIndx = 0.0f;
	muRenderCount = 0;
	mlRenderStep = CEinkuiSystem::eRenderStop;
	mpToCapture = NULL;
	mdBackColor = D2D1::ColorF(1.0f,1.0f,1.0f,1.0f);

	mpModalTrunk = NULL;

	mbStopPainting = false;// true;

	mdScalMatrixToCapture = mdIdentyMatrix = D2D1::Matrix3x2F::Identity();

	mlResetEinkBuf = 0;
	mpEinkUpdatingCallBack = NULL;
	mxUpdatingNumber = 0;
	mpEinkUpdatingContext = NULL;
}


CXD2dEngine::~CXD2dEngine()
{

	ReleaseDeviceResource(false);

	CMM_SAFE_RELEASE(mpD2dFactory);
	CMM_SAFE_RELEASE(mpWicFactory);
	CMM_SAFE_RELEASE(mpWriteFactory);
	CMM_SAFE_RELEASE(mpDbgTextFormat);

	CMM_SAFE_RELEASE(mpMessage);

	if (mpEinkBuf != NULL)
		EiReleaseDrawBuffer(mpEinkBuf);
}

//////////////////////////////////////////////////////////////////////////
// 创建绘图资源
//////////////////////////////////////////////////////////////////////////

// 创建单一颜色的画刷
IEinkuiBrush* __stdcall CXD2dEngine::CreateBrush(
	XuiBrushType niBrushType,
	D2D1_COLOR_F noColor
	)
{
	IEinkuiBrush* lpBrush = NULL;

	do 
	{
		lpBrush = CXD2DBrush::CreateInstance(niBrushType, noColor);
		BREAK_ON_NULL(lpBrush);

		CEinkuiSystem::gpXuiSystem->GetBrushList().RegisteBrush(lpBrush);

	} while (false);

	return lpBrush;

}

// 渐变画刷时，需要传入多个颜色点
IEinkuiBrush* __stdcall CXD2dEngine::CreateBrush(
	XuiBrushType niBrushType, 
	D2D1_GRADIENT_STOP* npGradientStop, 
	ULONG nuCount, 
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
	)
{
	IEinkuiBrush* lpBrush = NULL;

	do 
	{
		lpBrush = CXD2DBrush::CreateInstance(niBrushType, npGradientStop, nuCount, ndLinearGradientBrushProperties);
		BREAK_ON_NULL(lpBrush);

		CEinkuiSystem::gpXuiSystem->GetBrushList().RegisteBrush(lpBrush);

	} while (false);

	return lpBrush;

}



//////////////////////////////////////////////////////////////////////////
// 实例化引擎类
//////////////////////////////////////////////////////////////////////////

ULONG CXD2dEngine::InitOnCreate(ULONG Width, ULONG Height)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	mpElementManager = dynamic_cast<CXelManager*>(CEinkuiSystem::gpXuiSystem->GetElementManager());

	do 
	{
		muFixedW = Width;
		muFixedH = Height;
		RelocationPainboard();

		//建立设备无关资源
		luResult = CreateIndependentResource();
		BREAK_ON_FAILED(luResult);

		//建立设备相关资源
		luResult = CreateDeviceResource();
		BREAK_ON_FAILED(luResult);

		// 开始帧率统计
		muLastTick = GetCurrentTickCount();

		luResult = ERESULT_SUCCESS;
	} while (false);

	
	return luResult;

}

// 建立设备无关资源
ERESULT CXD2dEngine::CreateIndependentResource()
{
	static const WCHAR msc_fontName[] = L"Tahoma";
	static const FLOAT msc_fontSize = 13;
	HRESULT hr;

	do 
	{
		// Create D2D factory
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED/*D2D1_FACTORY_TYPE_SINGLE_THREADED*/, &mpD2dFactory);
		if(FAILED(hr))
			break;

		// Create a WIC factory
		hr = CoCreateInstance(
			CLSID_WICImagingFactory,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void **>(&mpWicFactory)
			);

		// Create DWrite factory
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(mpWriteFactory),
			reinterpret_cast<IUnknown **>(&mpWriteFactory)
			);
		if(FAILED(hr))
			break;

		hr = mpWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", 
			&mpDbgTextFormat
			);

	} while (false);

	return SUCCEEDED(hr)?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL;

}

// 建立设备相关资源
ERESULT CXD2dEngine::CreateDeviceResource()
{
	if (mpTarget2D != NULL)
		return ERESULT_SUCCESS;

	HRESULT hr = S_OK;
	ERESULT luResult = ERESULT_SUCCESS;
	D2D1_RENDER_TARGET_PROPERTIES ld2DTargetProt;
	HDC lhDefaultDC = NULL;
	BITMAPINFO BmpInfo;
	ID2D1DCRenderTarget* lpDcTarget;

	do
	{
		lhDefaultDC = ::GetDC(NULL);
		BREAK_ON_NULL(lhDefaultDC);

		//建立兼容位图
		BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
		BmpInfo.bmiHeader.biWidth = muEinkPanelW;
		BmpInfo.bmiHeader.biHeight = -(int)muEinkPanelH;
		BmpInfo.bmiHeader.biPlanes = 1;
		BmpInfo.bmiHeader.biBitCount = 32;
		BmpInfo.bmiHeader.biCompression = BI_RGB;
		BmpInfo.bmiHeader.biSizeImage = 0;
		BmpInfo.bmiHeader.biXPelsPerMeter = 1;
		BmpInfo.bmiHeader.biYPelsPerMeter = 1;
		BmpInfo.bmiHeader.biClrUsed = 0;
		BmpInfo.bmiHeader.biClrImportant = 0;
		mhForeBmp = CreateDIBSection(lhDefaultDC, &BmpInfo, DIB_RGB_COLORS, (void**)&mpForeBuffer \
			, NULL, 0);
		BREAK_ON_NULL(mhForeBmp);

		//建立兼容DC
		mhForeDc = CreateCompatibleDC(lhDefaultDC);
		BREAK_ON_NULL(mhForeDc);

		//替换位图对象
		mhOldForeBmp = (HBITMAP)SelectObject(mhForeDc, mhForeBmp);
		BREAK_ON_NULL(mhOldForeBmp);

		//建立D2d target
		ld2DTargetProt = D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(
				DXGI_FORMAT_B8G8R8A8_UNORM,
				D2D1_ALPHA_MODE_PREMULTIPLIED),
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_FORCE_BITMAP_REMOTING,
			D2D1_FEATURE_LEVEL_DEFAULT
		);

		hr = mpD2dFactory->CreateDCRenderTarget(&ld2DTargetProt, &lpDcTarget);
		if (FAILED(hr))
			break;

		RECT rc = { 0,0,(LONG)muEinkPanelW,(LONG)muEinkPanelH};

		lpDcTarget->BindDC(mhForeDc, &rc);

		mpTarget2D = lpDcTarget;

		//建立2D设备相关资源
		luResult = CreateD2dResource();
		if (ERESULT_FAILED(luResult))
			break;

		hr = S_OK;

	} while (false);

	if (lhDefaultDC != NULL)
		ReleaseDC(NULL, lhDefaultDC);

	return SUCCEEDED(hr) ? ERESULT_SUCCESS : ERESULT_UNSUCCESSFUL;
}

// 创建D2D相关资源
ERESULT CXD2dEngine::CreateD2dResource()
{
	HRESULT hr = S_OK;


	hr = mpTarget2D->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Blue,0.6f),
		&mpDbgTextBrush
		);

	hr = mpTarget2D->CreateSolidColorBrush(
		D2D1::ColorF(0.0f,0.0f,0.0f,0.3f),
		&mpFogBrush
		);

	return ERESULT_SUCCESS;// SUCCEEDED(hr)?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL; 上面两个画刷不重要，无需返回错误
}

// 回收释放资源
void CXD2dEngine::ReleaseDeviceResource(bool nbBroadcastToElement/* =true */)
{
	// 首先给所有Element发送消息，通知他们释放设备相关资源
	if(nbBroadcastToElement != false && mpTarget2D != NULL && mpElementManager != NULL) // mpTarget2D != NULL防止多次发送释放消息
		mpElementManager->EnumAllElement(
		false,
		this,
		(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::EnterForDiscardDeviceRes,
		(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::LeaveForDiscardDeviceRes
		);

	// 释放位图对象所持有的显存位图
	CEinkuiSystem::gpXuiSystem->GetBitmapList().ReleaseDeviceResource();
	// 释放全部Brush对象
	CEinkuiSystem::gpXuiSystem->GetBrushList().ReleaseDeviceResource();


	// 释放其他Device相关资源
	CMM_SAFE_RELEASE(mpDbgTextBrush);
	CMM_SAFE_RELEASE(mpFogBrush);
	CMM_SAFE_RELEASE(mpTarget2D);
	CMM_SAFE_RELEASE(mpTargetBitmap);

	if (mhForeDc != NULL)
	{
		if (mhOldForeBmp != NULL)
			SelectObject(mhForeDc, mhOldForeBmp);
		DeleteDC(mhForeDc);
		mhForeDc = NULL;
	}
	if (mhForeBmp != NULL)
	{
		DeleteObject(mhForeBmp);
		mhForeBmp = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
// 需要实现的基类接口
//////////////////////////////////////////////////////////////////////////

ERESULT __stdcall CXD2dEngine::DrawBitmap( IN const D2D1_RECT_F& rDestRect, IN const D2D1_RECT_F& rSrcRect, IN IEinkuiBitmap* npBitmap, IN ULONG nuMethod,IN float nfAlpha)
{

	ID2D1Bitmap* lpBmp=NULL;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	CXD2dBitmap* lpXuiD2DBitmap = dynamic_cast<CXD2dBitmap*>(npBitmap);


	do 
	{
		BREAK_ON_NULL(lpXuiD2DBitmap);

		// 初始化位图对象
		luResult = lpXuiD2DBitmap->InitBitmap(mpWicFactory, mpD2dFactory, mpTarget2D);
		BREAK_ON_FAILED(luResult);

		luResult = lpXuiD2DBitmap->GetD2DObject(mpTarget2D, &lpBmp);
		if(luResult != ERESULT_SUCCESS)
			break;


		if (nuMethod == ESPB_DRAWBMP_EXTEND && 
			((rDestRect.right - rDestRect.left - 1.0f > rSrcRect.right - rSrcRect.left) || 
			(rDestRect.bottom - rDestRect.top - 1.0f > rSrcRect.bottom - rSrcRect.top)))
		{
			//延展线方式
			//如果要延展的对象只有宽或高某一项比原图大，延展算法就会出错 2018-5-2 jaryee ????
			FLOAT lfExtendLineX = CExFloat::Round((FLOAT)npBitmap->GetExtnedLineX());	//读取延展线坐标，如果为-1表示使用中线
			FLOAT lfExtendLineY = CExFloat::Round((FLOAT)npBitmap->GetExtnedLineY());

			if(lfExtendLineX <= 0)	//如果延展线设置小于0，以图片中线为延展线
				lfExtendLineX = CExFloat::Round((rSrcRect.right - rSrcRect.left) / 2.0f);
			if(lfExtendLineY <= 0)
				lfExtendLineY = CExFloat::Round((rSrcRect.bottom - rSrcRect.top) / 2.0f);

			D2D1_RECT_F lDestRect,lSrcRect;
			D2D1_SIZE_F lDestSize,lSrcSize;

			lDestSize.width = CExFloat::Round(rDestRect.right - rDestRect.left);	//计算目标和源图的宽和高
			lDestSize.height = CExFloat::Round(rDestRect.bottom - rDestRect.top);
			lSrcSize.width = CExFloat::Round(rSrcRect.right - rSrcRect.left);
			lSrcSize.height = CExFloat::Round(rSrcRect.bottom - rSrcRect.top);

			//////////////////////////////////////////////////////////////////////////
			//左上角 这里不会因为延展的不同而变化
			lDestRect.left = rDestRect.left;
			lDestRect.right = lfExtendLineX + lDestRect.left;
			lDestRect.top = rDestRect.top;
			lDestRect.bottom = lfExtendLineY + lDestRect.top;

			lSrcRect.left = rSrcRect.left;
			lSrcRect.right = lfExtendLineX + rSrcRect.left;
			lSrcRect.top = rSrcRect.top;
			lSrcRect.bottom = lfExtendLineY + rSrcRect.top;
			
			if (nfAlpha >= 0.999f)
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
			else
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha * nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

			//////////////////////////////////////////////////////////////////////////
			//左上角和右上角中间的部分
			if (lDestSize.width > lSrcSize.width)
			{
				//如果是变宽，才需要执行这里
				lDestRect.left = lDestRect.right;
				lDestRect.right = lDestSize.width - lSrcSize.width + lDestRect.left + 1.0f;
				//lDestRect.top = rDestRect.top;
				//lDestRect.bottom = lfExtendLineY;

				lSrcRect.left = lSrcRect.right;
				lSrcRect.right ++;
				lSrcRect.top = rSrcRect.top;
				lSrcRect.bottom = lfExtendLineY + rSrcRect.top;

				if (nfAlpha >= 0.999f)
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
				else
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
			}


			//////////////////////////////////////////////////////////////////////////
			//右上角
			lDestRect.left = lDestRect.right;
			lDestRect.right = rDestRect.right;
			//lDestRect.top = rDestRect.top;
			//lDestRect.bottom = lfExtendLineY;

			lSrcRect.left = lSrcRect.right;
			lSrcRect.right = rSrcRect.right;
			lSrcRect.top = rSrcRect.top;
			lSrcRect.bottom = lfExtendLineY + rSrcRect.top;

			if (nfAlpha >= 0.999f)
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
			else
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

			//////////////////////////////////////////////////////////////////////////
			if (lDestSize.height > lSrcSize.height)
			{
				//如果需要变高才执行这里
				//////////////////////////////////////////////////////////////////////////
				//左上和左下中间部分
				lDestRect.left = rDestRect.left;
				lDestRect.right = lfExtendLineX + lDestRect.left;
				lDestRect.top = lDestRect.bottom;
				lDestRect.bottom = lDestSize.height - lSrcSize.height + lDestRect.top + 1.0f;

				lSrcRect.left = rSrcRect.left;
				lSrcRect.right = lfExtendLineX + rSrcRect.left;
				lSrcRect.top = lSrcRect.bottom;
				lSrcRect.bottom ++;

				if (nfAlpha >= 0.999f)
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
				else
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

				//////////////////////////////////////////////////////////////////////////
				//中间一大块
				if (lDestSize.width > lSrcSize.width && lDestSize.height > lSrcSize.height)
				{
					lDestRect.left = lDestRect.right;
					lDestRect.right = lDestRect.left + (lDestSize.width - lSrcSize.width) + 1.0f;
					//lDestRect.top = lfExtendLineY;
					//lDestRect.bottom = lfExtendLineY + (lDestSize.height - lSrcSize.height);

					lSrcRect.left = lfExtendLineX + rSrcRect.left;
					lSrcRect.right = lSrcRect.left + 1.0f;
					lSrcRect.top = lfExtendLineY + rSrcRect.top;
					lSrcRect.bottom = lSrcRect.top + 1.0f;

					if (nfAlpha >= 0.999f)
						mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
					else
						mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
				}

				//////////////////////////////////////////////////////////////////////////
				//右上和右下的中间部分
				lDestRect.left = lDestRect.right;
				lDestRect.right = rDestRect.right;


				lSrcRect.left = lSrcRect.right;
				lSrcRect.right = rSrcRect.right;

				if (nfAlpha >= 0.999f)
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
				else
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);


			}

			//////////////////////////////////////////////////////////////////////////
			//左下角
			lDestRect.left = rDestRect.left;
			lDestRect.right = lfExtendLineX + lDestRect.left;
			lDestRect.top = lDestRect.bottom;
			lDestRect.bottom = rDestRect.bottom;

			lSrcRect.left = rSrcRect.left;
			lSrcRect.right = lfExtendLineX + rSrcRect.left;
			lSrcRect.top = lSrcRect.bottom;
			lSrcRect.bottom = rSrcRect.bottom;


			if (nfAlpha >= 0.999f)
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
			else
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

			//////////////////////////////////////////////////////////////////////////
			//左下角和右下角中间的部分
			if (lDestSize.width > lSrcSize.width)
			{
				//如果是变宽，才需要执行这里
				lDestRect.left = lDestRect.right;
				lDestRect.right = lDestRect.left + (lDestSize.width - lSrcSize.width) + 1.0f;

				lSrcRect.left = lSrcRect.right;
				lSrcRect.right ++;

				if (nfAlpha >= 0.999f)
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
				else
					mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

			}

			//////////////////////////////////////////////////////////////////////////
			//右下角
			lDestRect.left = lDestRect.right;
			lDestRect.right = rDestRect.right;

			lSrcRect.left = lSrcRect.right;
			lSrcRect.right = rSrcRect.right; 

			if (nfAlpha >= 0.999f)
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);
			else
				mpTarget2D->DrawBitmap(lpBmp,&lDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)ESPB_DRAWBMP_LINEAR,&lSrcRect);

		}
		else
		{
			if(nuMethod == ESPB_DRAWBMP_EXTEND)
				nuMethod = ESPB_DRAWBMP_LINEAR;	//这是本来使用延展算法，但当前需求是不需要放大的，所以使用这里简单绘制，将Method参数修改为普通　

			if (nfAlpha >= 0.999f)	
				mpTarget2D->DrawBitmap(lpBmp,&rDestRect,moRenderState.Top().mfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)nuMethod,&rSrcRect);
			else
				mpTarget2D->DrawBitmap(lpBmp,&rDestRect,moRenderState.Top().mfAlpha*nfAlpha,(D2D1_BITMAP_INTERPOLATION_MODE)nuMethod,&rSrcRect);

		}

		luResult = ERESULT_SUCCESS;

	} while (false);


	CMM_SAFE_RELEASE(lpBmp);

	return luResult;


}


ERESULT __stdcall CXD2dEngine::DrawBitmap( IN const D2D1_RECT_F& rDestRect, IN IEinkuiBitmap* npBitmap, IN ULONG nuMethod,	IN float nfAlpha)
{


	D2D1_RECT_F ldSrcRect;
	ID2D1Bitmap* lpBmp=NULL;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	if(npBitmap == NULL)
		return ERESULT_WRONG_PARAMETERS;

	CXD2dBitmap* lpXuiD2DBitmap = dynamic_cast<CXD2dBitmap*>(npBitmap);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBitmap);

		// 初始化位图对象
		luResult = lpXuiD2DBitmap->InitBitmap(mpWicFactory, mpD2dFactory, mpTarget2D);
		BREAK_ON_FAILED(luResult);

		luResult = lpXuiD2DBitmap->GetD2DObject(mpTarget2D,&lpBmp);
		if(ERESULT_SUCCEEDED(luResult) && lpBmp != NULL)
		{
			ldSrcRect.left = ldSrcRect.top = 0;
			ldSrcRect.right = lpBmp->GetSize().width;
			ldSrcRect.bottom = lpBmp->GetSize().height;

			luResult = DrawBitmap(rDestRect,ldSrcRect,npBitmap,nuMethod, nfAlpha);
		}

	} while (false);

	CMM_SAFE_RELEASE(lpBmp);

	return luResult;

}


ERESULT __stdcall CXD2dEngine::DrawBitmap( IN const D2D1_POINT_2F& rDestLeftTop, IN IEinkuiBitmap* npBitmap,IN float nfAlpha)
{

	D2D1_RECT_F ldDstRect;
	D2D1_RECT_F ldSrcRect;
	ID2D1Bitmap* lpBmp=NULL;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	CXD2dBitmap* lpXuiD2DBitmap = dynamic_cast<CXD2dBitmap*>(npBitmap);

	if(npBitmap == NULL)
		return ERESULT_WRONG_PARAMETERS;

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBitmap);

		// 初始化位图对象
		luResult = lpXuiD2DBitmap->InitBitmap(mpWicFactory, mpD2dFactory, mpTarget2D);
		BREAK_ON_FAILED(luResult);

		luResult = lpXuiD2DBitmap->GetD2DObject(mpTarget2D, &lpBmp);
		BREAK_ON_FAILED(luResult);
		BREAK_ON_NULL(lpBmp);

		ldSrcRect.left = ldSrcRect.top = 0;
		ldSrcRect.right = lpBmp->GetSize().width;
		ldSrcRect.bottom = lpBmp->GetSize().height;

		ldDstRect.left = rDestLeftTop.x;
		ldDstRect.top = rDestLeftTop.y;
		ldDstRect.right = rDestLeftTop.x + ldSrcRect.right;
		ldDstRect.bottom = rDestLeftTop.y + ldSrcRect.bottom;

		// 矩形表示形式的转换
		luResult = DrawBitmap(ldDstRect,ldSrcRect,npBitmap,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nfAlpha);

	} while (false);

	CMM_SAFE_RELEASE(lpBmp);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::DrawLine( IN const D2D1_POINT_2F& noPointArray, IN const D2D1_POINT_2F& noEndPoint, IN IEinkuiBrush* npBrush )
{

	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1StrokeStyle* lpStrokeStyle = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 尝试获取stroke
		luResult = lpXuiD2DBrush->GetStrokeObject(&lpStrokeStyle);
		BREAK_ON_FAILED(luResult);


		// 调用D2D的画线函数
		if (lpStrokeStyle != NULL)
		{
			mpTarget2D->DrawLine(noPointArray, noEndPoint, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth(), lpStrokeStyle);
		}
		else
		{
			mpTarget2D->DrawLine(noPointArray, noEndPoint, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth());
		}


		luResult = ERESULT_SUCCESS;

	} while (false);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::DrawPlogon( IN const D2D1_POINT_2F* noPointArray, IN INT niCount, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;	

	ID2D1PathGeometry*	lpPathGeometry = NULL;
	ID2D1GeometrySink*	lpSink = NULL;
	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1StrokeStyle* lpStrokeStyle = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);


	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 尝试获取stroke
		luResult = lpXuiD2DBrush->GetStrokeObject(&lpStrokeStyle);
		BREAK_ON_FAILED(luResult);


		hr = mpD2dFactory->CreatePathGeometry(&lpPathGeometry);
		BREAK_ON_FAILED(hr);

		hr = lpPathGeometry->Open(&lpSink);
		BREAK_ON_FAILED(hr);

		{
			lpSink->SetFillMode(D2D1_FILL_MODE_WINDING);
			lpSink->BeginFigure(
				noPointArray[0],
				D2D1_FIGURE_BEGIN_FILLED
				);

			lpSink->AddLines(noPointArray, niCount);
			lpSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		lpSink->Close();

		if (lpStrokeStyle == NULL)
		{
			mpTarget2D->DrawGeometry(lpPathGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth());
		}
		else
		{
			mpTarget2D->DrawGeometry(lpPathGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth(), lpStrokeStyle);
		}

	} while (false);

	CMM_SAFE_RELEASE(lpPathGeometry);
	CMM_SAFE_RELEASE(lpSink);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::FillPlogon( IN const D2D1_POINT_2F* noPointArray, IN INT niCount, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;	

	ID2D1PathGeometry*	lpPathGeometry = NULL;
	ID2D1GeometrySink*	lpSink = NULL;

	ID2D1Brush*	lpD2DBrush;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);


	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreatePathGeometry(&lpPathGeometry);
		BREAK_ON_FAILED(hr);

		hr = lpPathGeometry->Open(&lpSink);
		BREAK_ON_FAILED(hr);

		{
			lpSink->SetFillMode(D2D1_FILL_MODE_WINDING);
			lpSink->BeginFigure(
				noPointArray[0],
				D2D1_FIGURE_BEGIN_FILLED
				);

			lpSink->AddLines(noPointArray, niCount);
			lpSink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}

		lpSink->Close();

		mpTarget2D->FillGeometry(lpPathGeometry, lpD2DBrush);
		
	} while (false);

	CMM_SAFE_RELEASE(lpPathGeometry);
	CMM_SAFE_RELEASE(lpSink);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::DrawEllipse( IN const D2D1_RECT_F& noRect, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1StrokeStyle* lpStrokeStyle = NULL;
	ID2D1EllipseGeometry *lpEllipseGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateEllipseGeometry(
			D2D1::Ellipse(D2D1::Point2F(noRect.left + (noRect.right - noRect.left)/2 , noRect.top +(noRect.bottom - noRect.top)/2 ), (noRect.right - noRect.left)/2, (noRect.bottom - noRect.top)/2),
			&lpEllipseGeometry
			);
		BREAK_ON_FAILED(hr);


		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 尝试获取stroke
		luResult = lpXuiD2DBrush->GetStrokeObject(&lpStrokeStyle);
		BREAK_ON_FAILED(luResult);

	
		if (lpStrokeStyle == NULL)
		{
			mpTarget2D->DrawGeometry(lpEllipseGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth());
		}
		else
		{
			mpTarget2D->DrawGeometry(lpEllipseGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth(), lpStrokeStyle);
		}

		luResult = ERESULT_SUCCESS;

	} while (false);

	// 释放资源
	CMM_SAFE_RELEASE(lpEllipseGeometry);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::FillEllipse( IN const D2D1_RECT_F& noRect, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1EllipseGeometry *lpEllipseGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{

		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateEllipseGeometry(
			D2D1::Ellipse(D2D1::Point2F(noRect.left + (noRect.right - noRect.left)/2 , noRect.top +(noRect.bottom - noRect.top)/2 ), (noRect.right - noRect.left)/2, (noRect.bottom - noRect.top)/2),
			&lpEllipseGeometry
			);
		BREAK_ON_FAILED(hr);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 填充椭圆区域
		mpTarget2D->FillGeometry(lpEllipseGeometry, lpD2DBrush);

		luResult = ERESULT_SUCCESS;

	} while (false);

	// 释放资源
	CMM_SAFE_RELEASE(lpEllipseGeometry);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::DrawRect( IN const D2D1_RECT_F& noRect, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1StrokeStyle* lpStrokeStyle = NULL;
	ID2D1RectangleGeometry *lpRectGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateRectangleGeometry(noRect,
			&lpRectGeometry);
		BREAK_ON_FAILED(hr);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 尝试获取stroke
		luResult = lpXuiD2DBrush->GetStrokeObject(&lpStrokeStyle);
		BREAK_ON_FAILED(luResult);


		if (lpStrokeStyle == NULL)
		{
			mpTarget2D->DrawGeometry(lpRectGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth());
		}
		else
		{
			mpTarget2D->DrawGeometry(lpRectGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth(), lpStrokeStyle);
		}

		luResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpRectGeometry);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::FillRect( IN const D2D1_RECT_F& noRect, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1RectangleGeometry *lpRectGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateRectangleGeometry(noRect,
			&lpRectGeometry);
		BREAK_ON_FAILED(hr);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);


		// 填充椭圆区域
		mpTarget2D->FillGeometry(lpRectGeometry, lpD2DBrush);

		luResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpRectGeometry);

	return luResult;
}


ERESULT __stdcall CXD2dEngine::DrawRoundRect( IN const D2D1_RECT_F& noRect, FLOAT radiusX, FLOAT radiusY, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1StrokeStyle* lpStrokeStyle = NULL;
	ID2D1RoundedRectangleGeometry* lpRoundedRectGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateRoundedRectangleGeometry(
			D2D1::RoundedRect
			(
				noRect,
				radiusX,
				radiusY
			),
			&lpRoundedRectGeometry);
		BREAK_ON_FAILED(hr);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);

		// 尝试获取stroke
		luResult = lpXuiD2DBrush->GetStrokeObject(&lpStrokeStyle);
		BREAK_ON_FAILED(luResult);


		if (lpStrokeStyle == NULL)
		{
			mpTarget2D->DrawGeometry(lpRoundedRectGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth());
		}
		else
		{
			mpTarget2D->DrawGeometry(lpRoundedRectGeometry, lpD2DBrush, lpXuiD2DBrush->GetStrokeWidth(), lpStrokeStyle);
		}

		luResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpRoundedRectGeometry);

	return luResult;
}

ERESULT __stdcall CXD2dEngine::FillRoundRect( IN const D2D1_RECT_F& noRect, FLOAT radiusX, FLOAT radiusY, IN IEinkuiBrush* npBrush )
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	HRESULT	hr;

	ID2D1Brush*	lpD2DBrush = NULL;
	ID2D1RoundedRectangleGeometry* lpRoundedRectGeometry = NULL;
	CXD2DBrush* lpXuiD2DBrush = dynamic_cast<CXD2DBrush*>(npBrush);

	do 
	{
		BREAK_ON_NULL(lpXuiD2DBrush);

		luResult = lpXuiD2DBrush->InitBrush(mpTarget2D, mpD2dFactory);
		BREAK_ON_FAILED(luResult);

		hr = mpD2dFactory->CreateRoundedRectangleGeometry(
			D2D1::RoundedRect
			(
				noRect,
				radiusX,
				radiusY
			),
			&lpRoundedRectGeometry);
		BREAK_ON_FAILED(hr);

		// 获取brush
		luResult = lpXuiD2DBrush->GetBrushObject(&lpD2DBrush);
		BREAK_ON_FAILED(luResult);


		// 填充椭圆区域
		mpTarget2D->FillGeometry(lpRoundedRectGeometry, lpD2DBrush);

		luResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpRoundedRectGeometry);

	return luResult;
}



//////////////////////////////////////////////////////////////////////////
// 辅助功能函数
//////////////////////////////////////////////////////////////////////////

// 获得Direct2D的RenderTarget，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
// 仅应该在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间执行绘制动作，不能在Prepare消息期间绘制，以免破坏渲染引擎的稳定
ID2D1RenderTarget* __stdcall CXD2dEngine::GetD2dRenderTarget(void)
{
	return mpTarget2D;
}

// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
ID2D1Factory* __stdcall CXD2dEngine::GetD2dFactory(void)
{
	return mpD2dFactory;
}

// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
IWICImagingFactory* __stdcall CXD2dEngine::GetWICFactory(void)
{
	return mpWicFactory;
}

// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
IDWriteFactory* __stdcall CXD2dEngine::GetDWriteFactory(void)
{
	return mpWriteFactory;
}


// 获得Eink屏的大小
void CXD2dEngine::GetPaintboardSize(
	OUT EI_SIZE* npSize	// 获取画板大小
)	
{
	if (npSize != NULL)
	{
		npSize->w = (LONG)muEinkPanelW;
		npSize->h = (LONG)muEinkPanelH;
	}
}

// 获得当前的D2d绘制用，局部坐标到世界坐标的转换矩阵
const D2D1::Matrix3x2F& __stdcall CXD2dEngine::GetCurrent2DWorldMatrix(void)
{
	if(moRenderState.Size()>0)
		return moRenderState.Top().mdWorld;
	else 
		return mdIdentyMatrix;
}

// 获得当前可视区设置，可视区用世界坐标描述，如果D2dTarget被切换，将给出的是切换后的Target对应的世界坐标
// 返回ERESULT_SUCCESS当前存在可视区设置，并且成功取得，ERESULT_UNSUCCESS不存在可视区设置，其他值表示错误
ERESULT __stdcall CXD2dEngine::GetVisibleRegion(
	OUT D2D1_RECT_F* npRegion
	)
{
	int i;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	D2D1_POINT_2F loPoint;

	for (i=moRenderState.Size()-1;i>=0;i--)
	{
		if(moRenderState[i].mpD2dTarget != NULL)	// 如果切换过分2DTarget就不需要继续查找了
			break;
		if(moRenderState[i].mbHasClipRect != false)	// 存在可视区
		{
			luResult = ERESULT_SUCCESS;
			if(npRegion != NULL)
			{
				loPoint.x = moRenderState[i].mdClipRect.left;
				loPoint.y = moRenderState[i].mdClipRect.top;
				loPoint = loPoint * moRenderState[i].mdWorld;

				npRegion->left = loPoint.x;
				npRegion->top= loPoint.y;

				loPoint.x = moRenderState[i].mdClipRect.right;
				loPoint.y = moRenderState[i].mdClipRect.bottom;
				loPoint = loPoint * moRenderState[i].mdWorld;

				npRegion->right = loPoint.x;
				npRegion->bottom = loPoint.y;
			}
			break;
		}
	}

	return luResult;
}

// 对于直接访问D3d和D2d对象的元素，使用本接口向系统汇报设备错误，系统返回ERESULT_SUCCESS表示可以继续执行，返回值满足宏ERESULT_FAILED则中止继续执行
ERESULT __stdcall CXD2dEngine::CheckDeviceResult(
	IN HRESULT nhrDeviceResult
	)
{
	if(nhrDeviceResult != S_OK)
	{
		//不是OK的DirectX返回值
		/*if(nhrDeviceResult == DXGI_STATUS_OCCLUDED)
			StopPainting(true);*/ //close by niu 2017-12-5 设备丢失不要停止，下次绘制时会重建设备

		if (
			nhrDeviceResult == DXGI_STATUS_OCCLUDED ||
			nhrDeviceResult == DXGI_ERROR_DEVICE_REMOVED ||
			nhrDeviceResult == DXGI_ERROR_INVALID_CALL ||
			nhrDeviceResult == DXGI_ERROR_DEVICE_RESET ||
			nhrDeviceResult == D2DERR_RECREATE_TARGET)
		{
			//显示设备丢失

			ReleaseDeviceResource();
			return ERESULT_DIRECTX_ERROR;
		}

	}
	return ERESULT_SUCCESS;
}


// 获得当前帧率
ULONG __stdcall CXD2dEngine::GetCurrentFps(void)
{
	return (ULONG)mfFpsIndx;
}

// 获得当前的渲染序列号，绘制序列号是用来协调和计算当前的渲染次数，每执行一次渲染该序号加一，某些情况下，元素可以会被要求绘制2次，计数达到最大值后会从0开始
ULONG __stdcall CXD2dEngine::GetRenderNumber(void)
{
	return muRenderCount;
}

// 获得当前的TickCount
ULONG __stdcall CXD2dEngine::GetCurrentTickCount()
{
	return muCrtTick;
}

// 清理画板
void CXD2dEngine::Clear()
{
	mpTarget2D->Clear(D2D1::ColorF(D2D1::ColorF::White, 0.0f));
}

// 提交到屏幕
HRESULT CXD2dEngine::Present(
	IN bool nbRefresh	// 必须提交全屏
	)
{
	if (mbStopPainting != false)
		return S_OK;

	if (mpCustomDraw != NULL)
		mpCustomDraw(mpForeBuffer, muEinkPanelW, muEinkPanelH,nbRefresh);
	else
		DrawEink(nbRefresh);

	return S_OK;
}

void CXD2dEngine::DrawEink(
	IN bool nbRefresh	// 必须提交全屏
	)
{
	// 获取Eink绘制缓冲区
	if (mpEinkBuf == NULL)
	{
		mpEinkBuf = EiGetDrawBuffer(FALSE, FALSE);
		if (mpEinkBuf == NULL)
			return;

		ClearEinkBuffer(true);
	}

	//DWORD luTick = GetTickCount();

	//for(int k=0;k<100;k++)
	//{

	moEiUpdate.SetPanel(muEinkPanelW,muEinkPanelH);

	// 将RGB32转换为灰度图像
	// Gray = R*0.299 + G*0.587 + B*0.114
#ifdef EI_PARALLEL
	parallel_for(size_t(0), (size_t)muEinkPanelH, [&](size_t i)
#else
	for (ULONG i = 0; i < muEinkPanelH; i++)
#endif//EI_PARALLEL
	{

		register unsigned char* lpDst = mpEinkBuf->Buf + muEinkPanelW*(ULONG)i;
		register unsigned char* lpSrc = mpForeBuffer + muEinkPanelW * 4 * (ULONG)i;
		register unsigned char lucNew;
		register ULONG luBegin;
		register ULONG luEnd;
		int j;
		//int liWidthMin = (int)(mpEinkBuf->ulWidth < muEinkPanelW ? mpEinkBuf->ulWidth : muEinkPanelW);// ???ax 未来必须相等
		luBegin = (ULONG)muEinkPanelW;// liWidthMin;
		luEnd = 0;
		// 转换颜色空间，同时，找寻第一处不相等
		for (j = 0; j < muEinkPanelW; j++, lpSrc += 4, lpDst++)
		{
			//lucNew = (((USHORT(lpSrc[0]) * 19 + USHORT(lpSrc[1]) * 38 + USHORT(lpSrc[2]) * 7) >> 6) & 0xF0);
			lucNew = ((USHORT(lpSrc[0]) * 19 + USHORT(lpSrc[1]) * 38 + USHORT(lpSrc[2]) * 7) >> 6);
			if (lucNew != *lpDst)
			{
				luBegin = j;
				j = muEinkPanelW; // 导致退出此循环
				*lpDst = lucNew;
			}
		}

		// 存在行中不等的情况
		if (luBegin < (ULONG)muEinkPanelW)
		{
			j = luBegin + 1; // 回到刚才的位置
			luEnd = luBegin;

			// 转换颜色空间，同时，记录不相等的末尾
			for (; j < muEinkPanelW; j++, lpSrc += 4, lpDst++)
			{
				//lucNew = (((USHORT(lpSrc[0]) * 19 + USHORT(lpSrc[1]) * 38 + USHORT(lpSrc[2]) * 7) >> 6) & 0xF0);
				lucNew = ((USHORT(lpSrc[0]) * 19 + USHORT(lpSrc[1]) * 38 + USHORT(lpSrc[2]) * 7) >> 6);
				if (lucNew != *lpDst)
				{
					luEnd = (short)j; // 记录最后一个位置
					*lpDst = lucNew;
				}
			}
			
			luEnd = luEnd + 1;	// 将End指向‘不同区域’之后的一个像素
		}

		if (luEnd > luBegin)
			moEiUpdate.SaveLineDiff((int)i, luBegin, luEnd); // 这个函数多线程安全 
	}
#ifdef EI_PARALLEL
	);
#endif//EI_PARALLEL

	//}
	//luTick = GetTickCount() - luTick;

	//////////////////////////////////////////////////////////////////////////
	// 写入文件，测试数据改变
	//static int gliTraceNumber = 0;
	//wchar_t glszTrackName[256];

	//swprintf_s(glszTrackName,256, L"c:\\test\\t%04d.bin", gliTraceNumber++);

	//ULONG luWritten;
	//HANDLE lhFile = CreateFile(glszTrackName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);

	//if (lhFile != NULL)
	//{
	//	WriteFile(lhFile,mpEinkBuf->Buf, mpEinkBuf->ulWidth*mpEinkBuf->ulHeight, &luWritten, NULL);
	//	CloseHandle(lhFile);
	//}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	////EI_RECT ldUpdateRect = { 0,0,muEinkPanelW,muEinkPanelH};
	//EI_RECT ldUpdateRect = { muEinkPanelW,muEinkPanelH,0,0 };

	//for (ULONG i = 0; i < muHeight; i++)
	//{
	//	if (mpDiff[i].Begin < mpDiff[i].End)
	//	{
	//		if (mpDiff[i].Begin < ldUpdateRect.x)
	//			ldUpdateRect.x = mpDiff[i].Begin;
	//		if (mpDiff[i].End > ldUpdateRect.w)
	//			ldUpdateRect.w = mpDiff[i].End;
	//		if (i < ldUpdateRect.y)
	//			ldUpdateRect.y = i;
	//		if (i > ldUpdateRect.h)
	//			ldUpdateRect.h = i;
	//	}
	//}

	//if (ldUpdateRect.w > ldUpdateRect.x && ldUpdateRect.h > ldUpdateRect.y)
	//{
	//	ldUpdateRect.w -= ldUpdateRect.x;
	//	ldUpdateRect.h -= ldUpdateRect.y;

	//	EiDraw(&ldUpdateRect, mpEinkBuf);
	//}
	//////////////////////////////////////////////////////////////////////////

	if (mpEinkUpdatingCallBack != NULL)
		mpEinkUpdatingCallBack(++mxUpdatingNumber,mpEinkUpdatingContext);

	if (mlResetEinkBuf != 0)
	{
		EI_RECT ldUpdateRect = { 0,0,muEinkPanelW,muEinkPanelH};
		EiDraw(&ldUpdateRect, mpEinkBuf);

		ClearEinkBuffer(false);
	}
	else
	{
		CEiBlocks loBlocks;

		moEiUpdate.GetDiffBlocks(loBlocks);
		moEiUpdate.Reset();

		for (int j = 0; j < loBlocks.Size(); j++)
		{
			EI_RECT ldUpdateRect;

			ldUpdateRect.x = loBlocks[j].x1;
			ldUpdateRect.w = loBlocks[j].x2 - loBlocks[j].x1;
			ldUpdateRect.y = loBlocks[j].y1;
			ldUpdateRect.h = loBlocks[j].y2 - loBlocks[j].y1;

			EiDraw(&ldUpdateRect, mpEinkBuf);
		}
	}

	//EI_RECT ldUpdateRect = { 0,0,mpEinkBuf->ulWidth,100 };

	//while (ldUpdateRect.y < muHeight)
	//{
	//	if (ldUpdateRect.y + ldUpdateRect.h > muHeight)
	//		ldUpdateRect.h = muHeight - ldUpdateRect.y;

	//	EiDraw(&ldUpdateRect, mpEinkBuf);

	//	ldUpdateRect.y += ldUpdateRect.h;

	//}
}

// 执行绘制任务
ERESULT CXD2dEngine::DoRender(
	IN ULONG nuCrtTick,	// 当前的TickCount
	IN bool nbRefresh,	// 必须提交全屏
	IN IEinkuiIterator* npToCapture	// 此值被设置用于抓取该元素与子元素的图像，并不会影响屏幕显示
	)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	 //检查是否处于操作线程
	if (CEinkuiSystem::gpXuiSystem->IsRunningInOperationThread() == false)
	{
		return ERESULT_WRONG_THREAD;
	}

	// 判断窗口是否隐藏
	if(mbStopPainting != false && npToCapture == NULL)
		return ERESULT_NOT_PAINT;
	
	//开始绘制

	InterlockedExchange(&mlRenderStep,CEinkuiSystem::eRenderBegin);
	InterlockedExchangePointer(reinterpret_cast<PVOID*>(&mpToCapture),npToCapture);

	try
	{
		if(mpElementManager == NULL)
		{
			mpElementManager = dynamic_cast<CXelManager*>(CEinkuiSystem::gpXuiSystem->GetElementManager());
			if(mpElementManager == NULL)
				THROW_NULL;
		}

		luResult = CreateDeviceResource();
		if(luResult != ERESULT_SUCCESS)
			THROW_FALSE;

		//分配绘制准备消息
		CMM_SAFE_RELEASE(mpMessage);
		mpMessage = mpElementManager->AllocateMessage();
		if(mpMessage == NULL)
			THROW_NULL;

		//绘制状态清空，表示现在是处理‘准备绘制’消息期间
		moRenderState.Clear();
		muCrtTick = nuCrtTick;

		//广播绘制准备消息
		InterlockedExchange(&mlRenderStep, CEinkuiSystem::eRenderParepare);

		luResult = mpElementManager->EnumAllElement(
				false,
				this,
				(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::EnterForPrepare,
				(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::LeaveForPrepare
				);
		if(ERESULT_FAILED(luResult))
		{
			//准备绘制失败
			THROW_FALSE;
		}

		//分配绘制消息
		CMM_SAFE_RELEASE(mpMessage);
		mpMessage = mpElementManager->AllocateMessage();
		if(mpMessage == NULL)
			THROW_NULL;

		//初始化绘制状态
		{
			CErdState loState;
			loState.mdWorld = mdIdentyMatrix;
			loState.mdTransLation = mdIdentyMatrix;
			loState.mdRotation = mdIdentyMatrix;
			loState.mdOffset = D2D1::Point2F(0.0f,0.0f);
			loState.mfAlpha = 1.0f;
			loState.mpEnhancer = NULL;
			loState.mbRotated = false;
			loState.mlPlLevelHost = -1;
			loState.mlCrtPtLevel = 0;

			moRenderState.Push(loState);
		}

		if (mpTarget2D == NULL)
			EinkuiGetSystem()->ExitXui(); //???? niu暂时不知道什么原因会这样，为了能正常工作，直接退出进程

		mpTarget2D->BeginDraw();
		//mbD2dBeginCalled = true;

		// 清屏
		Clear();
	
		//广播绘制消息
		InterlockedExchange(&mlRenderStep, CEinkuiSystem::eRenderRender);

		if(mpToCapture == NULL)
		{
			luResult = mpElementManager->EnumAllElement(
									false,
									this,
									(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::EnterForPaint,
									(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::LeaveForPaint
									);
		}
		else
		{
			mbCapturing = false;
			luResult = mpElementManager->EnumAllElement(
				false,
				this,
				(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::EnterForCapture,
				(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXD2dEngine::LeaveForCapture
				);
		}

		// 计算帧率
		{

			FLOAT lfTick = (FLOAT)(nuCrtTick - muLastTick);
			if(lfTick < 0.005f)
				lfTick = 0.005f;
			mfFpsIndx = mfFpsIndx*((1000.0f- lfTick*2.0f)/1000.0f) + (1000.0f/lfTick)*(lfTick*2.0f/1000.0f);
			muLastTick = nuCrtTick;
		}

		if(mpToCapture == NULL && mpElementManager->GetProbeMode()==4)
		{
			wchar_t lswFps[64];

			swprintf_s(lswFps,64,L"%2d [fps]",(ULONG)mfFpsIndx);

			mpTarget2D->SetTransform(mdIdentyMatrix);

			mpTarget2D->DrawText(
				lswFps,
				9,//wcslen(lswFps),
				mpDbgTextFormat,
				D2D1::RectF(0.0f, 0.0f,150.0f,30.0f),
				mpDbgTextBrush
				);

		}

		if(mpElementManager->GetProbeMode() == 2)
		{
			wchar_t lswDump[512]=L"";
			const wchar_t* lswEleType;
			IEinkuiIterator* lpKeyFocus = mpElementManager->GetMouseFocus();
			IEinkuiIterator* lpItr = lpKeyFocus;
			if(lpItr != NULL)
			{
				while(lpItr != lpItr->GetParent())
				{
					lswEleType = lpItr->GetElementObject()->GetType();
					wcscat_s(lswDump,512,lswEleType);
					wcscat_s(lswDump,512,L"\n\r");
					lpItr = lpItr->GetParent();
				}

				UINT luSize = (UINT)wcslen(lswDump);
				D2D1_POINT_2F ldPos = mpElementManager->GetCurrentMousePosition();
				if(ldPos.x+300.0f > (FLOAT)muEinkPanelW)
					ldPos.x = (FLOAT)muEinkPanelW - 300.0f;
				if(ldPos.y + 300.0f > (FLOAT)muEinkPanelH)
					ldPos.y = (FLOAT)muEinkPanelH- 300.0f;

				mpTarget2D->SetTransform(D2D1::Matrix3x2F::Identity());
				mpTarget2D->DrawText(lswDump,luSize,mpDbgTextFormat,
					D2D1::RectF(ldPos.x+30.0f,ldPos.y+10.0f,ldPos.x+300.0f,ldPos.y+300.0f),
					mpDbgTextBrush);

				mpElementManager->ReleaseIterator(lpKeyFocus);
			}
		}


		luResult = CheckDeviceResult(mpTarget2D->EndDraw());

		InterlockedExchange(&mlRenderStep,CEinkuiSystem::eRenderEnd);

		if(ERESULT_FAILED(luResult))
		{
			//绘制失败
			THROW_FALSE;
		}
	}
	catch(int)
	{
		if(ERESULT_SUCCEEDED(luResult))
			luResult = ERESULT_UNSUCCESSFUL;
	}
	catch (...)
	{
		//StopPainting(true);
		ReleaseDeviceResource();
		luResult = ERESULT_DIRECTX_ERROR;
	}

	if( mpToCapture == NULL && ERESULT_SUCCEEDED(luResult))
		luResult = CheckDeviceResult(Present(nbRefresh));

	if(ERESULT_FAILED(luResult))
	{
		//绘制失败

		if(ERESULT_SUCCEEDED(luResult))
			luResult = ERESULT_UNSUCCESSFUL;
	}
	else
	{
		// 换在调用本函数DoRender之后的外部，通过WindowsUI线程调用
		////因为我们的界面重新绘制了，所以需要激发系统下次仍然向我们索要显示的预览图像
		//DwmInvalidateIconicBitmaps(mhWindow);
		//绘制完成
	}

	InterlockedIncrement((LONG*)&muRenderCount);
	InterlockedCompareExchange((LONG*)&muRenderCount,0,MAXULONG32);
	InterlockedExchange(&mlRenderStep,CEinkuiSystem::eRenderStop);
	InterlockedExchangePointer(reinterpret_cast<PVOID*>(&mpToCapture),NULL);

	return luResult;

}

// 供Windows窗口过程调用，保存窗口最小化信息
void CXD2dEngine::StopPainting(bool nbMin){

	if(mbStopPainting != nbMin)
		mbStopPainting = nbMin;
}

// 改变窗口大小
void CXD2dEngine::ResetPaintboard(void)
{
	// 检查是否处于操作线程
	CMMASSERT(CEinkuiSystem::gpXuiSystem->IsRunningInOperationThread() != false);

	ClearEinkBuffer(true);
	ReleaseDeviceResource();

	RelocationPainboard();
}

// 重新定位
void CXD2dEngine::RelocationPainboard(void)
{
	EI_APP_CONTEXT ldContext;
	EiGetAppContext(&ldContext);
	if (ldContext.ulWidth == 0)
		return;	// 失败

	muEinkPanelW = ldContext.ulWidth;
	muEinkPanelH = ldContext.ulHeight;

	//muEinkPanelW = CEinkuiSystem::gpXuiSystem->GetEinkSystemInfo()->ulWidth;
	//muEinkPanelH = CEinkuiSystem::gpXuiSystem->GetEinkSystemInfo()->ulHeight;
	if (muFixedW != 0)
		muEinkPanelW = muFixedW;
	if (muFixedH != 0)
		muEinkPanelH = muFixedH;
}

// 发送Prepare Paint前预处理
ERESULT __stdcall CXD2dEngine::EnterForPrepare(IEinkuiIterator* npRecipient)
{
	if(npRecipient->IsVisible()==false)
		return ERESULT_STOP_ENUM_CHILD;

	PVOID lpThis = this;
	mpMessage->SetMessageID(EMSG_PREPARE_PAINT);
	mpMessage->SetInputData(&lpThis,sizeof(lpThis));

	ERESULT luResult = mpElementManager->SendMessage(npRecipient,mpMessage);

	if(ERESULT_FAILED(luResult))	// 如果出错了，就不继续枚举了，返回错误
		return luResult;

	return ERESULT_ENUM_CHILD;
}

// 发送Prepare Paint后处理
ERESULT __stdcall CXD2dEngine::LeaveForPrepare(IEinkuiIterator* npRecipient)
{
	return ERESULT_SUCCESS;
}

// 发送Paint前预处理
ERESULT __stdcall CXD2dEngine::EnterForPaint(IEinkuiIterator* npRecipient)
{
	ERESULT luResult = ERESULT_SUCCESS;
	CErdState loState;
	D2D1_POINT_2F ldPosition;
	D2D1_POINT_2F ldCenter;
	FLOAT lfAngle;
	FLOAT lfCrtAlpha;
	CXuiIterator* lpRecipient;
	D2D1_RECT_F ldClipRect;
	LONG llMyLevel;
	LONG llLevelCount;

	lpRecipient = dynamic_cast<CXuiIterator*>(npRecipient);

	llLevelCount = lpRecipient->GetPaintLevelCount();
	llMyLevel = lpRecipient->GetPaintLevel();

	// 决定是否需要绘制本对象及子对象
	// 如果隐藏，就压入一个空状态，然后退出; 新建了绘制层次，当前对象自身不显示时，不进入内部
	if(lpRecipient->IsVisible()==false || llLevelCount > 0 && (llMyLevel != moRenderState.Top().mlCrtPtLevel && (llMyLevel != -1 || moRenderState.Top().mlCrtPtLevel != 0)))
	{
		loState.mpEnhancer = NULL;
		loState.mlCrtPtLevel = 0;
		moRenderState.Push(loState);

		return ERESULT_STOP_ENUM_CHILD;
	}

	// 计算当前的世界矩阵和透明度，并且压入状态栈
	ldPosition = lpRecipient->GetPosition();
	lfAngle = lpRecipient->GetRotation(ldCenter);

	// 如果有位移就加上位移的矩阵
	if(ldPosition.x != 0.0f || ldPosition.y != 0.0f)
	{
		//loState.mdTransLation = moRenderState.Top().mdTransLation * D2D1::Matrix3x2F::Translation(ldPosition.x,ldPosition.y);
		loState.mdOffset.x = CExFloat::Round(moRenderState.Top().mdOffset.x + ldPosition.x);
		loState.mdOffset.y = CExFloat::Round(moRenderState.Top().mdOffset.y + ldPosition.y);
		loState.mdTransLation = D2D1::Matrix3x2F::Translation(loState.mdOffset.x,loState.mdOffset.y);
	}
	else
	{
		loState.mdTransLation = moRenderState.Top().mdTransLation;
		loState.mdOffset = moRenderState.Top().mdOffset;
	}

	// 从上层元素复制矩阵
	loState.mbRotated = moRenderState.Top().mbRotated;
	loState.mdRotation = moRenderState.Top().mdRotation;
	loState.mdWorld = loState.mdTransLation * loState.mdRotation;	// 计算上层元素旋转后的矩阵，如果上层元素都没有旋转，这儿就是乘以单位阵
	//loState.mbTopDraw = (moRenderState.Top().mbTopDraw || lpRecipient->CheckStyle(EITR_STYLE_TOPDRAW));
	// 如果本对象建立了新的绘制层次
	if(llLevelCount > 0)
	{
		// 获取新的绘制层次编号
		// 从当前堆栈取到前次遍历保存的绘制层编号,得到新的绘制层次编号
		loState.mlCrtPtLevel = lpRecipient->GetNextPaintLevel(moRenderState.Top().mlPlLevelHost);
		if(loState.mlCrtPtLevel > 0)
			loState.mbPaintItself = false;
	}
	else
	{
		// 从上层复制绘制层次
		loState.mlCrtPtLevel = moRenderState.Top().mlCrtPtLevel;

		// 下面决定本对象自身是否需要显示
		if(llMyLevel >= 0)
		{
			if(llMyLevel != loState.mlCrtPtLevel)
				loState.mbPaintItself = false;
		}
		else
		{
			loState.mbPaintItself = moRenderState.Top().mbPaintItself;
		}
	}
	// 为子树设定它可能展开的绘制层次的初始值
	loState.mlPlLevelHost = -1;



	// 如果当前增加新的旋转设定
	if(lfAngle >= 0.01f)
	{
		// 首先计算新的中心位置
		ldCenter = ldCenter * loState.mdWorld;
		//ldCenter = D2D1::Matrix3x2F::ReinterpretBaseType(&loState.mdWorld)->TransformPoint(ldCenter);点转换乘法

		loState.mdRotation = loState.mdRotation * D2D1::Matrix3x2F::Rotation(lfAngle,ldCenter);	// 两个旋转矩阵单独相乘
		loState.mdWorld = loState.mdTransLation * loState.mdRotation;
		loState.mbRotated = true;
	}

	lfCrtAlpha = lpRecipient->GetAlpha();
	if(lfCrtAlpha < 0.999 || moRenderState.Top().mfAlpha < 0.999)
	{
		loState.mfAlpha = moRenderState.Top().mfAlpha * lfCrtAlpha;
		if(loState.mfAlpha >= 0.999f)
			loState.mfAlpha = 1.0f;
	}
	else
		loState.mfAlpha = 1.0f;

	// 跟随父对象行为
	if(loState.mlCrtPtLevel == 0)
	{
		loState.mpEnhancer = lpRecipient->GetEnhancer();
	}

	//// 测试用代码，正式绘制没有支持缩放的必要
	// 	// 总的缩放控制
	// 	loState.mdWorld =  loState.mdWorld * mdScalMatrixToCapture;

	moRenderState.Push(loState);

	// 设置绘制用的世界转换矩阵
	mpTarget2D->SetTransform(loState.mdWorld);
	lpRecipient->SaveWorldMatrix(loState.mdWorld);

	// 判断是否注册了渲染增效器
	if(loState.mpEnhancer != NULL)
	{
		STEMS_ENHANCER_RENDER ldEnhanceRender;
		ldEnhanceRender.mpPaintBoard = this;
		ldEnhanceRender.mpDestElement = npRecipient;

		luResult = mpElementManager->SimpleSendMessage(loState.mpEnhancer,EMSG_PREPARE_ENHANCER,&ldEnhanceRender,sizeof(ldEnhanceRender),NULL,0);
		if(luResult == ERESULT_SKIP_RENDER_CONTENT || ERESULT_FAILED(luResult))
		{
			// 出错或者无需绘制，只要不是ERESULT_ENUM_CHILD就必然会跳过内容绘制
			return luResult;
		}
	}

	// 设置剪裁区
	if(npRecipient->GetVisibleRegion(ldClipRect)!=false)
	{
		moRenderState.Top().mbHasClipRect = true;
		moRenderState.Top().mdClipRect = ldClipRect;
		mpTarget2D->PushAxisAlignedClip(ldClipRect,D2D1_ANTIALIAS_MODE_ALIASED);
	}


	// 如果该元素要求TopDraw则，暂时不发送消息
	//if(loState.mbTopDraw == false)
	//{
		//// 发送绘制消息
		//mpMessage->SetMessageID(EMSG_PAINT);
		//PVOID lpThis = this;
		//mpMessage->SetInputData(&lpThis,sizeof(lpThis));

		//luResult = mpElementManager->SendMessage(npRecipient,mpMessage);
	//}
	//else
	//	luResult = ERESULT_SUCCESS;
	// 仅在第一次绘制时，绘制本
	if(loState.mbPaintItself != false)
	{
		// 发送绘制消息
		mpMessage->SetMessageID(EMSG_PAINT);
		PVOID lpThis = this;
		mpMessage->SetInputData(&lpThis,sizeof(lpThis));

		luResult = mpElementManager->SendMessage(npRecipient,mpMessage);
	}


	if(ERESULT_SUCCEEDED(luResult))
		return ERESULT_ENUM_CHILD;

//	return luResult;
	// modified by ax 防止某些Element写的不标准，返回错误，现在一律改为不成功则停止绘制它的子元素，但不耽误其他对象的绘制
	return ERESULT_SUCCESS;
}

// 发送Paint后处理
ERESULT __stdcall CXD2dEngine::LeaveForPaint(IEinkuiIterator* npRecipient)
{
	ERESULT luResult = ERESULT_SUCCESS;
	LONG llCrtPaintLevel = -1;
	LONG llNext;

	// 恢复剪裁区
	if(moRenderState.Top().mbHasClipRect != false)
	{
		mpTarget2D->PopAxisAlignedClip();
	}

	if(mpElementManager->GetProbeMode() == 1)
	{
		//D2D1_ANTIALIAS_MODE  ldOldMode = mpTarget2D->GetAntialiasMode();
		//mpTarget2D->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		mpTarget2D->DrawRectangle(D2D1::RectF(0.5f,0.5f,npRecipient->GetSizeX()-0.5f,npRecipient->GetSizeY()-0.5f),mpDbgTextBrush,1.0f);
		//mpTarget2D->SetAntialiasMode(ldOldMode);
	}

	llCrtPaintLevel = moRenderState.Top().mlCrtPtLevel;
	if(moRenderState.Top().mpEnhancer != NULL)
	{
		STEMS_ENHANCER_RENDER ldEnhanceRender;

		ldEnhanceRender.mpPaintBoard = this;
		ldEnhanceRender.mpDestElement = npRecipient;

		luResult = mpElementManager->SimpleSendMessage(moRenderState.Top().mpEnhancer,EMSG_RENDER_ENHANCER,&ldEnhanceRender,sizeof(ldEnhanceRender),NULL,0);
		if(luResult == ERESULT_REDO_RENDER_CONTENT)	// 再次执行绘制
			luResult = ERESULT_REDO_ENUM;
		else
			if(luResult != ERESULT_DIRECTX_ERROR)	// 如果是Dx错误，就不能继续执行绘制了
				luResult = ERESULT_SUCCESS;	// 忽略掉一般性地错误，我们不应该中止对其他对象的绘制

	}


	moRenderState.Pop();

	// 如果当前对象建立的绘制层次，并且还有没有执行完的绘制层次，则返回重新枚举
	if(luResult != ERESULT_REDO_ENUM && ERESULT_SUCCEEDED(luResult))// 如果绘制内容提出的重新枚举，就无需判断了
	{
		llNext = ((CXuiIterator*)npRecipient)->GetNextPaintLevel(llCrtPaintLevel);
		if(llNext > llCrtPaintLevel)
		{
			luResult = ERESULT_REDO_ENUM;
			moRenderState.Top().mlPlLevelHost = llNext;
		}
	}

	return luResult;
}


// 发送Capture前预处理
ERESULT __stdcall CXD2dEngine::EnterForCapture(IEinkuiIterator* npRecipient)
{
	ERESULT luResult;
	CErdState loState;
	D2D1_POINT_2F ldPosition;
	D2D1_POINT_2F ldCenter;
	FLOAT lfAngle;
	CXuiIterator* lpRecipient;
	D2D1_RECT_F ldClipRect;

	// 如果还没有开始捕获，则检查是不是祖先；如果已经开始捕获，则检查是否隐藏；对于无需处理的状态，就压入一个空状态，然后退出
	if(mbCapturing == false && mpToCapture->FindAncestor(npRecipient)==false || mbCapturing != false && npRecipient->IsVisible()==false)
	{
		loState.mpEnhancer = NULL;
		moRenderState.Push(loState);

		return ERESULT_STOP_ENUM_CHILD;
	}

	lpRecipient = dynamic_cast<CXuiIterator*>(npRecipient);

	if(mpToCapture == npRecipient)
	{
		mbCapturing = true;
		// 忽略局部坐标转换，直接定位到世界坐标原点

		loState.mdOffset.x = CExFloat::Round(-mdCaptureRegion.left);
		loState.mdOffset.y = CExFloat::Round(-mdCaptureRegion.top);
		loState.mdTransLation = D2D1::Matrix3x2F::Translation(-mdCaptureRegion.left,-mdCaptureRegion.top);

		loState.mdWorld = loState.mdTransLation;
		loState.mdRotation = mdIdentyMatrix;
		loState.mfAlpha = 1.0f;
		loState.mpEnhancer = NULL;
		loState.mbRotated = false;

	}
	else
	{
		// 计算当前的世界矩阵和透明度，并且压入状态栈
		ldPosition = lpRecipient->GetPosition();
		lfAngle = lpRecipient->GetRotation(ldCenter);

		// 如果有位移就加上位移的矩阵
		if(ldPosition.x != 0.0f || ldPosition.y != 0.0f)
		{
			//loState.mdTransLation = moRenderState.Top().mdTransLation * D2D1::Matrix3x2F::Translation(ldPosition.x,ldPosition.y);
			loState.mdOffset.x = CExFloat::Round(moRenderState.Top().mdOffset.x + ldPosition.x);
			loState.mdOffset.y = CExFloat::Round(moRenderState.Top().mdOffset.y + ldPosition.y);
			loState.mdTransLation = D2D1::Matrix3x2F::Translation(loState.mdOffset.x,loState.mdOffset.y);
		}
		else
		{
			loState.mdTransLation = moRenderState.Top().mdTransLation;
			loState.mdOffset = moRenderState.Top().mdOffset;
		}

		// 从上层元素复制矩阵
		loState.mbRotated = moRenderState.Top().mbRotated;
		loState.mdRotation = moRenderState.Top().mdRotation;
		loState.mdWorld = loState.mdTransLation * loState.mdRotation;	// 计算上层元素旋转后的矩阵，如果上层元素都没有旋转，这儿就是乘以单位阵
		//loState.mbTopDraw = (moRenderState.Top().mbTopDraw || lpRecipient->CheckStyle(EITR_STYLE_TOPDRAW));

		// 如果当前增加新的旋转设定
		if(lfAngle >= 0.01f)
		{
			// 首先计算新的中心位置
			ldCenter = ldCenter * loState.mdWorld;
			//ldCenter = D2D1::Matrix3x2F::ReinterpretBaseType(&loState.mdWorld)->TransformPoint(ldCenter);点转换乘法

			loState.mdRotation = loState.mdRotation * D2D1::Matrix3x2F::Rotation(lfAngle,ldCenter);	// 两个旋转矩阵单独相乘
			loState.mdWorld = loState.mdTransLation * loState.mdRotation;
			loState.mbRotated = true;

		}

		loState.mfAlpha = moRenderState.Top().mfAlpha * lpRecipient->GetAlpha();
		if(loState.mfAlpha >= 0.9999f)
			loState.mfAlpha = 1.0f;

		loState.mpEnhancer = lpRecipient->GetEnhancer();
	}
	// 总的缩放控制
	loState.mdWorld =  loState.mdWorld * mdScalMatrixToCapture;

	moRenderState.Push(loState);

	// 设置绘制用的世界转换矩阵
	mpTarget2D->SetTransform(loState.mdWorld);
	//lpRecipient->SaveWorldMatrix(loState.mdWorld); 捕捉图像，不能将坐标转换保存遗留

	if(mbCapturing == false)	// 还没开始捕捉，如果不希望抓取背景 Ax Jul.09,2012
	{
		return ERESULT_ENUM_CHILD;
	}

	// 设置剪裁区
	if(npRecipient->GetVisibleRegion(ldClipRect)!=false)	
	{
		moRenderState.Top().mbHasClipRect = true;
		moRenderState.Top().mdClipRect = ldClipRect;
		mpTarget2D->PushAxisAlignedClip(ldClipRect,D2D1_ANTIALIAS_MODE_ALIASED);
	}


	// 如果该元素要求拍照时隐藏，暂时不发送消息
	if(lpRecipient->CheckStyle(EITR_STYLE_SNAPSHOT_HIDE) == false)
	{
		 //发送绘制消息
		mpMessage->SetMessageID(EMSG_PAINT);
		PVOID lpThis = this;
		mpMessage->SetInputData(&lpThis,sizeof(lpThis));

		luResult = mpElementManager->SendMessage(npRecipient,mpMessage);

		if(ERESULT_SUCCEEDED(luResult))
			return ERESULT_ENUM_CHILD;
	}

	//	return luResult;
	// modified by ax 防止某些Element写的不标准，返回错误，现在一律改为不成功则停止绘制它的子元素，但不耽误其他对象的绘制
	return ERESULT_SUCCESS;
}

// 发送Capture后处理
ERESULT __stdcall CXD2dEngine::LeaveForCapture(IEinkuiIterator* npRecipient)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if(mpToCapture == npRecipient)
	{
		mbCapturing = false;
	}

	// 恢复剪裁区
	if(moRenderState.Top().mbHasClipRect != false)
	{
		mpTarget2D->PopAxisAlignedClip();
	}

	moRenderState.Pop();

	return luResult;
}

// 发送EMSG_DISCARD_DEVICE_RESOURCE前预处理
ERESULT __stdcall CXD2dEngine::EnterForDiscardDeviceRes(IEinkuiIterator* npRecipient)
{
	mpElementManager->SimpleSendMessage(npRecipient,EMSG_DISCARD_DEVICE_RESOURCE,NULL,0,NULL,0);
	return ERESULT_ENUM_CHILD;
}

// 发送EMSG_DISCARD_DEVICE_RESOURCE后处理
ERESULT __stdcall CXD2dEngine::LeaveForDiscardDeviceRes(IEinkuiIterator* npRecipient)
{
	return ERESULT_SUCCESS;
}

//获取当前画板图像，获取的HBITMAP对象，由调用者来释放
HBITMAP CXD2dEngine::GetCurrentBitmap(LONG nlWidth,LONG nlHeight)
{
	BITMAPINFO BmpInfo;
	HBITMAP lhBmpHandle = NULL;
	UCHAR* lpArgb = NULL;
	HDC lhCompatibleDc = NULL;
	HBITMAP lhOldBitmap = NULL;

	ERESULT luResult;
	D2D1_COLOR_F ldOldBackColor;

	float lfWidth = 0;
	float lfHeight = 0;
	float lfW = (float)nlWidth / muEinkPanelW;
	float lfH = (float)nlHeight / muEinkPanelH;

	do
	{

		if (lfW > lfH)
		{
			lfWidth = muEinkPanelW * lfH;
			lfHeight = muEinkPanelH * lfH;
		}
		else
		{
			lfWidth = muEinkPanelW * lfW;
			lfHeight = muEinkPanelH * lfW;
		}

		// 首先执行绘制操作
		mdCaptureRegion.left = mdCaptureRegion.top = 0.0f;
		mdCaptureRegion.right = (FLOAT)muEinkPanelW;
		mdCaptureRegion.bottom = (FLOAT)muEinkPanelH;
		mdCaptureBriefSize.width = lfWidth;
		mdCaptureBriefSize.height = lfHeight;




		// 按照缩放比例，准备一个转换矩阵
		mdScalMatrixToCapture = D2D1::Matrix3x2F::Scale(mdCaptureBriefSize.width / (mdCaptureRegion.right - mdCaptureRegion.left), mdCaptureBriefSize.height / (mdCaptureRegion.bottom - mdCaptureRegion.top),
			D2D1::Point2F(0.0f, 0.0f));
		//mdScalMatrixToCapture = D2D1::Matrix3x2F::Identity();

		ldOldBackColor = mdBackColor;
		mdBackColor = D2D1::ColorF(D2D1::ColorF::Black, 0.0f);

		luResult = DoRender(GetCurrentTickCount(),false, EinkuiGetSystem()->GetSystemWidget()->GetHomePage());

		mdBackColor = ldOldBackColor;

		if (luResult != ERESULT_SUCCESS)
			break;

		ZeroMemory(&BmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER));
		BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BmpInfo.bmiHeader.biWidth = nlWidth;
		BmpInfo.bmiHeader.biHeight = -(int)nlHeight;
		BmpInfo.bmiHeader.biPlanes = 1;
		BmpInfo.bmiHeader.biBitCount = 32;

		lhBmpHandle = CreateDIBSection(mhForeDc, &BmpInfo, DIB_RGB_COLORS, (void**)&lpArgb, NULL, 0);
		BREAK_ON_NULL(lhBmpHandle);

		lhCompatibleDc = CreateCompatibleDC(mhForeDc);
		BREAK_ON_NULL(lhCompatibleDc);

		lhOldBitmap = (HBITMAP)SelectObject(lhCompatibleDc, lhBmpHandle);
		::BitBlt(lhCompatibleDc, (nlWidth - (ULONG)lfWidth) / 2, (nlHeight - (ULONG)lfHeight) / 2, (ULONG)lfWidth, (ULONG)lfHeight, mhForeDc, 0, 0, SRCCOPY);

	} while (false);

	if (lhOldBitmap != NULL)
		SelectObject(lhCompatibleDc, lhOldBitmap);

	if (lhCompatibleDc != NULL)
		ReleaseDC(NULL, lhCompatibleDc);

	return lhBmpHandle;
}

// 设定自绘函数，设定后Xui系统不在调用Eink绘制，仅仅将rgb32的缓冲区提供给此处设定的回调函数
void CXD2dEngine::SetCustomDraw(PXUI_CUSTOM_DRAW_CALLBACK CustomDraw)
{
	InterlockedExchangePointer(reinterpret_cast<PVOID*>(&mpCustomDraw), CustomDraw);
}

// 执行拍照任务
IEinkuiBitmap* CXD2dEngine::TakeSnapshot(
	IEinkuiIterator* npToShot,
	const D2D1_RECT_F& crSourceRegion,	// 采样区域，目标元素的局部坐标系
	const D2D_SIZE_F& crBriefSize,		// 缩略图尺寸，快照的结果是一副缩略图
	const FLOAT* ColorRGBA
)
{
	ERESULT luResult;
	D2D1_COLOR_F ldOldBackColor;
	IEinkuiBitmap* lpBitmap = NULL;
	// 首先执行绘制操作
	mdCaptureRegion = crSourceRegion;
	mdCaptureBriefSize = crBriefSize;

	// 按照缩放比例，准备一个转换矩阵
	mdScalMatrixToCapture = D2D1::Matrix3x2F::Scale(crBriefSize.width / (crSourceRegion.right - crSourceRegion.left), crBriefSize.height / (crSourceRegion.bottom - crSourceRegion.top),
		D2D1::Point2F(0.0f, 0.0f));

	if (ColorRGBA != NULL)
	{
		ldOldBackColor = mdBackColor;
		mdBackColor.r = ColorRGBA[0];
		mdBackColor.g = ColorRGBA[1];
		mdBackColor.b = ColorRGBA[2];
		mdBackColor.a = ColorRGBA[3];
	}

	luResult = DoRender(GetCurrentTickCount(),false, npToShot);

	if (ColorRGBA != NULL)
	{
		mdBackColor = ldOldBackColor;
	}

	if (luResult != ERESULT_SUCCESS)
		return NULL;

	char *	lpBitmapBuffer = NULL;
	LONG liBrfW = CExFloat::ToLong(crBriefSize.width);
	LONG liBrfH = CExFloat::ToLong(crBriefSize.height);
	LONG liStrideSrc = muEinkPanelW * 4;
	LONG liStrideDst = liBrfW * 4;

	// 分配缩略图的内存
	lpBitmapBuffer = new char[liStrideDst * liBrfH + 4];
	RETURN_ON_NULL(lpBitmapBuffer,NULL);

	// 复制原始数据
	for (LONG Line = 0; Line < liBrfH; Line++)
		RtlCopyMemory(lpBitmapBuffer + liStrideDst*Line, mpForeBuffer + liStrideSrc*Line, liStrideDst);

	// 建立位图对象
	lpBitmap = CXD2dBitmap::CreateInstance(liBrfW,liBrfH, 4,liStrideDst, lpBitmapBuffer);

	CMM_SAFE_DELETE(lpBitmapBuffer);

	// 返回新建的对象
	return lpBitmap;
}
