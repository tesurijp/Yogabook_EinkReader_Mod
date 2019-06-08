/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

// 存放该PopupMenu下所有MenuItem公共属性信息的结构体
struct _ST_MENUITEM_INFO
{
	LONG LeftWidth;
	LONG RightWidth;
	LONG MiddleWidth;
	LONG LeftIconWidth;
	LONG LeftIconHeight;
	LONG RightIconWidth;
	LONG RightIconHeight;

	wchar_t FontName[MAX_PATH];
	LONG FontSize;
	LONG FontColor;
	LONG FontDisableColor;
	LONG FontFocusColor;

	LONG ItemHeight;
};

typedef _ST_MENUITEM_INFO ST_MENUITEM_INFO, *PST_MENUITEM_INFO;

class CEvPopupMenu;


#define MAX_HOTKEY_LENGTH  20
#define MAX_TITTLE_LENGTH  260

DECLARE_BUILTIN_NAME(MenuItem)
class CEvMenuItem:
	public CXuiElement<CEvMenuItem, GET_BUILTIN_NAME(MenuItem)>
{
	friend CXuiElement<CEvMenuItem,GET_BUILTIN_NAME(MenuItem)>;
public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	enum ENUM_MENUITEM_TYPE
	{
		ENUM_MENUITEM_TYPE_SEPARATION = 0,			// 分割线
		ENUM_MENUITEM_TYPE_NORMAL,					// 普通菜单项
		ENUM_MENUITEM_TYPE_ICON,					// 有图标菜单项
		ENUM_MENUITEM_TYPE_CHECK,					// 有Checked态的菜单项
		ENUM_MENUITEM_TYPE_EXTEND,					// 有弹出菜单的菜单项
		ENUM_MENUITEM_TYPE_APPLE_EXTEND,			// 苹果风格的扩展菜单，右侧扩展图标和热键右对齐
	};

	// 获取名称图宽度
	int GetNameAreaWidth()
	{
		if(NULL != mpoTextImage)
			return mpoTextImage->GetWidth();
		else
			return -1;
	}

	// 设置名称区域宽度
	void SetNameAreaWidth(
		int niNameAreaWidth)
	{
		miNameAreaWidth = niNameAreaWidth;
	}

	// 获取热键区域宽度
	int GetHotKeyAreaWidth()
	{
		if(NULL != mpoHotKeyImage)
			return mpoHotKeyImage->GetWidth();
		else
			return -1;
	}
	// 设置热键区域宽度
	void SetHotKeyAreaWidth(
		int niHotKeyAreaWidth
		)
	{
		miHotKeyAreaWidth = niHotKeyAreaWidth;
	}

	// 更新尺寸
	void UpdateSize()
	{
		if(NULL == mpoMenuItemInfo)
			return;

		float lfWidthCount = (FLOAT)(mpoMenuItemInfo->LeftWidth + miNameAreaWidth + 
			mpoMenuItemInfo->MiddleWidth + miHotKeyAreaWidth + mpoMenuItemInfo->RightWidth);

		mpIterator->SetSize(lfWidthCount, (FLOAT)mpoMenuItemInfo->ItemHeight);
	}

	// 描述：
	//		设置初始化结构体的指针（这里之所以说是设置指针，是为了表示不需要去释放）
	void SetMenuItemInfo(IN PST_MENUITEM_INFO npoMenuItemInfo);

	// 描述：
	//		获取当前菜单项的类型
	ENUM_MENUITEM_TYPE GetMenuItemType()
	{
		return meMemuItemType;
	}

	// 描述：
	//		设置当前菜单的类型
	void SetMenuItemType(IN ENUM_MENUITEM_TYPE neMenuItemType)
	{
		meMemuItemType = neMenuItemType;
	}

	// 描述：
	//		获取当前菜单项的弹出菜单
	CEvPopupMenu* GetPopupMenu()
	{
		if(NULL == mpoSubPopupMenu)
		{
			LoadSubPopupMenu();
		}

		if(NULL != mpoSubPopupMenu 
			&&(meMemuItemType == ENUM_MENUITEM_TYPE_APPLE_EXTEND 
			|| meMemuItemType == ENUM_MENUITEM_TYPE_EXTEND)
			)
			return mpoSubPopupMenu;
		else
			return NULL;
	}

	// 描述：
	//		获取当前菜单项的CommandID
	int GetCommandID()
	{
		return miCommandID;
	}

	// 描述：
	//		设置当前才当想的CommandID
	void SetCommandID(
		IN UINT niCommandID
		)
	{
		miCommandID = niCommandID;
	}

	// 描述：
	//		设置背景图
	void SetBgBitmapPtr(
		IN IEinkuiBitmap* niBitmap,
		IN int niFrameCount
		)
	{
		CMM_SAFE_RELEASE(mpoItemBgBmp);
		mpoItemBgBmp = niBitmap;

		miItemFrameCount = niFrameCount;
	}

	// 描述：
	//		隐藏级联菜单
	void HideCascadeMenu();

protected:
	CEvMenuItem(void);
	~CEvMenuItem(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	//// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	//virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);

	// 鼠标落点检测
	ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//鼠标悬停
	ERESULT OnMouseHover();

	//快捷键消息
	virtual ERESULT OnHotKey(const STEMS_HOTKEY* npHotKey);

private:
	void LoadResource();

	// 加载子菜单列表
	void LoadSubPopupMenu();

	// 描述：
	//		加载菜单项文字图片，调用这个函数后初始化两个参数mpoTextImage poFocusTextImage
	// 返回值：
	//		-1 表示失败，否则返回创建的图片中最大高度
	//		注意：若传入空指针或者指向空字符的指针，则会清空文字图片和热键图片，返回高度为0
	long LoadTextImage(
		IN wchar_t* nswMenuName					// 菜单文本
		);

	// 描述：
	//		加载菜单项热键图片，调用这个函数后初始化两个参数mpoHotKeyImage poFocusHotKeyImage
	// 返回值：
	//		-1 表示失败，否则返回创建的图片中最大高度
	//		注意：若传入空指针或者指向空字符的指针，则会清空热键图片，返回高度为0
	long LoadHotkeyImage(IN PSTCTL_MENUITEM_HOTKEY npdHotKeyInfo);


protected:
	ENUM_MENUITEM_TYPE meMemuItemType;
	int miCommandID;					// 该菜单项的唯一标识ID
	int miShotcutKey;					// 该菜单项的热键
	wchar_t mswMenuTittle[MAX_TITTLE_LENGTH];
	wchar_t mswHotKey[MAX_HOTKEY_LENGTH];
	PST_MENUITEM_INFO mpoMenuItemInfo;
	
	IEinkuiBitmap* mpoHotKeyImage;		// 快捷键图
	IEinkuiBitmap* mpoFocusHotKeyImage;	// 焦点态快捷键图
	IEinkuiBitmap* mpoFocusTextImage;	// 焦点态菜单名称图
	IEinkuiBitmap* mpoTextImage;		// 菜单名称图
	IEinkuiBitmap* mpoLeftImage;		// 名称左边的图
	IEinkuiBitmap* mpoCheckedImage;		// Checked态图
	IEinkuiBitmap* mpoRightImage;		// 名称右边的图
	IEinkuiBitmap* mpoSeparation;		// 分割线图
	IEinkuiBitmap* mpoItemBgBmp;		// 背景图指针

	int miItemFrameCount;			// 背景图片的帧数

	int miNameAreaWidth;
	int miHotKeyAreaWidth;

	bool mbIsChecked;				// 是否Checked态
	bool mbIsShowExtendMenu;		// 是否显示扩展菜单
	
	int miExtendMenuID;				// 弹出菜单ID

	CEvPopupMenu* mpoSubPopupMenu;
	bool mbIsSubMenuShow;

	LONG miDuration;			

	bool mbIsFocused;



	LONG mlHotKeyID;				// 热键ID，由热键组合而成      Ctrl | Shift | Alt | VirtualKey

};

#define MENUITEM_HOTKEY_ATL			0x00000100
#define MENUITEM_HOTKEY_CTRL		0x00000200
#define MENUITEM_HOTKEY_SHIFT		0x00000400






#define TIMER_ID_SHOW_POPUPMENU			1
#define DEFAULT_TIMER_DURATION			500

#define TF_ID_MENUITEM_COMMAND_ID		L"ID"						// 菜单项的唯一标识ID
#define TF_ID_MENUITEM_TYPE				L"Type"						// 菜单项类型,具体值见枚举类型ENUM_MENUITEM_TYPE的定义
#define TF_ID_MENUITEM_SEPARATION		L"Separation"				// 分割线图
#define TF_ID_MENUITEM_TEXT				L"Text"						// 菜单项文字， 如果有快捷键，也加载文字后面如 新建(&N)

// 热键
#define TF_ID_MENUITEM_HOTKEY			L"HotKey"					// 热键显示字符  如 Shift + X
#define TF_ID_MENUITEM_HOTKEY_VIRTUAL_KEY L"HotKey/VirtualKey"		// 虚拟键值（必须有，才能设置热键）（！！！注意，如果是字符，必须是大写的，因为小写的字符被其他键值占用了）
#define TF_ID_MENUITEM_HOTKEY_SHAFT_KEY	L"HotKey/ShiftKey"			// 指定是否需要使用Shift键，如果存在这个，则表示需要，不存在，则表示不需要。
#define TF_ID_MENUITEM_HOTKEY_CTRL_KEY	L"HotKey/CtrlKey"			// 指定是否需要使用Ctrl键，如果存在这个，则表示需要，不存在，则表示不需要。
#define TF_ID_MENUITEM_HOTKEY_ALT_KEY	L"HotKey/AltKey"			// 指定是否需要使用Alt键，如果存在这个，则表示需要，不存在，则表示不需要。

// 备用热键
#define TF_ID_MENUITEM_RESERVE_HOTKEY	L"ReserveHotKey"			// 用来备用的热键，这个热键是隐藏显示的，只有在有了普通热键之后才会去注册这个热键
#define TF_ID_MENUITEM__RESERVE_HOTKEY_VIRTUAL_KEY L"ReserveHotKey/VirtualKey"		// 虚拟键值（必须有，才能设置热键）（！！！注意，如果是字符，必须是大写的，因为小写的字符被其他键值占用了）
#define TF_ID_MENUITEM__RESERVE_HOTKEY_SHAFT_KEY	L"ReserveHotKey/ShiftKey"			// 指定是否需要使用Shift键，如果存在这个，则表示需要，不存在，则表示不需要。
#define TF_ID_MENUITEM__RESERVE_HOTKEY_CTRL_KEY	L"ReserveHotKey/CtrlKey"			// 指定是否需要使用Ctrl键，如果存在这个，则表示需要，不存在，则表示不需要。
#define TF_ID_MENUITEM__RESERVE_HOTKEY_ALT_KEY	L"ReserveHotKey/AltKey"			// 指定是否需要使用Alt键，如果存在这个，则表示需要，不存在，则表示不需要。

// (快捷键暂时不支持)

#define TF_ID_MENUITEM_CHECKED_TEXT		L"CheckedText"				// Checked状态下菜单项文字，热键和快捷键设置和MFC的设置方法相同. 如 新建(&N)\tCtrl+N
#define TF_ID_MENUITEM_LEFT_ICON		L"LeftIcon"					// 菜单项名称前面的图标
#define TF_ID_MENUITEM_LEFT_CHECKED_ICON L"LeftCheckedIcon"			// 当进入Checked状态的图标
#define TF_ID_MENUITEM_RIGHT_ICON		L"RightIcon"				// 菜单项名称后面的图标
#define TF_ID_MENUITEM_POPUPMENU_ID		L"PopupMenuID"				// 要弹出的菜单的ID
#define TF_ID_MENUITEM_DURATION			L"Duration"					// 弹出子菜单的周期，以毫秒为单位，默认为500毫秒

