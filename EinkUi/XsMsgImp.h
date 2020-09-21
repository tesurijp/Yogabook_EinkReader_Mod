#ifndef _ELMSGIMP_H_
#define _ELMSGIMP_H_
/*
	本文件定义Message对象的实现

*/

// 设置内置缓冲区大小，绝对不能小于64个字节
#define ELMSG_BUILDIN_BUFSIZE 512



DECLARE_BUILTIN_NAME(CXuiMessage)
class CXuiMessage : public cmmBaseObject<CXuiMessage, IEinkuiMessage, GET_BUILTIN_NAME(CXuiMessage)>
{
	friend class CXelManager;
	friend class CXuiMessageQueue;
public:

	DEFINE_CUMSTOMIZE_CREATE(CXuiMessage, (), ())

		// 释放对象
		virtual int __stdcall Release(void);

	// 获得消息的接收目标的Iterator接口
	virtual IEinkuiIterator* __stdcall GetDestination(void);

	// 设置消息的接受目标
	virtual ERESULT __stdcall SetDestination(IEinkuiIterator* npMsgDestItr);

	// 获得消息ID
	virtual ULONG __stdcall GetMessageID(void);

	// 设置消息ID
	virtual ERESULT __stdcall SetMessageID(ULONG nuMsgID);

	// 获得返回值
	virtual ERESULT __stdcall GetResult(void);

	// 设置返回值，函数返回的是本操作的成功与否
	virtual ERESULT __stdcall SetResult(ERESULT nuResult);

	// 设置消息发送者，当消息被用于从控件向控件使用者发送事件（控件产生的消息）时，需要设置消息的发送者；Iterator接口提供的PostMessageToParent方法会自动设置发送者
	virtual void __stdcall SetMessageSender(IEinkuiIterator* npSender);

	// 获得消息的发送者
	virtual IEinkuiIterator* __stdcall GetMessageSender(void);

	// 设置Input数据；如果消息是被Post的，nbVolatile必须为ture；如果消息是被Send给目标的，将按照参数nbVolatile决定是否复制输入缓冲
	virtual ERESULT __stdcall SetInputData(
		const void* npBuffer,	// 注意，如果希望传递的参数是一个指针本身，如：lpAnybody，应该如此调用SetInputData(&lpAnybody,sizeof(lpAnybody),ture/false);
		int niSize,
		bool nbVolatile = true	// true:此缓冲区是易失的，需要复制数据到内部缓冲; false 此缓冲区是非易失的，在消息发送和返回的过程中有效
	);

	// 获得Input数据指针，注意，获得的指针仅在持有消息，并且没有发生改变时有效，一旦将消息发送出去，或者调用了消息的设定值的方法，都将导致该指针失效
	// 注意，此方法获得的指针并不一定同前一次调用SetInputData设定的指针相同
	virtual const void* __stdcall GetInputData(void);

	// 获得输入缓冲区的大小
	virtual int __stdcall GetInputDataSize(void);

	// 设置Output缓冲区，大多数的消息无需Output Buffer，如果需要消息返回大量数据的，应该Send这条消息，而不是Post它；如果确实需要Post这条消息，那么请参考下面的设置反馈消息的方法;
	// 如果选择Post这条消息，请千万保证设定的Output缓冲区不被修改和释放，以免消息的接受方对该缓冲区访问产生错误。
	virtual ERESULT __stdcall SetOutputBuffer(void* npBuffer, int niSize);

	// 获得Output缓冲区指针，注意，获得的指针仅在持有该消息时有效；
	virtual void* __stdcall GetOutputBuffer(void);

	// 获得Output缓冲区的大小
	virtual int __stdcall GetOutputBufferSize(void);


	// 设置填入Output缓冲区的数据大小
	// 本方法仅能在消息被发送后调用，供消息的接收者调用
	virtual void SetOutputDataSize(int niSize);
	// 获得Output缓冲区的数据大小
	virtual int GetOutputDataSize(void);

	// 设置反馈消息，当消息被目标对象处理完毕后，系统将自动生成一个新的消息返回给发送者，这个新的消息叫做反馈消息
	virtual ERESULT __stdcall SetResponseMessage(
		IN IEinkuiIterator* npReceiver,	// 接受反馈消息的目标
		IN void* npContext = NULL	// 设置上下文，供调用者设置和使用，当该消息被反馈时，传递给接收者
	);

	// 消息接收者用于确定当前消息是否被Send而来，返回true,the message was sent from source element; false, the message was posted by the sender;
	virtual bool __stdcall IsSent(void);

protected:

	CXuiMessage();
	~CXuiMessage();

	// 重用本结构
	void Reuse();

protected:
	// 消息接收者的迭代器指针
	IEinkuiIterator* mpMsgDestItr;
	// 消息发送者
	IEinkuiIterator* mpSender;

	// 该指针指向消息缓冲区,当主调对象post消息时，需要复制消息的内容	
	void* mpInputData;

	// 消息缓冲区的大小	
	int	 miInputDataSize;

	// 消息的outputBuffer缓冲区指针
	void* mpMsgOutputBuffer;
	// 消息的outputBuffer缓冲区的大小
	int miOutputBufferSize;
	// 记录OutputBuffer中数据的多少
	int miOutputDataSize;

	// 当前消息对象的消息ID	
	ULONG muMsgID;

	// 存放某一操作的返回值
	ERESULT muResult;

	// 消息完成，供系统用同步事件
	HANDLE mhCompleteEvent;

	// 1K大小的输入缓冲区
	UCHAR mbufInputBuffer[ELMSG_BUILDIN_BUFSIZE];

	// 复制用户提供的输入数据
	__inline ERESULT CopyInputData(
		const void* npBuffer,
		int niSize	// 必须大于0
	);

	__inline void FreeInputBuffer();

};


#define	ELMSG_FLAG_INPUT_ALLOCATED  0 		// 表示Input缓冲区需要释放
#define ELMSG_FLAG_SEND		1		// 表示消息被Send而来，未设置表示消息是被Post而来
//#define ELMSG_FLAG_FAST		2		// 表示消息就有Fast属性，这类消息应该按先后排列在消息队列的前部，等待处理
//#define ELMSG_FLAG_REVERSE	3		// 当消息队列同时存在这类消息的时候，将会被后进先出的方式排列在消息队列中，它们的优先级高于普通消息，但低于Fast消息
//
//#if (EMSG_POST_FAST != ELMSG_FLAG_FAST || ELMSG_FLAG_REVERSE != EMSG_POST_REVERSE)
//#error "Must set the same value"
//#endif







































#endif//_ELMSGIMP_H_