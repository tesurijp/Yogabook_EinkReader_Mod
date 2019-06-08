/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef		__EV_TIME_SPIN_BUTTON_IMP_H__
#define		__EV_TIME_SPIN_BUTTON_IMP_H__

#include "EvEditImp.h"
#include "EvImageButtonImp.h"
#include "EvLabelImp.h"
#include <string>

DECLARE_BUILTIN_NAME(TimeSpinButton)
class CEvTimeSpinButton:
	public CXuiElement<CEvTimeSpinButton ,GET_BUILTIN_NAME(TimeSpinButton)>
{
	friend CXuiElement<CEvTimeSpinButton ,GET_BUILTIN_NAME(TimeSpinButton)>;

public:

	ULONG InitOnCreate(
		IN IXuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvTimeSpinButton();
	// 用于释放成员对象
	virtual ~CEvTimeSpinButton();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IXuiIterator* npIterator);
	//绘制
	virtual ERESULT OnPaint(IXuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IXuiMessage* npMsg);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

private:

	BOOL SetChildCtrlPara();
	void UpdateView();
	bool SetTimeSpinButtonEnable(bool nbIsEnable);

public:

	bool SetCurrentTime(XuiTimeFormat time);
	XuiTimeFormat GetCurrentTime() const;

private:

	CEvEditImp*			mpEditMinutes;			//	编辑分
	CEvEditImp*			mpEditSeconds;			//	编辑秒
	CEvEditImp*			mpEditMilliseconds;		//	编辑毫秒

	CEvImageButton*		mpBtnUpArrow;			//	向上箭头
	CEvImageButton*		mpBtnDownArrow;			//	向下箭头

	CEvLabelImp*		mpLabelColonOne;		//	冒号1
	CEvLabelImp*		mpLabelColonTwo;		//	冒号2

	XuiTimeFormat		mCurrentTime;			//	当前时间

	int					mnCurrentEditID;		//	记录当前获取焦点的编辑框

	bool				mbSendModifyMsgWhenEditModify;		//设置当编辑框被修改时是否发送 EEVT_TIMESPINBUTTON_TIME_MODIFIED
};


#endif		//__EV_TIME_SPIN_BUTTON_IMP_H__