#ifndef _EVCHECKBUTTONIMP_H_
#define _EVCHECKBUTTONIMP_H_



// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvButton将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。


DECLARE_BUILTIN_NAME(CheckButton)
class CEvCheckButton : public CEvButton
{
public:

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	);

	// 重载实例化接口
	DEFINE_CUMSTOMIZE_CREATE(CEvCheckButton, (IEinkuiIterator* npParent, ICfKey* npTemplete, ULONG nuEID = MAXULONG32), (npParent, npTemplete, nuEID))

		//////////////////////////////////////////////////////////////////////////
		// 一定要加入这一行，这是重载类型识别
		DEFINE_DERIVED_TYPECAST(CheckButton, CEvButton)

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvCheckButton();
	// 用于释放成员对象
	virtual ~CEvCheckButton();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
	//定位文字图片显示位置
	virtual void RelocateText(void);
private:

};

#endif//_EVCHECKBUTTONIMP_H_

#define TF_ID_CHECK_BT_CHECKED L"Checked"			//如果有这个键值，说明创建时就选中