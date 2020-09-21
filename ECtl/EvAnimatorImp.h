#ifndef _EVANIMATOR_H_
#define _EVANIMATOR_H_

//#include "D2dDrawX.h"


// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvAnimatorImp将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。
DECLARE_BUILTIN_NAME(Animator)
class CEvAnimatorImp :
	public CXuiElement<CEvAnimatorImp, GET_BUILTIN_NAME(Animator)>
{
	friend CXuiElement<CEvAnimatorImp, GET_BUILTIN_NAME(Animator)>;
public:

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

protected:
	//CEdxDialMask moMask;

	//显示资源
	IEinkuiBitmap* mpFrontFrame;

	IEinkuiBrush* mpSweepBrush;	// 设备相关

	CEvAnimatorImp();
	virtual ~CEvAnimatorImp();

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	ERESULT OnElementCreate(IEinkuiIterator* npIterator);

	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	// 更新帧
	ERESULT UpdateFrame(PSTCTL_ANIMATOR_FRAME npFrame);

	// 设定变换
	ERESULT SetTransform(PSTCTL_ANIMATOR_TRANSFORM npTransform);
};


#endif//_EVANIMATOR_H_
