/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVSliderButtonIMP_H_
#define _EVSliderButtonIMP_H_

#include "EvButtonImp.h"

// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvPictureFrame将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(SliderButton)
class CEvSliderButton :	public CEvButton
{
friend CEvButton;
public:

	//////////////////////////////////////////////////////////////////////////
	// 重载实例化函数，用在从CXuiElement的派生类再次派生新类
	DEFINE_CUMSTOMIZE_CREATE(CEvSliderButton,(IEinkuiIterator* npParent,ICfKey* npTemplete,ULONG nuEID),(npParent,npTemplete,nuEID))


	//////////////////////////////////////////////////////////////////////////
	// 重载类型识别，前一个参数是本类的ETYPE，同DECLARE_BUILTIN_NAME()宏中的设置一致
	// 用在从CXuiElement的派生类再次派生新类
	DEFINE_DERIVED_TYPECAST(SliderButton,CEvButton)


	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
protected:
	CEvSliderButton();
	virtual ~CEvSliderButton();
public:
	LONG mnMoveStyle; //滑动方向见Ectl.h
	D2D1_RECT_F mRectSlider;//滑动区域
	D2D1_POINT_2F mDragStartPoint;

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);
};

#endif//_EVPICTUREFRAMEIMP_H_
