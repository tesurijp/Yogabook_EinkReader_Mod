/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include <tuple>
#include "ToolbarH.h"
#include "PdfPicture.h"
#include "FileOpenDlg.h"
#include "TipFrame.h"
#include "JumpPage.h"
#include "PreNextButton.h"
#include "SnapShot.h"
#include "ToolbarBottom.h"
#include "FileHistoryDlg.h"
#include "Highlight.h"
#include "ThumbnailDlg.h"
#include "FileOpenFailDlg.h"

#include "CYesNoPromptDlg.h"
#include "PDFOverwriteDlg.h"
#include "ConvertProgressDlg.h"
#include "AskConvertDlg.h"

/*
	本类作为对话框通用基础框架定义使用，定义对话框的通用基本控件和行为
*/

DECLARE_BUILTIN_NAME(Reader_BaseFrame)

#define ZCT_FONTSIZE_LEVEL 5 //5级倍数

using std::tuple;

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

	ZoomStatus GetZoomStatus();

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
	//接收windows输入消息
	ERESULT __stdcall OnInputChangeMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//机器形态变化通知
	ERESULT __stdcall OnModeChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//打开文件的指令消息
	ERESULT __stdcall OnOpenOfficeFile(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	// Open Office Docs with 'TimeStamp' method. [zhuhl5@20200121]
	ERESULT __stdcall OnOpenOfficeFileWithTS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);

	void InternalOpenOfficeFile(uint64_t timeStamp);

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
	bool GetAskConvertStatus();
	bool showAskDlg();

private:
	CLoadingView* mpLoadingView;
	CToolbarH* mpToolbarH;
	CPdfPicture* mpPdfPicture;
	IEinkuiIterator* mpIterBackground;
	COfficeConvertDlg *m_pDemoDlg = nullptr;
	CFileOpenFailDlg* m_fileOpenFailDlg = nullptr;

	CYesNoPromptDlg* promptDlg;
	CPDFOverwriteDlg* pdfOverwriteDlg;
	CConvertProgressDlg* convertProgressDlg;
	CAskConvertDlg* askConvertDlg;
	
	CFileHistoryDlg* mpFileHistoryDlg;
	CJumpPage* mpJumpPage;
	CTipFrame* mpTipFrame;	//提示显示/隐藏功能区
	CToolbarBottom* mpToolbarBottom; //底部工具栏
	CHighlight* mpHighlight; //高亮选择界面
	CThumbnailDlg* mpThumbnailDlg; //缩略图界面
	//ULONG mulCurrentPageNumber; //当前页码
	CPreNextButton* mpPreNextButton;
	CSnapShot* mpSnapShot;
	cmmVector<HISTORY_FILE_ATTRIB*> mdHistroyPath; //历史文件
	wstring m_cmdLineOpenFileName;
	IEinkuiIterator* mpIterToast;
	volatile LONG mlHoldInput;
	DWORD mdwFontSizeArray[ZCT_FONTSIZE_LEVEL];
	float mfArrayPenWidthArray[5]; //笔宽
	BYTE mfArrayFwPenWidthArray[5]; //FW画线笔宽
	BYTE mfArrayFwFingerWidthArray[5]; //FW画线手指画线宽

	DWORD mdwFontsizeIndex;
	DWORD mdwPenWidthIndex;
	DWORD mdwPenColorIndex;
	ULONG mulPenMode; //当前状态，笔/橡皮/选择
	bool mbIsHand; //为真表示手指可用于画线

	ULONG muLastGcTick;
	ULONGLONG mxLastGcUiNumber;
	BOOL mbGcMode;
	bool mbIsTxt; //打开的是否是txt文件
	wchar_t mszTempFile[MAX_PATH];  //为了打开效率，把文件缓冲到programdata目录下
	wchar_t mszSrcFile[MAX_PATH]; //打开文件的实际路径
	bool mbIsSetPartial; //如果为false,则不关闭partial
	ULONG mulPageIndex; //页码
	ULONG mulPageCount; //总页数
	//HANDLE mhFile;
	bool mbIsLoadingSuccess; //文件加载是否完成
	bool mbIsScreenAuto; //为真表示自动旋转
	bool lockvertical;
	bool enableCapacitivepen; //是否开启电容笔顶部按钮功能响应

	ZoomStatus mZoomStatus = ZoomStatus::NONE;
	MoveForward mMoveForward;

	//设置页码跳转窗口的位置
	void SetJumpPagePos(void);
	//用户选择了要打开的文件,npFileAttrib!=NULL表示是历史记录中的文件
	bool OpenFile(wchar_t* npszFilePath, HISTORY_FILE_ATTRIB* npFileAttrib);
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

	tuple<bool, wstring, bool> ConvertAndOpenOfficeFile(const wchar_t* filePath);

	//重置隐藏定时器
	void ResetHideTime(bool lbIsOpen = true);
	//检查历史文件状态，如果不存在的文件就清掉
	void CheckHistoryList(void);
	//显示文件打开页面
	void ShowFileOpenDlg(void);
	//设置笔宽
	void SetHandWriteWidth();
	//清除缩略图文件
	void ClearThumbnail(const wchar_t* npszPath);
	// 显示打开失败对话框
	void ShowFileOpenFailDialog(const wchar_t* resourceName);
	// 将子元素居中
	void PlaceChildInCenter(IEinkuiIterator* childIterator);
	//自动缩放
	void PageAutoZoom();
};

#define  RBF_HISTROY_MAX 12 //最大历史记录


#define  RbF_REG_PAGE_NUMBER L"PageNumber" //上次关闭时显示的页码
#define  RbF_REG_PAGE_CONTENT L"PageContent" //上次关闭时显示的页码的关联数据
#define  RbF_REG_PAGE_CONTENT2 L"PageContent2" //上次关闭时显示的页码的关联数据
#define  RbF_REG_DOUBLE_SCREEN L"DoubleScreen" //单屏或双屏
#define  RbF_REG_RATIO L"Ratio" //放大比例
#define  RbF_REG_TXT_FONT_SIZE_INDEX L"TxtFontSizeIndex" //Txt放大比例数组下标
#define  RbF_REG_PEN_WIDTH_INDEX L"PenWidthIndex" //笔宽度数组下标
#define  RbF_REG_PEN_COLOR_INDEX L"PenColorIndex" //笔颜色
#define  RbF_REG_LOG L"LogFile" //日志开关
#define  RbF_REG_ONCE L"Once" //0或没有表示第一次打开
#define  RbF_REG_SCREEN_ORI L"Screen_ori" //屏幕方向10=随系统，11=横向 12=纵向
#define  RbF_REG_B_COVER L"BCoverState" //开启还是关闭B面屏

#define RBF_TIMER_TOAST 1
#define RBF_TIMER_TSHOW_TOOLBAR 2
#define RBF_TIMER_ENABL_PARTIAL 3 //开启partial,因为切换过来第一帧要关闭partial,否则可能出现刷新不清楚的情况
#define RBF_TIMER_INIT 4 //为了进程开启时更快的显示，使用定时器来做初始化
#define RBF_TIMER_EXIT 5 //收到切换到后台消息后，3秒关闭自己进程
#define RBF_TIMER_HOLDINPUT 6 //自动解除输入锁定
#define RBF_TIMER_ACTIVITY 7 //后台切换到前台


//文件打开历史记录
#define RBF_REG_HISTORY_PATH L"historyPath" 
#define RBF_REG_HISTORY_THUMBNAIL_PATH L"historyThumbanilPath" 
#define RBF_REG_HISTORY_MODIFY L"historyModify" 
#define RBF_REG_HISTORY_PROGRESS L"historyProgress" 
#define RBF_REG_HISTORY_TIMESTAMP L"historyTimestamp" 

// Scaling data name[zhuhl5@20200116]
#define RBF_REG_HISTORY_AUTO_ZOOM L"historyAutoZoom" 
#define RBF_REG_HISTORY_SCALING L"historyScaling" 
#define RBF_REG_HISTORY_SCALINGLEVEL L"historyScalingLevel" 
#define RBF_REG_HISTORY_SCALINGPOSX L"historyScalingPosX"
#define RBF_REG_HISTORY_SCALINGPOSY L"historyScalingPosY"