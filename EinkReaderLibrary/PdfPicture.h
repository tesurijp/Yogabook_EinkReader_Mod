/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "EdDoc.h"
#include "EdViewCtl.h"
#include "Loading.h"
#include "cmmstruct.h"
#include "RedoUndoManager.h"
#include "MsgDefine.h"
#include "tchar.h"
#include "cmmString.h"
#include "cmmBaseObj.h"
#include "cmmPath.h"

/*
	PDF图片显示
*/

DECLARE_BUILTIN_NAME(PdfPicture)

#define PAGEID_LIB(_X) (_X-1)
#define PAGEID_USER(_X) (_X+1)

class CPdfPicture:
	public CXuiElement<CPdfPicture,GET_BUILTIN_NAME(PdfPicture)>
{
	friend CXuiElement<CPdfPicture,GET_BUILTIN_NAME(PdfPicture)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//打开文档
	ULONG OpenFile(wchar_t* npszPath, PPAGE_PDF_CONTEXT npPageContext=NULL, wchar_t* npszSource=NULL);
	//关闭文档
	void CloseFile(bool nbUpdateView=true);
	//是不是Txt文档，这种文档格式可以通过缩放字体大小，调整文档的展示，
	bool IsTxtDocument(void);
	//页面跳转
	bool GoToPage(ULONG nulPageNumber, bool renderPage);
	//页面跳转
	bool GoToPage(IEdPage_ptr pageGoto, bool renderPage);
	//页面前翻、后翻
	bool PageFoward(bool forward, bool renderPage);
	//获得当前页码
	ULONG GetCurrentPageNumber(ULONG& secondPage);
	//获得当前第一页的上下文，当前页的页码在结构体的pageIndex返回
	bool GetCrtPageContext(PPAGE_PDF_CONTEXT npPageContext);
	//设置缩放
	//rMove4个方向，0表示不能移动了，1表示还能移动
	bool SetScaleRatio(float nfScale,OUT RECT& rMove);
	//获取总页数
	ULONG GetPageCount(void);


	//设置Txt文件的字体大小
	bool SetFontSize(int fontSize);
	//移动页面
	//niXOffset,niYOffset正数表示向右下移动，负数表示向左上移动
	//rMove4个方向，0表示不能移动了，1表示还能移动
	bool MovePage(int niXOffset,int niYOffset, OUT RECT& rMove);
	//移动到某位置
	bool MovePageTo(int niX, int niY);
	//屏幕发生旋转
	void SetRotation(ULONG nulRotation);
	// 设置进入双页显示
	void EnableDuopageView(bool nbEnable);
	//拷贝截屏到剪贴板
	bool CopyToClipboard(const D2D1_RECT_F& rRegion);
	//拷贝字符串到剪贴板
	bool CopyToClipboard(const wchar_t* npszText);
	// 获取Fat状态的放大倍数
	float GetFatRatio(void);
	// 获取当前页面可视区域在当前放大比例下的源图中的位置，以及对应源图的大小
	void GetRectOfViewportOnPage(D2D1_SIZE_F& nrImageSize, D2D1_RECT_F& nrViewPort);

	//页面加载
	void OnPageLoaded(int32 loadingStep, int32 pagecount);
	//缩略图加载
	void OnThumbnailLoaded(int32 loadingStep);

	void SetLoadingPageIndex(LONG nlPageIndex);

	//获取文档类型
	int GetDocType(void);
	//计算内容区域
	ED_RECTF CalImageContentRect();

	//输入事件
	void TouchDown(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode,bool nbIsHand);
	void TouchMove(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);
	void TouchUp(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);
	void PenLeave(ULONG nulPenMode);
	//记录点
	void SaveTouchPoint(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);

	//设置线宽
	void SetPenWidth(float nfPenWidth);
	//设置颜色
	void SetPenColor( ULONG nulPenColor);
	//undo 或 redo处理
	int RedoUndoProc(bool nbIsRedo);
	//获取当前页面标注数量
	int GetCurrentPageInkCount(void);
	//清空当前页面所有标注
	bool ClearCurrentPageAllInk(void);
	//计算界面可移动数据
	void CalcMovalbe(OUT RECT& rMove);
	//获取页面放大比例
	float32 GetRealRatio();
	float32 GetBaseRatio();
	//选中指定区域的文字,返回文字块数
	int SelectHighlight(const D2D1_RECT_F& rRegion, SELECT_HIGHLIGHT& rdHighltght);
	// 获取选中区域信息
	void GetSelectHighlightInfo(SELECT_HIGHLIGHT& rdHighltght, ED_RECTF& rdRect);
	//高亮工具条消息
	void HighlightEvent(int niEvent);
	//设置日志开关
	void SetLogStatus(bool nbIsEnable);
	//设置笔状态
	void SetPenmode(ULONG nulPenMode);
	//获取文档对象
	IEdDocument_ptr GetDoc(void);
	//输出日志到文件或debugview
	void SvcDebugOutFmt(char * nswString, ...);
	//加载缩略图
	void LoadThumbnails(const wchar_t* npszFilePath,bool nbIsOpen = true);
	//更新PDF缩略图
	void RefreshThumbnail();
	void RefreshThumbnail(IEdPage_ptr npPage1, IEdPage_ptr npPage2);
	bool32 GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen);
protected:
	CPdfPicture(void);
	~CPdfPicture(void);

	IEdModule_ptr pdfModuleArr[10];
	IEdDocument_ptr pdfDoc;
	IEdPage_ptr pdfCrtPage;
	IEdPage_ptr pdfCrtPage2;
	IEdBitmap_ptr pdfImage;
	IEdBitmap_ptr pdfImage2;
	IEinkuiBitmap* mpElBitmap;
	ED_SIZE imageSize;
	bin_ptr pageCanvas;
	int32  pageCanvasSize;
	bool32 landScope;
	bool32 duopageMode;
	int32 pageGap;
	int32 pageNo;
	CEdViewCtl mViewCtl;
	D2D1_RECT_F mRecentDst;
	D2D1_RECT_F mRecentSrc;
	bool mIsTxtDoc;
	int miDocType;
	float32 mTitleBarHeight;	// txt格式下，用来调整显示区域，空出titlebar
	IEinkuiBrush* mpLineBrush;	// 设备相关
	IEinkuiBrush* mpHigilightBrush;	// 设备相关

	uint32 mFontSize;
	bool mbIsPageChange;

	volatile LONG mLoading;
	CLoadingView* mpLoadingView;

	cmmVector<ED_POINTF> mStrokePoints; //点记录
	cmmVector<ED_POINTF> mStrokePoints2; //点记录,双页模式下第
	//PAGE_PDF_CONTEXT mPageContext;


	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	////消息处理函数
	//virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);

	bool RenderPages(void);
	

	bool32 GetPageSize(IEdPage_ptr pagePtr, ED_SIZE& sizeInit);

	bin_ptr PrepareCanvas(int32 totalBytes);

	//重置redo undo按钮状态
	void SetRedoUndoStatus(void);
	//根据页码获取页面对象指针
	IEdPage_ptr GetPageByPageNumber(int niPageNumber);
	//橡皮处理
	void EarsePro(void);
	//笔迹或橡皮刷新
	void InkInputUpdate(void);
	//删除高亮
	void DeleteHighlight(IEdAnnotList_ptr npSelectAnnotList,CRedoUndoStrokeList* npRedoUndoStrokeList);
	//获取缩略图路径
	bool GetThumbnailPath(const wchar_t* npDocPathName,bool nbIsOpen);
	//计算
	bool HashBuffer(cmmStringW& input, cmmStringW& output);

	void SvcDebugOut(char* nswString);
	CExclusiveAccess moLogFileLock;
	HANDLE mhandleLogFile;
	bool mbIsLog;
private:

	//IEinkuiIterator* mpIterBtShowTip;	//显示/隐藏功能区
	IEinkuiBrush*	mpXuiBrush;				// 虚线画刷
	ULONG mulPageCount; //文件总页数
	wchar_t mpszSourcePath[MAX_PATH]; //文件的真实路径
	float mfPenWidth;
	ED_COLOR mdPenColor;
	bool mbIsModify; //是否修改过文件
	ED_RECT mdFwLineRect; //FW画线区域
	float mfMiddleLineX; //双页模式下中线位置
	CFilePathName mCachePath; //缩略图路径

	CRedoUndoManager mcRedoUndoManager; //管理redo undo数据
	CRedoUndoStrokeList* mpEarseRedoUndoStrokeList; //用于记录橡皮记录，因为橡皮是时时刷新，还要一次undo,所以特殊处理
	

	IEdPage_ptr mpSelectPage; //选中区域在哪个页面
	IEdAnnotList_ptr mpSelectAnnotList; //选中区域的高亮对象list
	IEdStextQuadList_ptr mpSelectStextList; //选中区域的高亮对象块
};

#define PP_BT_SHOW_TIP 2 

#define PP_TIMER_ID_UPDATE 1 //刷新页面显示
#define PP_TIMER_ID_ENABLE_FRAME 2 //开启界面刷新
#define PP_TIMER_ID_SAVE_FILE 3 //写入文件