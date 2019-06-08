/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	不显示的翻页及中间功能区显示隐藏功能的按钮
*/

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
	//virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
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
	IEinkuiIterator* mpIterBtPre;		//上一页
	IEinkuiIterator* mpIterBtMiddle;	//中间
	IEinkuiIterator* mpIterBtNext;		//下一页

	D2D1_POINT_2F mdPressPos;
	float mfLeftButton; //用于判断点击是否是翻页区域
	DWORD mdwClickTicount; //用于判断长按 超过500ms算长按

};

#define PNB_BT_MIDDLE 100  //中间
#define PNB_BT_PRE 101	//上一页
#define PNB_BT_NEXT 102 //下一页