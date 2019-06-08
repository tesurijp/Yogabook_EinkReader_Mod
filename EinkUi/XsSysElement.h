/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


//////////////////////////////////////////////////////////////////////////
// 元素管理器代理元素，它接受所有发送给元素管理器的消息
DECLARE_BUILTIN_NAME(CEleMgrProxy)
class CEleMgrProxy: public cmmBaseObject<CEleMgrProxy,IXsElement,GET_BUILTIN_NAME(CEleMgrProxy)>
{
public:
	// 获得本元素的元素类型
	virtual const wchar_t* __stdcall  GetType(void);

	// 在全局范围内验证此对象是否是nswType指定的类型
	virtual bool __stdcall  GlobleVerification(const wchar_t* nswType);

	// 默认消息入口函数，用于接收输入消息
	virtual ERESULT __stdcall MessageReceiver(IEinkuiMessage* npMsg);

	// 获得本元素的迭代器接口
	virtual IEinkuiIterator* __stdcall GetIterator(void);

	// 获得Tab Order, -1 未设置
	virtual LONG __stdcall GetTabOrder(void) {
		return 0;
	}

	// 获得Z Order，-1 未设置
	virtual LONG __stdcall GetZOrder(void){
		return 0;
	}

};


