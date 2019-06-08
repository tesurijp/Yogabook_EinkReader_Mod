/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#ifndef _ECTL_H_
#define _ECTL_H_

/*
	本DLL输出XUI默认控件，所有的控件都在这定义
*/






#pragma pack(4)

// 通用宏定义
#define ES_SHOW_TEXT_MAX_LEN 200   //结构体用，显示文字最大Buffer长度




//////////////////////////////////////////////////////////////////////////
//元素类的描述方式
//////////////////////////////////////////////////////////////////////////
//元素类类型ID，命名方法，以ES_ETYPE_前缀，后面跟类型的名称；所有的扩展类型用ES_ETYPE_开头，紧跟着是扩展包名，然后是类型名，如: ES_ETYPE_STOCK_CHART，就可以理解为某个股票扩展模块的图表控件
//元素类的实例化数据结构，名称规则，ST_ELALC_前缀，后面跟数据类型的名称;如：ST_ELALC_ELEMENT
//元素类触发的消息，命名时，用EEVT_前缀
//元素类接受的访问消息，命名时，用EACT_前缀

//#define ES_ETYPE_DIRECTORPH L"DirectorPh"
//#define ES_ETYPE_TURN3D L"Turn3D"

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_BUTTON L"Button"			//按钮

//按钮显示文字
//struct _STEMS_BUTTON_TEXT{
//	wchar_t Text[ES_SHOW_TEXT_MAX_LEN];	    //按钮显示文字
//};
//typedef _STEMS_BUTTON_TEXT STEMS_BUTTON_TEXT,*PSTEMS_BUTTON_TEXT;	


//按钮控件会触发的消息
#define EMSG_BUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_BUTTON,_MJ_NUM,_MN_NUM)

//按钮被按下
#define EEVT_BUTTON_PRESSED		EMSG_BUTTON_REQUEST(1,1)
//Input			TEMS_MSG_BASE 结构体
//Output		None
//Result

//按下被释放
#define EEVT_BUTTON_RELEASE		EMSG_BUTTON_REQUEST(1,2)
//Input			TEMS_MSG_BASE 结构体
//Output		None
//Result

//被按下的按钮，鼠标意外地离开了按钮窗体，从而避免Release行为，产生这个事件
#define EEVT_BUTTON_ESCAPED		EMSG_BUTTON_REQUEST(1,3)
//Input			TEMS_MSG_BASE 结构体
//Output		None
//Result

//鼠标进入按钮有效范
#define EEVT_BUTTON_MOUSE_IN	EMSG_BUTTON_REQUEST(1,4) 
//Input			IEinkuiIterator* 该按钮
//Output		None
//Result

//鼠标移出按钮有效范
#define EEVT_BUTTON_MOUSE_OUT	EMSG_BUTTON_REQUEST(1,5) 
//Input			IEinkuiIterator* 该按钮
//Output		None
//Result

//按钮被单击
#define EEVT_BUTTON_CLICK		EMSG_BUTTON_REQUEST(1,6)
//Input			None
//Output		None
//Result

//自定义动画播放完成
#define EEVT_BUTTON_PLAYED		EMSG_BUTTON_REQUEST(1,7)
//Input			LONG 播放完成的动画序号
//Output		None
//Result


//按钮被选中
#define EEVT_BUTTON_CHECKED		EMSG_BUTTON_REQUEST(1,8)
//Input			None
//Output		None
//Result

//按钮被取消选中状态
#define EEVT_BUTTON_UNCHECK		EMSG_BUTTON_REQUEST(1,9)
//Input			None
//Output		None
//Result


//按钮控件会接收的消息
//设置按钮文字
#define EACT_BUTTON_SETTEXT EMSG_BUTTON_REQUEST(2,1)
//Input			wchat_t* 要改变的按钮文字
//Output		None
//Result

//获取按钮文字
#define EACT_BUTTON_GETTEXT EMSG_BUTTON_REQUEST(2,2)
//Input			None
//Output		wchat_t** 返回的字符串指针不应该长期持有，无需释放
//Result

//设置按钮Check状态
#define EACT_BUTTON_SET_CHECKED EMSG_BUTTON_REQUEST(2,3)	
//Input			bool
//Output		None
//Result

//获取按钮Check状态
#define EACT_BUTTON_GET_CHECKED EMSG_BUTTON_REQUEST(2,4)
//Input			None
//Output		bool
//Result

//播放按钮的自定义动画
#define EACT_BUTTON_PLAY_OTHER_ANIMATION EMSG_BUTTON_REQUEST(2,5)
//Input			LONG,指明播放几号动画
//Output		NONE
//Result

//更换背景图片,相对路径
#define EACT_BUTTON_CHANGE_PIC EMSG_BUTTON_REQUEST(2,6)
//Input			wchat_t* 图片相对于DLL所在目录的路径
//Output		NONE
//Result

//更换背景图片,全路径
#define EACT_BUTTON_CHANGE_PIC_FULLPATH EMSG_BUTTON_REQUEST(2,7)
//Input			wchat_t* 图片的全路径
//Output		NONE
//Result

//设置激活区域
#define EACT_BUTTON_SET_ACTION_RECT EMSG_BUTTON_REQUEST(2,8)
//Input			D2D1_SIZE_F 要设置的激活区域大小
//Output		NONE
//Result


//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_RADIO_BUTTON_GROUP L"RadioButtonGroup"			//单选按钮组
#define EMSG_RADIO_BUTTON_GROUP_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_RADIO_BUTTON_GROUP,_MJ_NUM,_MN_NUM)

//单选按键组会触发的消息
// 被选中的对象改变
#define EEVT_RBG_SELECTED_CHANGED EMSG_RADIO_BUTTON_GROUP_REQUEST(1,1)
//Input			ULONG 被选中对象的ID，重复选中一个对象，不会重复发送此消息
//Output		NONE
//Result		NONE

// 被选中的对象被再次点击
#define EEVT_RBG_SELECTED_ITEM_CLICK		 EMSG_RADIO_BUTTON_GROUP_REQUEST(1,2)
//Input			ULONG 被点击对象的ID，
//Output		NONE
//Result		NONE


//单选按钮组会接收的消息
//设置某项选中
#define EACT_RBG_SET_SELECT EMSG_RADIO_BUTTON_GROUP_REQUEST(2,1)
//Input			ULONG 要求选中对象的ID
//Output		NONE
//Result		NONE

//获取当前选中的项
#define EACT_RBG_GET_SELECT EMSG_RADIO_BUTTON_GROUP_REQUEST(2,2)
//Input			none
//Output		ULONG	当前被选中的对象ID
//Result		NONE

//把某项禁用
#define EACT_RBG_DISABLE EMSG_RADIO_BUTTON_GROUP_REQUEST(2,3)
//Input			ULONG 要求选中对象的ID
//Output		NONE
//Result		NONE

//把某项启用
#define EACT_RBG_ENABLE EMSG_RADIO_BUTTON_GROUP_REQUEST(2,4)
//Input			ULONG 要求选中对象的ID
//Output		NONE
//Result		NONE

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_PICTUREFRAME L"PictureFrame"	//图片元素
#define EMSG_PICTUREFRAME_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_PICTUREFRAME,_MJ_NUM,_MN_NUM)


//图片元素会触发的消息

//图片元素会接收的消息

//切换显示帧
#define EACT_PICTUREFRAME_SET_INDEX EMSG_PICTUREFRAME_REQUEST(2,1)
//Input			LONG,指明显示第几帧
//Output		NONE
//Result

//更换显示图片,相对路径
#define EACT_PICTUREFRAME_CHANGE_PIC EMSG_PICTUREFRAME_REQUEST(2,2)
//Input			wchat_t* 图片相对于DLL所在目录的路径，现在只能更换为只有1帧的图
//Output		NONE
//Result

//更换显示图片,全路径
#define EACT_PICTUREFRAME_CHANGE_PIC_FULLPATH EMSG_PICTUREFRAME_REQUEST(2,3)
//Input			wchat_t* 图片的全路径，现在只能更换为只有1帧的图
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_LIST L"List"	//List元素

//定义list的样式 
//smallItem 类似于SmallIcon
#define	LIST_VIEW_STYLE_SMALLITEM  0
#define	LIST_VIEW_STYLE_REPORT 1

//设置自动增长模式，该模式的意思是列表会根据项的多少而自动变长和变宽，
//该模式不会出现滚动条
#define LIST_VIEW_STYLE_AUTO_FIT_X 2
#define LIST_VIEW_STYLE_AUTO_FIT_Y 3

//定义List的子控件ID，List的1和2 被占用，其他控件必须大于SCROLLBAR_SUBID_BASE
#define V_SCROLL_BAR_ID 1
#define H_SCROLL_BAR_ID 2
#define INSERT_MARK_ID  3
#define LIST_SUBID_BASE 10

#define EMSG_LIST_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_LIST,_MJ_NUM,_MN_NUM)



//切换显示帧
#define EACT_LIST_SET_PIC_INDEX EMSG_LIST_REQUEST(2,1)
//Input			LONG,指明显示第几帧
//Output		NONE
//Result

//更换显示图片
#define EACT_LIST_CHANGE_PIC EMSG_LIST_REQUEST(2,2)
//Input			wchat_t* 图片的全路径，现在只能更换为只有1帧的图
//Output		NONE
//Result

//Item点击消息
#define EACT_LIST_ITEMCLICK EMSG_LIST_REQUEST(2,3)
//Input			Item 指针
//Output		NONE
//Result

//给list发这个消息，来强制list的滚动的位置
#define EACT_LIST_DOCSCROLL EMSG_LIST_REQUEST(2,4)
//Input			FLOAT （position）
//Output		NONE
//Result



//设置list的显示模式
#define EACT_LIST_SET_STYLE EMSG_LIST_REQUEST(2,5)
//Input			STYLE(long)
//Output		NONE
//Result
//获取显示模式消息
#define EACT_LIST_GET_STYLE EMSG_LIST_REQUEST(2,6)
//Input			NONE
//Output		STYLE(long)
//Result

//在尾部添加元素
#define EACT_LIST_ADD_ELEMENT EMSG_LIST_REQUEST(2,7)
//Input			LONG (CXuiElement*)
//Output		NONE
//Result


//删除元素
#define EACT_LIST_DELETE_ELEMENT EMSG_LIST_REQUEST(2,8)
//Input			LONG (CXuiElement*)
//Output		NONE
//Result

//删除所有元素，重新设置LIST
#define EACT_LIST_RESET EMSG_LIST_REQUEST(2,9)
//Input			NONE
//Output		NONE
//Result

//删除参数索引元素，重新设置LIST
#define EACT_LIST_DELETE_ELEMENT_INDEX EMSG_LIST_REQUEST(2,10)
//Input			Index
//Output		NONE
//Result

//通知List是不是需要其帮忙释放子元素
#define EACT_LIST_SET_MEMORY_MANAGER EMSG_LIST_REQUEST(2,11)
//Input			bool
//Output		NONE
//Result
//在头部添加元素
#define EACT_LIST_ADD_ELEMENT_HEAD EMSG_LIST_REQUEST(2,12)
//Input			LONG (CXuiElement*)
//Output		NONE
//Result
//通知父窗口滚动条显示和隐藏
#define EACT_LIST_VSCROLLBAR_SHOW EMSG_LIST_REQUEST(2,13)
//Input			bool (true for show /false for hide)
//Output		NONE
//Result

//获取滚动范围
#define EACT_LIST_GET_SCROLL_RANGE EMSG_LIST_REQUEST(2,14)
//Input			NONE
//Output		FLOAT 
//Result
//获取当前滚动位置
#define EACT_LIST_GET_SCROLL_POSITION EMSG_LIST_REQUEST(2,15)
//Input			NONE
//Output		FLOAT 
//Result

//返回list的父窗口，说明该List正在滚动
#define EACT_LIST_SCROLLING EMSG_LIST_REQUEST(2,16)
//Input			FLOAT（POSITION）
//Output		
//Result


//设置纵向滚动条相对于List父窗口的位置和大小
#define EACT_LIST_SET_VSCROLLBAR_RECT EMSG_LIST_REQUEST(2,17)
//Input			D2D1_RectF（rect）
//Output		
//Result

//返回list Item位置
#define EACT_LIST_GET_ITEM_POSITION EMSG_LIST_REQUEST(2,18)
//Input			Item Iterator
//Output		FLOAT（POSITION）
//Result
//通知List 重新计算子项的位置和布局
#define EACT_LIST_RECACULATE EMSG_LIST_REQUEST(2,19)
//Input			Item Iterator
//Output		FLOAT（POSITION）
//Result
//通知List的父窗口 list的布局发生了变化
#define EACT_LIST_LAYOUT_CHANGE EMSG_LIST_REQUEST(2,20)
//Input			Item Iterator
//Output		FLOAT（POSITION）
//Result
//返回list Item位置索引 0 1 2 3
#define EACT_LIST_GET_ITEM_INDEX EMSG_LIST_REQUEST(2,21)
//Input			Item Iterator
//Output		int（index）
//Result


//在list Item位置索引 0 1 2 3 插入
#define EACT_LIST_INSER_ITEM_INDEX EMSG_LIST_REQUEST(2,22)
//Input			_STCTL_LIST_INSERT
//Output		int（index）
//Result
struct _STCTL_LIST_INSERT{
	int mnIndex;		// index
	IEinkuiIterator*  mpElement;	//插入的IEinkuiIterator
	
};
//删除index后的元素
#define EACT_LIST_DELETE_ELEMENT_BIGGER_OR_EQUAL_INDEX EMSG_LIST_REQUEST(2,23)
//Input			Index
//Output		NONE
//Result

static const GUID EGUID_LIST_DROP_ITEM = 
{ 0x81efb6da, 0xea4b, 0x4e5f, { 0xa4, 0xcd, 0x9c, 0xa4, 0x3e, 0x6c, 0x45, 0xf0 } };
//设置链表是否接收ITEM DROP的消息，主要是两个同类型的链表之间的拖拽
#define EACT_LIST_SET_ACCEPT_DROP_ITEM EMSG_LIST_REQUEST(2,24)
//Input			bool
//Output		NONE
//Result

//开始移动元素
#define EACT_LIST_DRAG_ITEM_START EMSG_LIST_REQUEST(2,25)
//Input			Iterator*
//Output		NONE
//Result

//开始移动元素
#define EACT_LIST_DRAG_ITEM_DRAGING EMSG_LIST_REQUEST(2,26)
//Input			STMS_DRAGGING_ELE
//Output		NONE
//Result

//结束移动元素
#define EACT_LIST_DRAG_ITEM_END EMSG_LIST_REQUEST(2,27)
//Input			Iterator*
//Output		NONE
//Result

//设置跟随随便移动的Iterator
#define EACT_LIST_SET_DRAG_FOR_MOUSER_ITERATOR EMSG_LIST_REQUEST(2,28)
//Input			Iterator*
//Output		NONE
//Result
//设置插入标识Iterator
#define EACT_LIST_SET_DROP_MARK_ITERATOR EMSG_LIST_REQUEST(2,29)
//Input			Iterator*
//Output		NONE
//Result


struct _STCTL_LIST_ITEM_CLICK{
	int mnFlag;                   //带入标记
	D2D1_POINT_2F mPosWorld;		// 鼠标点击的位置
	IEinkuiIterator*  mpElement;	//插入的IEinkuiIterator

};
//Item右键点击消息
#define EACT_LIST_ITEMCLICK_RBUTTON EMSG_LIST_REQUEST(2,30)
//Input			_STCTL_LIST_ITEM_CLICK
//Output		NONE
//Result
//删除index后的元素,但是不Close这些元素
#define EACT_LIST_DELETE_ELEMENT_BIGGER_OR_EQUAL_INDEX_NO_CLOSE EMSG_LIST_REQUEST(2,31)
//Input			Index
//Output		NONE
//Result

struct _STCTL_LIST_SWAP{
	IEinkuiIterator*  mpFromElement;		// 交换FROM
	IEinkuiIterator*  mpToElement;	//交换TO

};
//交换元素
#define EACT_LIST_SWAP_ELEMENT EMSG_LIST_REQUEST(2,32)
//Input			_STCTL_LIST_SWAP
//Output		NONE
//Result

//删除元素但是不Close
#define EACT_LIST_DELETE_ELEMENT_NO_CLOSE EMSG_LIST_REQUEST(2,33)
//Input			IEinkuiIterator
//Output		NONE
//Result

//返回list Item的数量
#define EACT_LIST_GET_ITEM_NUM EMSG_LIST_REQUEST(2,34)
//Input			NULL 
//Output		int（）
//Result

//把指定项移动到可显示区域
#define EACT_LIST_SHOW_BY_INDEX EMSG_LIST_REQUEST(2,35)
//Input			LONG 
//Output		NULL
//Result

//换图
//#define EACT_PICTUREFRAME_CHANGE_PIC EMSG_BUTTON_REQUEST(2,2)
////Input			LONG,指明显示第几帧
//struct _STEMS_PICTURE_TEXT{
//	wchar_t Text[ES_SHOW_TEXT_MAX_LEN];	    //按钮显示文字
//};
//typedef _STEMS_BUTTON_TEXT STEMS_BUTTON_TEXT,*PSTEMS_BUTTON_TEXT;	
//
////Output		NONE
////Result

// 触发的消息

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_STATICTEXT L"StaticText"	//文本文字
#define EMSG_STATICTEXT_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_STATICTEXT,_MJ_NUM,_MN_NUM)
//文本元素会触发的消息


//文本元素接收的消息
//更换显示文字
#define EACT_STATICTEXT_SET_TEXT EMSG_STATICTEXT_REQUEST(2,1)
//Input			wchat* 要显示的新文本
//Output		NONE
//Result

//获取显示文字
#define EACT_STATICTEXT_GET_TEXT EMSG_STATICTEXT_REQUEST(2,2)
//Input			none
//Output		wchat_t** 返回的字符串指针不应该长期持有，无需释放
//Result


//文本元素的颜色
//更换显示文字
#define EACT_STATICTEXT_SET_TEXT_COLOR EMSG_STATICTEXT_REQUEST(2,3)
//Input			RGBA(LONG)
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_SLIDERBUTTON L"SliderButton"	//滑动按钮
#define EMSG_SLIDERBUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_SLIDERBTN,_MJ_NUM,_MN_NUM)

//定义Sliderbutton的style宏
#define ES_SLIDER_BUTTON_STYLE_HOR 1
#define ES_SLIDER_BUTTON_STYLE_VER 2
#define ES_SLIDER_BUTTON_STYLE_ANYWAY 3

//返回父窗口拖拽消息
#define EACT_SLIDERBUTTON_DRAG_START EMSG_SLIDERBUTTON_REQUEST(2,1)
#define EACT_SLIDERBUTTON_DRAG_END EMSG_SLIDERBUTTON_REQUEST(2,2)
#define EACT_SLIDERBUTTON_DRAGING EMSG_SLIDERBUTTON_REQUEST(2,3)
//Input			STMS_DRAGGING_ELE
//output        NONE
//Result

//设置sliderbutton的滚动方向，参数将上面的style宏
#define EACT_SLIDERBUTTON_SET_STYLE EMSG_SLIDERBUTTON_REQUEST(2,4)
//Input			nStyle(LONG)
//Output		NONE
//Result

//设置sliderbutton的滚动区域
#define EACT_SLIDERBUTTON_SET_SLIDERRECT EMSG_SLIDERBUTTON_REQUEST(2,5)
//Input			D2D1_RECT_F
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_SCROLLBAR L"ScrollBar"	//进度条
#define EMSG_SCROLLBAR_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_SCROLLBAR,_MJ_NUM,_MN_NUM)

//返回父窗口纵向滚动位置+1 或 -1 消息
#define EACT_SCROLLBAR_VSCROLL EMSG_SCROLLBAR_REQUEST(2,1)
//Input			0/UP 或 1/DOWN
//Output		NONE
//Result

//返回父窗口，其纵向滚动条滑块的位置
#define EACT_SCROLLBAR_VSCROLL_THUMB_POSITION EMSG_SCROLLBAR_REQUEST(2,2)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//返回父窗口横向滚动的位置+1 或 -1 消息
#define EACT_SCROLLBAR_HSCROLL EMSG_SCROLLBAR_REQUEST(2,2)
//Input			0/Lefe 或 1/Right
//Output		NONE
//Result

//返回父窗口，其横向滚动条滑块的位置
#define EACT_SCROLLBAR_HSCROLL_THUMB_POSITION EMSG_SCROLLBAR_REQUEST(2,4)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//设置纵向滚动条滑块位置
#define EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION EMSG_SCROLLBAR_REQUEST(2,5)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//设置横向滚动条滑块位置
#define EACT_SCROLLBAR_HSCROLL_SET_SLIDER_POSTION EMSG_SCROLLBAR_REQUEST(2,6)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//设置滚动条的滚动范围
#define EACT_SCROLLBAR_HVSCROLL_SET_DELTA_SIZE EMSG_SCROLLBAR_REQUEST(2,7)
//Input			fSize(FLOAT)
//Output		NONE
//Result

//获取横向滚动条的高度
#define EACT_SCROLLBAR_HSCROLL_GET_HEIGTH EMSG_SCROLLBAR_REQUEST(2,8)
//Input			NULL
//Output		fSize(FLOAT)
//Result

//获取纵向滚动条的宽度
#define EACT_SCROLLBAR_VSCROLL_GET_WIDTH EMSG_SCROLLBAR_REQUEST(2,9)
//Input			NULL
//Output		fSize(FLOAT)
//Result

//Relactio滚动条
#define EACT_SCROLLBAR_HVSCROLL_RELACATION EMSG_SCROLLBAR_REQUEST(2,10)
//Input			NULL
//Output		NULL
//Result

//模拟点击上下按钮
#define EACT_SCROLLBAR_BT_CLICK EMSG_SCROLLBAR_REQUEST(2,11)
//Input			bool 
//Output		NULL
//Result


//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_SLIDERBAR L"SliderBar"	//滚动条
#define EMSG_SLIDERBAR_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_SLIDEREBAR,_MJ_NUM,_MN_NUM)


// 被点击改变了滑块的位置
#define EACT_SLIDERBAR_THUMB_CLICK EMSG_SLIDERBAR_REQUEST(2,1)
//Input			fPositin(FLOAT)
//Output		NONE
//Result


//设置滑块范围,最小默认是0，Input 传入最大范围
#define EACT_SLIDERBAR_SET_RANGE EMSG_SLIDERBAR_REQUEST(2,2)
//Input			fPositin(FLOAT)，最大范围
//Output		NONE
//Result

//设置滑块位置，请先让Range 再设置位置
#define EACT_SLIDERBAR_SET_POSITION EMSG_SLIDERBAR_REQUEST(2,3)
//Input			fPositin(FLOAT)，
//Output		NONE
//Result

//开始拖动滑块
#define EACT_SLIDERBAR_DRAG_START EMSG_SLIDERBAR_REQUEST(2,4)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//滑块拖动过程中
#define EACT_SLIDERBAR_DRAGING EMSG_SLIDERBAR_REQUEST(2,4)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//结束拖动滑块
#define EACT_SLIDERBAR_DRAG_END EMSG_SLIDERBAR_REQUEST(2,5)
//Input			fPositin(FLOAT)
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_EDIT L"Edit"	//编辑框

// 编辑框控件可能触发消息 ××××××××××××××××××××××××××××××××××××××××××××

// 内容被修改
#define EEVT_EDIT_CONTENT_MODIFIED EMSG_DEFINE(LMSG_TP_EDIT,1,1)
// input none
// output none
// result na

// 内容修改完成，输入时按下"Enter"键，或者移除键盘焦点，都将导致内容完成消息被发送
// 应届收到"设置内容"消息而导致的内容修改，不会发送这条消息
#define EEVT_EDIT_CONTENT_COMPLETION EMSG_DEFINE(LMSG_TP_EDIT,1,5)
// input wchar[] the content string with a tail UNICODE_NULL
// output none
// result na

// 鼠标焦点变化
#define EEVT_EDIT_MOUSE_FOCUS EMSG_DEFINE(LMSG_TP_EDIT,2,1)
// input LONG  zero if the focus is lost,nonzero if we get the mousefocus
// output none
// result na

// 鼠标拖动，按下左中右任何一个按钮然后在界面上拖动
#define EEVT_EDIT_DRAGING	EMSG_DEFINE(LMSG_TP_EDIT,2,5)
// input none
// output none
// result na

// 鼠标双击
#define EEVT_EDIT_MOUSE_DCLICK	EMSG_DEFINE(LMSG_TP_EDIT,2,11)
// input LONG the number of the character was double-clicked
// oputput none
// result na

// 键盘焦点变化
#define EEVT_EDIT_KEYBOARD_FOCUS	EMSG_DEFINE(LMSG_TP_EDIT,3,1)
// input LONG  zero if the focus is lost,nonzero if we get the keyboardf
// output none
// result na

// 按键行为，父元素接收到这条消息，如果希望控件不要处理这个输入行为，请返回ERESULT_DISCARD
#define EEVT_EDIT_KEY_STIKE		EMSG_DEFINE(LMSG_TP_EDIT,3,5)
// intput STEMS_KEY_PRESSED
// output none
// result ERESULT_DISCARD to discard the operation

// 字符输入，父元素接收到这条消息，如果希望控件不要处理这个字符输入行为，请返回ERESULT_DISCARD
#define EEVT_EDIT_CHAR_INPUT	EMSG_DEFINE(LMSG_TP_EDIT,3,10)
// intput STEMS_CHAR_INPUT
// output none
// result ERESULT_DISCARD to discard the operation



// 编辑框控件的设置消息 ××××××××××××××××××××××××××××××××××××××××××××××

// 设置文本内容, must be send
#define EACT_EDIT_SET_TEXT EMSG_DEFINE(LMSG_TP_EDIT,20,1)
// input wchar[] must have the tail UNICODE_NULL and no more than 512 characters
// output none
// result ERESULT_SUCCESS or ERESULT_UNSUCCESSFUL

// 只接受数字
#define EACT_EDIT_NUMBER_ONLY EMSG_DEFINE(LMSG_TP_EDIT,20,5)
// input LONG, nonzero indicate that the edit shalln't accept characters except these are numbers '0' to '9'; zero set back to normal mode
// output none
// result na

// 密文显示
#define EACT_EDIT_PASSWORD_MODE EMSG_DEFINE(LMSG_TP_EDIT,20,6)
// intput LONG, nonzero inidcate that the edit shall draw every charactor as '*';zero set back to normal mode 
// output none
// result na

// 字数限制
#define EACT_EDIT_SET_LENGTH_LIMIT EMSG_DEFINE(LMSG_TP_EDIT,20,7)
// input LONG, the limit of characters the edit can save. < 0 to cancel the limit
// output none
// result na


// 获得文本内容，must be send
#define EACT_EDIT_GET_TEXT EMSG_DEFINE(LMSG_TP_EDIT,21,1)
// intput none
// output wchar[] buffer to receive the text
// result return ERESULT_INSUFFICIENT_RESOURCES if output buffer is short than the text to obtain. 

// 获得文本内容包含的字符总数，不包括结尾的NULL
#define EACT_EDIT_GET_TEXT_LENGTH	EMSG_DEFINE(LMSG_TP_EDIT,21,2)
// input none
// output ULONG
// result ERESULT_SUCCESS

// 获取选中信息
#define EACT_EDIT_GET_SELECTION EMSG_DEFINE(LMSG_TP_EDIT,22,1)
// intput none
// output
struct _STCTL_EDIT_SELECTION{
	LONG Chars;		// 控件内的字符数，不包括结尾'\0'；当用于消息EACT_EDIT_SET_SELECTION时无效
	LONG SelBegin;	// 选中区域的首字符索引，0表示控件中的第一个字符；没有字符选中时无意义；
	LONG SelCount;	// 选中区域的字符数，0表示没有字符被选中；-1表示全部选中		add by colin
};
typedef _STCTL_EDIT_SELECTION STCTL_EDIT_SELECTION,* PSTCTL_EDIT_SELECTION;
// success or failure

// 设置选中
#define EACT_EDIT_SET_SELECTION EMSG_DEFINE(LMSG_TP_EDIT,22,2)
// intput STCTL_EDIT_SELECTION
// output none
// success or failure







//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_LABEL L"Label"	//标签

// 标签控件可能触发消息，只有设置了超链接属性才会触发下列消息 ××××××××××××××××××××××××××××××××××××××××××××

// 鼠标焦点变化
#define EEVT_LABEL_MOUSE_FOCUS EMSG_DEFINE(LMSG_TP_LABEL,1,1)
// input LONG  zero if the focus is lost,nonzero if we get the mousefocus
// output none
// result na

// OPEN，鼠标左键单击，或者是获得键盘焦点是按下空格键
#define EEVT_LABEL_OPEN	EMSG_DEFINE(LMSG_TP_LABEL,1,2)
// input none
// oputput none
// result na

// 键盘焦点变化
#define EEVT_LABEL_KEYBOARD_FOCUS	EMSG_DEFINE(LMSG_TP_LABEL,2,1)
// input LONG  zero if the focus is lost,nonzero if we get the keyboardf
// output none
// result na

// 按键行为，父元素接收到这条消息，如果希望控件不要处理这个输入行为，请返回ERESULT_DISCARD
#define EEVT_LABEL_KEY_STIKE		EMSG_DEFINE(LMSG_TP_LABEL,2,2)
// intput STEMS_KEY_PRESSED
// output none
// result ERESULT_DISCARD to discard the operation



// 标签控件的设置消息 ××××××××××××××××××××××××××××××××××××××××××××××

// 设置文本内容, must be send
#define EACT_LABEL_SET_TEXT EMSG_DEFINE(LMSG_TP_LABEL,20,1)
// input wchar[] must have the tail UNICODE_NULL and no more than 512 characters
// output none
// result ERESULT_SUCCESS or ERESULT_UNSUCCESSFUL

// 获得文本内容，must be send
#define EACT_LABEL_GET_TEXT EMSG_DEFINE(LMSG_TP_LABEL,21,1)
// intput none
// output wchar[] buffer to receive the text
// result return ERESULT_INSUFFICIENT_RESOURCES if output buffer is short than the text to obtain. 

// 获得文本内容包含的字符总数，不包括结尾的NULL
#define EACT_LABEL_GET_TEXT_LENGTH	EMSG_DEFINE(LMSG_TP_LABEL,21,2)
// input none
// output ULONG
// result ERESULT_SUCCESS

// 设置字体前景颜色
#define EACT_LABEL_SET_FORE_COLOR EMSG_DEFINE(LMSG_TP_LABEL,21,5)
// intput ULONG color in 4 bytes as b,g,r,a
// output none
// result ERESULT_SUCCESS or ERESULT_WRONG_PARAMETERS or ERESULT_UNSUCCESSFUL

// 设置字体背景颜色
#define EACT_LABEL_SET_BACK_COLOR EMSG_DEFINE(LMSG_TP_LABEL,21,6)
// input  ULONG color in 4 bytes as b,g,r,a
// output none
// result ERESULT_SUCCESS or ERESULT_WRONG_PARAMETERS or ERESULT_UNSUCCESSFUL

// 获得字体前景颜色
#define EACT_LABEL_GET_FORE_COLOR EMSG_DEFINE(LMSG_TP_LABEL,21,7)
// input none
// output  ULONG color in 4 bytes as b,g,r,a
// result ERESULT_SUCCESS or ERESULT_WRONG_PARAMETERS or ERESULT_UNSUCCESSFUL

// 获得字体背景颜色
#define EACT_LABEL_GET_BACK_COLOR EMSG_DEFINE(LMSG_TP_LABEL,21,8)
// input none
// output  ULONG color in 4 bytes as b,g,r,a
// result ERESULT_SUCCESS or ERESULT_WRONG_PARAMETERS or ERESULT_UNSUCCESSFUL

// 设置它具有下划线
#define EACT_LABEL_SET_UNDERLINE EMSG_DEFINE(LMSG_TP_LABEL,21,10)
// input LONG, nonzero to set underline with the text,zero to remove the underline
// output none
// result na

// 设置超链接属性
#define EACT_LABEL_SET_HYPERLINK EMSG_DEFINE(LMSG_TP_LABEL,21,11)
// input LONG, nonzero to set the hyperlink attribute,zero to remove the hyperlink attribute
// output none
// result na

// 获得文字排布信息
#define EACT_LABEL_GET_LAYOUT EMSG_DEFINE(LMSG_TP_LABEL,21,15)
// input none
//output 
struct _STCTL_LABEL_LAYOUT{
	D2D1_SIZE_F MaxSize;			// 文字区域最大宽高
	D2D1_RECT_F Background;			// 背景区域
	D2D1_POINT_2F TextPos;			// 文字区域的左上角
	D2D1_SIZE_F TextSize;			// 文字区域的宽高
	LONG VisibleChars;				// 可见字符数，不包括缩短替换符'.'
	LONG Lines;						// 可见行数
};
typedef _STCTL_LABEL_LAYOUT STCTL_LABEL_LAYOUT,* PSTCTL_LABEL_LAYOUT;
// result ERESULT_SUCCESS or ERESULT_WRONG_PARAMETERS or ERESULT_UNSUCCESSFUL


// 设置最宽
#define EACT_LABEL_SET_MAX_WIDTH EMSG_DEFINE(LMSG_TP_LABEL,21,16)
// input LONG
// output none
// result na

// 设置最高
#define EACT_LABEL_SET_MAX_HEIGHT EMSG_DEFINE(LMSG_TP_LABEL,21,17)
// input LONG
// output none
// result na


//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_ANIMATOR L"Animator"	//标签

// 动画控件的触发消息××××××××××××××××××××××××××××××××××××××××××××××

// 准备绘制消息，让父对象有机会对自身设置蒙版，收到此消息后，不要执行其他代码，仅仅做蒙版设置
#define EEVT_ANIMATOR_BEFORE_PAINT EMSG_DEFINE(LMSG_TP_ANIMATOR,1,1)
// input IXuiPaintBoard*
// output none
// result na

// 结束绘制消息，让父对象有机会撤销已设置蒙版，收到此消息后，不要执行其他代码，仅仅做蒙版设置
#define EEVT_ANIMATOR_AFTER_PAINT EMSG_DEFINE(LMSG_TP_ANIMATOR,1,2)
// input IXuiPaintBoard*
// output none
// result na


// 动画控件的设置消息××××××××××××××××××××××××××××××××××××××××××××××

// 更新动画帧, must be send
#define EACT_ANIMATOR_SET_FRAME EMSG_DEFINE(LMSG_TP_ANIMATOR,20,1)
// input 
struct _STCTL_ANIMATOR_FRAME{
	ULONG PixelWidth;	// must be product of 4
	ULONG PixelHeight;
	PBYTE PixelData;	// the format must be RGB24
};
typedef _STCTL_ANIMATOR_FRAME STCTL_ANIMATOR_FRAME,* PSTCTL_ANIMATOR_FRAME;
// output none
// result ERESULT_SUCCESS or ERESULT_UNSUCCESSFUL


// 设置图像变换, 尚未实现
#define EACT_ANIMATOR_SET_TRANSFORM EMSG_DEFINE(LMSG_TP_ANIMATOR,21,1)
// input
struct _STCTL_ANIMATOR_TRANSFORM{
	ULONG Flag;				// 见下面的宏定义，可以是不同Flag的组合
	FLOAT RotationAngle;	// 旋转角度，正数顺时针
	D2D1_POINT_2F RotationCenter;	// 旋转中心
};
typedef _STCTL_ANIMATOR_TRANSFORM STCTL_ANIMATOR_TRANSFORM,* PSTCTL_ANIMATOR_TRANSFORM;
#define EFLAG_ANIMATOR_HORIZONTALFLIP 1 // 左右翻转
#define EFLAG_ANIMATOR_VIRTICALFLIP 2 // 上下翻转
#define EFLAG_ANIMATOR_ROTATION	4		// 设定旋转的角度
#define EFLAG_ANIMATOR_ROTATION_CENTER  8 // 设定旋转的中心 
// output none
// result ERESULT_SUCCESS or ERESULT_UNSUCCESSFUL


//////////////////////////////////////////////////////////////////////////
// 选择框
//////////////////////////////////////////////////////////////////////////

// NOTE:
// 1：选择框的位置和大小，需要父窗口去设置。
// 2：如果父窗口不设置，那么选择框的位置和大小则和父窗口重合。选择框会完全按照父窗口的最小矩形，完全罩住父窗口


#define ES_ETYPE_SELECT L"Select"
#define EMSG_SELECTFRAME_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_SELECT,_MJ_NUM,_MN_NUM)


//////////////////////////////////////////////////////////////////////////
// 以下是移动选择框的消息
//////////////////////////////////////////////////////////////////////////

#define EMSG_SELECTFRAME_BEGIN		EMSG_SELECTFRAME_REQUEST(1,2)		//选择框开始拖动
// input  none
// output none
// result none
#define EMSG_SELECTFRAME_DRAGED		EMSG_SELECTFRAME_REQUEST(1,3)		//选择框拖动完成
// input none
// output none
// result none

#define EMSG_SELECTFRAME_MOVING		EMSG_SELECTFRAME_REQUEST(1,4)		// 移动选择框，将偏移传递到父类
// input D2D1_POINT_2F
// output none
// result none

// 收到键盘消息，父对象如果需要识别处理，请返回ERESULT_KEY_ACCEPTED，返回其他值，选择框将自行解释该键盘消息，目前可能解释的是方向键导致的选择框移动
#define EMSG_SELECTFRAME_KEYBOARD	EMSG_SELECTFRAME_REQUEST(2,1)		
// input STEMS_KEY_PRESSED refer to EMSG_KEY_PRESSED message's definition
// output none
// result na

//////////////////////////////////////////////////////////////////////////
// 以下的移动锚点的消息
//////////////////////////////////////////////////////////////////////////

// NOTE:当正在移动某个锚点时，选择框需要发送给父窗口一个三元组{position偏移量,size变化量, 是否发生翻转},
// 父窗口收到消息后，根据自身逻辑判断是否需要发生翻转，并给选择框对象发送归一化的命令，并且重新设置选择框的位置和大小。

#define EMSG_SELECTPOINT_CHANGE_POSITION_SIZE		EMSG_SELECTFRAME_REQUEST(10,1)	// 移动某个锚点，向父窗口发此消息
// input PSTCTL_CHANGE_POSITION_SIZE
// output none
// result none

#define EMSG_SELECTPOINT_BEGIN	EMSG_SELECTFRAME_REQUEST(10,5)		//开始移动某一个锚点
// input none
// output none
// result none

#define EMSG_SELECTFPOINT_MOVED	 EMSG_SELECTFRAME_REQUEST(10,10)		//某个锚点移动完成
// input none
// output none
// result none

#define EMSG_SELECTFRAME_REGULAR		EMSG_SELECTFRAME_REQUEST(10,12)		// 父窗口给子窗口发送归一化命令
// input none
// output none
// result none


#define EMSG_SET_PROPORTIONAL_SCALING		EMSG_SELECTFRAME_REQUEST(10,15)		// 父窗口通知子窗口，需要强制等比缩放
// input bool，true表示需要等比缩放，false表示不需要
// output none
// result none

#define EMSG_SET_EDIT_STATUS				EMSG_SELECTFRAME_REQUEST(10,16)		// 表示是否允许编辑
// input bool，true表示允许编辑，false表示不允许
// output none
// result none


enum XuiSelectFrameActivePoint{
	XuiLeftTop = 1,		// 左上点
	XuiMidTop = 2,		// 中上点
	XuiRightTop = 3,		// 右上点
	XuiRightMid = 4,		// 右中点
	XuiRightBottom = 5,	// 右下点
	XuiMidBottom = 6,	// 中下点
	XuiLeftBottom = 7,	// 左下点
	XuiLeftMid = 8		// 左中点
};

typedef  struct _STCTL_CHANGE_POSITION_SIZE{
	D2D1_POINT_2F	mdPositionOffset;	// 位置偏移量
	D2D1_SIZE_F		mdSizeVariation;	// 大小变化量
	bool			mcHTurn;			// 判断是否水平翻转
	bool			mcVTurn;			// 判断是否垂直翻转
}STCTL_CHANGE_POSITION_SIZE,*PSTCTL_CHANGE_POSITION_SIZE;


//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_MENUITEM L"MenuItem"	// 菜单项

#define EMSG_MENUITEM_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_MENUITEMN,_MJ_NUM,_MN_NUM)

// 菜单项触发的消息，会直接发送给PopupMenu，PopupMenu、MenuButton、MenuBar等收到这个消息都会直接往父对象发送
// ××××××××××××××××××××××××××××××××××××××××××××××

// 菜单项被点击
#define EEVT_MENUITEM_CLICK						EMSG_MENUITEM_REQUEST(1,1)
// input LONG, 被点击的菜单项的ID

// 菜单项获取焦点
#define EEVT_MENUITEM_GET_FOCUS					EMSG_MENUITEM_REQUEST(1,2)
// input Iterator** 获取焦点的菜单项

// 菜单项鼠标悬停
#define EEVT_MENUITEM_MOUSE_HOVER				EMSG_MENUITEM_REQUEST(1,3)
// input STCTL_MENUITEM_MOUSE_HOVER
// output none
// result none

typedef struct _STCTL_MENUITEM_MOUSE_HOVER{
	UINT CommandID;
	IEinkuiIterator* MenuItemIter;
}STCTL_MENUITEM_MOUSE_HOVER,*PSTCTL_MENUITEM_MOUSE_HOVER;


// 菜单项访问消息
// 设置菜单项的Check状态
#define EACT_MENUITEM_SET_CHECK_STATE			EMSG_MENUITEM_REQUEST(2,1)
// input bool 是否设置Check状态

// 更改菜单项文本
#define EACT_MENUITEM_CHANGE_TEXT				EMSG_MENUITEM_REQUEST(2,2)
// input wchar_t*			新的菜单文本

// 更改热键(必须确保菜单项有文本）
#define EACT_MENUITEM_CHANGE_HOTKEY				EMSG_MENUITEM_REQUEST(2,3)
// PSTCTL_MENUITEM_HOTKEY

struct _STCTL_MENUITEM_HOTKEY
{
	wchar_t HotKeyToShow[MAX_PATH];		// 用来显示的热键文字
	ULONG VirtualKey;					// 虚拟键值
	bool NeedShift;						// 是否需要Shift
	bool NeedCtrl;						// 是否需要Ctrl
	bool NeedAlt;						// 是否需要Alt
};

typedef _STCTL_MENUITEM_HOTKEY STCTL_MENUITEM_HOTKEY, *PSTCTL_MENUITEM_HOTKEY;





//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_POPUPMENU L"PopupMenu"	

#define EMSG_POPUPMENU_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_POPUPMENU,_MJ_NUM,_MN_NUM)

// 弹出菜单访问消息
// ××××××××××××××××××××××××××××××××××××××××××××××

// 增加新的菜单项到指定位置
#define EACT_POPUPMENU_INSERT_MENUITEM_BY_CREATE								EMSG_POPUPMENU_REQUEST(1,1)
// input STCTL_POPUPMENU_MENUITEMINFO
struct _STCTL_POPUPMENU_MENUITEMINFO{
	LONG UniqueMenuItemId;				// MenuItem的全局唯一ID
	int Index;							// 菜单项的位置索引（-1表示插值末尾）
	LONG MenuItemId;					// MenuItem在PopupMenu下的ID，为0表示任意ID
	UINT Type;							// 菜单项的类型，参照MenuItem枚举的类型，目前仅支持1类型
	wchar_t MenuText[MAX_PATH];			// 菜单项显示的文字
	PSTCTL_MENUITEM_HOTKEY* HotKeyInfo;	// 热键信息
};
typedef _STCTL_POPUPMENU_MENUITEMINFO STCTL_POPUPMENU_MENUITEMINFO,* PSTCTL_POPUPMENU_MENUITEMINFO;


// 插入菜单项到指定位置
#define EACT_POPUPMENU_INSERT_MENUITEM_BY_EXIST								EMSG_POPUPMENU_REQUEST(1,2)
// input PSTCTL_POPUPMENU_MENUITEMINSERT

struct _STCTL_POPUPMENU_MENUITEMINSERT{
	IEinkuiIterator* MenuItem;
	int Index;							// 要插入的索引位置
};
typedef _STCTL_POPUPMENU_MENUITEMINSERT STCTL_POPUPMENU_MENUITEMINSERT,* PSTCTL_POPUPMENU_MENUITEMINSERT;


// 删除指定CommandID的菜单项
#define EACT_POPUPMENU_DELETE_MENUITEM_BY_COMMANDID					EMSG_POPUPMENU_REQUEST(1,4)
// input LONG commandid

// 删除指定索引的菜单项，索引传入-1表示删除全部菜单项
#define EACT_POPUPMENU_DELETE_MENUITEM_BY_INDEX						EMSG_POPUPMENU_REQUEST(1,5)
// input LONG index

// 获取指定CommandID的菜单项迭代器
#define EACT_POPUPMENU_GET_MENUITEM_BY_COMMANDID					EMSG_POPUPMENU_REQUEST(1,6)
// input LONG commandId
// output Iterator*

// 获取指定索引的菜单项的迭代器
#define EACT_POPUPMENU_GET_MENUITEM_BY_INDEX						EMSG_POPUPMENU_REQUEST(1,7)
// input  LONG index
// output Iterator* 

// 要求重新布局菜单项
#define EACT_POPUPMENU_RELAYOUT_MENUITEM							EMSG_POPUPMENU_REQUEST(1,8)


// 设置显示的时候不需要去管理菜单项的启用/禁用。 用菜单项自身去设置
#define EACT_POPUPMENU_IS_MANAGER_MENUITEM_ENABLE					EMSG_POPUPMENU_REQUEST(1, 9)
// input bool 

// 弹出菜单触发的消息，
// ××××××××××××××××××××××××××××××××××××××××××××××



//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_MENUBUTTON L"MenuButton"		// 菜单按钮

#define EMSG_MENUBUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_MENUBUTTON,_MJ_NUM,_MN_NUM)

// 菜单按钮触发的消息，会直接发送给父窗口
// ××××××××××××××××××××××××××××××××××××××××××××××

// 菜单按钮被点击
#define EEVT_MENUBUTTON_CLICK					EMSG_MENUBUTTON_REQUEST(1,1)
// input bool 当前点击的菜单按钮是否弹出子菜单


// 菜单按钮访问消息
// ××××××××××××××××××××××××××××××××××××××××××××××
// 设置子菜单可视状态
#define EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE		EMSG_MENUBUTTON_REQUEST(2,1)
// input bool 可视状态

// 插入菜单项到PopupMenu（包括子MenuItem的PopupMenu)
#define EEVT_MENUBUTTON_INSERT_MENUITEM			EMSG_MENUBUTTON_REQUEST(2,2)
// input STCTL_MENUBUTTON_INSERT_MENUITEM
struct _STCTL_MENUBUTTON_INSERT_MENUITEM{
	LONG UniquePopupMenuId;				// PopupMenu的全局唯一ID
	STCTL_POPUPMENU_MENUITEMINFO PopupMenuInfo;
};
typedef struct _STCTL_MENUBUTTON_INSERT_MENUITEM STCTL_MENUBUTTON_INSERT_MENUITEM,* PSTCTL_MENUBUTTON_INSERT_MENUITEM;





//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_MENUBAR L"MenuBar"		// 菜单按钮

#define EMSG_MENUBAR_REQUEST(_MJ_NUM,_MN_NUM)	EMSG_DEFINE(LMSG_TP_MENUBAR,_MJ_NUM,_MN_NUM)

// 菜单条访问消息
// ××××××××××××××××××××××××××××××××××××××××××××××

// 询问当前是否有其他菜单按钮被展开
#define EACT_MENUBAR_ANY_SUBMENU_VISIBLE		EMSG_MENUBAR_REQUEST(1,1)
// output  bool, ture表示有其他按钮被展开，false表示没有。

// 设置隐藏最后一次弹出的子菜单
#define EACT_MENUBAR_HIDE_LAST_SUBMENU			EMSG_MENUBAR_REQUEST(1,2)


// 要求创建新的MenuItem插入到指定的PopupMenu中
#define EACT_MENUBAR_INSERT_NEW_MENUITEM		EMSG_MENUBAR_REQUEST(1,3)
// input _STCTL_MENUBAR_INSERT_MENUITEM
// 返回值 ERESULT_SUCCESS 表示成功，否则表示失败

struct _STCTL_MENUBAR_INSERT_MENUITEM{
	UINT MenuButtonID;				// 要插入的MenuButtonID
	STCTL_MENUBUTTON_INSERT_MENUITEM MenuButtonInfo;
};
typedef _STCTL_MENUBAR_INSERT_MENUITEM STCTL_MENUBAR_INSERT_MENUITEM,* PSTCTL_MENUBAR_INSERT_MENUITEM;


// 获取指定CommandID的弹出菜单迭代器，成功返回SUCCESS，否则失败
#define EACT_MENUBAR_GET_POPUPMENU_BY_COMMANDID	EMSG_MENUBAR_REQUEST(1, 4)
// long commandid
// output Iterator* 





//////////////////////////////////////////////////////////////////////////

#define ES_ETYPE_IMAGEBUTTON	L"ImageButton"			//ImageButton元素

//定义ImageButton的样式 
#define	IB_STYLE_NORMAL				0		//普通风格
#define	IB_STYLE_KEEP_CHECKED		1		//保持选中按下风格

//定义ImageButton的子控件ID，其他控件必须大于等于 IB_ID_CTRL_OTHERS
#define IB_ID_CTRL_LEFT_PICTURE				1
#define IB_ID_CTRL_RIGHT_PICTURE			2

#define IB_ID_CTRL_OTHERS					3	

//按钮控件会触发的消息
#define EMSG_IMAGEBUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_IMAGEBUTTON,_MJ_NUM,_MN_NUM)

//按钮被按下
#define EEVT_IMAGEBUTTON_PRESSED		EMSG_IMAGEBUTTON_REQUEST(1,1)
//Input			TEMS_MSG_BASE 结构体
//Output		None
//Result

//按钮被释放
#define EEVT_IMAGEBUTTON_RELEASE		EMSG_IMAGEBUTTON_REQUEST(1,2)
//Input			TEMS_MSG_BASE 结构体
//Output		None
//Result

//按钮被单击
#define EEVT_IMAGEBUTTON_CLICK			EMSG_IMAGEBUTTON_REQUEST(1,3)
//Input			None
//Output		None
//Result

//按钮被单击，进入按下状态
#define EEVT_IMAGEBUTTON_CHECKED		EMSG_IMAGEBUTTON_REQUEST(1,4)
//Input			None
//Output		None
//Result

//按钮被单击，取消按下状态
#define EEVT_IMAGEBUTTON_UNCHECKED		EMSG_IMAGEBUTTON_REQUEST(1,5)
//Input			None
//Output		None
//Result

//改变颜色选择按钮的颜色值（只对颜色选择按钮有效）
#define EACT_IMAGEBUTTON_SET_COLOR		EMSG_IMAGEBUTTON_REQUEST(1,6)
//Input			ULONG （颜色值）要绘制的颜色，如果A通道为0，则不绘制 0xARGB 
//Output		None
//Result

//改变左图的背景图（相对路径）
#define EACT_IMAGEBUTTON_CHANGE_LEFT_IMAGE_BKG		EMSG_IMAGEBUTTON_REQUEST(1,7)
//Input			wchat_t* 图片相对于DLL所在目录的路径，现在只能更换为只有1帧的图
//Output		NONE
//Result

//改变左图的背景图（全路径）
#define EACT_IMAGEBUTTON_CHANGE_LEFT_IMAGE_BKG_FULL_PATH		EMSG_IMAGEBUTTON_REQUEST(1,8)
//Input			wchat_t* 图片全路径，现在只能更换为只有1帧的图
//Output		NONE
//Result

//设置按钮check状态
#define EACT_IMAGEBUTTON_SET_CHECKED							EMSG_IMAGEBUTTON_REQUEST(1,9)
//Input			bool
//Output		None
//Result

//设置按钮的缩放比例，防止按钮变形
#define EACT_IMAGEBUTTON_SET_RATIO								EMSG_IMAGEBUTTON_REQUEST(1,10)
//Input			Float （比例）
//Output		NONE
//Result

//设置按钮子菜单某项被选中
#define EACT_IMAGEBUTTON_SET_ITEM_SELECTED						EMSG_IMAGEBUTTON_REQUEST(1,11)
//Input			int （项ID， 从1开始）
//Output		NONE
//Result

//设置颜色按钮绘制矩形状态的开启或关闭
#define EACT_IMAGEBUTTON_DRAW_SHAPE						EMSG_IMAGEBUTTON_REQUEST(1,12)
//Input			bool （true: 绘制自定义形状 false：不绘制）
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////

#define ES_ETYPE_COMBOBOX L"ComboBox"			//ComboBox元素

//定义ComboBox的样式 
#define	COMBO_STYLE_EDIT		0	//带有编辑框的组合框
#define	COMBO_STYLE_STATIC		1	//带有静态文本控件的组合框

//定义ComboBox的子控件ID，其他控件必须大于等于 COMBO_ID_CTRL_OTHERS
#define COMBO_ID_CTRL_UPPER_PICTURE				1
#define COMBO_ID_CTRL_UNDER_PICTURE				2
#define COMBO_ID_CTRL_CURRENT_ITEM_EDIT			3
#define COMBO_ID_CTRL_CURRENT_ITEM_BUTTON		4
#define COMBO_ID_CTRL_DROP_DOWN_BUTTON			5
#define COMBO_ID_CTRL_LIST						6

#define COMBO_ID_CTRL_OTHERS					7	


#define EMSG_COMBOBOX_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_COMBO, _MJ_NUM, _MN_NUM)

// 添加选项
#define EACT_COMBOBOX_ADD_ITEM						EMSG_COMBOBOX_REQUEST(1,1)
//Input			wchar_t*  选项字符串
//Output		None
//Result


//	下拉列表项被点击
#define EEVT_COMBOBOX_LIST_ITEM_CLICK				EMSG_COMBOBOX_REQUEST(1,2)
//Input			int (ID值，配置文件中指定）
//Output		None
//Result

//	下拉列表项被点击
#define EEVT_COMBOBOX_LIST_ITEM_CLICK_COMPLEX			EMSG_COMBOBOX_REQUEST(1,3)
//Input			int (ID值，配置文件中指定）
//Output		None
//Result

// 某项被点击时，发送被选中项的文本内容			// add by colin
#define EEVT_COMBOBOX_ITEM_CLICK_WITH_TEXT			EMSG_COMBOBOX_REQUEST(1,4)
// Input wchar_t*
// output none
// result

// 某项被点击时，发送被选中项的索引				// add by colin
#define EEVT_COMBOBOX_ITEM_CLICK_WITH_INDEX			EMSG_COMBOBOX_REQUEST(1,5)
// Input ULONG*
// output none
// result

//定义ToolBar消息结构
enum COMBOBOX_MSG_PARA_TYPE
{
	COMBOBOX_TMPT_NONE,
	COMBOBOX_TMPT_BOOL,
	COMBOBOX_TMPT_INT,
	COMBOBOX_TMPT_FLOAT,
	COMBOBOX_TMPT_STRING,
	COMBOBOX_TMPT_OTHERS			//Never Use
};
struct COMBOBOX_MSG
{
	unsigned int				mnCtrlID;			//ToolBar子控件 ID
	COMBOBOX_MSG_PARA_TYPE		mnMsgParaType;		//消息类型标识
	void*						mpMsgBuf;			//消息携带的数据缓冲区

	COMBOBOX_MSG() : mnCtrlID(0x00000000), mnMsgParaType(COMBOBOX_TMPT_OTHERS), mpMsgBuf(0) {}
	~COMBOBOX_MSG() { mpMsgBuf = 0;}
};

//设置按钮子菜单某项被选中
#define EACT_COMBOBOX_SET_ITEM_SELECTED						EMSG_COMBOBOX_REQUEST(2,4)
//Input			int （项ID）
//Output		NONE
//Result

//设置下拉菜单某一项有效
#define EACT_COMBOBOX_SET_ENABLE							EMSG_COMBOBOX_REQUEST(2,5)
//Input			int （项ID）
//Output		NONE
//Result

//设置下拉菜单某一项无效
#define EACT_COMBOBOX_SET_DISABLE							EMSG_COMBOBOX_REQUEST(2,6)
//Input			int （项ID）
//Output		NONE
//Result

// 按索引删除项 ,传入负数表示删除全部 add by colin
#define EACT_COMBOBOX_DELETE_ITEM_BY_INDEX			EMSG_COMBOBOX_REQUEST(2,7)
//Input			int 索引
//Output		None
//Result

// 获取当前选择项显示文本 add by colin
#define EACT_COMBOBOX_GET_CURRENT_ITEM_TEXT			EMSG_COMBOBOX_REQUEST(2,8)
// input none
//Output		wchat_t** 返回的字符串指针不应该长期持有，无需释放
// result return ERESULT_INSUFFICIENT_RESOURCES if output buffer is short than the text to obtain. 

// 根据索引设置某项被选中 add by colin
#define EACT_COMBOBOX_SET_ITEM_SELECTED_BY_INDEX						EMSG_COMBOBOX_REQUEST(2,9)
//Input			ULONG （index)
//Output		NONE
//Result

//////////////////////////////////////////////////////////////////////////
//BiSliderBar消息
#define EMSG_BI_SLIDERBAR_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_BI_SLIDERBAR, _MJ_NUM, _MN_NUM)
struct _sBiSliderBarStruct
{
	float mfLeftPos;
	float mfRightPos;
	float mfMidPos;
};
//SLiderBar
#define EACT_BISLIDERBAR_THUMB_POSITION						EMSG_BI_SLIDERBAR_REQUEST(1,1)
//Input			NULL
//Output		_sBiSliderBarStruct
//Result

//SLiderBar
#define EACT_BISLIDERBAR_SET_POS						EMSG_BI_SLIDERBAR_REQUEST(1,2)
//Input			_sBiSliderBarStruct
//Output		NULL
//Result

//SLiderBar
#define EACT_BISLIDERBAR_GET_POS						EMSG_BI_SLIDERBAR_REQUEST(1,3)
//Input			NULL
//Output		_sBiSliderBarStruct
//Result

//SLiderBar
#define EACT_BISLIDERBAR_DRAG_START						EMSG_BI_SLIDERBAR_REQUEST(1,4)
//Input			_sBiSliderBarStruct
//Output		NULL
//Result

//SLiderBar
#define EACT_BISLIDERBAR_DRAG_END						EMSG_BI_SLIDERBAR_REQUEST(1,5)
//Input			_sBiSliderBarStruct
//Output		NULL
//Result
//SLiderBar
#define EACT_BISLIDERBAR_SET_RANGE						EMSG_BI_SLIDERBAR_REQUEST(1,6)
//Input			_sBiSliderBarStruct
//Output		NULL
//Result

#define EACT_BISLIDERBAR_SET_MIDLABEL_LEGTH						EMSG_BI_SLIDERBAR_REQUEST(1,7)
//Input			FLOAT
//Output		NULL
//Result

#define EACT_BISLIDERBAR_SET_MIDLABEL_VALUE						EMSG_BI_SLIDERBAR_REQUEST(1,8)
//Input			whcar_t*
//Output		NULL
//Result
//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_TOOLBAR	L"ToolBar"			// 工具栏

#define EMSG_TOOLBARITEM_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_TOOLBAR, _MJ_NUM, _MN_NUM)


// 工具栏项或其子项被点击
#define EEVT_TOOLBARITEM_CLICK					EMSG_TOOLBARITEM_REQUEST(1,1)
// input TOOLBAR_MSG
//定义ToolBar消息结构
enum TOOLBAR_MSG_PARA_TYPE
{
	TMPT_NONE,
	TMPT_BOOL,
	TMPT_INT,
	TMPT_FLOAT,
	TMPT_STRING,
	TMPT_HEX,

	TMPT_OTHERS			//Never Use
};
struct TOOLBAR_MSG
{
	unsigned int				mnCtrlID;			//ToolBar子控件 ID
	TOOLBAR_MSG_PARA_TYPE		mnMsgParaType;		//消息类型标识
	//void*						mpMsgBuf;			//消息携带的数据缓冲区
	union {
		bool mbBool;
		wchar_t mswString[MAX_PATH];
		LONG mlInterge;
		FLOAT mfFloat;
		ULONG muColor;
	};

	TOOLBAR_MSG() : mnCtrlID(0x00000000), mnMsgParaType(TMPT_OTHERS)/*, mpMsgBuf(0) */{}
	//~TOOLBAR_MSG() { mpMsgBuf = 0;}
};


//	更新界面消息
#define EEVT_UPDATE_UI							EMSG_TOOLBARITEM_REQUEST(1,2)
// input  none
// output none
// result none

//	更新Pane
#define EEVT_UPDATE_PANE						EMSG_TOOLBARITEM_REQUEST(1,4)
//Input			int (Pane ID值）
//Output		None
//Result

//	通知Pane子元素更新数据界面
#define EEVT_PANE_ITEM_SET_VALUE				EMSG_TOOLBARITEM_REQUEST(1,5)
//Input			None
//Output		None
//Result


//	获取子项UnificSetting ID
#define EEVT_GET_UNIFIC_SETTING_ID				EMSG_TOOLBARITEM_REQUEST(1,6)
//Input		None
//Output	int
//Result

//////////////////////////////////////////////////////////////////////////
#define ES_ETYPE_WHIRLANGLE L"WhirlAngle"		// 旋转角度

#define EMSG_WHIRLANGLE_REQUEST(_MJ_NUM,_MN_NUM)	EMSG_DEFINE(LMSG_TP_WHIRLANGLE,_MJ_NUM,_MN_NUM)

// 访问消息
// ××××××××××××××××××××××××××××××××××××××××××××××

// 获取当前角度
#define EACT_WHIRLANGLE_GET_ANGLE				EMSG_WHIRLANGLE_REQUEST(1,1)
// output  double

// 设置当前角度
#define EACT_WHIRLANGLE_SET_ANGLE				EMSG_WHIRLANGLE_REQUEST(1,2)
// input  double


// 触发消息
// ××××××××××××××××××××××××××××××××××××××××××××××
// 角度发生变化开始
#define EEVT_WHIRLANGEL_ANGLE_CHANGE_BEGIN		EMSG_WHIRLANGLE_REQUEST(2,1)

// 角度发生变化
#define EEVT_WHIRLANGEL_ANGLE_CHANGE			EMSG_WHIRLANGLE_REQUEST(2,2)
// input double 当前角度

// 角度发生变化结束
#define EEVT_WHIRLANGEL_ANGLE_CHANGE_END		EMSG_WHIRLANGLE_REQUEST(2,3)

//////////////////////////////////////////////////////////////////////////

#define ES_ETYPE_SPINBUTTON L"SpinButton"			//SpinButton元素

//定义SpinButton的子控件ID，其他控件必须大于等于 SB_ID_CTRL_OTHERS
#define SB_ID_CTRL_BACKGROUND				1
#define SB_ID_CTRL_EDIT						2
#define SB_ID_CTRL_BUTTON_UP				3
#define SB_ID_CTRL_BUTTON_DOWN				4

#define SB_ID_CTRL_OTHERS					5	

#define EMSG_SPINBUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_SPINBUTTON, _MJ_NUM, _MN_NUM)

//获取当前值
#define EACT_SPINBUTTON_GET_CURRENT_VALUE			EMSG_SPINBUTTON_REQUEST(1,1)
//Input		None
//Output	int*
//Result

//设置当前值
#define EACT_SPINBUTTON_SET_CURRENT_VALUE			EMSG_SPINBUTTON_REQUEST(1,2)
//Input		int*  
//Output	None
//Result

// 设置最小值
#define EACT_SPINBUTTON_SET_MIN_VALUE				EMSG_SPINBUTTON_REQUEST(1,3)
//Input		int*  
//Output	None
//Result

// 设置最大值
#define EACT_SPINBUTTON_SET_MAX_VALUE				EMSG_SPINBUTTON_REQUEST(1,4)
//Input		int*  
//Output	None
//Result


//编辑框内容被修改
#define EEVT_SPINBUTTON_CONTENT_MODIFING			EMSG_SPINBUTTON_REQUEST(2,1)
// input none
// output none
// result na

// 内容修改完成，输入时按下"Enter"键，或者移除键盘焦点，都将导致内容完成消息被发送
// 应届收到"设置内容"消息而导致的内容修改，不会发送这条消息
#define EEVT_SPINBUTTON_CONTENT_COMPLETION			EMSG_SPINBUTTON_REQUEST(2,2)
// input 当前值
// output none
// result na
//////////////////////////////////////////////////////////////////////////

#define ES_ETYPE_TIMESPINBUTTON L"TimeSpinButton"			//TimeSpinButton元素


#define EMSG_TIMESPINBUTTON_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(LMSG_TP_TIMESPINBUTTON, _MJ_NUM, _MN_NUM)

//定义SpinButton的子控件ID，其他控件必须大于等于 SB_ID_CTRL_OTHERS
#define TSB_ID_CTRL_EDIT_MINUTES			1
#define TSB_ID_CTRL_EDIT_SECONDS			2
#define TSB_ID_CTRL_EDIT_MILLISECONDS		3
#define TSB_ID_CTRL_BUTTON_UP				4
#define TSB_ID_CTRL_BUTTON_DOWN				5
#define TSB_ID_CTRL_LABEL_COLON_ONE			6
#define TSB_ID_CTRL_LABEL_COLON_TWO			7

#define TSB_ID_CTRL_OTHERS					10	


struct XuiTimeFormat
{
	//int nHours;		//00 - 23
	int nMinutes;		//分		00 - 59
	int nSeconds;		//秒		00 - 59
	int nMillisecond;	//毫秒	000 - 999 

	XuiTimeFormat() : nMinutes(0), nSeconds(0), nMillisecond(0) {}
};

//设置控件当前时间
#define EACT_TIMESPINBUTTON_SET_TIME			EMSG_TIMESPINBUTTON_REQUEST(1,1)
//Input		XuiTimeFormat 
//Output	None

//获取控件当前时间 (send)
#define EACT_TIMESPINBUTTON_GET_TIME			EMSG_TIMESPINBUTTON_REQUEST(1,2)
// intput none
// output XuiTimeFormat* buffer to receive the time

//通知父窗口时间被修改
#define EEVT_TIMESPINBUTTON_TIME_MODIFIED			EMSG_TIMESPINBUTTON_REQUEST(1,3)
//Input		XuiTimeFormat 
//Output	None

//////////////////////////////////////////////////////////////////////////


#pragma pack()



#endif//_ECTL_H_