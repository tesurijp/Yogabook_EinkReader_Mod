#ifndef		__EVTOOLPANEIMP_H__
#define		__EVTOOLPANEIMP_H__


DECLARE_BUILTIN_NAME(ToolPane)
class CEvToolPane:
	public CXuiElement<CEvToolPane ,GET_BUILTIN_NAME(ToolPane)>
{
	friend CXuiElement<CEvToolPane ,GET_BUILTIN_NAME(ToolPane)>;

public:

	ULONG InitOnCreate(
		IN IXuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvToolPane();
	// 用于释放成员对象
	virtual ~CEvToolPane();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IXuiIterator* npIterator);
	//绘制
	//virtual ERESULT OnPaint(IXuiPaintBoard* npPaintBoard);
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IXuiMessage* npMsg);

	virtual ERESULT OnCtlButtonClick(IXuiIterator* npSender);

private:

	bool SetPaneItemValue(int lID);

	//ERESULT LoadResource();			//装载配置资源

public:

	IUnificSetting* mpUnificSetting;

private:
	

private:


};

#endif		// __EVTOOLPANEIMP_H__