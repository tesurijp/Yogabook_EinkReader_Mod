#include "stdafx.h"

#include "CommonHeader.h"
#include "XsMsgImp.h"









DEFINE_BUILTIN_NAME(CXuiMessage)


CXuiMessage::CXuiMessage()
{
	// 对成员变量做一些必要的初始化
	muMsgID = 0;
	miInputDataSize = 0;
	miOutputDataSize = 0;
	mpMsgDestItr = NULL;
	mpInputData = NULL;
	mpMsgOutputBuffer = NULL;
	muResult = ERESULT_UNSUCCESSFUL;
	mhCompleteEvent = NULL;
	mpSender = NULL;
	miOutputBufferSize = 0;
}
CXuiMessage::~CXuiMessage()
{
	FreeInputBuffer();
}

void CXuiMessage::FreeInputBuffer()
{
	if (mpInputData == NULL && miInputDataSize == 0)
		return;

	// 根据消息标志位判断是否需要释放缓冲区
	if (cmmBaseObject::TestFlag(ELMSG_FLAG_INPUT_ALLOCATED) != false && mpInputData != NULL && mpInputData != mbufInputBuffer)
		delete mpInputData;//delete mpInputData;	//hy 20190723

	mpInputData = NULL;
	miInputDataSize = 0;

	cmmBaseObject::SetFlags(ELMSG_FLAG_INPUT_ALLOCATED, false);
	//cmmBaseObject<CXuiMessage,IEinkuiMessage,GET_BUILTIN_NAME(CXuiMessage)>::SetFlags(ELMSG_FLAG_INPUT_COPIED,false);
}

// 重用本结构
void CXuiMessage::Reuse()
{

	FreeInputBuffer();

	muMsgID = 0;
	miOutputDataSize = 0;
	mpMsgDestItr = NULL;
	mpMsgOutputBuffer = NULL;
	muResult = ERESULT_UNSUCCESSFUL;
	mhCompleteEvent = NULL;
	mpSender = NULL;
	miOutputBufferSize = 0;

	cmmBaseObject<CXuiMessage, IEinkuiMessage, GET_BUILTIN_NAME(CXuiMessage)>::Reuse();
}


// 释放对象
int __stdcall CXuiMessage::Release(void)
{
	static CXuiMessage* glMsg = NULL;
	if (glMsg == this)
		glMsg->Release();
	int liCount = cmmBaseObject<CXuiMessage, IEinkuiMessage, GET_BUILTIN_NAME(CXuiMessage)>::Release();
	// 如果返回值是1，则表示，除元素管理器的一次引用外，已经没有别人引用这个元素了，就通知元素管理器准备释放
	if (liCount == 1)
	{
		// 通知元素管理器
		CEinkuiSystem::gpXuiSystem->GetElementManagerObject()->ReleaseMessage(this);
	}

	return liCount;
}

// 获得消息的接收目标的Iterator接口
IEinkuiIterator* __stdcall CXuiMessage::GetDestination(void)
{
	return mpMsgDestItr;

}

// 设置消息的接受目标
ERESULT __stdcall CXuiMessage::SetDestination(IEinkuiIterator* npMsgDestItr)
{
	if (npMsgDestItr != NULL)
	{
		mpMsgDestItr = npMsgDestItr;
		return ERESULT_SUCCESS;
	}
	else
	{
		return ERESULT_UNSUCCESSFUL;
	}

}

// 获得消息ID
ULONG __stdcall CXuiMessage::GetMessageID(void)
{
	return muMsgID;
}

// 设置消息ID
ERESULT __stdcall CXuiMessage::SetMessageID(ULONG nuMsgID)
{
	// 无须判断消息ID的有效性
	muMsgID = nuMsgID;
	return ERESULT_SUCCESS;
}

// 获得返回值
ERESULT __stdcall CXuiMessage::GetResult(void)
{
	return muResult;
}

// 设置返回值
ERESULT __stdcall CXuiMessage::SetResult(ERESULT nuResult)
{
	muResult = nuResult;
	return ERESULT_SUCCESS;
}

// 设置消息发送者，当消息被用于从控件向控件使用者发送事件（控件产生的消息）时，需要设置消息的发送者；Iterator接口提供的PostMessageToParent方法会自动设置发送者
void __stdcall CXuiMessage::SetMessageSender(IEinkuiIterator* npSender)
{
	mpSender = npSender;
}

// 获得消息的发送者
IEinkuiIterator* __stdcall CXuiMessage::GetMessageSender(void)
{
	return mpSender;
}


// 设置Input数据；如果消息是被Post的，nbVolatile必须为ture；如果消息是被Send给目标的，将按照参数nbVolatile决定是否复制输入缓冲
ERESULT __stdcall CXuiMessage::SetInputData(
	const void* npBuffer,
	int niSize,
	bool nbVolatile	// true:此缓冲区是易失的，需要复制数据到内部缓冲; false 此缓冲区是非易失的，在消息发送和返回的过程中有效
)
{
	ERESULT luResult;

	if (niSize < 0 || npBuffer == NULL && niSize != 0)
		return ERESULT_UNSUCCESSFUL;

	FreeInputBuffer();

	if (nbVolatile != false && npBuffer != NULL)
	{
		luResult = CopyInputData(npBuffer, niSize);
	}
	else
	{
		// npBuffer == NULL or nbVolatile == false
		mpInputData = (void*)npBuffer;
		miInputDataSize = niSize;

		luResult = ERESULT_SUCCESS;
	}

	return ERESULT_SUCCESS;
}

// 获得Input数据指针，注意，获得的指针仅在持有消息，并且没有发生改变时有效，一旦将消息发送出去，或者调用了消息的设定值的方法，都将导致该指针失效
// 注意，此方法获得的指针并不一定同前一次调用SetInputData设定的指针相同
const void* __stdcall CXuiMessage::GetInputData(void)
{
	return mpInputData;
}

// 获得输入缓冲区的大小
int __stdcall CXuiMessage::GetInputDataSize(void)
{
	return miInputDataSize;
}

// 设置Output缓冲区，大多数的消息无需Output Buffer，如果需要消息返回大量数据的，应该Send这条消息，而不是Post它；如果确实需要Post这条消息，那么请参考下面的设置反馈消息的方法;
// 如果选择Post这条消息，请千万保证设定的Output缓冲区不被修改和释放，以免消息的接受方对该缓冲区访问产生错误。
ERESULT __stdcall CXuiMessage::SetOutputBuffer(void* npBuffer, int niSize)
{
	if (niSize <= 0)
		return ERESULT_UNSUCCESSFUL;

	miOutputBufferSize = niSize;

	if (npBuffer == NULL)
		return ERESULT_UNSUCCESSFUL;
	mpMsgOutputBuffer = npBuffer;

	return ERESULT_SUCCESS;

}

// 获得Output缓冲区指针，注意，获得的指针仅在持有该消息时有效；
void* __stdcall CXuiMessage::GetOutputBuffer(void)
{
	return mpMsgOutputBuffer;
}

// 获得Output缓冲区的大小
int __stdcall CXuiMessage::GetOutputBufferSize(void)
{
	return miOutputBufferSize;
}

// 设置填入Output缓冲区的数据大小
void CXuiMessage::SetOutputDataSize(int niSize)
{
	miOutputDataSize = niSize;
}

// 获得Output缓冲区的数据大小
int CXuiMessage::GetOutputDataSize(void)
{
	return miOutputDataSize;
}

// 设置反馈消息，当消息被目标对象处理完毕后，系统将自动生成一个新的消息返回给发送者，这个新的消息叫做反馈消息
ERESULT __stdcall CXuiMessage::SetResponseMessage(
	IN IEinkuiIterator* npReceiver,	// 接受反馈消息的目标
	IN void* npContext	// 设置上下文，供调用者设置和使用，当该消息被反馈时，传递给接收者
)
{
	return ERESULT_UNSUCCESSFUL;//???尚未实现
}


// 复制用户提供的输入数据
ERESULT CXuiMessage::CopyInputData(
	const void* npBuffer,
	int niSize
)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (npBuffer != NULL)
	{
		if (niSize < ELMSG_BUILDIN_BUFSIZE)
		{
			mpInputData = mbufInputBuffer;
		}
		else
		{
			mpInputData = new char[niSize];
			cmmBaseObject::SetFlags(ELMSG_FLAG_INPUT_ALLOCATED, true);
		}

		if (mpInputData != NULL)
		{
    		// CheckMarx fix by zhuhl5
			memcpy_s(mpInputData, niSize, npBuffer, niSize);

			miInputDataSize = niSize;

			luResult = ERESULT_SUCCESS;
		}
		else
			luResult = ERESULT_INSUFFICIENT_RESOURCES;
	}

	return luResult;
}

// 消息接收者用于确定当前消息是否被Send而来，返回true,the message was sent from source element; false, the message was posted by the sender;
bool __stdcall CXuiMessage::IsSent(void)
{
	return cmmBaseObject::TestFlag(ELMSG_FLAG_SEND);
}
