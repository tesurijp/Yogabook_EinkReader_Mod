/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

// 存放该PopupMenu下所有MenuItem公共属性信息的结构体
struct _ST_MENUITEM_INFO;
typedef _ST_MENUITEM_INFO ST_MENUITEM_INFO, *PST_MENUITEM_INFO;

class CEvMenuItem;

typedef  cmmVector<CEvMenuItem*, 5, 5>  CMenuItemVec;

DECLARE_BUILTIN_NAME(PopupMenu)
class CEvPopupMenu:
	public CXuiElement<CEvPopupMenu, GET_BUILTIN_NAME(PopupMenu)>
{
	friend CXuiElement<CEvPopupMenu,GET_BUILTIN_NAME(PopupMenu)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	// 描述：
	//		获取给定MainID的PopupMenu(包括MenuItem所弹出来的PopupMenu)
	// 返回值：
	//		成功返回对应ID的迭代器，失败返回NULL
	IEinkuiIterator* GetPopupMenuByMainID(
		IN UINT niUinqueID
		);

protected:
	CEvPopupMenu(void);
	~CEvPopupMenu(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	//// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	//virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
private:
	void LoadResource();

	// 初始化MenuItemInfo
	void InitMenuItemInfo();

	// 描述：
	//		处理当有子菜单项获得焦点
	void OnMenuItemGetFocused(
		IN IEinkuiIterator* npIterMenuItem
		);

	// 描述：
	//		插入新的菜单项
	 bool OnInsertMenuItem(
		 IN STCTL_POPUPMENU_MENUITEMINFO ndMenuInfo
		 );

	 // 描述：
	 //		重新计算菜单项的布局
	 void ReLayoutMenuItem();

	 // 描述：
	 //		根据CommandID删除菜单项
	 bool DeleteItemByCommandID(
		 IN UINT niCommandID
		 );

	 // 描述：
	 //		根据CommandID获取菜单项
	 IEinkuiIterator* GetItemByCommandID(
		 IN UINT niCommandID
		 );

	 // 描述：
	 //		根据索引删除菜单项,传入-1删除全部。
	 bool DeleteItemByIndex(
		 IN int niIndex
		 );

	 // 描述：
	 //		根据索引获取菜单项
	 IEinkuiIterator* GetItemByIndex(
		 IN UINT niIndex
		 );

private:

	IEinkuiBitmap* mpoItemBgBitmap;

	int miMainID;
	PST_MENUITEM_INFO mpoMenuItemInfo;

	CMenuItemVec moMenuItemVec;				

	int miItemBgFrameCount;

	bool mbIsManagerMenuItemEnable;				// 显示的时候是否管理菜单的可用状态
};

// MenuItem显示的位置
#define TF_ID_POPUPMENU_ALIGN_LEFT			L"Align/Left"		
#define TF_ID_POPUPMENU_ALIGN_RIGHT			L"Align/Right"
#define TF_ID_POPUPMENU_ALIGN_TOP			L"Align/Top"	
#define TF_ID_POPUPMENU_ALIGN_BOTTOM		L"Align/Bottom"	

#define TF_ID_POPUPMENU_MAIN_ID				L"ID"				// 该PopupMenu的唯一ID
#define TF_ID_POPUPMENU_MENU_ITEM			L"MenuItem"			// 存放MenuItem的结点

#define TF_ID_POPUPMENU_TEXT_WIDTH			L"TextWidth"		// 允许的最大菜单项中文字宽度（增加这个属性主要是提供给ComboBox使用，因为其弹出菜单通常是固定宽度的）

#define TF_ID_POPUPMENU_LEFT_WIDTH			L"LeftWidth"
#define TF_ID_POPUPMENU_RIGHT_WIDTH			L"RightWidth"
#define TF_ID_POPUPMENU_MIDDLE_WIDTH		L"MiddleWidth"
#define TF_ID_POPUPMENU_LEFT_ICON_WIDTH		L"LeftIconWidth"
#define TF_ID_POPUPMENU_LEFT_ICON_HEIGHT	L"LeftIconHeight"
#define TF_ID_POPUPMENU_RIGHT_ICON_WIDTH	L"RightIconWidth"
#define TF_ID_POPUPMENU_RIGHT_ICON_HEIGHT	L"RightIconHeight"
#define TF_ID_POPUPMENU_ITEM_HEIGHT			L"ItemHeight"		// 出去分割线以外每一项的高度，

#define TF_ID_POPUPMENU_ITEM_BACKGROUND		L"ItemBackGround"	// 菜单项的背景
#define TF_ID_POPUPMENU_ITEM_FRAMECOUNT		L"ItemFrameCount"	// 菜单项背景的帧数

#define TF_ID_POPUPMENU_FONT_NAME			L"Font/Name"			
#define TF_ID_POPUPMENU_FONT_SIZE			L"Font/Size"			
#define TF_ID_POPUPMENU_FONT_COLOR			L"Font/Color"		
#define TF_ID_POPUPMENU_FONT_DISABLE_COLOR	L"Font/DisableColor"// 禁用态菜单项字体颜色
#define TF_ID_POPUPMENU_FONT_FOCUS_COLOR	L"Font/FocusColor"// 焦点态菜单项字体颜色





// 一些默认参数值
#define DEFAULT_LEFT_WIDTH					40
#define DEFAULT_RIGHT_WIDTH					40
#define DEFAULT_MIDDLE_WIDTH				20
//#define DEFAULT_LEFT_ICON_WIDTH			20
//#define DEFUALT_LEFT_ICON_HEIGHT			20
//#define DEFAULT_RIGHT_ICON_WIDTH			20
//#define DEFAULT_RIGHT_ICON_HEIGHT			20

#define DEFAULT_ITEM_FRAMECOUNT				2

#define DEFAULT_FONT_NAME					L"Tahoma"
#define DEFAULT_FONT_SIZE					15
#define DEFAULT_FONT_COLOR					0x000000
#define DEFAULT_FONT_DISABLE_COLOR			0x7d7d7d00
#define DEFAULT_FONT_FOCUS_COLOR			0xffffff00




