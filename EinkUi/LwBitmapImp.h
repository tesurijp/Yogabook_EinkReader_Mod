/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _ELBITMAPIMP_H
#define _ELBITMAPIMP_H

// 透明度阈值，超出此值则判断为透明
#define  TRANSPARENT_THRESHOLD_VALUE	12


DECLARE_BUILTIN_NAME(CElBitmap)
class CElBitmap : public cmmBaseObject<CElBitmap, ILwBitmap, GET_BUILTIN_NAME(CElBitmap)>
{
public:

	CElBitmap();
	~CElBitmap();

	// 释放对象
	virtual int __stdcall Release(void);

	// 获取IWICBitmap接口指针，使用者需要自行释放此接口指针
	virtual ERESULT __stdcall GetWICObject(OUT IWICBitmap** ppWicBitmap);

	// 获取ID2D1Bitmap接口指针，使用者需要自行释放此接口指针
	virtual ERESULT __stdcall GetD2DObject(IN ID2D1RenderTarget *npRT,
		OUT ID2D1Bitmap **nppParentBitmap);

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

	//// 对位图进行缩放，并获取缩放结果的ID2D1Bitmap接口指针
	//virtual ERESULT GetScaledD2DObject(IN UINT npWidth, IN UINT npHeight,
	//	IN ID2D1RenderTarget *npRT,
	//	OUT ID2D1Bitmap **nppParentBitmap);

	// 不要自动回收显存对象，EUI系统会根据运行中位图的使用情况（调用Paintboard的DrawBitmap）来判断一个位图是否可以释放显存对象，如果一个位图的显存对象并不是使用在(Paintboard::DrawBitmap）方法中,
	// 而是有自己的特定用法，那么就调用本方法，关闭位图的自动回收Auto video object management;
	// 返回值是调用本方法前的状态
	virtual bool __stdcall SetAVOM(
		bool nbEnable	// false to disable AVOM,true to enable AVOM
		){return false;}


	// 供系统内部调用，废弃Direct2D的设备相关资源
	virtual void __stdcall Discards2DResource(void); 

	ULONG InitOnCreate(
		IN const wchar_t *npFileName);

	ULONG InitOnCreate(
		IN const wchar_t *npText,
		IN DWORD dwTextColor,
		IN const wchar_t *npFont,
		IN DWORD dwFontSize);

	ULONG InitOnCreate(wchar_t *npPeFileName, int niIconIdx, int niXSize, int niYSize);

	ULONG InitOnCreate(ID2D1RenderTarget* npRt, FLOAT nfWidth, FLOAT nfHeight);

	// 通过D3D Surface实例化位图对象
	DEFINE_CUMSTOMIZE_CREATE(CElBitmap, (ID2D1RenderTarget* npRt, FLOAT nfWidth, FLOAT nfHeight), (npRt, nfWidth, nfHeight))

	// 通过EXE文件名实例化位图对象
	DEFINE_CUMSTOMIZE_CREATE(CElBitmap, (wchar_t *npPeFileName, int nwIconIdx, int niXSize, int niYSize), (npPeFileName, nwIconIdx, niXSize, niYSize))

	// 通过位图文件名实例化位图对象
	DEFINE_CUMSTOMIZE_CREATE(CElBitmap,(const wchar_t *npFileName),(npFileName))

	// 通过文字实例化位图对象
	// 提供参数如下：
	//	npText -- 宽字符文字串（目前不支持换行）
	//	dwColor -- RGB格式的颜色值
	//	npFont -- 指定字体名，如"Tahoma"
	//	dwFontSize -- 指定字体大小
	DEFINE_CUMSTOMIZE_CREATE(CElBitmap, (const wchar_t *npText, DWORD dwColor, const wchar_t *npFont, DWORD dwFontSize), (npText, dwColor, npFont, dwFontSize));
protected:
//private:
	HANDLE m_hBitmap;
	ID2D1Bitmap* mpBitmap2D;
	IDXGISurface* mpDxgiSurface;
	IWICBitmapDecoder *m_pIDecoder;
	IWICBitmapFrameDecode *m_pIDecoderFrame;
	IWICBitmapScaler *m_pBmpScaler;
// 	static IWICImagingFactory *m_pWICFactory;
// 	static ID2D1Factory *m_pD2DFactory;

	HRESULT m_hResult;
	UINT m_nWidth;
	UINT m_nHeight;
	UINT m_nScaleWidth;
	UINT m_nScaleHeight;
	LONG m_lExtendLineX;
	LONG m_lExtendLineY;

	IWICBitmap *m_pWICBitmap;

	CBitArray *mpTransparentInfo;

	ULONG Load(IN const wchar_t *npFileName);

	ERESULT CreateD2DObjFromBmpSource(IN ID2D1RenderTarget *npRT, IN IWICBitmapSource *npSourceBmp,
		OUT ID2D1Bitmap **nppParentBitmap);

	ULONG GetBitmapDimension();

	ERESULT GenTransparentInfo();
};

#endif // _ELBITMAPIMP_H