/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	不显示的翻页及中间功能区显示隐藏功能的按钮
*/

#include "PdfPicture.h"
#include "Highlight.h"
#include "Define.h"

DECLARE_BUILTIN_NAME(PreNextButton)

class CPreNextButton:
	public CXuiElement<CPreNextButton,GET_BUILTIN_NAME(PreNextButton)>
{
	friend CXuiElement<CPreNextButton,GET_BUILTIN_NAME(PreNextButton)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置笔状态
	void SetPenMode(ULONG nulPenMode);
	//设置文档对象指针
	void SetPdfPicture(CPdfPicture* npPdfPicture, CHighlight* npHighlight);
	//设置是否是缩放状态,nbIsZoom为真表示当前是缩放状态
	void SetZoomStatus(ZoomStatus status);
	//设置滑动方向
	void SetMoveForward(MoveForward forward);
	//设置手是否可以用于画线
	void SetHandWrite(bool nbIsHand);
	//获取手写状态
	bool GetHandWrite(void);
	//设置上任务栏高度和下任务栏高度
	void SetToolbarHeight(int niTopHeight, int niBottomHeight);
	//设置文件属性
	void SetFileAttrib(DWORD ndwAttrib);
	//设置画线区域
	void SetFwLineRect(ED_RECT ndRect);
protected:
	CPreNextButton(void);
	~CPreNextButton(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	//virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
	//输入事件
	ERESULT __stdcall OnTouchMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result);
	//输入事件
	void TouchMsgPro(PEI_TOUCHINPUT_POINT npTouch);
	//重置FW画线区域
	void ResetFwRect(bool nbIsClose = false);
	//处理翻页事件
	void ProcessPageNext(D2D1_POINT_2F ndPos);
	//处理页面移动
	void ProcessPageMove(D2D1_POINT_2F ndpPos);

private:
	IEinkuiIterator* mpIterBtPre;		//上一页
	IEinkuiIterator* mpIterBtMiddle;	//中间
	IEinkuiIterator* mpIterBtNext;		//下一页

	MoveForward mMoveForward = MoveForward::HORIZONTAL_VERTICAL;

	D2D1_POINT_2F mdPressPos;
	float mfLeftButton; //用于判断点击是否是翻页区域
	DWORD mdwClickTicount; //用于判断长按 超过500ms算长按
	CPdfPicture* mpPdfPicture;
	CHighlight* mpHighlight;
	ULONG mulPenMode;

	bool mbIsPressed;
	ZoomStatus mZoomStatus = ZoomStatus::NONE;
	bool mbIsHand; //手是否可用于画线
	bool mbIsFwRect; //true表示开启fw画线

	int miTopHeight;
	int miBottomHeight;
	DWORD mdwAttrib;
	ED_RECT mdFwLineRect; //FW画线区域
	ULONG mulPointCount; //记录已发送点数量
	DWORD mdwLastPenTicket; //最后一次笔事件时间点
	EI_TOUCHINPUT_POINT mdPrePoint; //上一个点的信息
};

#define PNB_BT_MIDDLE_ZOOM 99  //放大状态下中间
#define PNB_BT_MIDDLE 100  //中间
#define PNB_BT_PRE 101	//上一页
#define PNB_BT_NEXT 102 //下一页