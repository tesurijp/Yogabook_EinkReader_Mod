#ifndef _EVBUTTONIMP_H_
#define _EVBUTTONIMP_H_



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvButton将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。

#define BUTTON_TIMER_ID_PAGE 1		//播放帧动画用的定时器ID


#define  MOUSE_LB(_X) ((_X&MK_LBUTTON)!=0)
#define  MOUSE_RB(_X) ((_X&MK_RBUTTON)!=0)
#define  MOUSE_MB(_X) ((_X&MK_MBUTTON)!=0)
#define  MOUSE_SHIFT(_X) ((_X&MK_SHIFT)!=0)
#define  MOUSE_CONTROL(_X) ((_X&MK_CONTROL)!=0)
#define  MOUSE_X1(_X) ((_X&MK_XBUTTON1)!=0)
#define  MOUSE_X2(_X) ((_X&MK_XBUTTON2)!=0)

#define BUTTON_FRAME_ARRAY_MAX 20  //每种状态的帧信息数组的最大成员个数

DECLARE_BUILTIN_NAME(Button)
class CEvButton :
	public CXuiElement<CEvButton, GET_BUILTIN_NAME(Button)>
{
	friend CXuiElement<CEvButton, GET_BUILTIN_NAME(Button)>;
public:
	enum	//存放帧数数组的对应下标
	{
		ARRAY_INDEX_DISABLE = 0,			//禁用态
		ARRAY_INDEX_NORMAL,					//普通态
		ARRAY_INDEX_FOCUS,					//焦点态
		ARRAY_INDEX_PRESSED,				//按下态
		ARRAY_INDEX_CHECKED_DISABLE,		//Checked态的禁用态
		ARRAY_INDEX_CHECKED_NORMAL,			//Checked态的普通态
		ARRAY_INDEX_CHECKED_FOCUS,			//Checked态的拥有焦点态
		ARRAY_INDEX_CHECKED_PRESSED,		//Checked态的按下态
		ARRAY_INDEX_OTHER					//其它动画
	}BUTTON_ARRAY_INDEX;

	//每种状态的帧信息
	typedef struct _XuiButtonFrame
	{
		LONG Index;		//图片上的第几帧
		LONG Count;		//一共有几帧
	}XuiButtonFrame;

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvButton();
	// 用于释放成员对象
	virtual ~CEvButton();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
	//设置Check状态
	virtual ERESULT SetChecked(bool nbIsChecked = false);
	//判断是否处于Check状态
	virtual bool IsChecked();
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
	);
	//指定播放第几帧动画
	virtual ERESULT OnPlayAnimation(LONG nlIndex = 0);

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//改变按钮文字
	virtual ERESULT OnChangeText(wchar_t* npswText);

	//更换显示图片
	virtual ERESULT OnChangeBackGround(wchar_t* npswPicPath = NULL, bool nbIsFullPath = false);
	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//获取当前状态所在的数组下标
	virtual LONG GetCurrentStatesArrayIndex();
	//装载配置资源
	virtual ERESULT LoadResource();
	//定位文字图片显示位置
	virtual void RelocateText(void);
protected:
	IEinkuiBitmap* mpTextBitmap;		 //显示文字转换成的图片
	XuiButtonFrame mdArrayFrame[BUTTON_FRAME_ARRAY_MAX];	 //存放每种状态的帧信息
	LONG mlCurrentPage;				 //当前播放到第几帧了
	D2D1_SIZE_F mdFrameSize;		 //每帧的真实尺寸
	D2D1_SIZE_F mdAcionSize;		 //感应区大小,0表示为背景图大小
	bool mbIsPressed;				 //是否被按下
	D2D1_RECT_F mdTextDestRect;		 //绘制文字时的目标Rect
	LONG mlAlignType;				 //文字对齐方式（1、左对齐 2、居中对齐 3、右对齐）
	FLOAT mfTextLeft;				 //文字左边距
	FLOAT mfTextRight;				 //文字右边距
	FLOAT mfTextTop;				 //文字左边距
	FLOAT mfTextBottom;				 //文字右边距
private:
	wchar_t* mpszButtonText;		 //按钮显示文字
	wchar_t* mpszNormolTip;
	wchar_t* mpszCheckedTip;
	bool mbIsMouseFocus;			 //是否拥有鼠标焦点
	bool mbIsKeyboardFocus;			 //是否拥有键盘焦点
	bool mbIsChecked;				 //是否处于Check状态
	bool mbIsCheckEnable;			 //是否允许Check状态
	bool mbShrinkOnPressed;			 //按下状态下是否向右下移动
	LONG mlFontSize;				 //字体大小
	LONG mlPageCountMax;			 //最大帧数
	bool mbIsOther;					 //是否是播放其它动画
	LONG mlOtherIndex;				 //播放的是哪一个其它动画
	DWORD mdwColor;				     //文字颜色
	DWORD mdwDisabledColor;			 // 禁用时文字的颜色
	DWORD mdwFontSize;				 //字体大小
	wchar_t* mpswFontName;			 //字体名称

	bool mbIsPlayTimer;				 //播放动画定时器是否开启



	//重新生成文字图片
	bool ReCreateTextBmp();
	//开启或关闭动画定时器
	void StartPlayTimer();
};




#define TF_ID_BT_TEXT L"Text"							//按钮文字
#define TF_ID_BT_FRAME L"FrameCount"					//四态按钮每态的帧数
#define TF_ID_BT_CHECKED_FRAME L"CheckFrameCount"		//Checked按钮每态的帧数
#define TF_ID_BT_OTHER_FRAME L"Other"		//其它自定义动画的帧数
#define TF_ID_BT_NORMAL_TIP L"Tip1"			//普通状态提示
#define TF_ID_BT_CHECKED_TIP L"Tip2"		//Checked状态提示
#define TF_ID_BT_COLOR L"TextColor"			//文字颜色
#define TF_ID_BT_DISABLED_COLOR L"TextDisabledColor"			//文字禁用时的颜色
#define TF_ID_BT_FONT L"FontName"			//字体
#define TF_ID_BT_FONT_SIZE L"FontSize"		//字号
#define TF_ID_BT_ALIGN_TYPE L"TextAlign"		//文字对齐方式
#define TF_ID_BT_TEXT_LEFT L"TextLeft"		//文字左边距
#define TF_ID_BT_TEXT_RIGHT L"TextRight"		//文字右边距
#define TF_ID_BT_TEXT_TOP L"TextTop"		//文字上边距
#define TF_ID_BT_TEXT_BOTTOM L"TextBottom"		//文字下边距
#define TF_ID_BT_ACTION_WIDTH L"ActionWidth"		//感应区宽
#define TF_ID_BT_ACTION_HEIGHT L"ActionHeight"		//感应区高
#define TF_ID_BT_TEXT_MAXWIDTH L"TextMaxWidth"		//文字最大宽度
#define TF_ID_BT_TEXT_MAXHEIGHT L"TextMaxHeight"	//文字最大高度

#endif//_EVBUTTONIMP_H_
