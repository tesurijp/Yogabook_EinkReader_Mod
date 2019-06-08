/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVRADIOBUTTONGROUPIMP_H_
#define _EVRADIOBUTTONGROUPIMP_H_



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvRadioButtonGroup将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。


DECLARE_BUILTIN_NAME(RadioButtonGroup)
class CEvRadioButtonGroup :
	public CXuiElement<CEvRadioButtonGroup ,GET_BUILTIN_NAME(RadioButtonGroup)>
{
	friend CXuiElement<CEvRadioButtonGroup ,GET_BUILTIN_NAME(RadioButtonGroup)>;
public:
	
	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);


protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvRadioButtonGroup();
	// 用于释放成员对象
	virtual ~CEvRadioButtonGroup();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);
private:
	IEinkuiIterator* mpCheckedItem;	//当前被选中的项
	//cmmVector<IEinkuiIterator*> mArrayGroup;	//所有项


	//有一项完成了一次单击
	ERESULT OnItemClick(IEinkuiIterator* npItem);
};

#endif//_EVRADIOBUTTONGROUPIMP_H_
