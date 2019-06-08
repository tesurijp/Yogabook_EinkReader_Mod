/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "ToolbarH.h"
#include "PdfPicture.h"
#include "FileOpenDlg.h"
#include "TipFrame.h"
#include "JumpPage.h"
#include "PreNextButton.h"
#include "ZoomControl.h"
#include "ZoomControltxt.h"
#include "SnapShot.h"

/*
	本类作为对话框通用基础框架定义使用，定义对话框的通用基本控件和行为
*/

DECLARE_BUILTIN_NAME(Reader_BaseFrame)

class CReaderBaseFrame:
	public CXuiElement<CReaderBaseFrame,GET_BUILTIN_NAME(Reader_BaseFrame)>
{
	friend CXuiElement<CReaderBaseFrame,GET_BUILTIN_NAME(Reader_BaseFrame)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CReaderBaseFrame(void);
	~CReaderBaseFrame(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//服务通知应用切换到前台或后台
	ERESULT __stdcall OnActivity(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//homebar状态变化通知
	ERESULT __stdcall OnHomebarChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//Tp被reset了，需要重新设置tp area
	ERESULT __stdcall OnNotifyResetTpArea(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//收到电源变化消息
	ERESULT __stdcall OnPowerChangeMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);

	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);

	//锁定输入
	void HoldInput(bool nbSet=true);

	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	void OnRotated(ULONG nuOrient);
	static bool CopyFileThread(LPVOID npData);
private:
	CLoadingView* mpLoadingView;
	CToolbarH* mpToolbarH;
	CPdfPicture* mpPdfPicture;
	IEinkuiIterator* mpIterBackground;
	CFileOpenDlg* mpFileOpenDlg;
	CJumpPage* mpJumpPage;
	CTipFrame* mpTipFrame;	//提示显示/隐藏功能区
	//ULONG mulCurrentPageNumber; //当前页码
	CPreNextButton* mpPreNextButton;
	CZoomControl* mpZoomControl;
	CZoomControlTxt* mpZoomControlTxt;
	CSnapShot* mpSnapShot;
	cmmVector<wchar_t*> mdHistroyPath; //历史文件
	IEinkuiIterator* mpIterToast;
	volatile LONG mlHoldInput;
	DWORD mdwFontSizeArray[ZCT_FONTSIZE_LEVEL];
	DWORD mdwFontsizeIndex;

	ULONG muLastGcTick;
	ULONGLONG mxLastGcUiNumber;
	BOOL mbGcMode;
	bool mbIsTxt; //打开的是否是txt文件
	wchar_t mszTempFile[MAX_PATH];  //为了打开效率，把文件缓冲到programdata目录下
	wchar_t mszSrcFile[MAX_PATH]; //打开文件的实际路径
	bool mbIsSetPartial; //如果为false,则不关闭partial
	ULONG mulPageIndex; //页码

	//设置打开文件窗口的位置
	void SetOpenFilePos(void);
	//设置页码跳转窗口的位置
	void SetJumpPagePos(void);
	//用户选择了要打开的文件
	bool OpenFile(wchar_t* npszFilePath);
	//初始化
	void Init(void);
	////切页
	//void GotoPage(ULONG nulPage);
	void ShowPageInfo(void);
	//显示toast
	void ShowToast(wchar_t* npszKeyName);
	//显示或隐藏工具栏
	void ShowToolBar(bool nbIsShow);
	//把要打开的文件copy到临时目录
	void CopyFileToTemp(IN wchar_t* npszSrc, OUT wchar_t* npszDest, IN LONG nlLen);
	//获取文件大小
	ULONG GetFileSize(wchar_t* npszFilePath);

	static ERESULT __stdcall EinkUpdating(ULONGLONG nxNumber, CReaderBaseFrame* npThis);	// nuNumber是更新序号，每次更新加一，达到最大值后回到零


};

#define  RBF_HISTROY_MAX 4 //最大历史记录


#define  RbF_REG_PAGE_NUMBER L"PageNumber" //上次关闭时显示的页码
#define  RbF_REG_PAGE_CONTENT L"PageContent" //上次关闭时显示的页码的关联数据
#define  RbF_REG_PAGE_CONTENT2 L"PageContent2" //上次关闭时显示的页码的关联数据
#define  RbF_REG_DOUBLE_SCREEN L"DoubleScreen" //单屏或双屏
#define  RbF_REG_RATIO L"Ratio" //放大比例
#define  RbF_REG_TXT_FONT_SIZE_INDEX L"TxtFontSizeIndex" //Txt放大比例数组下标

#define RBF_TIMER_TOAST 1
#define RBF_TIMER_TSHOW_TOOLBAR 2
#define RBF_TIMER_ENABL_PARTIAL 3 //开启partial,因为切换过来第一帧要关闭partial,否则可能出现刷新不清楚的情况
#define RBF_TIMER_INIT 4 //为了进程开启时更快的显示，使用定时器来做初始化
#define RBF_TIMER_EXIT 5 //收到切换到后台消息后，3秒关闭自己进程
#define RBF_TIMER_HOLDINPUT 6 //自动解除输入锁定