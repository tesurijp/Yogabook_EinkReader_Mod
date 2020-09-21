#pragma once

DECLARE_BUILTIN_NAME(MenuButton)
class CEvMenuButton :
	public CXuiElement<CEvMenuButton, GET_BUILTIN_NAME(MenuButton)>
{
	friend CXuiElement<CEvMenuButton, GET_BUILTIN_NAME(MenuButton)>;
public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

	// 描述：
	//		获取该MenuButton下指定UniqueID的PopupMenu（包括子孙的）
	IEinkuiIterator* GetPopupMenuByUniqueID(
		IN UINT niUniqueID
	);

protected:
	CEvMenuButton(void);
	~CEvMenuButton(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	//// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	//virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

private:
	void LoadResource();

	void LoadSubPopupMenu();

	void OnBtnClick();



	// 描述：
	//		设置好弹出菜单的位置
	void SetPopupMenuPosition();

	int miPopupMenuID;
	CEvPopupMenu* mpoPopupMenu;
	CEvButton*	mpoButton;
};

#define ID_OF_MENUBUTTON_BUTTON				1
#define ID_OF_MENUBUTTON_POPUPMENU			2

//#define ID_OF_MENUBUTTON_BASE				3


#define TF_ID_MENUBUTTON_POPUPMENU_ID		L"PopupMenuID"

#define TF_ID_MENUBUTTON_BUTTON				L"Button"

