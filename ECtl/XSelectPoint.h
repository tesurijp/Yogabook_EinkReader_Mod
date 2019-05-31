/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


DECLARE_BUILTIN_NAME(System_SelectPoint)


#define SF_POINT_MAX 8	//最大点个数

class CSelectPoint :
	public CXuiElement<CSelectPoint,GET_BUILTIN_NAME(System_SelectPoint)>
{
	friend CXuiElement<CSelectPoint,GET_BUILTIN_NAME(System_SelectPoint)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=0					// 如果不为0，则指定该元素的EID，否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动那个分配
		) ;

protected:
	CSelectPoint(void);
	virtual ~CSelectPoint(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);

private:
	float	mfWidth;			// 默认宽度
	float	mfHeight;			// 默认高度

	IEinkuiBrush* mpXuiBrush;
	IEinkuiBrush* mpFillBrush;


	//修改鼠标样式
	void SetCursor(HCURSOR nhInnerCursor);


};


