/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	菜单控件父对象
*/


DECLARE_BUILTIN_NAME(MenuBase)

class CMenuBase:
	public CXuiElement<CMenuBase,GET_BUILTIN_NAME(MenuBase)>
{
	friend CXuiElement<CMenuBase,GET_BUILTIN_NAME(MenuBase)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CMenuBase(void);
	~CMenuBase(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

private:



};



