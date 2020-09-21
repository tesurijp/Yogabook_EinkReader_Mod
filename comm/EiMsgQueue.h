/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once
// 本文件旨在建立单向跨进程的消息机制
// 通过一块共享内存和一个互斥对象名称建立的跨进程消息队列，能够让两个进程间进行通讯。
// 消息队列由监听进程首先初始化，而后发信进程连接到消息队列；监听进程在初始化时需要提供一个回调函数，这个回调函数将在监听线程中被调用。
// 同一块内存只能有一个监听对象

#pragma pack(4)

#include <vector>
#include <map>
#include <Windows.h>

#define RECIVE_DATA_BUFFER_LENGTH 1024*1024*3

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
		void* npBuffer,
		ULONG nuBufferSize
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
	void Recall(const CEiMsgMessage& nrMsg);
	void Recall(const ULONG& nuMsgId);

	// 获取消息数
	ULONG GetCount(void);

	// 关闭
	void RealseQueue(void);
};

template<class CEiMsgMessage>
ULONG CEiMsgQueue<CEiMsgMessage>::CreateQueue(
	void* npBuffer,
	ULONG nuBufferSize
)
{
	ULONG luResult = ERROR_SUCCESS;

	if (nuBufferSize < sizeof(EI_MSGQUE_HEAD) || npBuffer == NULL)
		return ERROR_NOT_ENOUGH_MEMORY;

	// 建立消息旗语
	mhNewArrived = CreateSemaphore(NULL, 0, 10000, NULL);
	if (mhNewArrived == NULL)
		return ERROR_OPEN_FAILED;

	// 建立互斥对象
	mhMutex = CreateMutex(NULL, TRUE, NULL);
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
void CEiMsgQueue<CEiMsgMessage>::Recall(const CEiMsgMessage& nrMsg)
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

// 撤回一类消息，将队列中此类消息全部撤回
template<class CEiMsgMessage>
void CEiMsgQueue<CEiMsgMessage>::Recall(const ULONG& nuMsgId)
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
			if (mpMsgBuffer[luIndex].IsTypeOf(nuMsgId) != false)
				mpMsgBuffer[luIndex].Recall();

			luIndex = ((luIndex + 1) % mpQueueHead->MsgCapacity);
			luCount--;
		}
	} while (false);

	ReleaseMutex(mhMutex);
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
typedef void (__stdcall *PEI_RECIVE_LISTENER)(const char* npData, ULONG nSize, void* npContext);

typedef struct _READ_THREAD_INFO
{
	CEiMsgQueueListener<CEiMsgMessage>* mpListener;
	HANDLE mhNamedPip;
	HANDLE mhThread;
	BOOL mbAlive;
}READ_THREAD_INFO;

private:
	wchar_t musPipeName[MAX_PATH];
	ULONG muReadBufferLenth;

	PEI_RECIVE_LISTENER mpReciveCallBack;
	PEI_MSG_LISTENER mpCallBack;
	void* mpContext;
	HANDLE mhListenerThread;
	HANDLE mhDispatchThread;
	std::vector<READ_THREAD_INFO*> mvReadThread;
	std::map<ULONG, ULONGLONG> mmTickCountReciveFromApp;

	CEiMsgQueue<CEiMsgMessage> moQueue;

	static ULONG WINAPI ListenThread(CEiMsgQueueListener<CEiMsgMessage>* npThis);
	static ULONG WINAPI DispatchThread(CEiMsgQueueListener<CEiMsgMessage>* npThis);
	static ULONG WINAPI ReadPipeThread(READ_THREAD_INFO* npThread);

public:
	CEiMsgQueueListener() {
		_wcsnset_s(musPipeName, MAX_PATH, 0, MAX_PATH);
		muReadBufferLenth = RECIVE_DATA_BUFFER_LENGTH;
		mhListenerThread = NULL;
		mhDispatchThread = NULL;
		mvReadThread.clear();
	}

	~CEiMsgQueueListener() {
		for (auto threadInfo : mvReadThread)
		{
			CancelIoEx(threadInfo->mhNamedPip, NULL);
			CloseHandle(threadInfo->mhNamedPip);
			WaitForSingleObject(threadInfo->mhThread, INFINITE);
			delete threadInfo;
		}
	}

	// 监听初始化
	ULONG CreateListener(
		const wchar_t* nusPipeName,
		void* npBuffer,
		ULONG nuBufferSize,
		PEI_RECIVE_LISTENER npReciveCallBack,
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
	void Recall(const CEiMsgMessage& nrMsg)
	{
		moQueue.Recall(nrMsg);
	}

	void Recall(const ULONG& nuMsgId)
	{
		moQueue.Recall(nuMsgId);
	}

	// 监听连接命名管道
	ULONG DoListen();

	// 创建读数据命名管道
	ULONG CreateReadPipe(HANDLE &nhPipe);

	// 从命名管道读数据
	ULONG ReadPipe(READ_THREAD_INFO*);

	// 更新从APP接收消息的时间戳
	void UpdateTickCountWithAppId(ULONG nuAppId)
	{
		mmTickCountReciveFromApp[nuAppId] = GetTickCount64();
	}

	// 获得当前未取走的最早一条消息已发出时间，单位为毫秒
	ULONGLONG GetMaxElapsedTimeOfMsg(ULONG nuAppId) {
		auto itr = mmTickCountReciveFromApp.find(nuAppId);
		if (itr != mmTickCountReciveFromApp.end())
			return GetTickCount64() - mmTickCountReciveFromApp[nuAppId];
		else
			return 0;
	}
};

template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::CreateListener(
	const wchar_t* nusPipeName,
	void* npBuffer,
	ULONG nuBufferSize,
	PEI_RECIVE_LISTENER npReciveCallBack,
	PEI_MSG_LISTENER npCallBack,
	void* npContext
	)
{
	wcscpy_s(musPipeName, MAX_PATH, nusPipeName);
	
	ULONG luResult = moQueue.CreateQueue(npBuffer, nuBufferSize);
	if (luResult != ERROR_SUCCESS)
		return luResult;

	// 启动监听线程
	mpReciveCallBack = npReciveCallBack;
	mpCallBack = npCallBack;
	mpContext = npContext;

	mhListenerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThread, (LPVOID)this, 0, NULL);
	if (mhListenerThread == NULL)
		return GetLastError();

	mhDispatchThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DispatchThread, (LPVOID)this, 0, NULL);
	if (mhDispatchThread == NULL)
		return GetLastError();

	return ERROR_SUCCESS;
}

template<class CEiMsgMessage>
ULONG WINAPI CEiMsgQueueListener<CEiMsgMessage>::ListenThread(CEiMsgQueueListener<CEiMsgMessage>* npThis)
{
	return npThis->DoListen();
}

template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::DoListen()
{
	try
	{
		while (TRUE)
		{
			//创建命名管道等待client端连接
			HANDLE hPipe = INVALID_HANDLE_VALUE;
			if (CreateReadPipe(hPipe) == ERROR_SUCCESS)
			{
				if (ConnectNamedPipe(hPipe, NULL))
				{
					//移除不再活跃的接收数据线程
					auto itr = mvReadThread.begin();
					while (itr != mvReadThread.end())
					{
						if (!(*itr)->mbAlive)
						{
							CloseHandle((*itr)->mhNamedPip);
							itr = mvReadThread.erase(itr);
						}
						else
							itr++;
					}

					READ_THREAD_INFO* pReadThread = new READ_THREAD_INFO();
					pReadThread->mpListener = this;
					pReadThread->mhNamedPip = hPipe;


					HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)pReadThread, 0, NULL);
					if (hThread != NULL)
					{
						pReadThread->mhThread = hThread;
						pReadThread->mbAlive = TRUE;

						mvReadThread.push_back(pReadThread);
					}
					else
					{
						delete pReadThread;
						CloseHandle(hPipe);
					}
				}
				else
				{
					CloseHandle(hPipe);
				}
			}
		}
	}
	catch (...)
	{
		OutputDebugString(L"CEiMsgQueueListener -- DoListen exception\n");
		//Log exception
	}

	return ERROR_SUCCESS;
}

template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::CreateReadPipe(HANDLE &nhPipe)
{
	//需要设置权限，否则服务创建的对象，普通进程无法打开
	SECURITY_DESCRIPTOR lsd;
	InitializeSecurityDescriptor(&lsd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&lsd, TRUE, (PACL)NULL, FALSE);
	SECURITY_ATTRIBUTES	lsa;
	lsa.nLength = sizeof(SECURITY_ATTRIBUTES);
	lsa.bInheritHandle = TRUE;
	lsa.lpSecurityDescriptor = &lsd;

	nhPipe = CreateNamedPipe(
		musPipeName,
		PIPE_ACCESS_INBOUND,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
		PIPE_UNLIMITED_INSTANCES,
		0,
		muReadBufferLenth,
		NMPWAIT_USE_DEFAULT_WAIT,
		&lsa);

	if (INVALID_HANDLE_VALUE == nhPipe)
	{
		return GetLastError();
	}

	return ERROR_SUCCESS;
}

template<class CEiMsgMessage>
ULONG WINAPI CEiMsgQueueListener<CEiMsgMessage>::DispatchThread(CEiMsgQueueListener<CEiMsgMessage>* npThis)
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

template<class CEiMsgMessage>
ULONG WINAPI CEiMsgQueueListener<CEiMsgMessage>::ReadPipeThread(READ_THREAD_INFO* npThread)
{
	return npThread->mpListener->ReadPipe(npThread);
}

template<class CEiMsgMessage>
ULONG CEiMsgQueueListener<CEiMsgMessage>::ReadPipe(READ_THREAD_INFO* npThread)
{
	char* pBuffer = new char[muReadBufferLenth];
	DWORD dwNumOfBytes;

	BOOL fSuccess = FALSE;
	do
	{
		dwNumOfBytes = 0;

		fSuccess = ReadFile(
			npThread->mhNamedPip,    // pipe handle 
			pBuffer,    // buffer to receive reply 
			muReadBufferLenth,  // size of buffer
			&dwNumOfBytes,  // number of bytes read 
			NULL);    // not overlapped 

		if (!fSuccess)
		{
			delete[] pBuffer;
			pBuffer = NULL;
			
			break;
		}

		mpReciveCallBack(pBuffer, dwNumOfBytes, mpContext);

	} while (fSuccess);

	npThread->mbAlive = FALSE;

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
	HANDLE mhPipe;
	ULONGLONG mluLastSendTickCount;

public:
	CEiMsgQueueConnector() {
		mhPipe = INVALID_HANDLE_VALUE;
		mluLastSendTickCount = GetTickCount64();
	}
	~CEiMsgQueueConnector() {
		if (mhPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(mhPipe);
		}
	}

	HANDLE GetPipeHandle() {
		return mhPipe;
	}

	// 连接器初始化
	ULONG CreateConnector(const wchar_t* nusPipeName)
	{
		WORD wTryTimes = 0;
		do
		{
			if (WaitNamedPipe(nusPipeName, INFINITE))
			{
				mhPipe = CreateFile(
					nusPipeName,
					GENERIC_WRITE,
					0,
					NULL,
					OPEN_EXISTING,
					0,
					NULL);

				if (mhPipe != INVALID_HANDLE_VALUE)
				{
					DWORD dwMode = PIPE_READMODE_MESSAGE;
					BOOL fSuccess = SetNamedPipeHandleState(
						mhPipe,    // pipe handle 
						&dwMode,  // new pipe mode 
						NULL,     // don't set maximum bytes 
						NULL);    // don't set maximum time 

					if (fSuccess)
					{
						break;
					}
				}
			}
			wTryTimes++;

			if (wTryTimes > 2)
			{
				return GetLastError();
			}

		} while (TRUE);

		return ERROR_SUCCESS;
	}

	// 发送一条消息
	ULONG PostMsg(const CEiMsgMessage& ncrMsg, const ULONG& nuMsgLen) {
		mluLastSendTickCount = GetTickCount64();

		if (mhPipe != INVALID_HANDLE_VALUE)
		{
			DWORD cbWritten = 0;
			BOOL fSuccess = WriteFile(
				mhPipe,                 // pipe handle
				(LPVOID)&ncrMsg,        // message
				nuMsgLen,  // message length
				&cbWritten,             // bytes written
				NULL);

			if (!fSuccess)
			{
				return GetLastError();
			}
		}
		else
		{
			return ERROR_INVALID_HANDLE;
		}

		return ERROR_SUCCESS;
	}

	// 发送一条消息
	ULONG PostMsg(const char* nsData, DWORD dwDataLength) {
		mluLastSendTickCount = GetTickCount64();

		if (mhPipe != INVALID_HANDLE_VALUE)
		{
			DWORD cbWritten;
			BOOL fSuccess = WriteFile(
				mhPipe,         // pipe handle
				(LPVOID)nsData, // message
				dwDataLength,   // message length
				&cbWritten,     // bytes written
				NULL);

			if (!fSuccess)
			{
				return GetLastError();
			}
		}
		else
		{
			return ERROR_INVALID_HANDLE;
		}

		return ERROR_SUCCESS;
	}

	// 获得当前未取走的最早一条消息已发出时间，单位为毫秒
	ULONGLONG GetMaxElapsedTimeOfMsg(void){
		return GetTickCount64() - mluLastSendTickCount;
	}
};
