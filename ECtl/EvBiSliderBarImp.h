/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVBiSliderBarIMP_H_
#define _EVBiSliderBarIMP_H_

#include "EvButtonImp.h"

// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvPictureFrame将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(BiSliderBar)
class CEvBiSliderBar :
	public CXuiElement<CEvBiSliderBar ,GET_BUILTIN_NAME(BiSliderBar)>
{
friend CXuiElement<CEvBiSliderBar ,GET_BUILTIN_NAME(BiSliderBar)>;
public:

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

protected:
	CEvBiSliderBar();
	virtual ~CEvBiSliderBar();

	//装载配置资源
	virtual ERESULT LoadResource();
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

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);
private:

	ULONG mulMethod;				 //采用什么缩放方式
	bool mbIsMouseFocus;			 //是否拥有鼠标焦点
	bool mbIsPressed;				 //是否被按下
private:
	int mnVertical;                  //0是横向，1是纵向
	FLOAT mfMinLeft;
	FLOAT mfMinRigth;
	FLOAT mfMaxLeft;
	FLOAT mfMaxRight;
	FLOAT mfMaxMid;
	FLOAT mfMinMid;
	FLOAT mnStep;

	FLOAT mfPosLeft;
	FLOAT mfPosRight;
	FLOAT mfPosMid;
	D2D1_RECT_F mSliderRect;
	D2D1_RECT_F mBarRect;
	FLOAT mfMaxScrollPixelLeft;
	FLOAT mfDestPixelPerScrollPixLeft;

	FLOAT mfMaxScrollPixelRight;
	FLOAT mfDestPixelPerScrollPixRight;

	FLOAT mfMaxScrollPixelMid;
	FLOAT mfDestPixelPerScrollPixMid;

	IEinkuiIterator* mpDragButtonLeft;
	IEinkuiIterator* mpDragButtonRigth;
	IEinkuiIterator* mpDragButtonMid; 
	IEinkuiIterator * mpDragButtonMidLable;
	FLOAT    mfLabelLength;
	IEinkuiIterator * mpBarPicture;
	IEinkuiIterator * mpLeftBarPicture;
	IEinkuiIterator * mpRightBarPicture;
public:
	bool SetRangMid(FLOAT nMin,FLOAT nMax);
	bool SetPosMid(FLOAT nPost);
	bool SetDeltaSizeMid(FLOAT nfSize);

	bool SetRangeLeft(FLOAT nMin,FLOAT nMax);
	bool SetRangeRight(FLOAT nMin,FLOAT nMax);
	bool SetPosLeft(FLOAT nPost);
	bool SetPosRight(FLOAT nPost);
	bool SetDeltaSizeLeft(FLOAT nfSize);
	bool SetDeltaSizeRigth(FLOAT nfSize);
};

#define SLIDER_BAR_PICTURE 1
#define SLIDER_BAR_LEFT_SLIDERBUTTON 2
#define SLIDER_BAR_RIGHT_SLIDERBUTTON 3
#define BI_SLIDER_BAR_LEFT_PICTURE 4
#define SLIDER_BAR_RIGHT_PICTURE 5
#define SLIDER_BAR_MID_SLIDERBUTTON 6
#define SLIDER_BAR_MID_LABEL 7

#define SLIDER_BAR_VER     L"Vertical"
#define SLIDER_BAR_MIN     L"MinValue"
#define SLIDER_BAR_MAX     L"MaxValue"
#define SLIDER_BAR_UP_BACK L"LeftOrUpBack"


#endif//_EVPICTUREFRAMEIMP_H_
