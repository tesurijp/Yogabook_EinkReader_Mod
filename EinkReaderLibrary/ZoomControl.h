/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	缩放界面
*/
#include "ZoomControlToolbar.h"

DECLARE_BUILTIN_NAME(ZoomControl)


class CZoomControl:
	public CXuiElement<CZoomControl,GET_BUILTIN_NAME(ZoomControl)>
{
	friend CXuiElement<CZoomControl,GET_BUILTIN_NAME(ZoomControl)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	// 设置当前的Fat放大倍率
	void SetFatRatio(float fatRatio);
	//设置放大后的图大小及显示区域
	void SetRectOfViewportOnPage(D2D1_SIZE_F& nrImageSize, D2D1_RECT_F& nrViewPort);

protected:
	CZoomControl(void);
	~CZoomControl(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//鼠标移动
	virtual ERESULT OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo);
	//鼠标进入或离开
	//virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
private:
	IEinkuiIterator* mpIterBtUP;	
	IEinkuiIterator* mpIterBtDown;		
	IEinkuiIterator* mpIterBtLeft;	
	IEinkuiIterator* mpIterBtRight;		
	IEinkuiIterator* mpIterScrH;//横向滚动条
	IEinkuiIterator* mpIterScrV;//纵向滚动条

	CZoomControlToolbar* mpZoomControlToolbar;

	bool mbIsBeginMove;
	D2D1_POINT_2F mdDropBeginPos;
	D2D1_POINT_2F mdPressPos;	//如果按下和抬起时坐标相差超过50，就认为不是点击
	RECT mdMoveBtShow;

	//上下左右移动
	void MovePage(ULONG nulID);
	//设置4个移动按钮的状态
	void ShowMoveButton(RECT ldRect);
	//显示或隐藏所有控件
	void ShowItem(bool nbIsShow);
	//设置ScrollBar的位置
	bool SetScrollBarPositionAndSize();
};

#define ZC_TIMER_ID_HIDE 1

#define ZC_BT_UP 101
#define ZC_BT_DOWN 102
#define ZC_BT_LEFT 103
#define ZC_BT_RIGHT 104

