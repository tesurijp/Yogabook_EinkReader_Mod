/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef		__EVSPINBUTTONIMP_H__
#define		__EVSPINBUTTONIMP_H__

#include "EvEditImp.h"
#include "EvImageButtonImp.h"
#include "EvPictureFrameImp.h"

#define SPINBUTTON_BUF_SIZE		256
#define SPINBUTTON_MAX_VALUE	10000
#define SPINBUTTON_MIN_VALUE	-10000


DECLARE_BUILTIN_NAME(SpinButton)
class CEvSpinButton:
	public CXuiElement<CEvSpinButton ,GET_BUILTIN_NAME(SpinButton)>
{
	friend CXuiElement<CEvSpinButton ,GET_BUILTIN_NAME(SpinButton)>;

public:

	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvSpinButton();
	// 用于释放成员对象
	virtual ~CEvSpinButton();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制
	//virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);

public:
	BOOL	SetCurrentValue(const int nValue);
	int		GetCurrentValue();
	
private:
	BOOL UpdateEditView();
	BOOL SetCurrentValueByDisplay();

private:
	BOOL SetChildCtrlPara();

public:


private:

	CEvEditImp*			mpEdit;				//编辑框
	CEvImageButton*		mpBtnUpArrow;		//向上箭头
	CEvImageButton*		mpBtnDownArrow;		//向下箭头
	CEvPictureFrame*	mpBkg;				//背景图

	int					mnCurrentValue;		//

	int					miMinValue;			//  最小值
	int					miMaxValue;			// 最大值

};


#endif		//__EVSPINBUTTONIMP_H__