/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"

#include "XgD2DBrush.h"


DEFINE_BUILTIN_NAME(CXD2DBrush)


CXD2DBrush::CXD2DBrush()
{
	mpSolidBrush = NULL;
	mpLinearGradientBrush = NULL;
	mpRadialGradientBrush = NULL;
	mpBitmapBrush = NULL;
	mpStrokeStyle = NULL;
	
	mcStrokeCreate = false;
	mfStrokeWidth = 1.0f;

	mpDashes = NULL;
	muCount = 0;



}

// 资源释放
CXD2DBrush::~CXD2DBrush()
{
	CMM_SAFE_RELEASE(mpSolidBrush);
	CMM_SAFE_RELEASE(mpLinearGradientBrush);
	CMM_SAFE_RELEASE(mpRadialGradientBrush);
	CMM_SAFE_RELEASE(mpBitmapBrush);
	CMM_SAFE_RELEASE(mpStrokeStyle);

	if (mpDashes != NULL)
		delete mpDashes;

}

// 实现这个接口很重要，否则程序在退出时，会出现释放问题
int CXD2DBrush::Release()
{
	int liCount = cmmBaseObject<CXD2DBrush, IEinkuiBrush, GET_BUILTIN_NAME(CXD2DBrush)>::Release();
	// 如果返回值是0，表示需要被删除
	if(liCount == 0)
	{
		CEinkuiSystem::gpXuiSystem->GetBrushList().UnregisteBrush(this);
	}

	return liCount;	
}


ULONG CXD2DBrush::InitOnCreate(XuiBrushType niBrushType, D2D1_COLOR_F noColor)
{
	miBrushType = niBrushType;
	moSolidBrushColor = noColor;
		
	return 0;
}

ULONG CXD2DBrush::InitOnCreate(XuiBrushType niBrushType, D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties)
{
	miBrushType = niBrushType;

	for (ULONG liLoop = 0; liLoop < nuCount; liLoop++)
	{
		mdGradientStop.Insert(-1,npGradientStop[liLoop]);
	}

	mdLinearGradientBrushProperties = ndLinearGradientBrushProperties;

	muCount = nuCount;

	return 0;
}

// 对象被使用时，需要创建画刷资源（与平台相关资源），此时，通过平台调用者把参数npRenderTarget传进初始化函数
ULONG CXD2DBrush::InitBrush(ID2D1RenderTarget* npRenderTarget,  ID2D1Factory* npD2D1Factory)
{
	ULONG luStatus = 0;
	HRESULT hr = 0;

	do 
	{
		switch(miBrushType)
		{
		case XuiStroke:
		case XuiSolidBrush:
			{
				if (mpSolidBrush == NULL)
				{
					hr = npRenderTarget->CreateSolidColorBrush(
						moSolidBrushColor,
						&mpSolidBrush
						);

				}
				else
				{
					// 对于已经创建的SolidBrush，其参数有可能发生改变，所以重新调用一次
					mpSolidBrush->SetColor(moSolidBrushColor);
				}

				break;
			}
		case XuiLinearGradientBrush:
			{
				ID2D1GradientStopCollection *lpGradientStops = NULL;
				if (mpLinearGradientBrush == NULL)
				{
					hr = npRenderTarget->CreateGradientStopCollection(
						mdGradientStop.GetBuffer(),
						mdGradientStop.Size(),
						D2D1_GAMMA_2_2,
						D2D1_EXTEND_MODE_CLAMP,
						&lpGradientStops
						);

					if (SUCCEEDED(hr))
					{
						hr = npRenderTarget->CreateLinearGradientBrush(
							mdLinearGradientBrushProperties,
							lpGradientStops,
							&mpLinearGradientBrush
							);
					}
				}
				else
				{
					// 对于已经创建的SolidBrush，其参数有可能发生改变，所以重新调用一次
					mpLinearGradientBrush->SetStartPoint(mdLinearGradientBrushProperties.startPoint);
					mpLinearGradientBrush->SetEndPoint(mdLinearGradientBrushProperties.endPoint);
				}

				CMM_SAFE_RELEASE(lpGradientStops);

				break;
			}
		case XuiRadialGradientBrush:
			{
				ID2D1GradientStopCollection *lpGradientStops = NULL;
				if (mpRadialGradientBrush == NULL)
				{
					hr = npRenderTarget->CreateGradientStopCollection(
						mdGradientStop.GetBuffer(),
						mdGradientStop.Size(),
						D2D1_GAMMA_2_2,
						D2D1_EXTEND_MODE_CLAMP,
						&lpGradientStops
						);

					float lfMaxRadius;
					if (mdLinearGradientBrushProperties.startPoint.x < mdLinearGradientBrushProperties.startPoint.y)
						lfMaxRadius = mdLinearGradientBrushProperties.startPoint.y;
					else
						lfMaxRadius = mdLinearGradientBrushProperties.startPoint.x;

					if (SUCCEEDED(hr))
					{
						hr = npRenderTarget->CreateRadialGradientBrush(
							D2D1::RadialGradientBrushProperties(
							mdLinearGradientBrushProperties.startPoint,
							mdLinearGradientBrushProperties.endPoint,
							lfMaxRadius,
							lfMaxRadius),
							lpGradientStops,
							&mpRadialGradientBrush
							);
					}
				}
				else
				{
					// 对于已经创建的SolidBrush，其参数有可能发生改变，所以重新调用一次
					mpRadialGradientBrush->SetCenter(mdLinearGradientBrushProperties.startPoint);
				}

				CMM_SAFE_RELEASE(lpGradientStops);

				break;
			}
		case XuiBitmapBrush:
			{
				break;
			}
		default:
			{
				luStatus = ERESULT_WRONG_PARAMETERS;
				break;
			}
		}


	} while (false);

	if (mcStrokeCreate != false)
	{
		// 线型对象好像一经创建，就不能改变其属性
		if (mpStrokeStyle == NULL)
		{
			hr = npD2D1Factory->CreateStrokeStyle(
				moStrokeStyleProperties,
				mpDashes,
				muDashCount,
				&mpStrokeStyle);
		}

	}



	if (SUCCEEDED(hr))
		luStatus = ERESULT_SUCCESS;
	else
		luStatus = ERESULT_UNSUCCESSFUL;

	return luStatus;
}

// 设置SOLID画刷的颜色
void CXD2DBrush::SetColor(IN D2D1_COLOR_F noColor)
{
	moSolidBrushColor = D2D1::ColorF(noColor.r, noColor.g, noColor.b, noColor.a);

}

// 设置渐变画刷的属性
void CXD2DBrush::SetLinearBrushProperties(
	const D2D1_GRADIENT_STOP* npGradientStop, 
	ULONG nuCount, 
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
	)
{
	
	for (ULONG liLoop = 0; liLoop < nuCount; liLoop++)
	{
		mdGradientStop.Insert(-1,npGradientStop[liLoop]);
	}

	mdLinearGradientBrushProperties = ndLinearGradientBrushProperties;
	muCount = nuCount;

}

// 设置线型，用D2D的结构体来描述
bool CXD2DBrush::SetStrokeType(const D2D1_STROKE_STYLE_PROPERTIES &strokeStyleProperties, const FLOAT *dashes, UINT dashesCount)
{
	bool lbStatus = false;

	do 
	{
		// 如果已经设置过线型，则不可以再次设置
		BREAK_ON_FALSE(!mcStrokeCreate);

		moStrokeStyleProperties = strokeStyleProperties;
		lbStatus = true;

		// 判断是否有自定义线型
		BREAK_ON_NULL(dashes);
		// 如果有，则重新设置状态
		lbStatus = false;
		BREAK_ON_FALSE(dashesCount != 0);
		CMM_SAFE_DELETE(mpDashes);

		mpDashes = new FLOAT[dashesCount];
		for (ULONG liLoop = 0; liLoop < dashesCount; liLoop++)
		{
			mpDashes[liLoop] = dashes[liLoop];
		}

		muDashCount = dashesCount;

		lbStatus = true;
	} while (false);

	if (lbStatus != false)
		mcStrokeCreate = true;	

	return lbStatus;
}

// 设置线型宽度
void CXD2DBrush::SetStrokeWidth(IN float nfWidth)
{
	mfStrokeWidth = nfWidth;

}

// 获取线型宽度
float CXD2DBrush::GetStrokeWidth()
{
	return mfStrokeWidth;
}

// 获取画刷对象
ERESULT CXD2DBrush::GetBrushObject(OUT ID2D1Brush** npBrushObject)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	// 首先判断是哪种类型的画刷，然后返回对应的画刷句柄
	
	switch(miBrushType)
	{
	case XuiStroke:
	case XuiSolidBrush:
		{
			if (mpSolidBrush != NULL)
			{
				*npBrushObject = mpSolidBrush;
				luResult = ERESULT_SUCCESS;
			}
			break;
		}
	case XuiLinearGradientBrush:
		{
			if (mpLinearGradientBrush != NULL)
			{
				*npBrushObject = mpLinearGradientBrush;
				luResult = ERESULT_SUCCESS;
			}
			break;
		}
	case XuiRadialGradientBrush:
		{
			if (mpRadialGradientBrush != NULL)
			{
				*npBrushObject = mpRadialGradientBrush;
				luResult = ERESULT_SUCCESS;
			}
			break;
		}
	case XuiBitmapBrush:
		{
			if (mpBitmapBrush != NULL)
			{
				*npBrushObject = mpBitmapBrush;
				luResult = ERESULT_SUCCESS;
			}
			break;
		}
	default:
		break;
	}

	return luResult;
}

// 获取线型对象
ERESULT CXD2DBrush::GetStrokeObject(OUT ID2D1StrokeStyle** npStrokeObject)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;	// 这个函数的返回值，好像没啥用处


	do 
	{
		if (mpStrokeStyle != NULL)
			*npStrokeObject = mpStrokeStyle;
		else
			*npStrokeObject = NULL;
	
		luResult = ERESULT_SUCCESS;

	} while (false);

	return luResult;
}


void CXD2DBrush::DiscardsBrushResource()
{
	CMM_SAFE_RELEASE(mpStrokeStyle);
	CMM_SAFE_RELEASE(mpSolidBrush);
	CMM_SAFE_RELEASE(mpLinearGradientBrush);
	CMM_SAFE_RELEASE(mpRadialGradientBrush);
	CMM_SAFE_RELEASE(mpBitmapBrush);

}

// 复制出一个新的对象
IEinkuiBrush* __stdcall CXD2DBrush::DuplicateBrush(void)
{

	IEinkuiBrush* lpNewBrush = NULL;

	do 
	{
		switch(miBrushType)
		{
		case XuiStroke:
			{
				lpNewBrush = CXD2DBrush::CreateInstance(XuiStroke, moSolidBrushColor);
				break;
			}
		case XuiSolidBrush:
			{
				lpNewBrush = CXD2DBrush::CreateInstance(XuiSolidBrush, moSolidBrushColor);
				break;
			}
		case XuiLinearGradientBrush:
			{
				lpNewBrush = CXD2DBrush::CreateInstance(XuiLinearGradientBrush, mdGradientStop.GetBuffer(),mdGradientStop.Size(), mdLinearGradientBrushProperties);
				break;
			}
		case XuiRadialGradientBrush:
			{
				lpNewBrush = CXD2DBrush::CreateInstance(XuiRadialGradientBrush, mdGradientStop.GetBuffer(),mdGradientStop.Size(), mdLinearGradientBrushProperties);
				break;
			}

		}

		BREAK_ON_NULL(lpNewBrush);

		// 是否需要设置线型
		if (mcStrokeCreate != false)
		{
			lpNewBrush->SetStrokeType(moStrokeStyleProperties, mpDashes, muDashCount);
		}


	} while (false);

	return lpNewBrush;
}