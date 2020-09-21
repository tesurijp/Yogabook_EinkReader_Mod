/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	移动界面的工具栏
*/
#include "cmmstruct.h"
#include "Define.h"

DECLARE_BUILTIN_NAME(MoveControlToolbar)

#define ZC_BT_UP 101
#define ZC_BT_DOWN 102
#define ZC_BT_LEFT 103
#define ZC_BT_RIGHT 104

class CMoveControlToolbar:
	public CXuiElement<CMoveControlToolbar,GET_BUILTIN_NAME(MoveControlToolbar)>
{
	friend CXuiElement<CMoveControlToolbar,GET_BUILTIN_NAME(MoveControlToolbar)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//初始化自己
	void initData(void);

	//设置4个移动按钮的状态
	void ShowMoveButton(RECT ldRect);
	//设置滑动方向
	void SetMoveForward(MoveForward forward);

protected:
	CMoveControlToolbar(void);
	~CMoveControlToolbar(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(PSTEMS_TIMER npStatus);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);

private:
	IEinkuiIterator* mpIterBtUP = nullptr;
	IEinkuiIterator* mpIterBtDown = nullptr;
	IEinkuiIterator* mpIterBtLeft = nullptr;
	IEinkuiIterator* mpIterBtRight = nullptr;

	MoveForward mMoveForward = MoveForward::HORIZONTAL_VERTICAL;

	//上下左右移动
	void MovePage(ULONG nulID);
};
