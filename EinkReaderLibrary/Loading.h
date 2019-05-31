/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	Loading过程
*/

DECLARE_BUILTIN_NAME(LoadingView)

class CLoadingView:
	public CXuiElement<CLoadingView,GET_BUILTIN_NAME(LoadingView)>
{
	friend CXuiElement<CLoadingView,GET_BUILTIN_NAME(LoadingView)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置文件名
	void SetData(wchar_t* npszFileName);
	//设置已加载页数
	void SetPage(LONG niIndex);

	// 模态显示该对话框
	void DoModal(volatile LONG* nplStep,HANDLE nWaitHandle);
	void ExitModal();

protected:
	CLoadingView(void);
	~CLoadingView(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);

private:
	IEinkuiIterator* mpIterFileName;
	IEinkuiIterator* mpIterDi;
	IEinkuiIterator* mpIterDd;
	IEinkuiIterator* mpIterIndex;
	volatile LONG* mplStep;
	HANDLE mWaitHandle;
};

#define LOAD_TIMER_MOVE 1