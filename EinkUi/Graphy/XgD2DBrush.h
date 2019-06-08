/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _LW_D2DBRUSH
#define _LW_D2DBRUSH



// 基于D2D的画刷类
// 主要功能：
// 1，可以创建各种类型的画刷
// 2，可以设置画刷的基本属性
// 3，可以获取对应平台的画刷对象
// 4，设置线型



DECLARE_BUILTIN_NAME(CXD2DBrush)
class CXD2DBrush: public cmmBaseObject<CXD2DBrush,IEinkuiBrush, GET_BUILTIN_NAME(CXD2DBrush)> 
{

//////////////////////////////////////////////////////////////////////////
// 初始化部分
//////////////////////////////////////////////////////////////////////////
public:
	CXD2DBrush();
	~CXD2DBrush();

	// 这个初始化函数中，只实例化对象本身，并不创建任何与平台相关的资源，直到该对象被使用时，调用InitBrush初始化画刷
	DEFINE_CUMSTOMIZE_CREATE(CXD2DBrush,(XuiBrushType niBrushType, D2D1_COLOR_F noColor),(niBrushType, noColor))
	ULONG InitOnCreate(XuiBrushType niBrushType, D2D1_COLOR_F noColor);

	// 渐变画刷时，需要传入多个颜色点
	DEFINE_CUMSTOMIZE_CREATE(CXD2DBrush,
		(XuiBrushType niBrushType, D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties),			(niBrushType, npGradientStop, nuCount, ndLinearGradientBrushProperties))
	ULONG InitOnCreate(XuiBrushType niBrushType, D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties);


	// 对象被使用时，需要创建画刷资源（与平台相关资源），此时，通过平台调用者把参数传进初始化函数
	ERESULT __stdcall InitBrush(ID2D1RenderTarget* npRenderTarget,  ID2D1Factory* npD2D1Factory);		


//////////////////////////////////////////////////////////////////////////
// 实现基类接口
//////////////////////////////////////////////////////////////////////////

// 基于D2D和GDIPlush平台的公共行为抽象
public:
	// 设置SOLID画刷的颜色
	virtual void __stdcall SetColor(IN D2D1_COLOR_F noColor);

	// 设置渐变画刷的属性
	virtual void __stdcall SetLinearBrushProperties(
		const D2D1_GRADIENT_STOP* npGradientStop, 
		ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties		
		);

	// 设置线型，用D2D的结构体来描述
	virtual bool __stdcall SetStrokeType(const D2D1_STROKE_STYLE_PROPERTIES &strokeStyleProperties, const FLOAT *dashes, UINT dashesCount);

	// 设置线宽
	virtual void __stdcall SetStrokeWidth(IN float nfWidth);

	// 获取线宽
	virtual float __stdcall GetStrokeWidth();

	// 供系统内部调用，废弃Direct2D的设备相关资源
	virtual void __stdcall DiscardsBrushResource(void);
	
	// 实现这个接口很重要，否则程序在退出时，会出现释放问题
	virtual int __stdcall Release(void);

	// 复制出一个新的对象,复制出来的对象，需要使用者自己释放
	virtual IEinkuiBrush* __stdcall DuplicateBrush(void);

// 基于D2D的特殊抽象行为
public:
	// 获取画刷对象
	virtual ERESULT __stdcall GetBrushObject(OUT ID2D1Brush** npBrushObject);

	// 获取描述线型的Stroke对象
	virtual ERESULT __stdcall GetStrokeObject(OUT ID2D1StrokeStyle** npStrokeObject);


//////////////////////////////////////////////////////////////////////////
// 定义相关属性及资源
//////////////////////////////////////////////////////////////////////////
protected:
	// 固定颜色的填充画刷
	ID2D1SolidColorBrush* mpSolidBrush;
	// 线性渐变画刷
	ID2D1LinearGradientBrush* mpLinearGradientBrush;
	// 径向渐变画刷
	ID2D1RadialGradientBrush* mpRadialGradientBrush;
	// 位图画刷
	ID2D1BitmapBrush* mpBitmapBrush;

	// 画刷类型
	XuiBrushType miBrushType;
	// SOLID画刷颜色
	D2D1_COLOR_F	moSolidBrushColor;		

	// 渐变画刷预设有4个可调节的点
	cmmVector<D2D1_GRADIENT_STOP,4> mdGradientStop;
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES	mdLinearGradientBrushProperties;
	ULONG	muCount;


	// 标志是否要创建线型对象
	bool	mcStrokeCreate;
	// 线型对象
	ID2D1StrokeStyle* mpStrokeStyle;
	// 线型宽度
	float	mfStrokeWidth;
	// 线型属性
	D2D1_STROKE_STYLE_PROPERTIES moStrokeStyleProperties;
	// 线型自定义属性
	FLOAT* mpDashes;
	UINT   muDashCount;

};


#endif