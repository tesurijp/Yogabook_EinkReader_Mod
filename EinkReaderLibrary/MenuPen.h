/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	笔迹菜单
*/

DECLARE_BUILTIN_NAME(MenuPen)

class CMenuPen:
	public CXuiElement<CMenuPen,GET_BUILTIN_NAME(MenuPen)>
{
	friend CXuiElement<CMenuPen,GET_BUILTIN_NAME(MenuPen)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	// 模态显示该对话框
	void DoModal();

	void ExitModal();
	// 设置初始化数据
	void SetData(DWORD ndwPenWidthIndex, DWORD ndwPenColorIndex);

protected:
	CMenuPen(void);
	~CMenuPen(void);

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
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

private:

	IEinkuiIterator* mpIterMenuBase;
	IEinkuiIterator* mpIterPenWidthGroup;
	IEinkuiIterator* mpIterPenColorGroup;
	bool mbIsPressed;
};

#define MT_BT_SHAPSHOT 2

#define MP_CBT_LINE1 1
#define MP_CBT_LINE2 2
#define MP_CBT_LINE3 3
#define MP_CBT_LINE4 4

#define MP_CBT_RED 100
#define MP_CBT_YELLOW 101
#define MP_CBT_BLUE 102


