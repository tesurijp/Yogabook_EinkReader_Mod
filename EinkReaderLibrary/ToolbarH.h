/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	横屏时的工具栏
*/

DECLARE_BUILTIN_NAME(ToolbarH)

class CToolbarH:
	public CXuiElement<CToolbarH,GET_BUILTIN_NAME(ToolbarH)>
{
	friend CXuiElement<CToolbarH,GET_BUILTIN_NAME(ToolbarH)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置文件名称
	void SetFileName(wchar_t* npszString);
	//设置页码字符串
	void SetPage(wchar_t* npszString);
	//设置双页显示按钮状态
	void SetDuopageButton(bool nbSingle);
	//获取当前双页显示状态
	bool GetDuopageStatus(void);

protected:
	CToolbarH(void);
	~CToolbarH(void);

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
private:
	IEinkuiIterator* mpIterFileName;	//文件名
	IEinkuiIterator* mpIterPage;		//页码
	IEinkuiIterator* mpIterBtFileOpen;	//打开文件对话框
	IEinkuiIterator* mpIterBtTwo;		//双屏显示
	IEinkuiIterator* mpIterBtOne;		//单屏显示
	IEinkuiIterator* mpIterBtJump;		//页码跳转
	IEinkuiIterator* mpIterBtSnap;		//截屏
	IEinkuiIterator* mpIterBtSuofang;		//缩放
	IEinkuiIterator* mpIterBackground;	//背景图

	bool mbIsTwoScreen;//true表示双屏
	bool mbIsTxt;
};

#define TBH_BT_OPEN_FILE 8
#define TBH_BT_JUMP 3
#define TBH_BT_TWO 2
#define TBH_BT_SNAPSHOT 4
#define TBH_BT_ZOOM 5
#define TBH_BT_ONE_PIC 9