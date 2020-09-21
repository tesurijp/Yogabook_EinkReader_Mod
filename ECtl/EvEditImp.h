#ifndef _EVEDITIMP_H_
#define _EVEDITIMP_H_

__interface IDWriteTextLayout;

class CEvEditTextSection {
public:
	D2D1_RECT_F mdRegion;
	LONG mlBgBrushIndex;
	LONG mlStart;
	LONG mlLength;

	void operator=(const CEvEditTextSection& src) {
		mdRegion = src.mdRegion;
		mlBgBrushIndex = src.mlBgBrushIndex;
		mlStart = src.mlStart;
		mlLength = src.mlLength;
	}
};




// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvEditImp将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(Edit)
class CEvEditImp :
	public CXuiElement<CEvEditImp, GET_BUILTIN_NAME(Edit)>
{
	friend CXuiElement<CEvEditImp, GET_BUILTIN_NAME(Edit)>;
public:
	enum EBRUSHINDEX {
		eForeBrush = 0,
		eBackBrush = 1,
		eSelForeBrush = 2,
		eSelBackBrush = 3
	};

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

protected:
	cmmVector<wchar_t> moText;	//存储输入的字符串
	LONG mlLimit;
	LONG mlCursorAt;		//光标所在位置
	LONG mlViewBegin;			//视图内的第一个字符
	LONG mlViewEnd;				//视图内的最后一个字符
	LONG mlSelBegin;			//选中字符开始
	LONG mlSelEnd;				//选中的最后一个字符
	bool mbDirty;
	bool mbInsertMode;
	bool mbCompletion;
	LONG mlBlinking;
	D2D1_RECT_F mdLayoutRect;		//文字的显示区域
	D2D1_RECT_F mdValidRect;
	D2D1_POINT_2F mdCursorPos;

	//显示资源
	IDWriteTextFormat* mpTextFormat;	//设备无关
	ID2D1SolidColorBrush* mpBrush[4];	// 设备相关

	//格式化文字
	cmmVector<CEvEditTextSection, 4> moTextSections;
	IDWriteTextLayout* mpTextLayout;	//设备无关

	// 鼠标选择
	D2D1_POINT_2F mdDragFrom;
	LONG mlDragedText;


	CEvEditImp();
	virtual ~CEvEditImp();

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	ERESULT OnElementCreate(IEinkuiIterator* npIterator);

	//virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//键盘消息
	virtual ERESULT OnKeyPressed(const STEMS_KEY_PRESSED* npInfo);

	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

	//鼠标双击
	virtual ERESULT OnMouseDbClick(const STEMS_MOUSE_BUTTON* npInfo);

	//字符输入消息
	virtual ERESULT OnChar(const PSTEMS_CHAR_INPUT npChar);

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//键盘焦点获得或者失去
	virtual void OnKeyBoardFocus(PSTEMS_STATE_CHANGE npState);

	//命令
	virtual ERESULT OnCommand(const nes_command::ESCOMMAND neCmd);

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

	// Left Arrow striked
	virtual void OnKeyLeft(const STEMS_KEY_PRESSED* npInfo);

	// Right Arrow striked
	virtual void OnKeyRight(const STEMS_KEY_PRESSED* npInfo);

	// Back Space striked
	virtual void OnKeyBack(const STEMS_KEY_PRESSED* npInfo);

	// Delete striked
	virtual void OnDeleteCommand(void);

	// Home striked
	virtual void OnKeyHome(const STEMS_KEY_PRESSED* npInfo);

	// End striked
	virtual void OnKeyEnd(const STEMS_KEY_PRESSED* npInfo);

	// Insert striked
	virtual void OnKeyInsert(const STEMS_KEY_PRESSED* npInfo);

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);

	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	// copy
	virtual void OnCopyCommand(void);

	// Paste
	virtual void OnPasteCommand(void);

	// Cut
	virtual void OnCutCommand(void);

	// Undo
	virtual void OnUndoCommand(void);

	//清除指针位置字符,nlFrom为从哪个索引开始;nlCount为删除个数
	virtual ERESULT RemoveChars(LONG nlFrom, LONG nlCount = 1);

	//插入新的字符,nlInsertTo表示从哪个位置进行插入,-1表示插到最后;nswChars表示要插入的字符串; nlLength == -1 表示nszChars的全部有效字符，并且确认nszChars带有\0结尾
	virtual LONG InsertChars(LONG nlInsertTo, wchar_t* nswChars, LONG nlLength = -1);

	// 插入一个字符
	virtual LONG InsertChar(LONG nlInsertTo, wchar_t nwcChar);

	// 重新生成格式化文字
	virtual ERESULT GenerateTextLayout(IEinkuiPaintBoard* npPaintBoard);

	// 计算文字的排布区域
	virtual void CalculateTextRect(IEinkuiPaintBoard* npPaintBoard);

	// 释放格式化文字
	void ClearTextLayout();

	// 删除设备相关资源
	void ReleaseDeviceResource();

	//从剪切板读字符串，return the character count
	int GetClipboardString(OUT wchar_t* npTextBuffer, LONG nlBufCharSize);

	//save string to clipboard
	void SetClipboardString(const wchar_t* nswString, LONG nlCharCount);

	// 清除剪贴板数据
	void ClearClipboard(void);

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

	void EndModifying(void);

};


#define EEDT_FLAG_NUMBER 0
#define EEDT_FLAG_PASSWORD 1
#define EEDT_FLAG_READONLY 2





#endif//_EVEDITIMP_H_
