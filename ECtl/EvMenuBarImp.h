/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

DECLARE_BUILTIN_NAME(MenuBar)

class CEvMenuBar:
	public CXuiElement<CEvMenuBar, GET_BUILTIN_NAME(MenuBar)>
{
	friend CXuiElement<CEvMenuBar,GET_BUILTIN_NAME(MenuBar)>;
public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CEvMenuBar(void);
	~CEvMenuBar(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	//// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	//virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	ERESULT ParseMessage(IEinkuiMessage* npMsg);

	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);			// 本来MenuBar应该没有按钮的，这里特殊情况处理

	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	// 获取指定CommandID的PopupMenu
	IEinkuiIterator* GetPopupMenuByCommandID(IN ULONG niCommandID);

	////快捷键消息
	//ERESULT OnHotKey(const STEMS_HOTKEY* npHotKey);
private:
	void LoadResource();

	bool mbIsSubMenuVisible;

	IEinkuiIterator* mpoLastShowMenuButton;

	//ULONG mulHotKeyID;
};


#define TF_ID_MENUBAR_BTN_MIN			101		// 最小化按钮
#define TF_ID_MENUBAR_BTN_CLOSE			102		// 关闭按钮

