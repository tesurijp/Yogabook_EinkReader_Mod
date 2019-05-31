/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


DECLARE_BUILTIN_NAME(System_SelectFrame)


#define SF_POINT_MAX 8	//最大点个数

class CSelectFrame :
	public CXuiElement<CSelectFrame,GET_BUILTIN_NAME(System_SelectFrame)>
{
	friend CXuiElement<CSelectFrame,GET_BUILTIN_NAME(System_SelectFrame)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=0					// 如果不为0，则指定该元素的EID，否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动那个分配
		) ;

protected:
	CSelectFrame(void);
	virtual ~CSelectFrame(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);
	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);
	//键盘消息
	virtual ERESULT OnKeyPressed(const STEMS_KEY_PRESSED* npInfo);

protected:
	
	// 设置对应点的鼠标状态
	void SetCursor(D2D1_POINT_2F ndPoint, D2D1_RECT_F ndNormalRect, int niIndex);

	// 设置八个点的显示状态
	void SetPointVisible(bool nbFlag);

	//调整八个点的位置
	void Relocation(void);

	//处理八个点拖动
	ERESULT OnPointDrag(IEinkuiIterator* npDragItem,D2D1_SIZE_F* npOffset);

	// 等比缩放
	ERESULT OnProportionalScaling(IEinkuiIterator* npDragItem, STCTL_CHANGE_POSITION_SIZE& noChangePositionSize);

	// 任意比缩放
	ERESULT OnNormalScaling(IEinkuiIterator* npDragItem, STCTL_CHANGE_POSITION_SIZE& noChangePositionSize);

private:
	IEinkuiIterator* mpArrayPoint[SF_POINT_MAX];
	D2D1_POINT_2F mdOriginPosition;		// 原始位置
	D2D1_SIZE_F	mdOriginSize;			// 原始大小
	IEinkuiBrush*	mpXuiBrush;				// 虚线画刷
	bool 		mbModifying;			// 指示当前编辑状态

	HCURSOR mhCursorNwse;		// 鼠标样式
	HCURSOR mhCursorNesw;		// 鼠标样式

	bool	mcLastVTurn;	// 上一次的垂直翻转状态	
	bool	mcLastHTurn;	// 上一次的水平翻转状态

	bool	mcVTurn;	// 是否发生垂直翻转
	bool	mcHTurn;	// 是否发生水平翻转


	// 等比缩放相关的属性
	bool	mcProportionalScaling;	// 是否按照强制比例缩放
	bool	mcProportionalScalingByKey; // 等比状态，当按下shift键的时候为true

	// 是否允许编辑，如果不允许编辑,则所有的锚点状态均为移动状态
	bool	mcIsEdit;


};


