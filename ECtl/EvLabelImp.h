#ifndef _EVLABEL_H_
#define _EVLABEL_H_

__interface IDWriteTextLayout;



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvLabelImp将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(Label)
class CEvLabelImp :
	public CXuiElement<CEvLabelImp, GET_BUILTIN_NAME(Label)>
{
	friend CXuiElement<CEvLabelImp, GET_BUILTIN_NAME(Label)>;
public:

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

protected:
	HCURSOR mdPreviousCursor;		//在鼠标进入自己前的状态
	cmmVector<wchar_t> moText;	//存储输入的字符串
	bool mbDirty;
	bool mbLbPressed;
	LONG mlBlinking;
	LONG mlVisibleCount;
	UINT32 muLines;
	ULONG muForeColor;
	ULONG muBackColor;
	D2D1_RECT_F mdTextEdge;		//文字区域同底图的边界
	D2D1_SIZE_F mdTextSize;		//当前文字的实际显示矩形的宽高
	D2D1_RECT_F mdBgRect;		//背景的位置和大小
	D2D1_POINT_2F mdTextPos;	//当前显示文字的位置，从mdTextEdge的边界开始计算，左上角对齐是就是x=0，y=0

	//显示资源
	IDWriteTextFormat* mpTextFormat;	//设备无关
	IDWriteTextLayout* mpTextLayout;

	ID2D1SolidColorBrush* mpForeBrush;	// 设备相关
	ID2D1SolidColorBrush* mpBackBrush;
	FLOAT mfTextMaxHeight;
	FLOAT mfTextMaxWidth;
	CEvLabelImp();
	virtual ~CEvLabelImp();

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	ERESULT OnElementCreate(IEinkuiIterator* npIterator);

	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//键盘消息
	virtual ERESULT OnKeyPressed(const STEMS_KEY_PRESSED* npInfo);

	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//键盘焦点获得或者失去
	virtual void OnKeyBoardFocus(PSTEMS_STATE_CHANGE npState);

	//Set Text
	virtual ERESULT OnSetText(const wchar_t* nswText);

	//Get Text
	virtual ERESULT OnGetText(wchar_t* nswTextBuf, LONG nlCharCount);

	// 慢刷新
	void  OnLazyUpdate(
		PSTEMG_LAZY_UPDATE npLazyUpdate
	);

	//准备画笔
	ERESULT PrepareBrush(IEinkuiPaintBoard* npPaintBoard);

	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	// 重新生成格式化文字
	virtual ERESULT GenerateTextLayout(void);

	// 计算文字的排布区域
	virtual void CalculateTextRect(void);

	// 删除设备相关资源
	void ReleaseDeviceResource();

	__inline void UpdateView(void);

	// 设置位标志；可以 0 - 24
	bool SetFlags(
		int niIndex,		// 标志的序号，从0开始；如果派生类重载这个函数，并且该派生类有2个不希望被后续类和用户修改的标志，那么它的函数调用时的niIndex=0表示的是它的基类的2
		bool nbSet		// 设置或者清除标志
	) {
		return CXuiElement::SetFlags(niIndex + 4, nbSet);
	}

	// 获取标志
	bool TestFlag(int niIndex) {
		return CXuiElement::TestFlag(niIndex + 4);
	}

	// 允许输入
	void EnableInput(LONG nlSet);

	ERESULT SetBrushColor(IEinkuiMessage* npMsg, bool nbFore);

	ERESULT GetBrushColor(IEinkuiMessage* npMsg, bool nbFore);

	ERESULT GetLayout(IEinkuiMessage* npMsg);

	ERESULT CreateFont(void);

};


#define ELAB_FLAG_UNDERLINE 0	// 加下划线
#define ELAB_FLAG_HYPERLINK 1	// 超链接，这种方式下，将支持鼠标和键盘事件，从而向上层反馈一个Open事件
#define ELAB_FLAG_FORE_COLOR 2
#define ELAB_FLAG_BACK_COLOR 3

#define TF_ID_LABEL_HYPER_LINK L"HyperLink"				// 是否支持超链接
#define TF_ID_LABEL_UNDER_LINE	L"UnderLine"			// 是否需要下划线
#define TF_ID_LABEL_EDGE_LEFT L"Edge/Left"				// 左边距
#define TF_ID_LABEL_EDGE_RIGHT L"Edge/Right"			// 右边距
#define TF_ID_LABEL_EDGE_TOP L"Edge/Top"				// 上边距
#define TF_ID_LABEL_EDGE_BOTTOM L"Edge/Bottom"			// 下边距
#define TF_ID_LABEL_VALUE		L"Value"				// 显示的文本
#define TF_ID_LABEL_MAX_WIDTH		L"MaxWidth"			// 最大宽度
#define TF_ID_LABEL_MAX_HEIGHT		L"MaxHeight"		// 最大高度
#define TF_ID_LABEL_COLOR_FORE	L"Color/Fore"			// 文本前景色
#define TF_ID_LABEL_COLOR_BACK	L"Color/Back"			// 文本背景色
#define TF_ID_LABEL_FONT		L"Font"					// 字体
#define TF_ID_LABEL_FONT_SIZE	L"Font/Size"		// 字号
#define TF_ID_LABEL_ALIGN_VERTICAL L"Align/Vertical"	// 垂直对齐方式
#define TF_ID_LABEL_ALIGN_HORIZONTAL L"Align/Horizontal"	// 水平对齐方式



#endif//_EVLABEL_H_
