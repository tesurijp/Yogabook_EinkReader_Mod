/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once
// 本文件旨在建立单向跨进程的消息机制
// 通过一块共享内存和一个互斥对象名称建立的跨进程消息队列，能够让两个进程间进行通讯。
// 消息队列由监听进程首先初始化，而后发信进程连接到消息队列；监听进程在初始化时需要提供一个回调函数，这个回调函数将在监听线程中被调用。
// 同一块内存只能有一个监听对象

#pragma pack(4)

typedef struct _EI_MSGQUE_HEAD {
	ULONG HeadSize;
	volatile ULONG BufSize;
	volatile ULONG MsgSize;
	volatile ULONG MsgCapacity;
	volatile ULONG FirstMsg;
	volatile ULONG LastMsg;
	volatile ULONG MsgCount;
}EI_MSGQUE_HEAD,* PEI_MSGQUE_HEAD;
#pragma pack()


// 消息队列
template<class CEiMsgMessage>
class CEiMsgQueue
{
private:
	PEI_MSGQUE_HEAD mpQueueHead;
	CEiMsgMessage* mpMsgBuffer;
	HANDLE mhNewArrived;
	HANDLE mhMutex;
	BOOL mbStop;

public:
	CEiMsgQueue() {
		mpQueueHead = NULL;
		mpMsgBuffer = NULL;
		mhNewArrived = NULL;
		mhMutex = NULL;
		mbStop = FALSE;
	}
	~CEiMsgQueue() {
	}

	BOOL GetStopState(void) {
		return mbStop;
	}

	// 从共享内存建立队列，返回ERROR_SUCCESS，或者错误码
	ULONG CreateQueue(
		const wchar_t* nusMutexName,
		const wchar_t* nusSemaphoreName,
		void* npBuffer,
		ULONG nuBufferSize,
		BOOL nbListener	// TURE: 监听者
	);

	// 增加一条消息，返回ERROR_SUCCESS，或者错误码
	ULONG Push(
		const CEiMsgMessage& nrMsg
		);

	// 取一条消息，返回ERROR_SUCCESS，或者错误码
	// 如果消息队列为空，将进入等待
	// 供监听对象调用
	ULONG Pop(
		CEiMsgMessage& nrMsg
		);

	// 撤回一类消息，将队列中此类消息全部撤回
	// 如果调用此函数，需要CEiMsgMessage实现bool IsTypeOf(const CEiMsgMessage& nrRefTo)函数，改函数返回true表示同类，返回false表示非同类
	// 当一个消息被撤回时，会调用CEiMsgMessage的Recall()函数，请在该函数内实现对消息撤回操作
	void Recall(
		const CEiMsgMessage& nrMsg	
	);

	// 获取消息数
	ULONG GetCount(void);

	// 关闭
	void RealseQueue(void);

	// 获得当前未取走的最早一条消息已发出时间，单位为毫秒
	ULONGLONG GetMaxElapsedTimeOfMsg(void);

};

template<class CEiMsgMessage>
ULONG CEiMsgQueue<CEiMsgMessage>::CreateQueue(
	const wchar_t* nusMutexName,
	const wchar_t* nusSemaphoreName,
	void* npBuffer,
	ULONG nuBufferSize,
	BOOL nbListener	// TURE: 监听者
)
{
	ULONG luResult = ERROR_SUCCESS;

	if (nuBufferSize < sizeof(EI_MSGQUE_HEAD) || npBuffer == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	if (nbListener != FALSE)
	{	// 监听者
		// 建立消息旗语
		//需要设置权限，否则服务创建的对象，普通进程无法打开
		SECURITY_DESCRIPTOR lsd;
		InitializeSecurityDescriptor(&lsd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&lsd, TRUE, (PACL)NULL, FALSE);
		SECURITY_ATTRIBUTES	lsa;
		lsa.nLength = sizeof(SECURITY_ATTRIBUTES);
		lsa.bInheritHandle = TRUE;
		lsa.lpSecurityDescriptor = &lsd;

		mhNewArrived = CreateSemaphore(&lsa, 0, 10000, nusSemaphoreName);
		if (mhNewArrived == NULL)
			return ERROR_OPEN_FAILED;

		// 建立互斥对象
		mhMutex = CreateMutex(&lsa, TRUE, nusMutexName);
		if (mhMutex == NULL)
			return ERROR_OPEN_FAILED;

		// 初始化消息队列共享内存
		mpQueueHead = (PEI_MSGQUE_HEAD)npBuffer;
		mpMsgBuffer = (CEiMsgMessage*)(mpQueueHead + 1);

		mpQueueHead->HeadSize = sizeof(EI_MSGQUE_HEAD);
		mpQueueHead->BufSize = nuBufferSize - sizeof(EI_MSGQUE_HEAD);
		mpQueueHead->MsgSize = sizeof(CEiMsgMessage);
		mpQueueHead->MsgCapacity = mpQueueHead->BufSize / mpQueueHead->MsgSize;
		mpQueueHead->FirstMsg = 0;
		mpQueueHead->LastMsg = 0;
		mpQueueHead->MsgCount = 0;

		luResult = ERROR_SUCCESS;

		ReleaseMutex(mhMutex);

	}
	else
	{	// 非监听者
		// 连接旗语对象
		mhNewArrived = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, nusSemaphoreName);
		if (mhNewArrived == NULL)
			return ERROR_OPEN_FAILED;

		// 连接互斥对象
		mhMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, nusMutexName);
		if (mhMutex == NULL)
			return ERROR_OPEN_FAILED;
		
		if (WaitForSingleObject(mhMutex, INFINITE) != WAIT_OBJECT_0)
			return ERROR_NOT_READY;

		// 从共享内存中装入数据
		if (((PEI_MSGQUE_HEAD)npBuffer)->MsgSize == sizeof(CEiMsgMessage) &&
			((PEI_MSGQUE_HEAD)npBuffer)->HeadSize == sizeof(EI_MSGQUE_HEAD))
		{
			mpQueueHead = (PEI_MSGQUE_HEAD)npBuffer;
			mpMsgBuffer = (CEiMsgMessage*)(mpQueueHead + 1);
			luResult = ERROR_SUCCESS;
		}

		ReleaseMutex(mhMutex);
	}

	return luResult;
}

// 增加一条消息，返回ERROR_SUCCESS，或者错误码
template<class CEiMsgMessage>
ULONG CEiMsgQueue<CEiMsgMessage>::Push(
	const CEiMsgMessage& nrMsg
)
{
	ULONG luResult = ERROR_SUCCESS;

	if (mbStop != FALSE || WaitForSingleObject(mhMutex, INFINITE) != WAIT_OBJECT_0)
		return ERROR_NOT_READY;

	do 
	{
		if (mpQueueHead->MsgCount >= mpQueueHead->MsgCapacity)
		{
			luResult = ERROR_NOT_ENOUGH_MEMORY;
			break;
		}

		if(mpQueueHead->MsgCount > 0)
			mpQueueHead->LastMsg = ((mpQueueHead->LastMsg + 1) % mpQueueHead->MsgCapacity);

		mpMsgBuffer[mpQueueHead->LastMsg] = nrMsg;
		mpMsgBuffer[mpQueueHead->LastMsg].SaveTickCount();

		mpQueueHead->MsgCount++;

		ReleaseSemaphore(mhNewArrived, 1, NULL);
	} while (false);

	ReleaseMutex(mhMutex);

	return luResult;
}

// 取一条消息，返回ERROR_SUCCESS，或者错误码
// 如果消息队列为空，将进入等待
template<class CEiMsgMessage>
ULONG CEiMsgQueue<CEiMsgMessage>::Pop(
	CEiMsgMessage& nrMsg
)
{
	ULONG luResult = ERROR_SUCCESS;

	if (mhNewArrived== NULL || WaitForSingleObject(mhNewArrived,INFINITE) != WAIT_OBJECT_0)
		return ERROR_NOT_READY;

	if (mbStop != FALSE)
		return ERROR_NOT_READY;

	if (WaitForSingleObject(mhMutex, INFINITE) != WAIT_OBJECT_0)
		return ERROR_NOT_READY;

	do
	{
		if (mpQueueHead->MsgCount == 0 || mpQueueHead->FirstMsg >= mpQueueHead->MsgCapacity)
		{
			luResult = ERROR_NOT_READY;
			break;
		}

		nrMsg = mpMsgBuffer[mpQueueHead->FirstMsg];
		mpQueueHead->MsgCount--;

		if (mpQueueHead->MsgCount == 0)
			mpQueueHead->FirstMsg = mpQueueHead->LastMsg = 0;
		else
			mpQueueHead->FirstMsg = ((mpQueueHead->FirstMsg + 1) % mpQueueHead->MsgCapacity);

	} while (false);

	ReleaseMutex(mhMutex);

	return luResult;
}

// 撤回一类消息，将队列中此类消息全部撤回
// 如果调用此函数，需要CEiMsgMessage实现bool IsTypeOf(const CEiMsgMessage& nrRefTo)函数，改函数返回true表示同类，返回false表示非同类
// 当一个消息被撤回时，会调用CEiMsgMessage的Recall()函数，请在该函数内实现对消息撤回操作
template<class CEiMsgMessage>
void CEiMsgQueue<CEiMsgMessage>::Recall(
	const CEiMsgMessage& nrMsg
)
{
	ULONG luIndex, luCount;
	if (WaitForSingleObject(mhMutex, INFINITE) != WAIT_OBJECT_0)
		return;

	do
	{
		if (mpQueueHead->MsgCount == 0 || mpQueueHead->FirstMsg >= mpQueueHead->MsgCapacity)
		{
			break;
		}

		luIndex = mpQueueHead->FirstMsg;
		luCount = mpQueueHead->MsgCount;

		while (luCount > 0)
		{
			if (mpMsgBuffer[luIndex].IsTypeOf(nrMsg) != false)
				mpMsgBuffer[luIndex].Recall();

			luIndex = ((luIndex + 1) % mpQueueHead->MsgCapacity);
			luCount--;
		}
	} while (false);

	ReleaseMutex(mhMutex);
}

// 获得当前未取走的最早一条消息已发出时间，单位为毫秒
template<class CEiMsgMessage>
ULONGLONG CEiMsgQueue<CEiMsgMessage>::GetMaxElapsedTimeOfMsg(void)
{
	ULONGLONG TickCount = 0;

	if (mbStop != FALSE)
		return 0;

	if (WaitForSingleObject(mhMutex, INFINITE) != WAIT_OBJECT_0)
		return 0;

	do
	{
		if(mpQueueHead == NULL || mpMsgBuffer == NULL)
			break;

		if (mpQueueHead->MsgCount == 0 || mpQueueHead->FirstMsg >= mpQueueHead->MsgCapacity)
			break;

		CEiMsgMessage& nrMsg = mpMsgBuffer[mpQueueHead->FirstMsg];

		TickCount = nrMsg.GetElapsedTick();

	} while (false);

	ReleaseMutex(mhMutex);

	return TickCount;
}


// 获取消息数
template<class CEiMsgMessage>
ULONG CEiMsgQueue<CEiMsgMessage>::GetCount(void)
{
	ULONG luCount = 0;

	if (WaitForSingleObject(mhMutex, INFINITE) == WAIT_OBJECT_0)
	{
		luCount = mpQueueHead->MsgCount;
		ReleaseMutex(mhMutex);
	}

	return luCount;
}

// 关闭
template<class CEiMsgMessage>
void CEiMsgQueue<CEiMsgMessage>::RealseQueue(void)
{
	if(mhNewArrived!=NULL)
	{ 
		SetEvent(mhNewArrived);
		CloseHandle(mhNewArrived);
		mhNewArrived = NULL;
	}

	mbStop = TRUE;
	CloseHandle(mhMutex);
	mhMutex = NULL;
}




// 监听类，初始化后，自动建立一个监听线程，当有消息到来时，会调用初始化时设立的回调函数
template<class CEiMsgMessage>
class CEiMsgQueueListener
{
typedef void (__stdcall *PEI_MSG_LISTENER)(CEiMsgMessage& nrMsg,void* npContext);
private:
	PEI_MSG_LISTENER mpCallBack;
	void* mpContext;
	HANDLE mhListenerThread;
	CEiMsgQueue<CEiMsgMessage> moQueue;
	
	static ULONG WINAPI Listener(CEiMsgQueueListener<CEiMsgMessage>* npThis);

public:
	CEiMsgQueueListener() {
		mhListenerThread = NULL;
	}
	~CEiMsgQueueListener(){}

	// 监听初始化
	ULONG CreateListener(
		const wchar_t* nusMutexName, 
		const wchar_t* nusSemaphoreName,
		void* npBuffer,
		ULONG nuBufferSize,
		PEI_MSG_LISTENER npCallBack,
		void* npContext
	);

	// 停止监听
	ULONG Stop(void);

	// 给自己发送一条消息，不能再监听线程和监听回调函数中调用!!!
	ULONG PostMsgToListener(const CEiMsgMessage& ncrMsg) {
		return moQueue.Push(ncrMsg);
	}

	// 召回发送给自身监听线程的一类消息，将队列中此类消息全部撤回
	// 如果调用此函数，需要CEiMsgMessage实现bool IsTypeOf(const CEiMsgMessage& nrRefTo)函数，改函数返回true表示同类，返回false表示非同类
	// 当一个消息被撤回时，会调用CEiMsgMessage的Recall()函数，请在该函数内实现对消息撤回操作
	void Recall(
		const CEiMsgMessage& nrMsg
	)
	{
		moQueue.Recall(nrMsg);
	}


};

template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::CreateListener(
	const wchar_t* nusMutexName,
	const wchar_t* nusSemaphoreName,
	void* npBuffer,
	ULONG nuBufferSize,
	PEI_MSG_LISTENER npCallBack,
	void* npContext
	)
{
	ULONG luResult = moQueue.CreateQueue(nusMutexName, nusSemaphoreName, npBuffer, nuBufferSize, TRUE);
	if (luResult != ERROR_SUCCESS)
		return luResult;

	// 启动监听线程
	mpCallBack = npCallBack;
	mpContext = npContext;

	mhListenerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Listener, (LPVOID)this, 0, NULL);
	if (mhListenerThread == NULL)
		return GetLastError();

	return ERROR_SUCCESS;
}

template<class CEiMsgMessage>
ULONG WINAPI CEiMsgQueueListener<CEiMsgMessage>::Listener(CEiMsgQueueListener<CEiMsgMessage>* npThis)
{
	ULONG luResult;
	CEiMsgMessage loMsg;

	while (true)
	{
		try
		{
			while (npThis->moQueue.GetStopState() == FALSE)
			{
				luResult = npThis->moQueue.Pop(loMsg);
				if (luResult == ERROR_SUCCESS)
					npThis->mpCallBack(loMsg, npThis->mpContext);
			}
		}
		catch (...)
		{
			
		}
	}

	return ERROR_SUCCESS;
}

// 停止监听
template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::Stop(void)
{
	moQueue.RealseQueue();

	return ERROR_SUCCESS;
}


// 消息连接器，用于连接到消息通道，而后就可以给消息通道发送消息了
template<class CEiMsgMessage>
class CEiMsgQueueConnector
{
private:
	CEiMsgQueue<CEiMsgMessage> moQueue;

public:
	CEiMsgQueueConnector() {
	}
	~CEiMsgQueueConnector() {
	}

	// 连接器初始化
	ULONG CreateConnector(
		const wchar_t* nusMutexName,
		const wchar_t* nusSemaphoreName,
		void* npBuffer,
		ULONG nuBufferSize
	) {
		return moQueue.CreateQueue(nusMutexName, nusSemaphoreName, npBuffer, nuBufferSize, FALSE);
	}

	// 插入一条消息
	ULONG PostMsg(const CEiMsgMessage& ncrMsg) {
		return moQueue.Push(ncrMsg);
	}

	// 撤回一类消息，将队列中此类消息全部撤回
	// 如果调用此函数，需要CEiMsgMessage实现bool IsTypeOf(const CEiMsgMessage& nrRefTo)函数，改函数返回true表示同类，返回false表示非同类
	// 当一个消息被撤回时，会调用CEiMsgMessage的Recall()函数，请在该函数内实现对消息撤回操作
	void Recall(
		const CEiMsgMessage& nrMsg
	)
	{
		moQueue.Recall(nrMsg);
	}

	// 获得当前未取走的最早一条消息已发出时间，单位为毫秒
	ULONGLONG GetMaxElapsedTimeOfMsg(void){
		return moQueue.GetMaxElapsedTimeOfMsg();
	}
};
