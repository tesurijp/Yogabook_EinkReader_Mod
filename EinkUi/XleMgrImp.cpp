/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"
#include "ElementImp.h"




#define LOSHORT(_X) ((LONG)(SHORT)LOWORD(_X))
#define HISHORT(_X) ((LONG)(SHORT)HIWORD(_X))

DEFINE_BUILTIN_NAME(CXelManager)




CXelManager::CXelManager()
{
	moIteratorRoot.gpElementManager = this;
	moIteratorRoot.mpElement = &moElementRoot;
	moIteratorRoot.mpParent = &moIteratorRoot;	// 父元素等于自身

	moIteratorVerification.Insert(&moIteratorRoot);

	mpDesktopIterator = NULL;
	mhMessageInserted = NULL;
	mpMouseFocus = NULL;
	mpActivatedElement = NULL;
	mpMouseMoveOn = NULL;
	mbDragging = false;
	mbHoverTest= false;
	mpKbFocus = NULL;
	mbXuiDragDrop = false;
	muTickOnTipShow = 0;
	mpTipOwner = NULL;
	mbTipHide = true;
	mbExitXui = false;
	mlCleanHumanInput = 0;

	mpDraggingElement = NULL;
	mpDragMsgReceiver = NULL;
	mpRecentAskedDrop =NULL;
	mpLastAccepter = NULL;
	mlMsgAllocated = 0;
	mbTrackMouse = false;

	mlProbeMode = -1;
	mlProbePass = 0;
	mdTopLeftInPanel = { 0.0f,0.0f };
}


CXelManager::~CXelManager()
{
	// ??? 释放迭代器树
	if (mpDesktopIterator != NULL)
		mpDesktopIterator->KRelease();

	while(moFreeMessagePool.Size() > 0)
	{
		IEinkuiMessage* lpMsgIntf = moFreeMessagePool.Top();
		moFreeMessagePool.Pop();

		CMM_SAFE_RELEASE(lpMsgIntf);
	}
}

// 设置根元素的Widget属性，即System Widget
void CXelManager::SetRootWidget(IXsWidget* npWidget)
{
	moIteratorLock.Enter();
	moIteratorRoot.mpWidget = npWidget;
	moIteratorLock.Leave();
}


ULONG CXelManager::InitOnCreate(void)
{
	mhMessageInserted = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	if(mhMessageInserted == NULL)
		return ERESULT_UNSUCCESSFUL;

	// 允许根对象接受消息
	moIteratorRoot.SetFlags(EITR_FLAG_INIT);

	return ERESULT_SUCCESS;
}

// 等待消息旗语，内部将调用WaitForSingleObject等待旗语，返回值同WaitForSingleObject一致
ULONG CXelManager::WaitMessageReach(ULONG nuMilliseconds)
{
	return WaitForSingleObject(mhMessageInserted,nuMilliseconds);
}


// 向系统管理器注册一个Element，返回迭代器对象；失败返回NULL
// 成功返回的接口对象，不使用时需要释放
IEinkuiIterator* __stdcall CXelManager::RegisterElement(
	IN IEinkuiIterator* npParent,	// 父元素的迭代器
	IN IXsElement* npElement,	// 待注册的子元素
	IN ULONG nuEID	// 本元素的EID，在同一个父元素下，各子元素的EID必须唯一，如果不关心EID，请设置=0，系统自动分配
	)
{
	CXuiIterator* lpNewItr=NULL;
	IEinkuiIterator* lpReturn=NULL;
	ERESULT luResult=ERESULT_UNSUCCESSFUL;
	IXsWidget* lpCrtWidget;

	LockIterators();

	do 
	{
		// 检查父迭代器是否合法
		if(moIteratorVerification.Find(npParent)== moIteratorVerification.End())
			break;

		CXuiIterator* lpNewItr = CXuiIterator::CreateInstance();
		if(lpNewItr == NULL)
			break;

		lpCrtWidget = CEinkuiSystem::gpXuiSystem->GetCurrentWidget();
		lpNewItr->mpParent = dynamic_cast<CXuiIterator*>(npParent);
		lpNewItr->mpElement = npElement;
		lpNewItr->mpWidget = lpCrtWidget;
		lpNewItr->muEID = nuEID;
		lpNewItr->mlTabOrder = npElement->GetTabOrder();
		lpNewItr->mlZOrder = npElement->GetZOrder();

		CXuiIterator* lpFarther = dynamic_cast<CXuiIterator*>(npParent);

		// 先插入尾部
		if(ERESULT_FAILED(lpFarther->AddSubElement(lpNewItr)))
			break;

		// 加入验证库
		moIteratorVerification.Insert(lpNewItr);

		// 检查是不是Widget的homepage
		if(lpCrtWidget->GetHomePage() == NULL)
		{	// 是当前Widget注册的第一个Element，那一定是homepage
			CXsWidget* lpWidgetObj = dynamic_cast<CXsWidget*>(lpCrtWidget);
			lpWidgetObj->SetHomePage(lpNewItr);
			lpNewItr->SetWidgetHomeFlag(true);
		}

		UnlockIterators();

		lpReturn = dynamic_cast<IEinkuiIterator*>(lpNewItr);

		// 发送被建立消息
		luResult = CExMessage::PostMessage(lpNewItr,NULL,EMSG_CREATE,lpReturn,EMSG_POSTTYPE_REVERSE);

		LockIterators();

		// 不成功
		if(luResult!=ERESULT_SUCCESS)
			lpReturn = NULL;
		else
		{
			// 检查是不是第一个客户Element，设置为桌面
			if(mpDesktopIterator == NULL)
			{
				//Trace_Point(27373);//保存DeskTop Page
				mpDesktopIterator = dynamic_cast<CXuiIterator*>(lpReturn);
				mpDesktopIterator->KAddRefer();
			}
		}
	} while (false);

	UnlockIterators();

	CMM_SAFE_RELEASE(lpNewItr);

	return lpReturn;
}

// 向系统管理器注销一个Element，此功能仅应该被待注销Element自身或者XUI系统调用；这个方法已经废弃
ERESULT __stdcall CXelManager::UnregisterElement(
	IN IEinkuiIterator* npElementIterator	// 该元素对应的迭代器
	)
{
	return  ERESULT_UNSUCCESSFUL;
}

// 销毁元素
ERESULT CXelManager::DestroyElement(
	IN IEinkuiIterator* npElementIterator	// 该元素对应的迭代器
	)
{
	CXuiIterator* lpFartherObj;
	CXuiIterator* lpToDestroy;

	if(npElementIterator == NULL || npElementIterator->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
		return ERESULT_ITERATOR_INVALID;

	// 首先获取父节点
	lpFartherObj = dynamic_cast<CXuiIterator*>(npElementIterator->GetParent());
	lpToDestroy = dynamic_cast<CXuiIterator*>(npElementIterator);

	// 将本节点自身从父对象上移除
	if(lpFartherObj != NULL)
		lpFartherObj->RemoveSubElement((CXuiIterator*)npElementIterator);

	// 销毁本元素和子元素
	DestroyElementSubTree(npElementIterator);

	//  在DestroyElementSubTree内部释放当前的npElementIterator

	return ERESULT_SUCCESS;
}


// 递归销毁某个元素，首先销毁父元素，后销毁子元素
void CXelManager::DestroyElementSubTree(IEinkuiIterator* npToDestroy)
{
	ERESULT luResult= ERESULT_SUCCESS;
	IEinkuiIterator* lpSubElement;
	CXuiIterator* lpToDestroy;

	lpToDestroy = dynamic_cast<CXuiIterator*>(npToDestroy);
	if(lpToDestroy == NULL)
		return;

	// 给目标发送销毁消息
	if(CExMessage::SendMessage(lpToDestroy,NULL,EMSG_DESTROY,CExMessage::DataInvalid)==ERESULT_ITERATOR_INVALID)
		return;

	// Iterator是否有效
	if(luResult == ERESULT_SUCCESS)
	{
		//// 递归调用,注销所有子元素

		// Modified by Archims Jul.8,2012 原来的算法有诸多的问题
		// 但新的方法容错性仍然不高，如果错误释放的对象是当前焦点，仍然会导致UI系统崩溃
		// fixed by ax 有可能已经解决了这个问题，现在还没有做更多的验证分析
		do
		{
			LockIterators();
			try
			{
				lpSubElement = lpToDestroy->GetSubElementByZOder(0);
			}
			catch (...)
			{
				lpSubElement = NULL;
				lpToDestroy = NULL;	//不知道发生了什么，就不要继续使用这个变量了
			}
			UnlockIterators();

			try
			{
				if(lpSubElement != NULL)
					lpSubElement->Close();
			}
			catch (...)
			{
				lpSubElement = NULL;
			}
		}while(lpSubElement != NULL);

	}

	//////////////////////////////////////////////////////////////////////////
	// 下面的代码做如下修改，直接从验证库中删除；源代码存在严重错误，如果再次降低引用并没有删除对象，则对象将没有机会从验证库中删除了
	//int EleRef = 0;
	//// 删除自身
	//if(lpToDestroy != NULL)
	//{
	//	lpToDestroy->SetAsDeleted();
	//	EleRef = lpToDestroy->KRelease();	// 当Iterator被释放时，将会释放Element对象，也将释放对父节点的引用
	//}

	//if (EleRef == 0)
	//{
	//	LockIterators();
	//	// 从验证库中移除
	//	moIteratorVerification.Remove(lpToDestroy);
	//	UnlockIterators();
	//}
	// 删除自身
	if (lpToDestroy != NULL)
	{
		LockIterators();
		// 从验证库中移除
		moIteratorVerification.Remove(lpToDestroy);
		UnlockIterators();

		lpToDestroy->SetAsDeleted();
		lpToDestroy->KRelease();	// 当Iterator被释放时，将会释放Element对象，也将释放对父节点的引用
	}

}


// 启动一个Iterator的消息接收
void CXelManager::StartMessageReceiver(IEinkuiIterator* npIterator)
{
	// 找到自己的Create而后去执行；那么为什么不等到现在才发送Create，而要在此之前就发送一条尚不执行的消息呢，是为了防止本函数不被调用而漏掉Create消息
	IEinkuiMessage* lpMsg;

	lpMsg = moNormalMessages.GetMessage(EMSG_CREATE,npIterator);
	if(lpMsg != NULL)
	{
		ProcessNextMessage(lpMsg);
	}
}


// 验证一个Iterator是否有效，返回ERESULT_SUCCESS有效，返回ERESULT_ITERATOR_INVALID无效
ERESULT __stdcall CXelManager::VerifyIterator(
	IN IEinkuiIterator* npIterator	// 迭代器
	)
{
	ERESULT luResult;

	moIteratorLock.Enter();

	luResult = (moIteratorVerification.Find(npIterator) == moIteratorVerification.End())?ERESULT_ITERATOR_INVALID:ERESULT_SUCCESS;

	moIteratorLock.Leave();

	return luResult;
}


// 在对象管理器中查找一个Element，返回该Element的迭代器对象；失败返回NULL
// 成功返回的接口对象，不使用时需要释放； 如果经常需要通过元素获得它的注册迭代器，请保存迭代器的指针，因为调用本方法使用全树遍历查找获取迭代器对象，耗时较大；
IEinkuiIterator* __stdcall CXelManager::FindElement(
	IN IXsElement* npElement
	)
{
	LockIterators();

	CXuiIterator* lpItrObj = moIteratorRoot.SeekIteratorInChild(npElement);

	UnlockIterators();

	return lpItrObj;
}

// 获得根元素；如果是为了给对象管理器发送消息，也可以直接使用NULL指针表示对象管理器
// 成功返回的接口对象，不使用时需要释放
IEinkuiIterator* __stdcall CXelManager::GetRootElement(void)
{
	return &moIteratorRoot;
}

// 获得菜单页，所有的菜单都建立在这个页
IEinkuiIterator* __stdcall CXelManager::GetMenuPage(void)
{
	return NULL;
}

// 获得ToolTip平面，这个最高的页
IEinkuiIterator* __stdcall CXelManager::GetToolTipPage(void)
{
	return NULL;
}


// 获得桌面元素；桌面元素在启动XUI引擎时指定，桌面元素实现具体应用的全局功能，如：Idealife的全局应用由"Idealife"元素类的实例提供，
// 通过给它发Idealife的应用消息执行Idealife的系统调用
// 成功返回的接口对象，不使用时需要释放
IEinkuiIterator* __stdcall CXelManager::GetDesktop(void)
{
	return mpDesktopIterator;
}

// 重新设定父元素，nbZTop==true设置于Zoder顶层，false设置在底层
ERESULT __stdcall CXelManager::SetParentElement(IEinkuiIterator* npParent,IEinkuiIterator* npChild,bool nbZTop)
{
	CXuiIterator* lpMoveTo;
	CXuiIterator* lpMoveFrom;
	CXuiIterator* lpChild;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	// 检查是否处于操作线程
	if(CEinkuiSystem::gpXuiSystem->IsRunningInOperationThread()==false)
		return ERESULT_WRONG_THREAD;

	if(npParent->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false || npChild->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
		return ERESULT_ITERATOR_INVALID;

	lpMoveTo = dynamic_cast<CXuiIterator*>(npParent);
	lpMoveFrom = dynamic_cast<CXuiIterator*>(npChild->GetParent());
	lpChild = dynamic_cast<CXuiIterator*>(npChild);

	if(lpMoveFrom == NULL)
		return ERESULT_ELEMENT_INVALID;

	LockIterators();
	
	do 
	{
		// 检查能否从当前父对象移除
		// 如果子对象是父对象的增效器，不能移动
		if(lpMoveFrom->GetEnhancer() == npChild)
		{
			luResult = ERESULT_ACCESS_CONFLICT;
			break;
		}

		// 从父对象移除
		lpMoveFrom->RemoveSubElement(lpChild);

		// 插入新的父对象，认贼为父了
		lpMoveTo->AddSubElement(lpChild);

		// 完成
		luResult = ERESULT_SUCCESS;
	} while (false);

	UnlockIterators();

	if(ERESULT_SUCCEEDED(luResult))
		CEinkuiSystem::gpXuiSystem->UpdateView();

	return luResult;
}

// 分配一个消息
// 返回的消息，处理完毕需要释放
IEinkuiMessage* __stdcall CXelManager:: AllocateMessage(void)
{
	CXuiMessage* lpMsg = NULL;
	
	moFreeMessageLock.Enter();

	try
	{
		if(moFreeMessagePool.Size() > 0)
		{
			//if(Trace_Count(2,5)<5)
				//Trace_Point(29417);// 从空闲池分配消息

			IEinkuiMessage* lpMsgIntf = moFreeMessagePool.Top();
			moFreeMessagePool.Pop();

			if(lpMsgIntf != NULL && (lpMsg = dynamic_cast<CXuiMessage*>(lpMsgIntf)))
			{
				lpMsg->AddRefer();
			}
		}
	}
	catch (...)
	{
	}

	moFreeMessageLock.Leave();

	if(lpMsg == NULL)
	{
		//if(Trace_Count(3,5)<5)
			//Trace_Point(24675);// 新分配消息
		lpMsg = CXuiMessage::CreateInstance();
		lpMsg->AddRefer();

		InterlockedIncrement(&mlMsgAllocated);
		//if(mlMsgAllocated > 1000)
		//{
		//	IEinkuiMessage* lpNext;
		//	wchar_t lswText[MAX_PATH];
		//	int liIndex = moNormalMessages.Front();

		//	__asm int 3;

		//	for(;liIndex > 0;liIndex = moNormalMessages.Next(liIndex))
		//	{
		//		lpNext = moNormalMessages.GetEntry(liIndex);
		//		StringCchPrintf(lswText,MAX_PATH,L"Target=%s,Send=%s,Type=%8d,Maj=%02d,Min=%02d\n",
		//			lpNext->GetDestination()!=NULL?lpNext->GetDestination()->GetElementObject()->GetType():L"NULL",
		//			lpNext->GetMessageSender()!=NULL?lpNext->GetMessageSender()->GetElementObject()->GetType():L"NULL",
		//			LMSG_GET_TYPE(lpNext->GetMessageID()),
		//			LMSG_GET_MJNUM(lpNext->GetMessageID()),
		//			LMSG_GET_MNNUM(lpNext->GetMessageID()));

		//		OutputDebugString(lswText);
		//	}

		//}
	}

	return lpMsg;
}

// 分配一个消息，初始化相关参数
// 返回的消息，处理完毕需要释放
IEinkuiMessage* __stdcall CXelManager::AllocateMessage(
	IN ULONG nuMsgID,	// 消息编码
	IN const void* npInputBuffer,	// 输入数据的缓冲区
	IN int niInputSize,	// 输入数据的大小
	OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
	IN int niOutputSize,	// 输出缓冲区大小
	IN bool nbInputVolatile	// 输入缓冲区是否是易失的，参见IXuiMessage::SetInputData获得更多信息
	)
{
	IEinkuiMessage* lpMsgIntf = AllocateMessage();
	CXuiMessage* lpMsg;

	if(lpMsgIntf == NULL || (lpMsg = dynamic_cast<CXuiMessage*>(lpMsgIntf))==NULL)
		return NULL;

	lpMsg->SetMessageID(nuMsgID);
	if(npInputBuffer != NULL)
		lpMsg->SetInputData(npInputBuffer,niInputSize,nbInputVolatile);

	if(npOutputBuffer != NULL)
		lpMsg->SetOutputBuffer(npOutputBuffer,niOutputSize);

	return lpMsg;
}

// 供消息对象本身调用，释放控制
void CXelManager::ReleaseMessage(IEinkuiMessage* npMsg)
{
	moFreeMessageLock.Enter();

	try
	{
		if(moFreeMessagePool.Size() < ELMGR_MAX_FREE_MESSAGE)
		{
			//if(Trace_Count(4,5)<5)
				//Trace_Point(31609);// 保存到空闲池

			CXuiMessage* lpMsg = dynamic_cast<CXuiMessage*>(npMsg);

			lpMsg->Reuse();
			moFreeMessagePool.Push(npMsg);

			npMsg = NULL;
		}
	}
	catch (...)
	{
	}
	moFreeMessageLock.Leave();

	if(npMsg != NULL)
	{
		//if(Trace_Count(5,5)<5)
			//Trace_Point(31092);// 释放消息

		npMsg->Release();
		InterlockedDecrement(&mlMsgAllocated);
	}
}


// 给指定元素发送一条消息，发送模式是Send
ERESULT __stdcall CXelManager::SendMessage(
	IEinkuiIterator* npDestElement,	// 接收消息的目标元素
	IEinkuiMessage* npMsg
	)
{
	ERESULT luResult= ERESULT_UNSUCCESSFUL;

	//if(mbExitXui != false && npMsg->GetMessageID() != EMSG_QUIT_XUI) discarded by AX 2013.1.21 check it only if is running in non-ui thread
	//	return ERESULT_UNEXPECTED_CALL;

	if(npMsg == NULL || npMsg->IsKindOf(GET_BUILTIN_NAME(CXuiMessage))==false)
		return ERESULT_ELEMENT_INVALID;

	CXuiMessage* lpMsgObj = dynamic_cast<CXuiMessage*>(npMsg);

	if(npDestElement==NULL)
		npDestElement = &moIteratorRoot;

	// 检查是不是同一个线程，如果不是同一个线程则需要线程同步
	if(CEinkuiSystem::gpXuiSystem->IsRunningInOperationThread()==false)
	{
		// 判断XUI系统是否已经准备退出，处于销毁过程中时，不在接收其他线程的消息
		if(mbExitXui != false && npMsg->GetMessageID() != EMSG_QUIT_XUI)
			return ERESULT_UNEXPECTED_CALL;
		
		lpMsgObj->mhCompleteEvent = CreateEvent(NULL,true,false,NULL);
		if(lpMsgObj->mhCompleteEvent == NULL)
			return ERESULT_INSUFFICIENT_RESOURCES;
	}

	lpMsgObj->SetFlags(ELMSG_FLAG_SEND);
	lpMsgObj->SetDestination(npDestElement);
	lpMsgObj->AddRefer();

	if(lpMsgObj->mhCompleteEvent != NULL)
	{
		// 消息压入队列头部

		if(moFastMessages.Push_Front(npMsg)!=false)
			luResult = ERESULT_SUCCESS;
		else
			luResult = ERESULT_INSUFFICIENT_RESOURCES;

		if(luResult == ERESULT_SUCCESS)
		{
			ReleaseSemaphore(mhMessageInserted,1,NULL);

			while(WaitForSingleObject(lpMsgObj->mhCompleteEvent,10) == WAIT_TIMEOUT)
			{
				// 为防止windows UI线程中调用本函数向Xui发送消息的时候，同时遇到Xui Operation Thread中调用WinUiCallBack，从而出现死锁，
				// 再次执行Windows UI Callback，接触死锁条件
				if(CEinkuiSystem::gpXuiSystem->RunWindowsUICallback()==false)	// must quit
				{
					luResult = ERESULT_TIMEOUT;
					break;
				}
			}
			CloseHandle(lpMsgObj->mhCompleteEvent);

			luResult = lpMsgObj->GetResult();
		}
	}
	else
	{
		luResult = ProcessNextMessage(npMsg);
	}

	return luResult;
}


// 给指定元素发送一条消息，发送模式是Post
ERESULT __stdcall CXelManager::PostMessage(
	IEinkuiIterator* npDestElement,	// 接收消息的目标元素
	IEinkuiMessage* npMsg,
	IN ULONG nuPostType	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
	)
{
	ERESULT luResult= ERESULT_UNSUCCESSFUL;

	//if(mbExitXui != false && npMsg->GetMessageID() != EMSG_QUIT_XUI) discarded by AX 2013.1.21 check it only if is running in non-ui thread
	//	return ERESULT_UNEXPECTED_CALL;

	if(npMsg == NULL || npMsg->IsKindOf(GET_BUILTIN_NAME(CXuiMessage))==false)
		return ERESULT_ELEMENT_INVALID;

	CXuiMessage* lpMsgObj = dynamic_cast<CXuiMessage*>(npMsg);

	if(npDestElement==NULL)
		npDestElement = &moIteratorRoot;

	if(CEinkuiSystem::gpXuiSystem->IsRunningInOperationThread()==false)
	{
		// 判断XUI系统是否已经准备退出，处于销毁过程中时，不在接收其他线程的消息
		if(mbExitXui != false && npMsg->GetMessageID() != EMSG_QUIT_XUI)
			return ERESULT_UNEXPECTED_CALL;
	}

	lpMsgObj->SetDestination(npDestElement);
	lpMsgObj->AddRefer();
	lpMsgObj->SetFlags(nuPostType);

	switch(nuPostType)
	{
	case EMSG_POSTTYPE_FAST:
		// 压缩快速队列尾部
		luResult = moFastMessages.Push_Back(npMsg)?ERESULT_SUCCESS:ERESULT_INSUFFICIENT_RESOURCES;
		break;
	case EMSG_POSTTYPE_REVERSE:
		// 压入普通任务队列头部
		luResult = moNormalMessages.Push_Front(npMsg)?ERESULT_SUCCESS:ERESULT_INSUFFICIENT_RESOURCES;
		break;
	case EMSG_POSTTYPE_NORMAL:
	case EMSG_POSTTYPE_REDUCE:
		// 压入普通任务队列尾部
		luResult = moNormalMessages.Push_Back(npMsg)?ERESULT_SUCCESS:ERESULT_INSUFFICIENT_RESOURCES;
		break;
	case EMSG_POSTTYPE_QUIT:
		// 压入快速队列头部
		luResult = moFastMessages.Push_Front(npMsg)?ERESULT_SUCCESS:ERESULT_INSUFFICIENT_RESOURCES;
		break;
	//case EMSG_POSTTYPE_REDUCE:
	//	// 压入简约任务队列尾部，并检查重复的消息
	//	{
	//		luResult = /*moReduceMessages*/moNormalMessages.Push_Back(npMsg)?ERESULT_SUCCESS:ERESULT_INSUFFICIENT_RESOURCES;
	//	}
	//	break;
	default:
		luResult = ERESULT_WRONG_PARAMETERS;
	}

	if(luResult == ERESULT_SUCCESS)
		ReleaseSemaphore(mhMessageInserted,1,NULL);

	return luResult;
}

// 简单地给指定元素发送一条消息，发送模式是Send；此函数的返回成功就是消息处理的返回值，错误的原因就不一定是消息处理的返回值，可能是消息发送失败
ERESULT __stdcall CXelManager::SimpleSendMessage(
	IEinkuiIterator* npDestElement,	// 接收消息的目标元素
	IN ULONG nuMsgID,	// 消息编码
	IN const void* npInputBuffer,	// 输入数据的缓冲区
	IN int niInputSize,	// 输入数据的大小
	OUT void* npOutputBuffer,	// 输出缓冲区(返回缓冲区)
	IN int niOutputSize	// 输出缓冲区大小
	)
{
	IEinkuiMessage* lpMsgIntf;
	bool lbVolatile;
	
	// 如果输入数据量足够大，并且缓冲区不冲突，就不复制输入缓冲区了
	if(niInputSize < 64 || npInputBuffer == NULL || ( npOutputBuffer != NULL &&
		((UCHAR*)npInputBuffer+niInputSize) >= (UCHAR*)npOutputBuffer &&
		 (UCHAR*)npInputBuffer				< ((UCHAR*)npOutputBuffer+niOutputSize)) )
		lbVolatile = true;
	else
		lbVolatile = false;


	lpMsgIntf = AllocateMessage(nuMsgID,npInputBuffer,niInputSize,npOutputBuffer,niOutputSize,false);

	if(lpMsgIntf == NULL)
		return ERESULT_UNSUCCESSFUL;

	ERESULT luResult= SendMessage(npDestElement,lpMsgIntf);

	lpMsgIntf->Release();

	return luResult;
}

// 简单地给指定元素发送一条消息，发送模式是Post；无法获得消息处理的返回值；此函数的返回值仅表示消息是否被成功填入消息队列
ERESULT __stdcall CXelManager::SimplePostMessage(
	IEinkuiIterator* npDestElement,	// 接收消息的目标元素
	IN ULONG nuMsgID,	// 消息编码
	IN const void* npInputBuffer,	// 输入数据的缓冲区
	IN int niInputSize,	// 输入数据的大小
	IN ULONG nuPostType	// 消息的优先级，EMSG_POST_FAST,EMSG_POST_REVERSE
	)
{
	IEinkuiMessage* lpMsgIntf = AllocateMessage(nuMsgID,npInputBuffer,niInputSize,NULL,0);

	if(lpMsgIntf == NULL)
		return ERESULT_UNSUCCESSFUL;

	ERESULT luResult= PostMessage(npDestElement,lpMsgIntf,nuPostType);

	lpMsgIntf->Release();

	return luResult;
}



// 从消息队列提取一条消息，并且分发处理
ERESULT CXelManager::ProcessNextMessage(
	IEinkuiMessage* npMsg	//不为空，表示直接处理这条消息，而不从消息队列读取
	)
{
	IEinkuiMessage* lpMsg;
	CXuiMessage* lpMsgObj;

	// 取一条消息
	if(npMsg == NULL)
	{
		lpMsg = moFastMessages.GetMessage();

		if(lpMsg == NULL)
			lpMsg = moNormalMessages.GetMessage();

		//if(lpMsg == NULL)
		//	lpMsg = moReduceMessages.GetMessage();

		if(lpMsg == NULL)
			return ERESULT_NO_MESSAGE;

	}
	else
		lpMsg = npMsg;

	lpMsgObj = dynamic_cast<CXuiMessage*>(lpMsg);
	if (lpMsgObj == NULL)
		return ERESULT_MESSAGE_EXCEPTION;

	// 分析目标
	IEinkuiIterator* lpIterator = lpMsgObj->GetDestination();

	ERESULT luResult = VerifyIterator(lpIterator);
	
	if(luResult != ERESULT_SUCCESS)
		return luResult;

	CXuiIterator* lpItrObj = dynamic_cast<CXuiIterator*>(lpIterator);
	if(lpItrObj == NULL)
		return ERESULT_ITERATOR_INVALID;

	if (lpItrObj->IsDeleted() != false)
		return ERESULT_ITERATOR_INVALID;

	// 检查目标是否被Hook，注意，Hook的转发消息本身不能被Hook转发
	IEinkuiIterator* lpHooker = lpItrObj->GetHooker();
	if(lpHooker != NULL)
	{
		STEMS_HOOKED_MSG ldHookMsg;

		ldHookMsg.OriginalElement = lpIterator;
		ldHookMsg.OriginalMessage = lpMsg;

		luResult = CExMessage::SendMessage(lpHooker,lpIterator,EMSG_HOOKED_MESSAGE,ldHookMsg,NULL,0);
		if(luResult != ERESULT_MSG_SENDTO_NEXT && luResult != ERESULT_ITERATOR_INVALID)
			return luResult;
	}

	// 发送到目标对象
	CEinkuiSystem::gpXuiSystem->PushWidget(lpItrObj->mpWidget);

	ULONG luMsgID = lpMsg->GetMessageID();

	lpItrObj->KAddRefer();

	try	{
		CThreadAbort::Dummy();
		luResult = lpItrObj->mpElement->MessageReceiver(lpMsg);
	}
	catch(CThreadAbort())
	{
		// 阻死了，那么就禁用这个Widget
		luResult = ERESULT_STALLED_THREAD;
	}
	catch(...)
	{
		luResult = ERESULT_MESSAGE_EXCEPTION;
	}

	lpItrObj->KRelease();

	//if(luMsgID == EMSG_CREATE && luResult == ERESULT_SUCCESS)
	//{
	//	//Trace_StringW(6350,lpItrObj->mpElement->GetType());//收到Create消息
	//}

	CEinkuiSystem::gpXuiSystem->PopWidget();

	// 判断是否有等待事件
	if(lpMsgObj->mhCompleteEvent != NULL)
		SetEvent(lpMsgObj->mhCompleteEvent);

	// 消息处理完毕，就释放它
	lpMsg->Release();

	return luResult;
}

// 锁定XUI元素树
void CXelManager::LockIterators(void)
{
	moIteratorLock.Enter();
}

// 解除锁定XUI元素树
void CXelManager::UnlockIterators(void)
{
	moIteratorLock.Leave();
}

// 枚举全部元素，每当发现一个Element时调用枚举请求者提供的ElementEnter函数；当一个元素没有子元素时，将调用提供的ElementLeave
ERESULT __stdcall CXelManager::EnumAllElement(
	bool nbReverse,				// 反向，指的是枚举子节点时，按照Zorder的顺序枚举，或者按照Zorder的逆序枚举
	IBaseObject* npApplicant,	// 发起对象
	ERESULT (__stdcall IBaseObject::*ElementEnter)(IEinkuiIterator* npRecipient),//如果返回ERESULT_ENUM_CHILD继续枚举；返回ERESULT_STOP_ENUM_CHILD或任意其他值将停止枚举此元素的此元素的子元素
	ERESULT (__stdcall IBaseObject::*ElementLeave)(IEinkuiIterator* npRecipient) //返回值无意义
	)
{
	ERESULT luResult = ERESULT_SUCCESS;
	TElEnumIndexStack loIndexStack;
	int liIndex;
	int liDirection = (nbReverse!=false)?-1:1;
	CXuiIterator* lpCrtItr;
	CXuiIterator* lpParents;

	lpParents = &moIteratorRoot;

	if(ElementEnter == NULL || ElementLeave == NULL || npApplicant == NULL)
		return ERESULT_WRONG_PARAMETERS;

	//if(Trace_Count(/*EnumEle*/1,3)<3) Trace_Point(27454);//开始枚举所有元素
	liIndex = (nbReverse!=false)?lpParents->GetSubElementCount():-1;

	while(ERESULT_SUCCEEDED(luResult))
	{
		liIndex += liDirection;
		lpCrtItr = dynamic_cast<CXuiIterator*>(lpParents->GetSubElementByZOder(liIndex));

		if(lpCrtItr != NULL)
		{
			//if(Trace_CountRead(/*EnumEle*/1)<3) Trace_PVOID(23278,lpCrtItr);// 回调Enter函数
			luResult = (npApplicant->*ElementEnter)(lpCrtItr);

			loIndexStack.Push(liIndex);
			lpParents = lpCrtItr;

			if(luResult != ERESULT_ENUM_CHILD)
			{
				liIndex = -2; // 迫使处理子元素时自动终止
				//Trace_CountClear(/*EnumEle*/1);
			}
			else
				liIndex = (nbReverse!=false)?lpParents->GetSubElementCount():-1;

		}
		else
		{
			// 回溯上一层
			if(loIndexStack.Size() <= 0)
				break;		// 已经在根部，没有上一层了

			liIndex = loIndexStack.Top();
			loIndexStack.Pop();

			//if(Trace_CountRead(/*EnumEle*/1)<3) Trace_PVOID(26454,lpParents);// 回调退出函数
			luResult = (npApplicant->*ElementLeave)(lpParents);
			if(ERESULT_SUCCEEDED(luResult)==false || luResult == ERESULT_EXIT_ENUM)
				break;

			// 绝对不可能为空，如果lpParents是根节点的第一级子节点，那么就应该在前面if(loIndexStack.Size() <= 0)退出了
			CMMASSERT(lpParents->mpParent != NULL);

			// 检查刚才的根节点位置是否漂移, ???需要调试一次
			lpCrtItr = dynamic_cast<CXuiIterator*>(lpParents->mpParent->GetSubElementByZOder(liIndex));
			if(lpCrtItr != lpParents)	// 刚才的父节点改变了位置
			{
				int i=-1;
				do 
				{
					i++;
					lpCrtItr = dynamic_cast<CXuiIterator*>(lpParents->mpParent->GetSubElementByZOder(i));
				} while (lpCrtItr != NULL && lpCrtItr != lpParents);

				if(lpCrtItr == NULL)
					luResult = ERESULT_UNSUCCESSFUL;
				else
					liIndex = i;
			}
			lpParents = lpParents->mpParent;

			if(luResult == ERESULT_REDO_ENUM)
				liIndex -= liDirection;	//如果目标要求再次枚举，那么就预先将当前索引退位，回到循环前面的代码就会恢复为当前目标

		}
	}

	return luResult;
}





// 处理鼠标输入转发
ERESULT CXelManager::OnMsgMouseForward(const PSTELEMGR_WINMSG_FORWARD npMouseInput)
{
	CElMouseFootPrint loMouseFoot;
	ULONG luWsgID;
	bool lbStopDrag = false;
	CXuiIterator* lpCrtMouseFocus;
	IEinkuiIterator* lpModal;

	if (mlCleanHumanInput != 0)
		return ERESULT_UNSUCCESSFUL;

	//////////////////////////////////////////////////////////////////////////
	// 准备鼠标位置
	loMouseFoot.TickCount = GetTickCount();
	if(npMouseInput != NULL)
	{
		loMouseFoot.Point.x = (FLOAT)LOSHORT(npMouseInput->lParam) - mdTopLeftInPanel.x;
		loMouseFoot.Point.y = (FLOAT)HISHORT(npMouseInput->lParam) - mdTopLeftInPanel.y;
		luWsgID = npMouseInput->WinMsgID; 

		// 保存鼠标痕迹
		loMouseFoot.KeyFlag = LOWORD(npMouseInput->wParam);
		if(moMouseTrace.Back().Point.x !=loMouseFoot.Point.x || moMouseTrace.Back().Point.y != loMouseFoot.Point.y)
			moMouseTrace.Push_Back(loMouseFoot);

		if(moMouseTrace.Size() > ELMSG_MAX_MOUSE_TRACK)	// 鼠标记录数过大，就移除
			moMouseTrace.Pop_Front();
	}
	else
	{
		// 如果没有鼠标痕迹，直接退出
		if(moMouseTrace.Size() ==0)
			return ERESULT_SUCCESS;

		loMouseFoot = moMouseTrace.Back();
		luWsgID = MAXULONG32;
		loMouseFoot.TickCount = GetTickCount();
	}


	// 获得鼠标焦点，下面注意要释放
	lpCrtMouseFocus = InnerGetMouseFocus();

	//////////////////////////////////////////////////////////////////////////
	// 是否处于拖拽状态
	if(mbDragging != false && lpCrtMouseFocus != NULL)
	{
		bool lbLBReleased;

		// 渲染后重拾鼠标位置，不用处理拖拽状态
		if(npMouseInput == NULL)
		{
			if(lpCrtMouseFocus != NULL)
				lpCrtMouseFocus->KRelease();
			return ERESULT_SUCCESS;
		}
		
		switch(muKeyWithDragging)
		{
		case MK_LBUTTON:
			lbLBReleased = ((npMouseInput->wParam&MK_LBUTTON)==0 || luWsgID == WM_LBUTTONUP);
			break;
		case MK_MBUTTON:
			lbLBReleased = ((npMouseInput->wParam&MK_MBUTTON)==0 || luWsgID == WM_MBUTTONUP);
			break;
		case MK_RBUTTON:
			lbLBReleased = ((npMouseInput->wParam&MK_RBUTTON)==0 || luWsgID == WM_RBUTTONUP);
			break;
		default:
			lbLBReleased = false;
		}
			

		// 是拖拽行为吗？
		if(luWsgID == WM_MOUSEMOVE || lbLBReleased!=false/*luWsgID == WM_LBUTTONUP*/)
		{
			// 计算拖拽距离
			STMS_DRAGGING_ELE ldDragged;

			ldDragged.DragOn = mpDragMsgReceiver;
			CalculateMouseMoving(mpDragMsgReceiver,loMouseFoot.Point,mdDragFrom,ldDragged.Offset);
			ldDragged.ActKey = muKeyWithDragging;
			ldDragged.KeyFlag = LOWORD(npMouseInput->wParam);

			if(ldDragged.Offset.x >= 1.0f || ldDragged.Offset.x <= -1.0f || ldDragged.Offset.y >= 1.0f || ldDragged.Offset.y <= -1.0f)
			{
				// 计算局部坐标
				if(mpDragMsgReceiver->WorldToLocal(loMouseFoot.Point,ldDragged.CurrentPos) != false)
				{
					CExMessage::SendMessage(mpDragMsgReceiver,NULL,EMSG_DRAGGING_ELEMENT,ldDragged);					
				}
				// else 出错，无法计算局部坐标，则不发送拖拽消息
			}
		}

		// 是否结束拖拽状态
		if(lbLBReleased != false)
		{
			STMS_DRAGGING_ELE ldDragged;

			ldDragged.DragOn = lpCrtMouseFocus;
			CalculateMouseMoving(mpDragMsgReceiver,loMouseFoot.Point,mdDragFrom,ldDragged.Offset);
			ldDragged.ActKey = muKeyWithDragging;
			ldDragged.KeyFlag = LOWORD(npMouseInput->wParam);

			// 计算局部坐标
			mpDragMsgReceiver->WorldToLocal(loMouseFoot.Point,ldDragged.CurrentPos);

			mbDragging = false;	
			
			//Trace_ULONG(10252,muKeyWithDragging);//EMSG_DRAG_END
			CExMessage::SendMessage(mpDragMsgReceiver,NULL,EMSG_DRAG_END,ldDragged);

			// 下面需要继续检测鼠标，标识刚才有拖拽行为，避免将Button Up消息发给新的鼠标落点;
			// 同时继续检测鼠标用于判断是否执行Drag&Drop的Drop down行为
			lbStopDrag = true;
		}
		else
		if(mbXuiDragDrop == false && luWsgID != WM_MOUSEWHEEL)	// 如果没有结束拖拽,不处于Drag&Drop状态并且不是Mouse Wheel消息
		{
			// 那么就不再继续鼠标检测了
			if(lpCrtMouseFocus != NULL)
				lpCrtMouseFocus->KRelease();
			return ERESULT_SUCCESS;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//Trace_float(20125,loMouseFoot.Point.x);// 检测鼠标落点
	//Trace_float(22462,loMouseFoot.Point.y);
	if(luWsgID != WM_MOUSELEAVE)
	{
		if(mpMouseMoveOn != NULL)
			mpMouseMoveOn->KRelease();
		mpMouseMoveOn = NULL;
		mlMouseMoveOnLevel = -1;

		// 填入初始位置
		moPointToTest.Clear();
		CElMouseTestState loState;
		loState.Point = loMouseFoot.Point;
		loState.mpElement = NULL;
		loState.mbSeekInLevels = false;
		loState.mlCrtLevel = 0;
		moPointToTest.Push(loState);

		// 检查全部对象，目前鼠标检测没有支持嵌套的绘制层
		mbTopDrawTest = false;
		EnumAllElement(true,this,
			(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXelManager::EnterForMouseTest,
			(ERESULT (__stdcall IBaseObject::*)(IEinkuiIterator* npRecipient))&CXelManager::LeaveForMouseTest
			);	// 不必判断返回值
	}
	else
	{
		mpMouseMoveOn = NULL;
		mbTrackMouse = false;
	}


	//////////////////////////////////////////////////////////////////////////
	// 过滤掉锁定区域
	lpModal = CEinkuiSystem::gpXuiSystem->GetTopModalElement();
	if(mpMouseMoveOn != NULL && lpModal != NULL && lpModal->IsVisible()!= false)
	{
		if(mpMouseMoveOn->FindAncestor(lpModal)==false)
		{
			//// 如果是鼠标按键消息，这激发模态窗口闪烁
			//if(luWsgID == WM_LBUTTONDOWN || luWsgID == WM_RBUTTONDOWN || luWsgID == WM_MBUTTONDOWN)
			//{
			//	CEinkuiSystem::gpXuiSystem->FlashModalElement(lpModal);
			//}
			mpMouseMoveOn = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 是否处于Drag&drop状态
	if(mbXuiDragDrop != false)
	{
		if(lbStopDrag == false)
		{
			// 询问是否支持Drop
			DropDetect(mpMouseMoveOn);

			// 不用向下继续执行了
			return ERESULT_SUCCESS;
		}
		else
		{
			// 判断是否执行Drop down操作
			TryDropDown(mpMouseMoveOn);
		}
	}

	if(lbStopDrag != false)
	{
		if(mpDragMsgReceiver != NULL)
			mpDragMsgReceiver->KRelease();
		mpDragMsgReceiver = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 是否变换鼠标焦点
	if(mpMouseMoveOn != lpCrtMouseFocus)
	{
		if(lpCrtMouseFocus != NULL)
		{
			STEMS_STATE_CHANGE ldChange;
			ldChange.State = 0;//lose
			ldChange.Related = mpMouseMoveOn;

			CExMessage::SendMessage(lpCrtMouseFocus,NULL,EMSG_MOUSE_FOCUS,ldChange);

			ChangeMouseFocus(NULL);
		}

		if(mpMouseMoveOn != NULL)
		{
			STEMS_STATE_CHANGE ldChange;
			ldChange.State = 1;//got
			ldChange.Related = lpCrtMouseFocus;

			CExMessage::SendMessage(mpMouseMoveOn,NULL,EMSG_MOUSE_FOCUS,ldChange);

			ChangeMouseFocus(mpMouseMoveOn);

			// 刚切换到新元素，下次进入将探测悬浮
			mbHoverTest = true;
		}
		
		// 如果当前的情况是：释放了鼠标拖拽同时变换了焦点，那么就不应该把鼠标按钮抬起消息发送到新的焦点
		if(lbStopDrag != false)
		{
			if(lpCrtMouseFocus != NULL)
				lpCrtMouseFocus->KRelease();
			return ERESULT_SUCCESS;
		}

		if(mpTipOwner != NULL)
		{
			//促使Tip显示超时
			muTickOnTipShow = 0;
			mpTipOwner->KRelease();
			mpTipOwner = NULL;
		}
	}
	else
	if(mbHoverTest !=false && luWsgID == MAXULONG32 && lpCrtMouseFocus != NULL)	// 但本函数是系统重绘后调用时，才需要探测悬浮
	{
		// 探测悬浮
		ULONG luTotal = 0;

		if(moMouseTrace.Back().Point.x >= loMouseFoot.Point.x-1.0f && moMouseTrace.Back().Point.x <= loMouseFoot.Point.x+1.0f &&
			moMouseTrace.Back().Point.y >= loMouseFoot.Point.y-1.0f && moMouseTrace.Back().Point.y <= loMouseFoot.Point.y+1.0f &&
			(loMouseFoot.TickCount - moMouseTrace.Back().TickCount) >= ELMGR_HOVER_DURATION)
		{
			CExMessage::SendMessage(lpCrtMouseFocus,NULL,EMSG_MOUSE_HOVER,CExMessage::DataInvalid);

			// 最近一次Tip显示者不是此元素
			if(mpTipOwner != lpCrtMouseFocus)
			{
				// 判断是否有Tip
				const wchar_t* lswTipText = lpCrtMouseFocus->GetToolTip();
				if(lswTipText != NULL)
				{
					STEMS_SWGT_SHOWTIP ldShow;

					muTickOnTipShow = loMouseFoot.TickCount;
					if(mpTipOwner != NULL)
						mpTipOwner->KRelease();
					mpTipOwner = lpCrtMouseFocus;
					mpTipOwner->KAddRefer();
					mbTipHide = false;

					ldShow.Text = lswTipText;
					ldShow.Position.x = loMouseFoot.Point.x+25.0f;
					ldShow.Position.y = loMouseFoot.Point.y+20.0f;
					CExMessage::SendMessage(GetDesktop(),NULL,EMSG_SWGT_SHOW_TIP,ldShow);
				}
			}
		}
	}

	// 释放先前的鼠标焦点
	if(lpCrtMouseFocus != NULL)
		lpCrtMouseFocus->KRelease();

	// 检查Tip显示是否到期
	if(mbTipHide == false && ((loMouseFoot.TickCount - muTickOnTipShow) >= ELMGR_TIP_DURATION || npMouseInput!=NULL))
	{
		mbTipHide = true;
		CExMessage::SendMessage(GetDesktop(),NULL,EMSG_SWGT_HIDE_TIP,CExMessage::DataInvalid);
	}

	//  如果仅仅是绘制后检测鼠标位置，那就可以退出了
	if(npMouseInput == NULL)
		return ERESULT_SUCCESS;

	// 执行Windows鼠标控制

	// 重新获得鼠标
	lpCrtMouseFocus = InnerGetMouseFocus();

	// 如果当前没有鼠标焦点，那么就检测一下活动窗口是否改变，就退出了
	if(lpCrtMouseFocus == NULL)
	{
		if(luWsgID == WM_LBUTTONDOWN || luWsgID == WM_RBUTTONDOWN || luWsgID == WM_MBUTTONDOWN)
			ChangeActiveElement(NULL);

		return ERESULT_SUCCESS;
	}

	// 将鼠标行为翻译为自己的消息格式
	switch(luWsgID)
	{
	case WM_LBUTTONDOWN:
		{
			// 有可能还要改变窗口层叠，需要改变键盘焦点
			BringFocusedPopupToTop(lpCrtMouseFocus);

			// 判断当前目标是不是一个可拖拽的窗口，决定是否进入拖拽状态
			DetectMouseDragBegin(lpCrtMouseFocus,MK_LBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.Point);

			SendMouseButtonPressed(lpCrtMouseFocus,true,MK_LBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);

			// 设置键盘焦点
			DetectKeyboardFocus(lpCrtMouseFocus);

			// 设置活跃窗口
			ChangeActiveElement(lpCrtMouseFocus);
		}
		break;
	case WM_LBUTTONUP:
		{
			SendMouseButtonPressed(lpCrtMouseFocus,false,MK_LBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_LBUTTONDBLCLK:
		{
			SendMouseButtonDbClick(lpCrtMouseFocus,MK_LBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_RBUTTONDOWN:
		{
			// 有可能还要改变窗口层叠，需要改变键盘焦点
			BringFocusedPopupToTop(lpCrtMouseFocus);

			// 判断当前目标是不是一个可拖拽的窗口，决定是否进入拖拽状态
			DetectMouseDragBegin(lpCrtMouseFocus,MK_RBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.Point);

			SendMouseButtonPressed(lpCrtMouseFocus,true,MK_RBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);

			// 设置键盘焦点
			DetectKeyboardFocus(lpCrtMouseFocus);

			// 设置活跃窗口
			ChangeActiveElement(lpCrtMouseFocus);
		}
		break;
	case WM_RBUTTONUP:
		{
			SendMouseButtonPressed(lpCrtMouseFocus,false,MK_RBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_RBUTTONDBLCLK:
		{
			SendMouseButtonDbClick(lpCrtMouseFocus,MK_RBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_MBUTTONDOWN:
		{
			// 有可能还要改变窗口层叠，需要改变键盘焦点
			BringFocusedPopupToTop(lpCrtMouseFocus);

			// 判断当前目标是不是一个可拖拽的窗口，决定是否进入拖拽状态
			DetectMouseDragBegin(lpCrtMouseFocus,MK_MBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.Point);

			SendMouseButtonPressed(lpCrtMouseFocus,true,MK_MBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);

			// 设置键盘焦点
			DetectKeyboardFocus(lpCrtMouseFocus);

			// 设置活跃窗口
			ChangeActiveElement(lpCrtMouseFocus);
		}
		break;
	case WM_MBUTTONUP:
		{
			SendMouseButtonPressed(lpCrtMouseFocus,false,MK_MBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_MBUTTONDBLCLK:
		{
			SendMouseButtonDbClick(lpCrtMouseFocus,MK_MBUTTON,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_XBUTTONDOWN:
		{
			ULONG luActKey;

			if(HIWORD(npMouseInput->wParam)==XBUTTON1)
				luActKey = MK_XBUTTON1;
			else
			if(HIWORD(npMouseInput->wParam)==XBUTTON2)
				luActKey =  MK_XBUTTON2;
			else
				break;

			SendMouseButtonPressed(lpCrtMouseFocus,true,luActKey,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);

		}
		break;
	case WM_XBUTTONUP:
		{
			ULONG luActKey;

			if(HIWORD(npMouseInput->wParam)==XBUTTON1)
				luActKey = MK_XBUTTON1;
			else
				if(HIWORD(npMouseInput->wParam)==XBUTTON2)
					luActKey =  MK_XBUTTON2;
				else
					break;

			SendMouseButtonPressed(lpCrtMouseFocus,false,luActKey,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);

		}
		break;
	case WM_XBUTTONDBLCLK:
		{
			ULONG luActKey;

			if(HIWORD(npMouseInput->wParam)==XBUTTON1)
				luActKey = MK_XBUTTON1;
			else
				if(HIWORD(npMouseInput->wParam)==XBUTTON2)
					luActKey =  MK_XBUTTON2;
				else
					break;

			SendMouseButtonDbClick(lpCrtMouseFocus,luActKey,LOWORD(npMouseInput->wParam),loMouseFoot.TickCount,mdPointRelative);
		}
		break;
	case WM_MOUSEMOVE:
		{
			STEMS_MOUSE_MOVING ldMouse;

			ldMouse.KeyFlag = LOWORD(npMouseInput->wParam);
			ldMouse.TickCount = loMouseFoot.TickCount;
			ldMouse.Position = mdPointRelative;

			CExMessage::SendMessage(lpCrtMouseFocus,NULL,EMSG_MOUSE_MOVING,ldMouse);
		}
		break;
	case WM_MOUSEWHEEL:	// Mouse wheel 消息携带的point位置是屏幕坐标，当非全屏窗口程序的时候，需要特殊处理，目前这个问题还没有处理，请开发新版的时候注意??? Ax Nov.7,2012
		{
			STEMS_MOUSE_WHEEL ldMouse;

			ldMouse.Delta = HIWORD(npMouseInput->wParam);
			ldMouse.KeyFlag = LOWORD(npMouseInput->wParam);
			ldMouse.TickCount = loMouseFoot.TickCount;
			ldMouse.MouseFocus = lpCrtMouseFocus;
			//ldMouse.Position = mdPointRelative;

			TransferMouseWheel(lpCrtMouseFocus,ldMouse);
		}
		break;
	default:
		break;
	}


	if(lpCrtMouseFocus != NULL)
	{
		// 处理与windows鼠标跟踪，如果没有开始跟踪，则跟踪windows的鼠标
		if(mbTrackMouse == false)
		{
			//关闭了Track mouse 的操作 ax
			//TRACKMOUSEEVENT TrackMouse;
			//TrackMouse.cbSize = sizeof(TrackMouse);
			//TrackMouse.dwFlags = TME_LEAVE;
			//TrackMouse.hwndTrack = EinkuiGetSystem()->GetMainWindow();

			//TrackMouseEvent(&TrackMouse);
			mbTrackMouse = true;
		}

		lpCrtMouseFocus->KRelease();
	}


	return ERESULT_SUCCESS;
}



// 修改此代码，将触屏消息按照鼠标消息转发，触屏逻辑未完整，Ax.2017.08.16
// 处理鼠标输入转发
// ERESULT CXelManager::OnMsgEinkTouchForward(const PSTEMS_TOUCH npMouseInput)
// {
// 	return ERESULT_SUCCESS;
// }

// 询问是否支持Drop
void CXelManager::DropDetect(CXuiIterator* npToDetect)
{
	 CElMouseFootPrint loMouseFoot;
	 if(npToDetect == NULL || npToDetect == mpDraggingElement/* || npToDetect == mpRecentAskedDrop*/)
		 return;

	 if(mpRecentAskedDrop != NULL && mpRecentAskedDrop->CheckStyle(EITR_STYLE_XUIDRAGDROP)==false && npToDetect == mpRecentAskedDrop)
		 return;	// 不具有这个属性的，并且已经记录了

	 if(mpRecentAskedDrop != NULL && mpRecentAskedDrop->CheckStyle(EITR_STYLE_XUIDRAGDROP)!=false && npToDetect != mpRecentAskedDrop)
	 {
		 // 给离开的对象发送Drop leave消息
		 CExMessage::SendMessage(mpRecentAskedDrop,NULL,EMSG_DRAGDROP_LEAVE,CExMessage::DataInvalid);
	 
	 }

	 // 跟新最近访问记录
	 if(npToDetect != mpRecentAskedDrop)
	 {
		 if(mpRecentAskedDrop != NULL)
			 mpRecentAskedDrop->KRelease();
		 mpRecentAskedDrop = npToDetect;
		 mpRecentAskedDrop->KAddRefer();

		 // 给新对象发送进入消息
		 if(npToDetect->CheckStyle(EITR_STYLE_XUIDRAGDROP)!=false)
		 {
			 CExMessage::SendMessage(mpRecentAskedDrop,NULL,EMSG_DRAGDROP_ENTER,CExMessage::DataInvalid);
		 }
	 }

	 if(npToDetect->CheckStyle(EITR_STYLE_XUIDRAGDROP)!=false)
	 {
		 // 计算局部坐标
		loMouseFoot = moMouseTrace.Back();
		npToDetect->WorldToLocal(loMouseFoot.Point,mdDrgdrpRequest.CurrentPos);
		mdDrgdrpRequest.KeyFlag = loMouseFoot.KeyFlag;

		// 询问是否接受Drop
		 ERESULT luResult = CExMessage::SendMessage(npToDetect,NULL,EMSG_XUI_DROP_TEST,mdDrgdrpRequest);
		 if(luResult == ERESULT_EDRAGDROP_ACCEPT)
		 {
			 if(mpLastAccepter != NULL)
				 mpLastAccepter->KRelease();
			 mpLastAccepter = npToDetect;
			 mpLastAccepter->KAddRefer();
		 }
	 }
}

// 执行Drop down
void CXelManager::TryDropDown(CXuiIterator* npToTry)
{
	if(npToTry != NULL && npToTry == mpLastAccepter && mpDraggingElement != NULL)
	{
		CElMouseFootPrint loMouseFoot;
		IEinkuiMessage* lpMsg=NULL;
		ERESULT luResult;

		do 
		{
			// 计算局部坐标
			loMouseFoot = moMouseTrace.Back();
			npToTry->WorldToLocal(loMouseFoot.Point,mdDrgdrpRequest.CurrentPos);
			mdDrgdrpRequest.KeyFlag = loMouseFoot.KeyFlag;

			// send 'drag off' to Drag&Drop host
			lpMsg = AllocateMessage(EMSG_XUI_DRAG_OFF,&mpLastAccepter,sizeof(mpLastAccepter),&mdDrgdrpRequest,sizeof(mdDrgdrpRequest));
			BREAK_ON_NULL(lpMsg);

			luResult = SendMessage(mpDragMsgReceiver,lpMsg);
			if(luResult!=ERESULT_SUCCESS)
				break;

			if(lpMsg->GetOutputDataSize() == sizeof(mdDrgdrpRequest))
			{
				//防止该参数被修改
				mdDrgdrpRequest.Host = mpDraggingElement;
			}
			CMM_SAFE_RELEASE(lpMsg);

			// send 'drop down' to Drag&Drop acceptor
			lpMsg = AllocateMessage(EMSG_XUI_DROP_DOWN,&mdDrgdrpRequest,sizeof(mdDrgdrpRequest),NULL,0);
			BREAK_ON_NULL(lpMsg);

			luResult = SendMessage(mpLastAccepter,lpMsg);

		} while (false);

		CMM_SAFE_RELEASE(lpMsg);
	}

	mbXuiDragDrop = false;

	if(mpDraggingElement!=NULL)
		mpDraggingElement->KRelease();
	mpDraggingElement = NULL;

	if(mpLastAccepter!= NULL)
		mpLastAccepter->KRelease();
	mpLastAccepter = NULL;

	if(mpRecentAskedDrop!=NULL)
		mpRecentAskedDrop->KRelease();
	mpRecentAskedDrop = NULL;

}


// 发送鼠标按键消息
void CXelManager::SendMouseButtonPressed(IEinkuiIterator* npFocus,bool nbPressed,ULONG nuActKey,ULONG nuKeyFlag,ULONG nuTickCount,const D2D1_POINT_2F& rPosition)
{
	STEMS_MOUSE_BUTTON ldMouse;

	ldMouse.Presssed = nbPressed;
	ldMouse.ActKey = nuActKey;
	ldMouse.KeyFlag = nuKeyFlag;
	ldMouse.TickCount = nuTickCount;
	ldMouse.Position = rPosition;

	SimpleSendMessage(npFocus,EMSG_MOUSE_BUTTON,&ldMouse,sizeof(ldMouse),NULL,0);
}

// 发送鼠标按键双击消息
void CXelManager::SendMouseButtonDbClick(IEinkuiIterator* npFocus,ULONG nuActKey,ULONG nuKeyFlag,ULONG nuTickCount,const D2D1_POINT_2F& rPosition)
{
	STEMS_MOUSE_BUTTON ldMouse;

	ldMouse.Presssed = false;
	ldMouse.ActKey = nuActKey;
	ldMouse.KeyFlag = nuKeyFlag;
	ldMouse.TickCount = nuTickCount;
	ldMouse.Position = rPosition;

	SimpleSendMessage(npFocus,EMSG_MOUSE_DBCLICK,&ldMouse,sizeof(ldMouse),NULL,0);
}


// 检查是否启动拖拽并且发送开始拖拽消息
void CXelManager::DetectMouseDragBegin(CXuiIterator* npFocus,ULONG nuActKey,ULONG nuKeyFlag,const D2D1_POINT_2F& rPosition)
{
	if(mbDragging != false)
		return;

	// 当前焦点是否支持拖拽
	if(npFocus->CheckStyle(EITR_STYLE_DRAG)==false)
	{
		// 不支持，那么判断是否有父窗口设置了全拖拽
		CXuiIterator* lpItrObj = (CXuiIterator*)npFocus->GetParent();

		if(mpDragMsgReceiver != NULL)
			mpDragMsgReceiver->KRelease();
		mpDragMsgReceiver = NULL;

		// 没有到达根节点
		while(lpItrObj != (CXuiIterator*)lpItrObj->GetParent())
		{
			if(lpItrObj->CheckStyle(EITR_STYLE_ALL_DRAG)!=false)
			{
				mpDragMsgReceiver = lpItrObj;
				mpDragMsgReceiver->KAddRefer();
				break;
			}
			lpItrObj = (CXuiIterator*)lpItrObj->GetParent();
		}
		if(mpDragMsgReceiver == NULL)
			return;
	}
	else
	{
		if(mpDragMsgReceiver != npFocus)
		{
			if(mpDragMsgReceiver != NULL)
				mpDragMsgReceiver->KRelease();
			mpDragMsgReceiver = npFocus;
			mpDragMsgReceiver->KAddRefer();
		}
	}

	STMS_DRAGGING_ELE ldDragged;
	IEinkuiMessage* lpMsg;
	ERESULT luResult;

	// 发送拖拽开始消息
	ldDragged.Offset.x = 0.0f;
	ldDragged.Offset.y = 0.0f;
	ldDragged.ActKey = nuActKey;
	ldDragged.KeyFlag = nuKeyFlag;
	ldDragged.DragOn = npFocus;

	// 计算局部坐标，默认情况下，世界转换的逆阵已经转换了
	if(npFocus->WorldToLocal(rPosition,ldDragged.CurrentPos) == false) //出错，无法计算局部坐标，则不发送拖拽消息
	{
		if(mpDragMsgReceiver != NULL)
			mpDragMsgReceiver->KRelease();
		mpDragMsgReceiver = NULL;
		return;
	}


	lpMsg = AllocateMessage(EMSG_DRAG_BEGIN,&ldDragged,sizeof(ldDragged),&mdDrgdrpRequest,sizeof(mdDrgdrpRequest));
	
	luResult = SendMessage(mpDragMsgReceiver,lpMsg);
	if(ERESULT_SUCCEEDED(luResult))	// ERESULT_WDRAGDROP_BEGIN for windows drag&drop
	{

		mbDragging = true;
		muKeyWithDragging = nuActKey;
		mdDragFrom = rPosition;

		if(lpMsg->GetResult() == ERESULT_EDRAGDROP_BEGIN && lpMsg->GetOutputDataSize() == sizeof(mdDrgdrpRequest))
		{
			// 开始执行拖放操作
			mdDrgdrpRequest.Host = mpDragMsgReceiver;
			mbXuiDragDrop = true;

			if(mpDraggingElement!=NULL)
				mpDraggingElement->KRelease();
			mpDraggingElement = npFocus;
			mpDraggingElement->KAddRefer();

			if(mpRecentAskedDrop!=NULL)
				mpRecentAskedDrop->KRelease();
			mpRecentAskedDrop = NULL;

			if(mpLastAccepter != NULL)
				mpLastAccepter->KRelease();
			mpLastAccepter = NULL;
		}
	}
	else
	{
		if(mpDragMsgReceiver != NULL)
			mpDragMsgReceiver->KRelease();
		mpDragMsgReceiver = NULL;
	}

	lpMsg->Release();
}

// 将目标元素从子孙到祖先全部调整到Zorder最高层
void CXelManager::BringFocusedPopupToTop(CXuiIterator* npFocus)
{
	CXuiIterator* lpItrObj = (CXuiIterator*)npFocus->GetParent();

	// 到达根节点
	while(npFocus != lpItrObj)
	{
		if(npFocus->CheckStyle(EITR_STYLE_POPUP)!=false)
		{
			lpItrObj->BringSubElementToTop(npFocus);
		}

		npFocus = lpItrObj;
		lpItrObj = (CXuiIterator*)npFocus->GetParent();
	}

}

// 检查是否会获得键盘焦点，如果目标恰好是当前键盘焦点则直接返回，否则将释放当前键盘焦点
void CXelManager::DetectKeyboardFocus(CXuiIterator* npToDetect)
{
	// 不支持，那么判断是否有父窗口设置了EITR_STYLE_ALL_KEY
	if(npToDetect->CheckStyle(EITR_STYLE_KEYBOARD)==false)
	{
		CXuiIterator* lpItrObj = (CXuiIterator*)npToDetect->GetParent();
		npToDetect = NULL;

		// 没有到达根节点
		while(lpItrObj != (CXuiIterator*)lpItrObj->GetParent())
		{
			if(lpItrObj->CheckStyle(EITR_STYLE_ALL_KEY)!=false)
			{
				npToDetect = lpItrObj;
				break;
			}
			if(lpItrObj->CheckStyle(EITR_STYLE_POPUP) != false)
				break;
			lpItrObj = (CXuiIterator*)lpItrObj->GetParent();
		}
	}

	ChangeKeyFocus(npToDetect);
}


// 鼠标落点检测预处理
ERESULT __stdcall CXelManager::EnterForMouseTest(IEinkuiIterator* npRecipient)
{
	ERESULT luResult;
	D2D1_RECT_F ldVisibleRegion;
	//IEinkuiIterator* lpEnhancer;
	CElMouseTestState loState;
	CXuiIterator* lpRecipient = dynamic_cast<CXuiIterator*>(npRecipient);

	do 
	{
		loState = moPointToTest.Top();

		moPointToTest.Push(loState);
		// 如果隐藏，退出
		if(npRecipient->IsVisible()==false)
		{
			luResult= ERESULT_STOP_ENUM_CHILD;
			break;
		}

		if(moPointToTest.Top().mbSeekInLevels == false)
		{
			if(lpRecipient->GetPaintLevelCount() > 0)
			{
				moPointToTest.Top().mbSeekInLevels = true;
			}
		}

		if(lpRecipient->GetPaintLevel() >= 0)
			moPointToTest.Top().mlCrtLevel = lpRecipient->GetPaintLevel();

		// 如果设置了可视区域，则判断是否在区域中
		if(lpRecipient->GetVisibleRegion(ldVisibleRegion)!=false)
		{
			D2D1_POINT_2F ldPosition;
			// 提起计算世界坐标矩阵的逆阵
			if(lpRecipient->WorldToLocal(moPointToTest.Top().Point,ldPosition) == false || 
				ldPosition.x < ldVisibleRegion.left || ldPosition.x >= ldVisibleRegion.right || 
				ldPosition.y < ldVisibleRegion.top || ldPosition.y >= ldVisibleRegion.bottom )
			{
				return ERESULT_STOP_ENUM_CHILD;	// 不在区域中
			}
		}

		luResult = ERESULT_ENUM_CHILD;

	} while (false);

	//// 判断是否有增效器，增效器状态下的鼠标处理没有很好地实现???，功能没有调试
	//lpEnhancer = lpRecipient->GetEnhancer();
	//if(lpEnhancer != NULL)
	//{
	//	CElMouseTestState loState;
	//	// 发送增效器鼠标检测预处理
	//	luResult = SimpleSendMessage(lpEnhancer,EMSG_ENHANCER_PRE_MOUSE_TEST,&moPointToTest.Top().Point,sizeof(loState.Point),&loState.Point,sizeof(loState.Point));
	//	if(luResult != ERESULT_SUCCESS)
	//		return ERESULT_STOP_ENUM_CHILD;

	//	// 将新的鼠标位置压入测试用堆栈
	//	loState.mpElement = npRecipient;
	//	moPointToTest.Push(loState);
	//}

	return luResult;
}

// 鼠标落点检测后处理
ERESULT __stdcall CXelManager::LeaveForMouseTest(IEinkuiIterator* npRecipient)
{
	D2D1_RECT_F ldVisibleRegion;
	ERESULT luResult = ERESULT_SUCCESS;
	D2D1_POINT_2F ldLocalPos;
	CXuiIterator* lpRecipient = dynamic_cast<CXuiIterator*>(npRecipient);

	do 
	{
		if((mbXuiDragDrop != false && npRecipient == mpDraggingElement))
		{
			luResult = ERESULT_NOT_POINTOWNER;
			break;
		}
		if(npRecipient->IsVisible()==false)
			break;

		if(lpRecipient->CheckStyle(EITR_STYLE_MOUSE)==false)
		{
			luResult =  ERESULT_NOT_POINTOWNER;
			break;
		}

		// 计算局部坐标
		if(lpRecipient->WorldToLocal(moPointToTest.Top().Point,ldLocalPos) != false)
		{
			// 如果设置了可视区域，则判断是否在区域中
			if(lpRecipient->GetVisibleRegion(ldVisibleRegion)!=false && (ldLocalPos.x < ldVisibleRegion.left || ldLocalPos.x >= ldVisibleRegion.right || 
				ldLocalPos.y < ldVisibleRegion.top || ldLocalPos.y >= ldVisibleRegion.bottom) )
			{
				luResult = ERESULT_NOT_POINTOWNER;
			}
			else
			{
				// 给目标元素发送消息
				luResult = SimpleSendMessage(lpRecipient,EMSG_MOUSE_OWNER_TEST,&ldLocalPos,sizeof(ldLocalPos),NULL,0);
				if(luResult == ERESULT_MOUSE_OWNERSHIP)
				{
					// 试图承认是鼠标落点拥有者
					if(mpMouseMoveOn != NULL)
					{
						if(moPointToTest.Top().mbSeekInLevels == false)
						{
							luResult = ERESULT_EXIT_ENUM;
							break;
						}

						// 比较一下，新的对象是否被已有的层次高
						if(mlMouseMoveOnLevel >= moPointToTest.Top().mlCrtLevel)
						{
							// 低于或者等于，则不切换
							luResult = ERESULT_NOT_POINTOWNER;
							break;
						}
						// 释放当前
						mpMouseMoveOn->KRelease();
					}
					mpMouseMoveOn = lpRecipient;
					mpMouseMoveOn->KAddRefer();
					mlMouseMoveOnLevel = moPointToTest.Top().mlCrtLevel;
					mdPointRelative = ldLocalPos;
					if(moPointToTest.Top().mbSeekInLevels == false)
						luResult = ERESULT_EXIT_ENUM;
					else
						luResult = ERESULT_ENUM_CHILD;//ERESULT_EXIT_ENUM;
				}
			}
		}
		else
		{
			//Trace_Point(11900);// 矩阵不可逆，无法运算
			luResult= ERESULT_UNSUCCESSFUL;
		}

	} while (false);

	//  退栈
	moPointToTest.Pop();

	return luResult;
}

// 快捷键，返回false表示不是快捷键
bool CXelManager::HotKeyProcessor(CXuiIterator* npHost,const PSTELEMGR_WINMSG_FORWARD npKeyStrike)
{
	CXuiHotkeyEntry loFind;
	//int liIndex;
	STEMS_HOTKEY ldHotKey;

	// 取得按键的全部信息
	loFind.msuVkCode = (USHORT)npKeyStrike->wParam;
	loFind.mcuExKey = 0;
	if((GetKeyState( VK_CONTROL ) & 0x8000) != 0)
		loFind.mcuExKey |= loFind.eControl;
	if((GetKeyState( VK_SHIFT ) & 0x8000) != 0)
		loFind.mcuExKey |= loFind.eShit;
	if((GetKeyState( VK_MENU ) & 0x8000) != 0)
		loFind.mcuExKey |= loFind.eAlt;
	loFind.mpApplicant = NULL;

	// 检查全局的快捷键
	if(DetectHotKey(&moIteratorRoot,loFind)==false)
	{
		// 全局没有找到
		if(npHost != NULL)
			DetectHotKey(npHost,loFind);
	}

	if(loFind.mpApplicant != NULL)
	{
		// 发现快捷键了
		ldHotKey.HotKeyID = loFind.muHotKeyID;
		ldHotKey.Focus = npHost;
		ldHotKey.VkCode = loFind.msuVkCode;
		ldHotKey.Control = (loFind.mcuExKey&loFind.eControl)!=0;
		ldHotKey.Shift = (loFind.mcuExKey&loFind.eShit)!=0;
		ldHotKey.Alt = (loFind.mcuExKey&loFind.eAlt)!=0;

		if(CExMessage::SendMessage(loFind.mpApplicant,NULL,EMSG_HOTKEY_PRESSED,ldHotKey,NULL,0)==ERESULT_KEY_UNEXPECTED)
			return false;

		return true;
	}

	return false;
}

// 检测快捷键，返回false表示不是快捷键
bool CXelManager::DetectHotKey(CXuiIterator* npHost,CXuiHotkeyEntry& rToFind)
{
	bool lbFound = false;

	// 检查全局的快捷键
	//moIteratorLock.Enter();

	try
	{
		do 
		{
			lbFound = npHost->DetectHotKey(rToFind);
			if(lbFound !=false)
				break;

			npHost = (CXuiIterator*)npHost->GetParent();
		} while (npHost != (CXuiIterator*)npHost->GetParent());	// 一直找到根节点下一层
	}
	catch (...)
	{
		lbFound = false;
	}

	//moIteratorLock.Leave();

	return lbFound;
}


// 处理键盘输入转发
ERESULT CXelManager::OnMsgKeyboardForward(const PSTELEMGR_WINMSG_FORWARD npKeyStrike)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	CXuiIterator* lpKeyFocus = NULL;

	do 
	{
		if(mlCleanHumanInput != 0)
			break;

		lpKeyFocus = InnerGetKeyFocus();

		// 当键盘焦点为空时的普通按键消息，或者当不论键盘焦点状况的系统按键消息	<- 修改后，一律先判断快捷键
		if(npKeyStrike->WinMsgID == WM_KEYDOWN /*&& lpKeyFocus==NULL*/ || npKeyStrike->WinMsgID == WM_SYSKEYDOWN)
		{
			CXuiIterator* lpHotkeyFrom;
			bool lbFound = false;

			if(lpKeyFocus != NULL)
			{
				lpHotkeyFrom = lpKeyFocus;
				lpHotkeyFrom->KAddRefer();
			}
			else
			{
				lpHotkeyFrom = GetActiveElement();
			}

			if(lpHotkeyFrom != NULL)
			{
				lbFound = HotKeyProcessor(lpHotkeyFrom,npKeyStrike);

				lpHotkeyFrom->KRelease();
			}

			// 找到了快捷键，或，系统按键消息不再向元素发送
			if(lbFound!=false || npKeyStrike->WinMsgID == WM_SYSKEYDOWN)
			{
				luResult = ERESULT_SUCCESS;
				break;
			}
		}

		if(lpKeyFocus == NULL)
		{
			if(npKeyStrike->WinMsgID == WM_KEYUP)
			{
				STEMS_KEY_PRESSED ldStrike;

				ldStrike.IsPressDown = false;
				ldStrike.VirtualKeyCode = (USHORT)npKeyStrike->wParam;
				ldStrike.ExtendedKeyFlag = (ULONG)npKeyStrike->lParam;

				if(ldStrike.VirtualKeyCode == VK_TAB)// 如果是Tab消息，则转移键盘焦点
				{
					ShiftKeyBoardFocus(NULL);
				}
				//else
				//{
				//	KeyToCommand(&ldStrike);
				//}

				luResult = ERESULT_SUCCESS;
				break;
			}

			if(mlProbePass == 0)	// 判断是否进入探测模式
			{
				if(npKeyStrike->WinMsgID == WM_KEYDOWN)
				{
					if(LOSHORT(npKeyStrike->wParam)==VK_F9)
						mlProbePass = 1;
					else
					if(mlProbeMode > 0)
					{
						// 进入不同探测模式
						switch(LOSHORT(npKeyStrike->wParam))
						{
						case VK_F5:
							mlProbeMode = 1;
							break;
						case VK_F6:
							mlProbeMode = 2;
							break;
						case VK_F7:
							mlProbeMode = 3;
							break;
						case VK_F8:
							mlProbeMode = 4;
							break;
						default:
							mlProbePass = 0;
						}
					}
					else
						mlProbePass = 0;
				}
			}
			else
			if(npKeyStrike->WinMsgID == WM_CHAR)
			{
				if(	mlProbePass == 1 && LOSHORT(npKeyStrike->wParam)==L'e' || 
					mlProbePass == 2 && LOSHORT(npKeyStrike->wParam)==L's' ||
					mlProbePass == 3 && LOSHORT(npKeyStrike->wParam)==L'i' )
				{
					if(mlProbePass++ == 3)
					{
						if(mlProbeMode > 0)
							mlProbeMode = -1;
						else
							mlProbeMode = 1;

						mlProbePass = 0;
					}
				}
				else
					mlProbePass = 0;
			}

			luResult = ERESULT_SUCCESS;
			break;
		}

		if (npKeyStrike->WinMsgID == WM_CHAR)
		{
			STEMS_CHAR_INPUT ldChar;

			ldChar.CharIn = (USHORT)npKeyStrike->wParam;
			ldChar.Flags= (ULONG)npKeyStrike->lParam;

			luResult = SimpleSendMessage(lpKeyFocus,EMSG_CHAR_INPUT,&ldChar,sizeof(ldChar),NULL,0);
		}
		else
		if(npKeyStrike->WinMsgID == WM_KEYDOWN || npKeyStrike->WinMsgID == WM_KEYUP)
		{
			STEMS_KEY_PRESSED ldStrike;

			ldStrike.IsPressDown = (npKeyStrike->WinMsgID == WM_KEYDOWN);
			ldStrike.VirtualKeyCode = (USHORT)npKeyStrike->wParam;
			ldStrike.ExtendedKeyFlag = (ULONG)npKeyStrike->lParam;

			luResult = KeyStrike(lpKeyFocus,&ldStrike);
			if(luResult == ERESULT_KEY_UNEXPECTED && npKeyStrike->WinMsgID == WM_KEYDOWN)
			{
				// 键盘焦点不识别的按键，转换为快捷键
				//HotKeyProcessor(lpKeyFocus,npKeyStrike);

				luResult = ERESULT_SUCCESS;
			}

		}
		else
			luResult = ERESULT_WRONG_PARAMETERS;

	} while (false);

	if(lpKeyFocus != NULL)
		lpKeyFocus->KRelease();
	
	return luResult;
}

ERESULT CXelManager::OnMsgEnalbeInput(void)
{
	InterlockedExchange(&mlCleanHumanInput, 0);
	return ERESULT_SUCCESS;
}

// 发送键盘消息给目标，如果目标反馈ERESULT_UNEXPECTED_KEY，则，向上传递不支持的键盘按键信息，到Popup元素为止
ERESULT CXelManager::KeyStrike(CXuiIterator* npKeyFocus,const PSTEMS_KEY_PRESSED npStrike)
{
	ERESULT luResult;

	luResult = SimpleSendMessage(npKeyFocus,EMSG_KEY_PRESSED,npStrike,sizeof(STEMS_KEY_PRESSED),NULL,0);

	if(luResult == ERESULT_KEY_UNEXPECTED && npStrike->IsPressDown == false && npStrike->VirtualKeyCode == VK_TAB)// 如果是Tab消息，则转移键盘焦点
	{
		ShiftKeyBoardFocus(npKeyFocus);
	}

	return luResult;
}

// 处理Tab按键带来的键盘焦点切换
bool CXelManager::ShiftKeyBoardFocus(CXuiIterator* npKeyFocus)
{

	CXuiIterator* lpHost;
	CXuiIterator* lpMoveTo=NULL;
	bool lbOK = false;

	LockIterators();

	try
	{
		if(npKeyFocus == NULL)
		{
			lpHost = GetActiveElement();
			if(lpHost == NULL)
				THROW_FALSE;

			lpHost->KRelease();	// 直接减少一次引用计数，因为在互斥体中，不存在被释放的风险
		}
		else
		{
			lpHost = (CXuiIterator*)npKeyFocus->GetParent();
		}

		while(lpHost != (CXuiIterator*)lpHost->GetParent())
		{
			lpMoveTo = lpHost->GetNextKeyBoardAccepter(npKeyFocus);
			if(lpMoveTo != NULL)	// 找到新的接收点
				break;
			if(lpHost->CheckStyle(EITR_STYLE_POPUP)!=false)	// 当前的Host已经是Popup窗口了，就不能继续向上翻了
				break;
			if(npKeyFocus != NULL)	// 切换Host对象了，这个当前焦点就没意义了
				npKeyFocus = NULL;
			lpHost = (CXuiIterator*)lpHost->GetParent();
		}

		lbOK = true;
	}
	catch (...)
	{
	}

	UnlockIterators();

	if(lpMoveTo != NULL)
	{
		// 切换键盘焦点
		ChangeKeyFocus(lpMoveTo);
	}

	return lbOK;
}


//// 将Key消息转换为Command
//bool CXelManager::KeyToCommand(const PSTEMS_KEY_PRESSED npStrike)
//{
//	//ULONG luCmd;
//	bool lbDone = false;
//	CXuiIterator* lpAct = NULL;
//
//	//do
//	//{
//	//	lpAct = GetActiveElement();
//	//	if(lpAct == NULL || lpAct->CheckStyle(EITR_STYLE_KEY_COMMAND)==false)
//	//		break;
//
//	//	// 检查是否符合Command要求
//	//	switch(npStrike->VirtualKeyCode)
//	//	{
//	//	case VK_F1:
//	//		luCmd = (ULONG)nse_command::eInfo;
//	//		break;
//	//	case VK_ESCAPE:
//	//		luCmd = (ULONG)nse_command::eDeny;
//	//		break;
//	//	case VK_RETURN:
//	//		luCmd = (ULONG)nse_command::eConfirm;
//	//		break;
//	//	default:
//	//		luCmd = (ULONG)nse_command::eInvalid;
//	//	}
//	//	if(luCmd == (ULONG)nse_command::eInvalid && npStrike->VirtualKeyCode != VK_CONTROL && (GetKeyState(VK_CONTROL)&0x8000)!=0)
//	//	{
//	//		switch(npStrike->VirtualKeyCode)
//	//		{
//	//		case VK_LEFT:
//	//			luCmd = (ULONG)nse_command::eMoveLeft;
//	//			break;
//	//		case VK_RIGHT:
//	//			luCmd = (ULONG)nse_command::eMoveRight;
//	//			break;
//	//		case VK_UP:
//	//			luCmd = (ULONG)nse_command::eMoveUp;
//	//			break;
//	//		case VK_DOWN:
//	//			luCmd = (ULONG)nse_command::eMoveDown;
//	//			break;
//	//		case VK_NEXT:
//	//			luCmd = (ULONG)nse_command::eTurnRound;
//	//			break;
//	//		default:;
//	//		}
//	//	}
//	//	if(luCmd == (ULONG)nse_command::eInvalid)
//	//		break;
//
//	//	CExMessage::SendMessage(lpAct,NULL,EMSG_COMMAND,luCmd);
//
//	//	lbDone = true;
//
//	//}while(false);
//
//	//if(lpAct != NULL)
//	//	lpAct->KRelease();
//
//	return lbDone;
//}

// 增加Iterator的引用，由于XUI的客户程序可能会遗漏对Iterator的释放和引用操作，所以默认的Iterator->AddRef()和Iterator->Release()方法是假的，并不会产生实际的调用，但Element被Close后，对应的Iterator一定
// 会被释放；在本接口中提供了真实的引用和释放的方法操作Iterator对象，切记要谨慎操作，过多地释放将会导致XUI崩溃；
// 增加Iterator引用
int __stdcall CXelManager::AddRefIterator(IEinkuiIterator* npIterator)
{
	CXuiIterator* lpItrObj;

	if(npIterator == NULL || npIterator->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
		return ERESULT_ITERATOR_INVALID;

	lpItrObj = dynamic_cast<CXuiIterator*>(npIterator);
	if(lpItrObj == NULL)
		return ERESULT_ITERATOR_INVALID;

	return lpItrObj->KAddRefer();
}

// 释放Iterator
int __stdcall CXelManager::ReleaseIterator(IEinkuiIterator* npIterator)
{
	CXuiIterator* lpItrObj;

	if(npIterator == NULL || npIterator->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
		return ERESULT_ITERATOR_INVALID;

	lpItrObj = dynamic_cast<CXuiIterator*>(npIterator);
	if(lpItrObj == NULL)
		return ERESULT_ITERATOR_INVALID;

	return lpItrObj->KRelease();
}

// 获得鼠标焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
// 因为鼠标焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
IEinkuiIterator* __stdcall CXelManager::GetMouseFocus(void)
{
	return InnerGetMouseFocus();
}

// 获得键盘焦点，!!!注意!!!，返回的对象一定要调用ReleaseIterator释放；
// 因为键盘焦点随时可能改变，所有，返回的对象不一定能完全真实的反应当前的情况；
IEinkuiIterator* __stdcall CXelManager::GetKeyboardFocus(void)
{
	return InnerGetKeyFocus();
}

// 重置拖拽起点，仅当系统正处于拖拽行为中时，可以将拖转转移给他人
// 如果试图转移到的目标元素不能支持拖拽行为，当前的拖拽行为也会终止，当前拖拽的目标元素会收到Drag_end消息
ERESULT __stdcall CXelManager::ResetDragBegin(IEinkuiIterator* npToDrag)
{
	CElMouseFootPrint loMouseFoot;
	STMS_DRAGGING_ELE ldDragged;
	CXuiIterator* lpToDrag;

	// 必须处于拖拽状态，不能处于Drag-drop状态
	if(mbDragging == false || mbXuiDragDrop != false || npToDrag == NULL)
		return ERESULT_UNEXPECTED_CALL;

	lpToDrag = dynamic_cast<CXuiIterator*>(npToDrag);

	// 目标焦点是否支持拖拽
	if(lpToDrag->CheckStyle(EITR_STYLE_DRAG)==false)
		return ERESULT_DRAGGING_UNSUPPORT;

	if(moMouseTrace.Size() <= 0)
		return ERESULT_UNEXPECTED_CALL;

	loMouseFoot = moMouseTrace.Back();
	
	if(mpDragMsgReceiver != NULL)
	{
		// 发送鼠标拖拽结束消息
		ldDragged.DragOn = InnerGetMouseFocus();
		CalculateMouseMoving(mpDragMsgReceiver,loMouseFoot.Point,mdDragFrom,ldDragged.Offset);
		ldDragged.ActKey = muKeyWithDragging;
		ldDragged.KeyFlag = moMouseTrace.Back().KeyFlag;

		// 计算局部坐标
		mpDragMsgReceiver->WorldToLocal(loMouseFoot.Point,ldDragged.CurrentPos);

		CExMessage::SendMessage(mpDragMsgReceiver,NULL,EMSG_DRAG_END,ldDragged);
		// ??? 需要检查ldDragged.DragOn是否正确释放引用，Ax Jul.8
	}

	mbDragging = false;
	if(mpDragMsgReceiver != NULL)
		mpDragMsgReceiver->KRelease();
	mpDragMsgReceiver = NULL;
	//if(mpMouseFocus != NULL)
	//	mpMouseFocus->KRelease();	???
	//mpMouseFocus = NULL;//lpToDrag;

	DetectMouseDragBegin(lpToDrag,muKeyWithDragging,moMouseTrace.Back().KeyFlag,loMouseFoot.Point);

	return mbDragging!=false?ERESULT_SUCCESS:ERESULT_UNSUCCESSFUL;
}

// 清理输入消息LMSG_GET_TYPE(MsgId) == LMSG_TP_WIN_INPUT
void __stdcall CXelManager::CleanHumanInput(bool nbStallInput)
{
	InterlockedExchange(&mlCleanHumanInput, 1);

	if (nbStallInput == false)
		SimplePostMessage(NULL, EMSG_INPUT_ENABLE, NULL, 0, EMSG_POSTTYPE_FAST);
}


// 获得键盘焦点，需要释放
CXuiIterator* CXelManager::InnerGetKeyFocus(void)
{
	CXuiIterator* lpFocus;

	moMFocusLock.Enter();

	lpFocus = mpKbFocus;

	moMFocusLock.Leave();

	if(lpFocus != NULL)
		lpFocus->KAddRefer();

	return lpFocus;
}

// 改变键盘焦点
void CXelManager::ChangeKeyFocus(CXuiIterator* npNewFocus)
{
	STEMS_STATE_CHANGE ldChange;
	CXuiIterator* lpOld = NULL;
	CXuiIterator* lpNew = NULL;
	HWND lhWnd = FindWindow(L"OskMainClass",NULL);

	if (npNewFocus == NULL && lhWnd != NULL)
		::SendMessage(lhWnd,WM_SYSCOMMAND,SC_CLOSE,NULL);

	moMFocusLock.Enter();

	if(mpKbFocus != npNewFocus)
	{
		lpOld = mpKbFocus;

		mpKbFocus = npNewFocus;
		lpNew = npNewFocus;
		if(mpKbFocus!=NULL)
		{
			mpKbFocus->KAddRefer();
			lpNew->KAddRefer();
		}
	}
	else if(npNewFocus != NULL)
	{
		if(EinkuiGetSystem()->IsTouch() != false && lhWnd == NULL)
		{
			SHELLEXECUTEINFO ShExecInfo = {0}; 
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
			ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 
			ShExecInfo.hwnd = NULL; 
			ShExecInfo.lpVerb = NULL; 
			ShExecInfo.lpFile = L"Osk"; 
			ShExecInfo.nShow = SW_SHOW; 
			ShExecInfo.hInstApp = NULL; 
			ShellExecuteEx(&ShExecInfo); 

			CExMessage::PostMessage(npNewFocus,NULL,EMSG_KEYBOARD_SCREEN_KEYBOARD,CExMessage::DataInvalid);
		}
	}

	moMFocusLock.Leave();

	if(lpOld != lpNew)
	{
		if(lpOld != NULL)
		{
			ldChange.State = 0;	// lose
			ldChange.Related = lpNew;
			CExMessage::PostMessage(lpOld,NULL,EMSG_KEYBOARD_FOCUS,ldChange,EMSG_POSTTYPE_FAST);
		}

		if(lpNew != NULL)
		{
			ldChange.State = 1;	// got
			ldChange.Related = lpOld;

			if(EinkuiGetSystem()->IsTouch() != false && lhWnd == NULL)
			{
				SHELLEXECUTEINFO ShExecInfo = {0}; 
				ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO); 
				ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; 
				ShExecInfo.hwnd = NULL; 
				ShExecInfo.lpVerb = NULL; 
				ShExecInfo.lpFile = L"Osk"; 
				ShExecInfo.nShow = SW_SHOW; 
				ShExecInfo.hInstApp = NULL; 
				ShellExecuteEx(&ShExecInfo); 

				CExMessage::PostMessage(npNewFocus,NULL,EMSG_KEYBOARD_SCREEN_KEYBOARD,CExMessage::DataInvalid);
			}

			CExMessage::PostMessage(lpNew,NULL,EMSG_KEYBOARD_FOCUS,ldChange,EMSG_POSTTYPE_FAST);

// 			if(lpAccProxyObject != NULL)
// 				lpAccProxyObject->SetEditable(TRUE);

		}

		if(lpNew != NULL && lpNew->CheckStyle(EITR_STYLE_DISABLE_IME)!=false)
		{
			CEinkuiSystem::gpXuiSystem->GetImeContext()->DisableIme();
		}
		else
		{
			CEinkuiSystem::gpXuiSystem->GetImeContext()->EnableIme();
		}


	}


	if(lpOld!=NULL)
		lpOld->KRelease();
	if(lpNew!=NULL)
		lpNew->KRelease();
}

// 获得鼠标焦点，需要释放
CXuiIterator* CXelManager::InnerGetMouseFocus(void)
{
	CXuiIterator* lpFocus;

	moMFocusLock.Enter();

	lpFocus = mpMouseFocus;

	moMFocusLock.Leave();

	if(lpFocus != NULL)
		lpFocus->KAddRefer();

	return lpFocus;
}

// 获得活跃元素
CXuiIterator* CXelManager::GetActiveElement(void)
{
	CXuiIterator* lpAct;

	moMFocusLock.Enter();

	lpAct = mpActivatedElement;

	moMFocusLock.Leave();

	if(lpAct != NULL)
		lpAct->KAddRefer();

	return lpAct;
}

// 改变鼠标焦点
void CXelManager::ChangeMouseFocus(CXuiIterator* npNewFocus)
{
	CXuiIterator* lpOld;

	moMFocusLock.Enter();

	lpOld = mpMouseFocus;
	mpMouseFocus = npNewFocus;
	if(mpMouseFocus!=NULL)
		mpMouseFocus->KAddRefer();

	moMFocusLock.Leave();

	if(lpOld != NULL)
		lpOld->KRelease();

}

// 设置Active Popup元素
ERESULT CXelManager::AssignActivation(IEinkuiIterator* npToSet)
{
	if(VerifyIterator(npToSet)!= ERESULT_SUCCESS)
		return ERESULT_ITERATOR_INVALID;

	CXuiIterator* lpItr = dynamic_cast<CXuiIterator*>(npToSet);
	if(lpItr->CheckStyle(EITR_STYLE_ACTIVABLE)==false)
		return ERETULT_WRONG_STYLE;

	DetectKeyboardFocus(lpItr);

	ChangeActiveElement(lpItr);

	return ERESULT_SUCCESS;
}

// 设置Active Popup元素
void CXelManager::ChangeActiveElement(CXuiIterator* npSeekFrom)
{
	STEMS_ELEMENT_ACTIVATION ldActivation;
	CXuiIterator* lpOld = NULL;
	CXuiIterator* lpNew = NULL;

	moMFocusLock.Enter();

	lpOld = mpActivatedElement;
	//if(lpOld!=NULL)
	//	lpOld->KAddRefer(); 不该增加了，就是要释放它

	if(npSeekFrom != NULL)
	{
		while(npSeekFrom != (CXuiIterator*)npSeekFrom->GetParent())
		{
			if(npSeekFrom->CheckStyle(EITR_STYLE_ACTIVABLE)!=false)
			{
				mpActivatedElement = npSeekFrom;
				mpActivatedElement->KAddRefer();
				break;
			}
			npSeekFrom = (CXuiIterator*)npSeekFrom->GetParent();
		}
		// 到达根节点，没有元素获得激活，这不可能发生
		if(npSeekFrom == (CXuiIterator*)npSeekFrom->GetParent())
		{
			mpActivatedElement = NULL;
			lpNew = NULL;

		}
		else
		{
			lpNew = mpActivatedElement;
			lpNew->KAddRefer();		// twice
		}
	}
	else
	{
		mpActivatedElement = NULL;
		lpNew = NULL;
	}


	moMFocusLock.Leave();

	if(lpOld != lpNew)
	{
		ldActivation.Activated = lpNew;
		ldActivation.InActivated = lpOld;

		if(lpOld != NULL && lpOld->IsDeleted()==false)
		{
			ldActivation.State = 0;	// lose
//			ldChange.Related = lpNew;
			CXuiIterator* lpUp = lpOld;

			do 
			{
				// 向前一个活跃元素，及其所有同时失去活跃的父元素发送失去活跃消息
				SimpleSendMessage(lpUp,EMSG_ELEMENT_ACTIVATED,&ldActivation,sizeof(ldActivation),NULL,0);
				lpUp = (CXuiIterator*)lpUp->GetParent();
				if(lpUp == (CXuiIterator*)lpUp->GetParent() || lpUp->IsDeleted()!=false)
					break;
			} while (lpNew == NULL || lpNew->FindAncestor(lpUp)==false); // 没有新的激活元素，或者新的激活元素不是目前检查元素的子元素
		}

		if(lpNew != NULL)
		{
			ldActivation.State = 1;	// got
			//ldChange.Related = lpOld;
			CXuiIterator* lpUp = lpNew;

			do 
			{
				// 向前一个活跃元素，及其所有激活父元素发送激活消息
				SimpleSendMessage(lpUp,EMSG_ELEMENT_ACTIVATED,&ldActivation,sizeof(ldActivation),NULL,0);
				lpUp = (CXuiIterator*)lpUp->GetParent();
				if(lpUp == (CXuiIterator*)lpUp->GetParent() || lpUp->IsDeleted()!=false)
					break;
			} while (lpOld == NULL || lpOld->FindAncestor(lpUp)==false); // 改变状态前没有激活元素，或者先前的激活元素不是目前检查元素的子元素
		}
	}

	if(lpOld != NULL)
		lpOld->KRelease();
	if(lpNew != NULL)
		lpNew->KRelease();
}

// 进入模态对话状态
void CXelManager::EnterModal(void)
{
	// 清楚可能存在的Drag&drop状态
	TryDropDown(NULL);
}

// 申请键盘焦点，如果该元素具有popup属性，也将被调整到合适的上层
ERESULT CXelManager::ApplyKeyBoard(IEinkuiIterator* npIterator)
{
	CXuiIterator* lpAppEle;

	if(npIterator == NULL || npIterator->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
		return ERESULT_ITERATOR_INVALID;

	lpAppEle = dynamic_cast<CXuiIterator*>(npIterator);

	if(lpAppEle->CheckStyle(EITR_STYLE_KEYBOARD)==false)
		return ERESULT_ITERATOR_INVALID;

	ChangeKeyFocus(lpAppEle);

	return ERESULT_SUCCESS;
}

// 释放键盘焦点，这将导致Tab Order的下一个键盘接收者获得焦点
void CXelManager::ReleaseKeyBoard(PST_RELEASE_KEYFOCUS npRelease)
{
	CXuiIterator* lpKeyFocus;

	do 
	{
		lpKeyFocus = InnerGetKeyFocus();
		BREAK_ON_NULL(lpKeyFocus);

		if(lpKeyFocus == npRelease->CrtFocus)
		{
			if(npRelease->ShiftTab != false)
				ShiftKeyBoardFocus(lpKeyFocus);
			else
				ChangeKeyFocus(NULL);

		}

		lpKeyFocus->KRelease();

	} while (false);

}

// 发送Mouse wheel消息，如果当前鼠标焦点不接受这条消息，那么就判断在到达第一个Popup(包括第一个popup)之前是否有EITR_STYLE_ALL_MWHEEL的元素，需要接受Mouse Wheel消息
void CXelManager::TransferMouseWheel(CXuiIterator* npMouseFocus,STEMS_MOUSE_WHEEL& rInfor)
{
	ERESULT luResult = CExMessage::SendMessage(npMouseFocus,NULL,EMSG_MOUSE_WHEEL,rInfor);

	if(luResult != ERESULT_UNEXPECTED_MESSAGE)
		return;

	while(npMouseFocus->CheckStyle(EITR_STYLE_POPUP)==false)
	{
		npMouseFocus = (CXuiIterator*)npMouseFocus->GetParent();

		if(npMouseFocus == (CXuiIterator*)npMouseFocus->GetParent())
			break;

		if(npMouseFocus->CheckStyle(EITR_STYLE_ALL_MWHEEL)!=false)
		{
			CExMessage::SendMessage(npMouseFocus,NULL,EMSG_MOUSE_WHEEL,rInfor);
			break;
		}
	}
}

void CXelManager::CalculateMouseMoving(IEinkuiIterator* npOwner,const D2D1_POINT_2F& rCrtPos,const D2D1_POINT_2F& rLastPos,D2D1_POINT_2F& rResult)
{
	D2D1_POINT_2F ldCrt,ldLast;
	rResult.x = rResult.y = 0.0f;	//要初始化，否则是无效值
	if(npOwner->WorldToLocal(rLastPos,ldLast)!= false && npOwner->WorldToLocal(rCrtPos,ldCrt)!=false)
	{
		rResult.x = ldCrt.x - ldLast.x;
		rResult.y = ldCrt.y - ldLast.y;
	}
}

// 注册快捷键，当快捷键被触发，注册快捷键的Element将会受到；
// 如果普通按键组合（不包含Alt键)按下的当时，存在键盘焦点，按键消息会首先发送给键盘焦点，如果焦点返回ERESULT_KEY_UNEXPECTED才会判断是否存在快捷键行为
bool __stdcall CXelManager::RegisterHotKey(
	IN IEinkuiIterator* npApplicant,	// 注册的元素，将有它收到注册是快捷键消息
	IN ULONG nuHotKeyID,	// 事先定义好的常数，用来区分Hotkey；不能出现相同的ID，试图注册已有的Hotkey将会失败
	IN ULONG nuVkNumber,	// 虚拟键码
	IN bool nbControlKey,	// 是否需要Control组合
	IN bool nbShiftKey,		// 是否需要Shift组合
	IN bool nbAltKey,		// 是否需要Alt组合
	IN IEinkuiIterator* npFocus	// 指定焦点范围，仅当该元素及其子元素获得键盘焦点时，才会触发本次注册的快捷键;
	// 使用NULL作为参数而不指定焦点范围，则无论键盘焦点在何处都能够收到注册的快捷键的消息；
	)
{
	bool lbOK;

	if(npFocus == NULL)
	{
		moIteratorLock.Enter();
		// 增加到root
		lbOK = moIteratorRoot.RegisterHotKey(npApplicant,nuHotKeyID,nuVkNumber,nbControlKey,nbShiftKey,nbAltKey);
		moIteratorLock.Leave();
	}
	else
	{
		// 增加到作用域
		CXuiIterator* lpItrObj;
		if(npFocus->IsKindOf(GET_BUILTIN_NAME(CXuiIterator))==false)
			return false;

		lpItrObj = dynamic_cast<CXuiIterator*>(npFocus);

		moIteratorLock.Enter();
		lbOK = lpItrObj->RegisterHotKey(npApplicant,nuHotKeyID,nuVkNumber,nbControlKey,nbShiftKey,nbAltKey);
		moIteratorLock.Leave();

	}

	return lbOK;
}

// 注销快捷键
bool __stdcall CXelManager::UnregisterHotKey(
	IN IEinkuiIterator* npApplicant,	// 注册者
	IN ULONG UnuKeyNumber
	)
{
	// 暂不支持
	return false;
}

// 发送命令到合适的元素
ERESULT CXelManager::SendCommand(nes_command::ESCOMMAND neCmd)
{
	CXuiIterator* lpFocus;
	CXuiIterator* lpFrom;

	lpFocus = InnerGetKeyFocus();
	if(lpFocus == NULL)
	{
		lpFocus = GetActiveElement();
		if(lpFocus == NULL)
		{
			lpFocus = (CXuiIterator*)GetDesktop();
			lpFocus->KAddRefer();
		}
	}

	lpFrom = lpFocus;
	do 
	{
		moIteratorLock.Enter();

		do 
		{
			if(moIteratorVerification.Find(lpFrom) == moIteratorVerification.End())
			{	// 设置为根，阻止继续执行
				lpFrom = &moIteratorRoot;
				break;
			}

			if(lpFrom->CheckStyle(EITR_STYLE_COMMAND)!=false)
				break;

			lpFrom = (CXuiIterator*)lpFrom->GetParent();
		} while (lpFrom != (CXuiIterator*)lpFrom->GetParent());	// 一直找到根节点下一层

		moIteratorLock.Leave();

		// 如果当前目标不接受这条命令，将继续寻找小一个
		if(CExMessage::SendMessage(lpFrom,NULL,EMSG_COMMAND,neCmd)==ERESULT_SUCCESS)
			break;

		// 如果已经是根对象，则推出
		if(lpFrom == (CXuiIterator*)lpFrom->GetParent())
			break;

		lpFrom = (CXuiIterator*)lpFrom->GetParent();
		
	} while (lpFrom != NULL);

	if(lpFocus != NULL)
		lpFocus->KRelease();

	return ERESULT_SUCCESS;
}
