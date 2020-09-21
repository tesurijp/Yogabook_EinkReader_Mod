#include "stdafx.h"

#include "CommonHeader.h"
#include "XsSysElement.h"

DEFINE_BUILTIN_NAME(CEleMgrProxy)




const wchar_t* CEleMgrProxy::GetType(void)
{
	return GetObjectName();
}

bool CEleMgrProxy::GlobleVerification(const wchar_t* nswType)	// 验证此对象是否是nswType指定的类型
{
	return (_wcsicmp(nswType, GetObjectName()) == 0);
}

// 默认消息入口函数，用于接收输入消息
ERESULT __stdcall CEleMgrProxy::MessageReceiver(IEinkuiMessage* npMsg)
{
	return CEinkuiSystem::gpXuiSystem->SystemMessageReceiver(npMsg);
}

// 获得本元素的迭代器接口
IEinkuiIterator* __stdcall CEleMgrProxy::GetIterator(void)
{
	return NULL;
}


