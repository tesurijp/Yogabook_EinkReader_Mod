/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "EdDoc.h"
#include "EdViewCtl.h"

/*
	PDF图片显示
*/

DECLARE_BUILTIN_NAME(PdfPicturePDF)

class CPdfPicturePDF:
	public CXuiElement<CPdfPicturePDF,GET_BUILTIN_NAME(PdfPicturePDF)>
{
	friend CXuiElement<CPdfPicturePDF,GET_BUILTIN_NAME(PdfPicturePDF)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//打开文档
	bool OpenFile(wchar_t* npszPath);
	//关闭文档
	void CloseFile(bool nbUpdateView=true);
	//页面跳转
	bool GoToPage(ULONG nulPageNumber);
	//页面前翻、后翻
	bool PageFoward(bool foward);
	//获得当前页码
	ULONG GetCurrentPageNumber(ULONG& secondPage);
	//设置缩放
	//rMove4个方向，0表示不能移动了，1表示还能移动
	bool SetScaleRatio(float nfScale,OUT RECT& rMove);
	//获取总页数
	ULONG GetPageCount(void);
	//移动页面
	//niXOffset,niYOffset正数表示向右下移动，负数表示向左上移动
	//rMove4个方向，0表示不能移动了，1表示还能移动
	bool MovePage(int niXOffset,int niYOffset, OUT RECT& rMove);
	//屏幕发生旋转
	void SetRotation(ULONG nulRotation);
	// 设置进入双页显示
	void EnableDuopageView(bool nbEnable);
	//拷贝截屏到剪贴板
	bool CopyToClipboard(const D2D1_RECT_F& rRegion);
	// 获取Fat状态的放大倍数
	float GetFatRatio(void);
	// 获取当前页面可视区域在当前放大比例下的源图中的位置，以及对应源图的大小
	void GetRectOfViewportOnPage(D2D1_SIZE_F& nrImageSize, D2D1_RECT_F& nrViewPort);
	

protected:
	CPdfPicturePDF(void);
	~CPdfPicturePDF(void);

	IEdModule_ptr pdfModule;
	IEdDocument_ptr pdfDoc;
	IEdPage_ptr pdfCrtPage;
	IEdPage_ptr pdfCrtPage2;
	IEdBitmap_ptr pdfImage;
	IEdBitmap_ptr pdfImage2;
	IEinkuiBitmap* mpElBitmap;
	ED_SIZE imageSize;
	bin_ptr duopageBuf;
	bool32 landScope;
	bool32 duopageMode;
	int32 pageGap;
	int32 pageNo;
	CEdViewCtl mViewCtl;
	D2D1_RECT_F mRecentDst;
	D2D1_RECT_F mRecentSrc;


	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
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

	IEdPage_ptr loadPage(int32 pageNo, ED_SIZE& sizeInit);

	bool RenderPages(void);
	void CalcMovalbe(OUT RECT& rMove);

private:

	IEinkuiIterator* mpIterBtShowTip;	//显示/隐藏功能区
	IEinkuiBrush*	mpXuiBrush;				// 虚线画刷
	ULONG mulPageCount; //文件总页数
};

#define PP_BT_SHOW_TIP 2 