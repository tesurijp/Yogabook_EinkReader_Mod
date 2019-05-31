/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _XS_D2DENGINE
#define _XS_D2DENGINE
#include "eiupdate.h"



// 渲染状态
class CErdState{
public:
	CErdState(){
		mbRotated = false;
		mpD2dTarget = NULL;
		mbHasClipRect = false;
		mpEnhancer = NULL;
		mbPaintItself = true;
		//mbTopDraw = false;
		//mlCrtPtLevel = -1;
		//mlPlLevelHost = -1;
	}
	~CErdState(){}

	D2D1::Matrix3x2F mdTransLation;
	D2D1::Matrix3x2F mdRotation;
	D2D1::Matrix3x2F mdWorld;
	D2D1_POINT_2F mdOffset;
	FLOAT mfAlpha;
	IEinkuiIterator* mpEnhancer;
	bool mbRotated;
	bool mbHasClipRect;
	bool mbPaintItself;
	//bool mbTopDraw;
	LONG mlCrtPtLevel;
	LONG mlPlLevelHost;
	ID2D1RenderTarget* mpD2dTarget;
	D2D1_RECT_F mdClipRect;

	void operator=(const class CErdState& src){
		mdTransLation = src.mdTransLation;
		mdRotation = src.mdRotation;
		mdWorld = src.mdWorld;
		mfAlpha = src.mfAlpha;
		mpEnhancer = src.mpEnhancer;
		mbRotated = src.mbRotated;
		//mbTopDraw = src.mbTopDraw;
		mlCrtPtLevel = src.mlCrtPtLevel;
		mlPlLevelHost = src.mlPlLevelHost;
		mbPaintItself = src.mbPaintItself;
		mpD2dTarget = src.mpD2dTarget;
		mbHasClipRect = src.mbHasClipRect;
		if(mbHasClipRect != false)
			mdClipRect = src.mdClipRect;
		mdOffset = src.mdOffset;
	}
};

typedef cmmStack<CErdState,23,32> TRrdStateStack;

#pragma pack(1)
typedef struct _EI_LINE_DIFF {
	unsigned short Begin;
	unsigned short End;
}EI_LINE_DIFF, *PEI_LINE_DIFF;
#pragma pack()



//////////////////////////////////////////////////////////////////////////
// D2D 图形引擎
//////////////////////////////////////////////////////////////////////////


DECLARE_BUILTIN_NAME(CXD2dEngine)
class CXD2dEngine: public cmmBaseObject<CXD2dEngine,IEinkuiPaintBoard, GET_BUILTIN_NAME(CXD2dEngine)>, public IEinkuiPaintResouce
{

//////////////////////////////////////////////////////////////////////////
// 引擎初始化部分
//////////////////////////////////////////////////////////////////////////
public:

	// 默认构造函数
	CXD2dEngine();
	// 默认析构函数
	~CXD2dEngine();

	// 自定义初始化函数
	DEFINE_CUMSTOMIZE_CREATE(CXD2dEngine,(ULONG Width,ULONG Height),(Width, Height))
	ULONG InitOnCreate(ULONG Width, ULONG Height);

	// 建立设备无关资源
	virtual ERESULT CreateIndependentResource();

	// 建立设备相关资源
	virtual ERESULT CreateDeviceResource();

	// 释放全部资源
	virtual void ReleaseDeviceResource(bool nbBroadcastToElement=true);

	// 清理画板
	virtual void Clear();

	// 提交到屏幕
	virtual HRESULT Present(
		IN bool nbRefresh = false	// 必须提交全屏
		);

	//// 建立D3d资源
	//ERESULT CreateD3dResource();

	// 建立D2d资源
	ERESULT CreateD2dResource();

	//获取当前画板图像，获取的HBITMAP对象，由调用者来释放
	virtual HBITMAP GetCurrentBitmap(LONG nlWidth,LONG nlHeight);
//////////////////////////////////////////////////////////////////////////
// 创建绘图资源，实现IXuiPaintResouce的接口
//////////////////////////////////////////////////////////////////////////
public:
	// 创建渐变画刷
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType,
		D2D1_COLOR_F noColor
		);

	// 渐变画刷时，需要传入多个颜色点
	virtual IEinkuiBrush* __stdcall CreateBrush(
		XuiBrushType niBrushType, 
		D2D1_GRADIENT_STOP* npGradientStop, ULONG nuCount, 
		D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES ndLinearGradientBrushProperties
		);	


//////////////////////////////////////////////////////////////////////////
// 需要实现的图形引擎基类接口
//////////////////////////////////////////////////////////////////////////

// 封装位图的绘制操作
public:

	// 绘制一个图像到绘图板
	// 需要指定目标和源区域；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_RECT_F& rDestRect,	// 目标区域
		IN const D2D1_RECT_F& rSrcRect,	// 位图的采样区域
		IN IEinkuiBitmap* npBitmap,		// 待绘制的位图
		IN ULONG nuMethod,			// 采样方法，见下文定义
		IN float nfAlpha = 1.0f		
		);

	// 绘制一个图像到绘图板
	// 仅指定目标区域，采样区域是整个位图；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_RECT_F& rDestRect,	// 目标区域
		IN IEinkuiBitmap* npBitmap,		// 待绘制的位图
		IN ULONG nuMethod,			// 采样方法，见下文定义
		IN float nfAlpha = 1.0f			
		);

	// 绘制一幅图像到绘图板
	// 完全按照图像的大小绘制到目标；仅在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间有效
	virtual ERESULT __stdcall DrawBitmap(
		IN const D2D1_POINT_2F& rDestLeftTop,	// 目标区域的左上角
		IN IEinkuiBitmap* npBitmap,			// 待绘制的位图
		IN float nfAlpha = 1.0f			
		);


// 封装画点画线的操作接口
public:

	// 画点线函数
	virtual ERESULT __stdcall DrawLine(
		IN const D2D1_POINT_2F&	noStartPoint,	// 起始点
		IN const D2D1_POINT_2F& noEndPoint,		// 结束点
		IN IEinkuiBrush* npBrush
		);

	// 画多边形
	virtual ERESULT __stdcall DrawPlogon(
		IN const D2D1_POINT_2F*	noPointArray,	// 顶点列表
		IN INT	niCount,
		IN IEinkuiBrush* npBrush 
		);

	// 填充多边形
	virtual ERESULT __stdcall FillPlogon(
		IN const D2D1_POINT_2F*	noPointArray,	// 顶点列表
		IN INT	niCount,
		IN IEinkuiBrush* npBrush 
		);

	// 画椭圆
	virtual ERESULT __stdcall DrawEllipse(
		IN const D2D1_RECT_F& noRect,			
		IN IEinkuiBrush* npBrush
		);

	// 填充椭圆
	virtual ERESULT __stdcall FillEllipse(
		IN const D2D1_RECT_F& noRect,			
		IN IEinkuiBrush* npBrush
		);


	// 画矩形
	virtual ERESULT __stdcall DrawRect(
		IN const D2D1_RECT_F& noRect,
		IN IEinkuiBrush* npBrush
		);

	// 填充矩形
	virtual ERESULT __stdcall FillRect(
		IN const D2D1_RECT_F& noRect,
		IN IEinkuiBrush* npBrush
		);


	// 画圆角矩形
	virtual ERESULT __stdcall DrawRoundRect(
		IN const D2D1_RECT_F& noRect,
		FLOAT radiusX,
		FLOAT radiusY,
		IN IEinkuiBrush* npBrush
		);


	// 填充圆角矩形
	virtual ERESULT __stdcall FillRoundRect(
		IN const D2D1_RECT_F& noRect,
		FLOAT radiusX,
		FLOAT radiusY,
		IN IEinkuiBrush* npBrush
		);	

	// ??以下接口需要确认

	// 获得Direct2D的RenderTarget，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	// 仅应该在EMSG_PAINT/EMSG_RENDER_ENHANCER消息处理期间执行绘制动作，不能在Prepare消息期间绘制，以免破坏渲染引擎的稳定
	virtual ID2D1RenderTarget* __stdcall GetD2dRenderTarget(void);

	// 获得Direct2D的工厂接口，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual ID2D1Factory* __stdcall GetD2dFactory(void);

	// 获得WIC工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual IWICImagingFactory* __stdcall GetWICFactory(void);

	// 获得Direct Write工厂，不用释放，只能够在获得本IXuiPaintBoard接口的消息响应期间使用返回的RenderTarget接口，不要长期保存此处返回的接口，以免失效造成未知错误
	virtual IDWriteFactory* __stdcall GetDWriteFactory(void);


	// 获得当前的D2d绘制用，局部坐标到世界坐标的转换矩阵
	virtual const D2D1::Matrix3x2F& __stdcall GetCurrent2DWorldMatrix(void);

	// 获得当前可视区设置，可视区用世界坐标描述，如果D2dTarget被切换，将给出的是切换后的Target对应的世界坐标
	// 返回ERESULT_SUCCESS当前存在可视区设置，并且成功取得，ERESULT_UNSUCCESS不存在可视区设置，其他值表示错误
	virtual ERESULT __stdcall GetVisibleRegion(
		OUT D2D1_RECT_F* npRegion
		);

	// 对于直接访问D3d和D2d对象的元素，使用本接口向系统汇报设备错误，系统返回ERESULT_SUCCESS表示可以继续执行，返回值满足宏ERESULT_FAILED则中止继续执行
	virtual ERESULT __stdcall CheckDeviceResult(
		IN HRESULT nhrDeviceResult
		);


	// 获得当前的TickCount
	virtual ULONG __stdcall GetCurrentTickCount();

	// 获得当前帧率
	virtual ULONG __stdcall GetCurrentFps(void);

	// 获得当前的渲染序列号，绘制序列号是用来协调和计算当前的渲染次数，每执行一次渲染该序号加一，某些情况下，元素可以会被要求绘制2次，计数达到最大值后会从0开始
	virtual ULONG __stdcall GetRenderNumber(void);


//////////////////////////////////////////////////////////////////////////
// 辅助功能函数
//////////////////////////////////////////////////////////////////////////
public:


	// 执行绘制任务
	ERESULT DoRender(
		IN ULONG nuCrtTick,	// 当前的TickCount
		IN bool nbRefresh=false,	// 必须提交全屏
		IN IEinkuiIterator* npToCapture=NULL	// 此值被设置用于抓取该元素与子元素的图像，并不会影响屏幕显示
		);

	// 执行拍照任务
	IEinkuiBitmap* TakeSnapshot(
		IEinkuiIterator* npToShot,
		const D2D1_RECT_F& crSourceRegion,	// 采样区域，目标元素的局部坐标系
		const D2D_SIZE_F& crBriefSize,		// 缩略图尺寸，快照的结果是一副缩略图
		const FLOAT* ColorRGBA = NULL
	);

	// 供Windows窗口过程调用，保存窗口最小化信息
	void StopPainting(bool nbMin);

	// 重设画板大小
	void ResetPaintboard(void);

	// 获得主窗口的大小
	void GetPaintboardSize(
		OUT EI_SIZE* npSize	// 获取画板大小
		);

	// 发送Prepare Paint前预处理
	ERESULT __stdcall EnterForPrepare(IEinkuiIterator* npRecipient);

	// 发送Prepare Paint后处理
	ERESULT __stdcall LeaveForPrepare(IEinkuiIterator* npRecipient);

	// 发送Paint前预处理
	ERESULT __stdcall EnterForPaint(IEinkuiIterator* npRecipient);

	// 发送Paint后处理
	ERESULT __stdcall LeaveForPaint(IEinkuiIterator* npRecipient);

	// 发送Capture前预处理
	ERESULT __stdcall EnterForCapture(IEinkuiIterator* npRecipient);

	// 发送Capture后处理
	ERESULT __stdcall LeaveForCapture(IEinkuiIterator* npRecipient);

	// 发送EMSG_DISCARD_DEVICE_RESOURCE前预处理
	ERESULT __stdcall EnterForDiscardDeviceRes(IEinkuiIterator* npRecipient);

	// 发送EMSG_DISCARD_DEVICE_RESOURCE后处理
	ERESULT __stdcall LeaveForDiscardDeviceRes(IEinkuiIterator* npRecipient);

	// 设定自绘函数，设定后Xui系统不在调用Eink绘制，仅仅将rgb32的缓冲区提供给此处设定的回调函数
	void SetCustomDraw(PXUI_CUSTOM_DRAW_CALLBACK CustomDraw);

	// 重置Eink缓存
	void ClearEinkBuffer(bool nbClear) {
		InterlockedExchange(&mlResetEinkBuf, (LONG)nbClear);
	}

	//HRESULT SaveToImageFile(D3D10_MAPPED_TEXTURE2D* npMapped); for test

private:
	void DrawEink(
		IN bool nbRefresh = false	// 必须提交全屏
	);

	// 重新定位
	void RelocationPainboard(void);

//////////////////////////////////////////////////////////////////////////
// 定义与绘图有关的设备相关或设备无关资源
//////////////////////////////////////////////////////////////////////////
public:
	CXelManager* mpElementManager;
	ULONG muEinkPanelW;
	ULONG muEinkPanelH;
	ULONG muFixedW;
	ULONG muFixedH;
	volatile bool mbStopPainting;
	PXUI_CUSTOM_DRAW_CALLBACK mpCustomDraw;
	ULONGLONG mxUpdatingNumber;
	PXUI_EINKUPDATING_CALLBACK mpEinkUpdatingCallBack;
	LPVOID mpEinkUpdatingContext;

	//////////////////////////////////////////////////////////////////////////fs
	// 设备无关资源
	ID2D1Factory* mpD2dFactory;
	IWICImagingFactory* mpWicFactory;
	IDWriteFactory* mpWriteFactory;
	IDWriteTextFormat* mpDbgTextFormat;

	//////////////////////////////////////////////////////////////////////////
	// 设备相关资源
	ID2D1RenderTarget* mpTarget2D;		// Direct2D的渲染目标
	ID2D1Bitmap* mpTargetBitmap;
	ID2D1SolidColorBrush* mpDbgTextBrush;
	ID2D1SolidColorBrush* mpFogBrush;

	// 绘制上下文
	volatile LONG mlRenderStep;	//绘制阶段
	IEinkuiMessage* mpMessage;
	ULONG muCrtTick;
	TRrdStateStack moRenderState;
	D2D1::Matrix3x2F mdIdentyMatrix;
	IEinkuiIterator* mpToCapture;
	bool mbCapturing;
	D2D1_RECT_F  mdCaptureRegion;
	D2D_SIZE_F	 mdCaptureBriefSize;
	D2D1::Matrix3x2F mdScalMatrixToCapture;
	D2D1_COLOR_F mdBackColor;

	// 模态对话框闪烁控制
	IEinkuiIterator* mpModalTrunk;
	ULONG muFlashModalTick;

	// Layered Window 用
	HBITMAP mhForeBmp;			// 前台绘图板用的GDI位图对象，用于Layered window 模式
	HDC mhForeDc;				// 用于Layered window 模式下，供调用UpdateLayeredWindow时使用
	HBITMAP mhOldForeBmp;			// 保存上面换出的默认位图
	UCHAR* mpForeBuffer;			// 前台用绘图位图的缓冲区，用于Layered window 模式
	PEI_BUFFER mpEinkBuf;
	CEiUpdate moEiUpdate;
	LONG mlResetEinkBuf;



	// 统计帧率
	volatile ULONG muRenderCount;
	volatile ULONG muLastTick;
	volatile FLOAT mfFpsIndx;


};





#endif