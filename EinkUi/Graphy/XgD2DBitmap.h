/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _LW_D2DBITMAPIMP
#define _LW_D2DBITMAPIMP

// 透明度阈值，超出此值则判断为透明
#define  TRANSPARENT_THRESHOLD_VALUE	12




// 定义bitmap类
// 代码变动有以下两点：
// 1，对象第一次初始化的时候，只初始化数据部分。数据抽象成四种类型，位图型对象数据，文字型对象数据，纹理型对象数据，ICON型对象数据。
// 2，在位图对象被使用时，进行与平台资源相关的初始化。

enum BitmapType{
	CommonBitmap,
	IconBitmap,
	TextBitmap,
	MemoryBitmap,
	None,
};



DECLARE_BUILTIN_NAME(CXD2dBitmap)
class CXD2dBitmap : public cmmBaseObject<CXD2dBitmap, IEinkuiBitmap, GET_BUILTIN_NAME(CXD2dBitmap)>
{

//////////////////////////////////////////////////////////////////////////
// 初始化部分
//////////////////////////////////////////////////////////////////////////
public:
	CXD2dBitmap();
	~CXD2dBitmap();

	// 通过位图文件实例化位图对象
	DEFINE_CUMSTOMIZE_CREATE(CXD2dBitmap,(const wchar_t *npFileName),(npFileName))
	ULONG InitOnCreate(IN const wchar_t *npFileName);

	//// 通过文字实例化位图对象		好像从来没有用过，屏蔽了 ax nov.28,2017
	//// 提供参数如下：
	////	npText -- 宽字符文字串（目前不支持换行）
	////	dwColor -- RGB格式的颜色值
	////	npFont -- 指定字体名，如"Tahoma"
	////	dwFontSize -- 指定字体大小
	//DEFINE_CUMSTOMIZE_CREATE(CXD2dBitmap, (const wchar_t *npText, DWORD dwColor, const wchar_t *npFont, DWORD dwFontSize), (npText, dwColor, npFont, dwFontSize));
	//ULONG InitOnCreate(IN const wchar_t *npText, IN DWORD dwTextColor, IN const wchar_t *npFont, IN DWORD dwFontSize);

	// 通过Icon创建位图对象
	DEFINE_CUMSTOMIZE_CREATE(CXD2dBitmap, (wchar_t *npPeFileName, int nwIconIdx, int niXSize, int niYSize), (npPeFileName, nwIconIdx, niXSize, niYSize))
	ULONG InitOnCreate(wchar_t *npPeFileName, int niIconIdx, int niXSize, int niYSize);

	// 指定位图大小，以及位图数据，创建一个位图对象
	DEFINE_CUMSTOMIZE_CREATE(CXD2dBitmap, (LONG nuWidth, LONG nuHeight,LONG PixelSize, LONG Stride, void* npRawData), (nuWidth, nuHeight,PixelSize,Stride,npRawData))
	ULONG InitOnCreate(LONG nuWidth, LONG nuHeight,LONG PixelSize,LONG Stride,void* npRawData);

	// 释放对象
	virtual int __stdcall Release(void);

	
	// 采用后加载模型。对象被使用时，需要创建位图资源（与平台相关资源），此时，通过平台调用者把参数传进初始化函数

	// 初始化普通位图对象
	ERESULT __stdcall InitCommonBitmap(IWICImagingFactory* npWICFactory);		
	
	//// 初始化文字型位图对象
	//ERESULT __stdcall InitTextBitmap(IWICImagingFactory* npWICFactory, ID2D1Factory* npD2D1Factory);	

	// 初始化ICON型位图对象
	ERESULT __stdcall InitIconBitmap(IWICImagingFactory* npWICFactory);
	
	// 初始化内存数据型对象
	ERESULT __stdcall InitMemoryBitmap(ID2D1RenderTarget* npRenderTarget);	

	// 初始化位图，根据位图类型，间接的调用不同类型的初始化函数
	ERESULT __stdcall InitBitmap(IWICImagingFactory* npWICFactory, ID2D1Factory* npD2D1Factory, ID2D1RenderTarget* npRenderTarget);

//////////////////////////////////////////////////////////////////////////
// 实现基类接口
//////////////////////////////////////////////////////////////////////////
public:

	// 获取指定坐标的透明度
	virtual ERESULT __stdcall GetPixel(IN DWORD x, IN DWORD y, DWORD &nPixel);

	virtual ERESULT __stdcall GetBmpInfo(OUT BITMAP* npBmpInfo);

	// 获取位图宽（像素）
	virtual UINT __stdcall GetWidth();
	// 获取位图高（像素）
	virtual UINT __stdcall GetHeight();
	//设置延展线
	virtual void __stdcall SetExtendLine(IN LONG nlX,IN LONG nlY);
	//获取横向延展线
	virtual LONG __stdcall GetExtnedLineX(void);
	//获取纵向延展线
	virtual LONG __stdcall GetExtnedLineY(void);

	// 对位图进行缩放，使用者需要重新调用GetWICObject/GetD2DObject以获取
	// 缩放后的位图操作接口
	virtual ERESULT __stdcall Scale(UINT npWidth, UINT npHeight);

	// 获取IWICBitmap接口指针，使用者需要自行释放此接口指针
	virtual ERESULT __stdcall GetWICObject(OUT IWICBitmap** ppWicBitmap);

	// 供系统内部调用，废弃Direct2D的设备相关资源
	virtual void __stdcall DiscardsBitmapResource(void);

	// 复制出一个新的对象,复制出来的对象，需要使用者自己释放
	virtual IEinkuiBitmap* __stdcall DuplicateBitmap(void);

public:
	// 获取指定区域的位图数据
	virtual bool __stdcall GetBitmapBuffer(IN D2D1_RECT_F ndSrcRect, OUT void* npRawData);

	// 保存成缩略图
	virtual bool __stdcall SaveAsThumbnail(IN LONG nuWidth, IN LONG nuHeight, IN D2D1_RECT_F ndSampleRect, const wchar_t* nswOutputFilePath);


public:

	// 获取ID2D1Bitmap接口指针，使用者需要自行释放此接口指针
	ERESULT __stdcall GetD2DObject(IN ID2D1RenderTarget *npRT, OUT ID2D1Bitmap **nppParentBitmap);

	ERESULT CreateD2DObjFromBmpSource(IN ID2D1RenderTarget *npRT, IN IWICBitmapSource *npSourceBmp, OUT ID2D1Bitmap **nppParentBitmap);

	// 获取位图的宽高信息
	ULONG GetBitmapDimension();

	ERESULT GenTransparentInfo();

protected:
	//private:
	HANDLE m_hBitmap;
	ID2D1Bitmap* mpBitmap2D;
	IWICBitmapDecoder *m_pIDecoder;
	IWICBitmapFrameDecode *m_pIDecoderFrame;
	IWICBitmapScaler *m_pBmpScaler;

	HRESULT m_hResult;
	UINT m_nWidth;
	UINT m_nHeight;
	UINT m_nScaleWidth;
	UINT m_nScaleHeight;
	LONG m_lExtendLineX;
	LONG m_lExtendLineY;

	IWICBitmap *m_pWICBitmap;

	CBitArray *mpTransparentInfo;

	// 渲染对象,初始化位图的时候，暂存下来，主要是为了生成缩略图的时候利用该参数获取原始图的D2D位图对象。
	// 这个方法不好，应该考虑其他方法
	ID2D1RenderTarget* mpTempRenderTarget;


// 关于位图对象的数据定义，按创建位图的不同方式，分为四种，位图型对象数据，文字型对象数据，纹理型对象数据，ICON型对象数据
protected:
	// 标识位图对象类型
	BitmapType meBitmapType;

	// 指示是否已经初始化
	bool	mcIsInit;		

	// 位图型对象数据
	wchar_t mswFileName[MAX_PATH];

	//// 文字型对象数据
	//wchar_t* mpText;		// 因为文字内容长度不固定，所以要动态分配内存
	//wchar_t* mpFont;
	//DWORD	 mdwFontSize;
	//DWORD	 mdwTextColor;


};

#endif // _ELBITMAPIMP_H