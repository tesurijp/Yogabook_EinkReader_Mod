/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef		__EVCOMBOBOXIMP_H__
#define		__EVCOMBOBOXIMP_H__

#include "EvPictureFrameImp.h"
#include "EvEditImp.h"
#include "EvButtonImp.h"
#include "EvListImp.h"
#include "EvImageButtonImp.h"


//定义常量
#define BUF_SIZE	256


DECLARE_BUILTIN_NAME(ComboBox)
class CEvComboBox:
	public CXuiElement<CEvComboBox ,GET_BUILTIN_NAME(ComboBox)>
{
	friend CXuiElement<CEvComboBox ,GET_BUILTIN_NAME(ComboBox)>;

public:

	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvComboBox();
	// 用于释放成员对象
	virtual ~CEvComboBox();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制
	//virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//装载配置资源
	//ERESULT LoadResource();

public:
	BOOL InsertItem(wchar_t* lswName);

	// 删除指定项，小于0表示删除全部。 ————Add by colin
	bool DeleteItem(IN int niIndexToDel);

private:
	BOOL SetChildCtrlPara();

	BOOL SetDefaultValueList();

	//	读取UnificSetting数据，重置行为
	bool SetValue();

	bool SetComboBoxEnable(bool lbEnalbe);

	bool SetItemSelected(int nID);

	bool SetItemEnable(int nID);
	bool SetItemDisable(int nID);

	

private:

	//IUnificSetting* mpUnificSetting;

public:

	struct ComboMenuItem 
	{
		int			mnID;
		wchar_t	mpText[BUF_SIZE];

		ComboMenuItem() : mnID(0) {}
	};

private:

	CEvPictureFrame*	mpUpperPicture;				//编辑区域的背景（上背景）
	CEvEditImp*			mpCurrentItemEdit;			//组合框中当前项（编辑模式）
	CEvButton*			mpCurrentItemButton;		//组合框中当前项（非编辑模式）
	CEvImageButton*		mpDropDownButton;			//组合框中的下拉按钮
	
	//wchar_t*			mpCurItem;					//当前选中项的文本

	//ULONG				mpNumItem;					//项的个数

	ICfKey*				mpComboBoxKey;				//默认ComboBox键值

	int					mnStyle;					//控件风格 0：可编辑模式 1：不可编辑模式

	IEinkuiIterator*		mpIterPopMenu;				//弹出菜单
	IEinkuiIterator*		mpIterList;

	cmmVector<ComboMenuItem>	mpVecComboMenuItem;

	COMBOBOX_MSG			mMsgInfo;	

	//控件消息
	TOOLBAR_MSG			mToolbarMsgInfo;

	bool				mbOnlyAcceptNum;			//	编辑状态下是够只接收数字输入
	int					mnMaxInputNum;
	int					mnMinInputNum;

};


#endif		//__EVCOMBOBOXIMP_H__