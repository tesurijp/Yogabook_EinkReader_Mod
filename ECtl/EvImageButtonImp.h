/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef		__EVIMAGEBUTTONIMP_H__
#define		__EVIMAGEBUTTONIMP_H__

#include "EvPictureFrameImp.h"
#include "cmmstruct.h"
//#include "CommandIDDef.h"



#define BUTTON_FRAME_ARRAY_MAX 20		//每种状态的帧信息数组的最大成员个数

DECLARE_BUILTIN_NAME(ImageButton)

class CEvImageButton:
	public CXuiElement<CEvImageButton, GET_BUILTIN_NAME(ImageButton)>
{
	friend CXuiElement<CEvImageButton, GET_BUILTIN_NAME(ImageButton)>;

public:

	//enum	//存放帧数数组的对应下标
	//{
	//	ARRAY_INDEX_DISABLE = 0,			//禁用态
	//	ARRAY_INDEX_NORMAL,					//普通态
	//	ARRAY_INDEX_FOCUS,					//焦点态
	//	ARRAY_INDEX_PRESSED,				//按下态
	//	ARRAY_INDEX_CHECKED_DISABLE,		//Checked态的禁用态
	//	ARRAY_INDEX_CHECKED_NORMAL,			//Checked态的普通态
	//	ARRAY_INDEX_CHECKED_FOCUS,			//Checked态的拥有焦点态
	//	ARRAY_INDEX_CHECKED_PRESSED,		//Checked态的按下态
	//	ARRAY_INDEX_OTHER					//其它动画
	//};

	////每种状态的帧信息
	//struct ImageButtonFrame
	//{
	//	LONG Index;		//图片上的第几帧
	//	LONG Count;		//一共有几帧
	//};


	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvImageButton();
	// 用于释放成员对象
	virtual ~CEvImageButton();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	BOOL SetChildCtrlPara();			//装载配置资源，设置子控件

	void SetChecked(bool nbChecked);

	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	bool SetItemSelected(int nID);

public:

	struct ExpandMenuItem
	{
		int			id;
		wchar_t		pPicPath[MAX_PATH];
	};


private:

	//IUnificSetting* mpUnificSetting;

private:

	//	读取UnificSetting数据，重置行为
	bool SetValue();

	bool SetImageButtonEnable(bool nbEnable);

private:

	CEvPictureFrame*	mpLeftPicture;
	CEvPictureFrame*	mpRightPicture;
		
	IEinkuiBitmap*			mpBitmapSelectOrOver;		//选中或者滑过的背景
	bool				mbIsMouseFocus;				//是否拥有鼠标焦点
	bool				mbIsKeyboardFocus;			//是否拥有键盘焦点
	bool				mbIsPressed;				//是否被按下
	

	int					mnStyle;					//ImageButton风格  0:uncheckable   1:checkable 

	bool				mbHasColorFlag;				//颜色按钮标记
	ULONG				mluRGBColor;					//颜色值

	//扩展子控件属性
	bool				mbExpandable;				//支持扩展子控件
	IEinkuiIterator*		mpIterExpandCtrl;			//扩展子控件
	bool				mbShowExpandCtrl;			//扩展子控件显示或隐藏
	cmmVector<ExpandMenuItem>	mpVecExpandMenuItem;		//扩展菜单信息列表


	//控件消息
	TOOLBAR_MSG			mMsgInfo;
	//消息携带的数据
	bool				mbChecked;						//是否被选中
	int					mnSubItemID;					//子项ID
	double				mdValue;						//double值
	wchar_t*			pszValue;						//string值


	//其他
	float				mfRadio;

	bool				mbDrawShape;


	//ImageButtonFrame	mdArrayFrame[BUTTON_FRAME_ARRAY_MAX];	//存放每种状态的帧信息
	//LONG				mlCurrentPage;							//当前播放到第几帧了
	//LONG				mlPageCountMax;							//最大帧数
};

#endif		// __EVIMAGEBUTTONIMP_H__