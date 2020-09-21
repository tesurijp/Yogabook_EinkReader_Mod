/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	底部工具栏
*/

#include "ZoomControlToolbar.h"
#include "MoveControlToolbar.h"
#include "PageProgress.h"
#include "Define.h"

DECLARE_BUILTIN_NAME(ToolbarBottom)

class CToolbarBottom:
	public CXuiElement<CToolbarBottom,GET_BUILTIN_NAME(ToolbarBottom)>
{
	friend CXuiElement<CToolbarBottom,GET_BUILTIN_NAME(ToolbarBottom)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置页码字符串
	void SetPage(ULONG nulCurrentPage,ULONG nulPageCount);
	// 设置当前的Fat放大倍率
	void SetFatRatio(float fatRatio);
	//显示或隐藏元素
	void ShowItem(bool nbIsShow);
	//设置当前打开的文件类型
	void SetDoctype(int niType);
	//设置4个移动按钮的状态
	void ShowMoveButton(RECT ldRect);
	//隐藏/显示翻页滚动条
	void HidePageProcess(bool nbIsShow);
	//设置缩放模式
	void SetZoomStatus(ZoomStatus status, int scaleLevel = 0);

	void SetMoveToolbarVisible(bool visible);
	//设置滑动方向
	void SetMoveForward(MoveForward forward);
	//按照自适应缩放比例，对应到合适的缩放等级
	float AjustAutoZoomLevel(float ratio);
	//设置切白边按钮的状态
	void EnableAutoZoomButton(bool enable);
	//显示缩放比例
	void SetRatioString(double ratio);

protected:
	CToolbarBottom(void);
	~CToolbarBottom(void);

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

	//重新定位元素
	void RelocationItem(void);

	// Interface for ReaderBaseFrame to active Zoom [zhuhl5@20200116]
	void ActiveZoom(int scaleLevel = 0);

private:

	IEinkuiIterator* mpIterBtPageNumber;	//页码
	IEinkuiIterator* mpIterBtPre;	//上一页
	IEinkuiIterator* mpIterBtNext;	//下一页
	IEinkuiIterator* mpIterBackground;	//背景图
	CMoveControlToolbar* mpMoveControlToolbar;
	CZoomControlToolbar* mpZoomControlToolbar;
	CPageProgress* mpPageProgress; //页面跳转

	ZoomStatus mZoomStatus = ZoomStatus::NONE;
	int miDocType;

	D2D1_POINT_2F mdDropBeginPos;
	D2D1_POINT_2F mdPressPos;	//如果按下和抬起时坐标相差超过50，就认为不是点击
	
	ULONG mulCurrentPage;
	ULONG mulPageCount;
	bool mbIsPressed;
};

#define TBB_BT_PRE 2
#define TBB_BT_NEXT 3
#define TBB_BT_ZOOM 4
#define TBB_BT_NUMBER 5
