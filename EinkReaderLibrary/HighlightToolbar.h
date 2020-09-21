/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	高亮的工具栏
*/
#include "cmmstruct.h"

DECLARE_BUILTIN_NAME(HighlightToolbar)


class CHighlightToolbar:
	public CXuiElement<CHighlightToolbar,GET_BUILTIN_NAME(HighlightToolbar)>
{
	friend CXuiElement<CHighlightToolbar,GET_BUILTIN_NAME(HighlightToolbar)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//初始化自己
	void initData(void);
	//设置highlight数量
	void SetHighlightCount(int niCount);

protected:
	CHighlightToolbar(void);
	~CHighlightToolbar(void);

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

private:

	IEinkuiIterator* mpIterBtCopy;		//复制
	IEinkuiIterator* mpIterBtHighlight;	//高亮
	IEinkuiIterator* mpIterBtDeleteLine;//删除线
	IEinkuiIterator* mpIterBtLine;		//下划线
	IEinkuiIterator* mpIterBtDelete;		//删除按钮
};

//#define HT_BT_COPY 1  //复制
//#define HT_BT_HIGHLIGHT 2  //高亮
//#define HT_BT_DELETE_LINE 3  //删除线
//#define HT_BT_UNDER_LINE 4  //下滑线
//#define HT_BT_DELETE 5  //删除