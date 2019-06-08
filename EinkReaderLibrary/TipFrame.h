/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	显示/隐藏功能区页面
*/

DECLARE_BUILTIN_NAME(TipFrame)

class CTipFrame:
	public CXuiElement<CTipFrame,GET_BUILTIN_NAME(TipFrame)>
{
	friend CXuiElement<CTipFrame,GET_BUILTIN_NAME(TipFrame)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);
	// 模态显示该对话框
	void DoModal();
	void ExitModal();
protected:
	CTipFrame(void);
	~CTipFrame(void);

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
	IEinkuiIterator* mpIterBtClose;	//点击关闭界面
	IEinkuiIterator* mpIterPicLeft;	//左边图片
	IEinkuiIterator* mpIterPicRight;	//右边图片
	IEinkuiIterator* mpIterPicPic;	//手图片
	IEinkuiIterator* mpIterPicBack;	//背景图片
	IEinkuiIterator* mpIterText;	//文字
};

#define TIP_F_BT_CLOSE 6 //关闭按钮