/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	缩放界面的工具栏
*/
#include "cmmstruct.h"

DECLARE_BUILTIN_NAME(ZoomControlToolbar)

#define ZC_BT_AUTO_ZOOM 106
#define ZC_BT_DEFAULT 107
#define ZC_BT_ADD 109
#define ZC_BT_SUB 108
#define ZC_BT_RESET_ZOOM 111

#define ZC_MAX_ZOOM 64 //最多可以放大几级，0是100%

class CZoomControlToolbar:
	public CXuiElement<CZoomControlToolbar,GET_BUILTIN_NAME(ZoomControlToolbar)>
{
	friend CXuiElement<CZoomControlToolbar,GET_BUILTIN_NAME(ZoomControlToolbar)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//初始化自己
	void initData(void);

	// 设置当前的Fat放大倍率
	void SetFatRatio(double fatRatio);
	//进入缩放状态
	void EnterZoomStatus(int scaleLevel = 0);
	//按照自适应缩放比例，对应到合适的缩放等级
	float AjustAutoZoomLevel(float ratio);
	void EnableAutoZoomButton(bool enable);
	//设置显示比例
	void SetRatioString(double ratio);

protected:
	CZoomControlToolbar(void);
	~CZoomControlToolbar(void);

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
	IEinkuiIterator* mpIterBili;	//显示比例
	IEinkuiIterator* mpIterBtAdd;
	IEinkuiIterator* mpIterBtSub;
	int mlCurrentZoomLevel; //当前放大级别，默认0级
	cmmVector<double> mfZoom;

	double mfFatRatio;

	IEinkuiIterator* mpIterBtAutoZoom;	//自适应
	IEinkuiIterator* mpIterBtResetZoom;	//恢复缩放

	//设置放大级别
	void SetLevel(bool nbIsAdd);
	//设置显示比例
	void SetString(ULONG nulLevel);
};
