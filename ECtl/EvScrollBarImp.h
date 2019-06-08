/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVSCROLLBARIMP_H_
#define _EVSCROLLBARIMP_H_

//进度条

// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvScrollBar将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(ScrollBar)
class CEvScrollBar :
	public CXuiElement<CEvScrollBar ,GET_BUILTIN_NAME(ScrollBar)>
{
friend CXuiElement<CEvScrollBar ,GET_BUILTIN_NAME(ScrollBar)>;
public:
	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CEvScrollBar();
	virtual ~CEvScrollBar();

	//绘制
	//virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);

	//点击button信息
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//鼠标按下;
	ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	// 鼠标落点检测
	ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	ERESULT LoadResource();
public:
	//设置滑块可以滑动的距离，
	bool SetDeltaSize(FLOAT nnSize);
	//设置滑块位置向上或向下
	bool SetPosition(bool bUp);
	//获取纵向滚动条的宽度
	FLOAT GetVScrollBarWidth();
	//获取横向滚动条的高度
	FLOAT GetHScrollBarHeigth();
	//修改元素布局
	void Relocation(void);
private:
	bool mbVertical;
	IEinkuiIterator* mpBtUp;    //向上或向左
	IEinkuiIterator* mpBtDrag;	//滑块
	IEinkuiIterator* mpBtDown;  //向下或向右
	D2D1_RECT_F  mRectSlider;//可以滑动的大小
	D2D1_POINT_2F  mSliderPoint;//滑块的位置
	FLOAT mfDeltaSize; //目标需要滑动的像素大小
	FLOAT mfMinBarSize; //滑块能缩小的最小值，如果是垂直就是高度，如果是水平就是宽度
	FLOAT mfDestPixelPerScrollPix;//滑块滑动一个像素对应的目标滚动几个像素

	FLOAT mfPosition;   //滑块当前所在的像素位置
	FLOAT mfSVScrollBarWidth; //滚动条的宽度，取滑块的宽度
	FLOAT mfSHScrollBarHeigth; //滚动条的高度，取滑块的宽度

	//装载配置资源
	//virtual ERESULT LoadResource();
	
};

#define SCB_BT_UP 1		//向上或向左按钮
#define SCB_BT_DRAG 2	//拖动块
#define SCB_BT_DOWN 3	//向下或向右按钮

#define SCROLLBAR_HOR L"Hor"

#endif//_EVSCROLLBARIMP_H_
