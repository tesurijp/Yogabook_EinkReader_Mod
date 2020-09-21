#include "stdafx.h"

#include "CommonHeader.h"
#include "XsMsgQueue.h"




//// 分离消息，将符合条件的消息依次分离到一个新的消息队列中
//void CXuiMessageQueue::DispartMessage(
//	IN ULONG nuMsgID,
//	IN IEinkuiIterator* npItr,	// 消息的发送目标是此对象及其子对象
//	OUT CXuiMessageQueue& rSaveTo
//	)
//{
//	CXuiMessage* lpMsgObj;
//	int liIndex;
//	int liPick;
//
//	rSaveTo.Clear();
//
//	moLock.Enter();
//
//	do 
//	{
//		if(Size()<=0)
//			break;
//		liIndex = CEsMessageFastList::Front();
//		while(liIndex > 0)
//		{
//			liPick = liIndex;
//			liIndex = CEsMessageFastList::Next(liIndex);
//
//			lpMsgObj = CEsMessageFastList::GetEntry(liPick);
//			if(lpMsgObj->muMsgID == nuMsgID &&( lpMsgObj->mpMsgDestItr == npItr || lpMsgObj->mpMsgDestItr->FindAncestor(npItr)!=false))
//			{
//				rSaveTo.Push_Back(lpMsgObj);
//				CEsMessageFastList::Remove(liPick);
//			}
//		}
//	} while (false);
//
//	moLock.Leave();
//}

// 获取特定消息
IEinkuiMessage* CXuiMessageQueue::GetMessage(
	IN ULONG nuMsgID,
	IN IEinkuiIterator* npItr	// 消息的发送目标是此对象及其子对象
)
{
	CXuiMessage* lpMsgObj;
	int liIndex;
	int liPick;
	IEinkuiMessage* lpGot = NULL;


	moLock.Enter();

	__try
	{
		if (Size() > 0)
		{
			liIndex = CEsMessageFastList::Front();
			while (liIndex > 0)
			{
				liPick = liIndex;

				lpMsgObj = CEsMessageFastList::GetEntry(liPick);
				if (lpMsgObj->muMsgID == nuMsgID && lpMsgObj->mpMsgDestItr == npItr)
				{
					lpGot = lpMsgObj;
					CEsMessageFastList::Remove(liPick);
					break;
				}

				liIndex = CEsMessageFastList::Next(liIndex);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	moLock.Leave();

	return lpGot;
}


// 清除全部消息
void CXuiMessageQueue::Clear(void)
{
	IEinkuiMessage* lpMsg;
	int liIndex;

	moLock.Enter();

	__try
	{
		while (CEsMessageFastList::Size() > 0)
		{
			liIndex = CEsMessageFastList::Front();
			if (liIndex >= 0)
			{
				lpMsg = CEsMessageFastList::GetEntry(liIndex);
				CEsMessageFastList::Remove(liIndex);
				lpMsg->Release();
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
	moLock.Leave();
}

bool CXuiMessageQueue::Push_Front(IEinkuiMessage* npMsg) {
	bool lbReval = false;

	moLock.Enter();

	__try
	{
		lbReval = Insert(CEsMessageFastList::Front(), dynamic_cast<CXuiMessage*>(npMsg), true) >= 0;

		if (miHistroy < Size())
		{
			miHistroy = Size();
			////Trace_int(27513,miHistroy);//消息队列扩大
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	moLock.Leave();

	return lbReval;
}

bool CXuiMessageQueue::Push_Back(IEinkuiMessage* npMsg) {
	bool lbReval = false;

	moLock.Enter();

	__try
	{
		lbReval = Insert(CEsMessageFastList::Back(), dynamic_cast<CXuiMessage*>(npMsg)) >= 0;

		if (miHistroy < Size())
		{
			miHistroy = Size();
			////Trace_int(13738,miHistroy);//消息队列扩大
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	moLock.Leave();

	return lbReval;
}

//bool CXuiMessageQueue::Push_Back_Reduce(IEinkuiMessage* npMsg){
//	bool lbReval;
//	IEinkuiIterator* lpSender;
//	IEinkuiIterator* lpReceiver;
//	PElMessage lpNextMsg;
//	int liIndex;
//	bool lbDel = false;
//	ULONG luMsgID;
//
//	moLock.Enter();
//
//	lpSender = npMsg->GetMessageSender();
//	lpReceiver = npMsg->GetDestination();
//	luMsgID = npMsg->GetMessageID();
//
//
//	// 从前向后搜寻同样的消息接收者和发送者，然后删除遇到的第二条消息
//	for(liIndex = CEsMessageFastList::Front();liIndex > 0;liIndex = CEsMessageFastList::Next(liIndex))
//	{
//		lpNextMsg = CEsMessageFastList::GetEntry(liIndex);
//		if(lpNextMsg->GetMessageID() == luMsgID && lpNextMsg->GetMessageSender()==lpSender && lpNextMsg->GetDestination()==lpReceiver)
//		{
//			if(lbDel != false)
//			{
//				// 删掉当前消息
//				CEsMessageFastList::RemoveByIndex(liIndex);
//
//				// 无需继续找了，按照这种原理，不可能已经存在3条和3条以上的相同消息
//				break;
//			}
//			else
//				lbDel = true;
//		}
//	}
//
//	lbReval = Insert(CEsMessageFastList::Back(),dynamic_cast<CXuiMessage*>(npMsg))>=0;
//
//	moLock.Leave();
//
//	if(miHistroy < Size())
//	{
//		miHistroy = Size();
//	}
//
//	return lbReval;
//}
//

IEinkuiMessage* CXuiMessageQueue::GetMessage(void) {
	IEinkuiMessage* lpMsg = NULL;
	int liIndex;

	moLock.Enter();
	__try
	{
		liIndex = CEsMessageFastList::Front();
		if (liIndex > 0)
		{
			lpMsg = CEsMessageFastList::GetEntry(liIndex);
			CEsMessageFastList::Remove(liIndex);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	moLock.Leave();

	return lpMsg;
}

// 获取对象数
int CXuiMessageQueue::Size(void) {
	int liSize;

	moLock.Enter();
	liSize = CEsMessageFastList::Size();
	moLock.Leave();

	return liSize;
}


//// 清楚全部原始输入消息
//int CXuiMessageQueue::RemoveMessages(
//	unsigned short nusType,
//	unsigned short nusMajNum,
//	unsigned short nusMinNum,
//	unsigned long nusMask 	// combination of the values that they are defined below
//	)
//{
//	CXuiMessage* lpMsgObj;
//	int liIndex;
//	int liPick;
//	int liRemoved = 0;
//
//	if (nusMask == 0)
//		return liRemoved;
//
//	moLock.Enter();
//
//	__try
//	{
//		if (Size() <= 0)
//			__leave;
//		liIndex = CEsMessageFastList::Front();
//		while(liIndex > 0)
//		{
//			liPick = liIndex;
//			liIndex = CEsMessageFastList::Next(liIndex);
//
//			lpMsgObj = CEsMessageFastList::GetEntry(liPick);
//			if(lpMsgObj == NULL)
//				continue;
//
//			if((nusMask&XSMSG_REMOVE_TYPE)!=0 && LMSG_GET_TYPE(lpMsgObj->muMsgID)!=nusType)
//				continue;
//
//			if((nusMask&XSMSG_REMOVE_MAJ)!=0 && LMSG_GET_MJNUM(lpMsgObj->muMsgID)!=nusMajNum)
//				continue;
//
//			if((nusMask&XSMSG_REMOVE_MIN)!=0 && LMSG_GET_MNNUM(lpMsgObj->muMsgID)!=nusMinNum)
//				continue;
//
//			CEsMessageFastList::Remove(liPick);
//			lpMsgObj->Release();
//			liRemoved++;
//		}
//	}
//	__except (EXCEPTION_EXECUTE_HANDLER)
//	{
//		liRemoved = 0;
//	}
//
//	moLock.Leave();
//
//	return liRemoved;
//}

// 清除掉指定的定时器消息
int CXuiMessageQueue::RemoveTimerMessage(
	const IEinkuiIterator* npTarget,	// null for all targets
	unsigned long nuTimerID		// 0 for all timer-message sending to the target
)
{
	CXuiMessage* lpMsgObj;
	PSTEMS_TIMER lpTimer;
	int liIndex;
	int liPick;
	int liRemoved = 0;

	moLock.Enter();

	__try
	{
		if (Size() <= 0)
			__leave;
		liIndex = CEsMessageFastList::Front();
		while (liIndex > 0)
		{
			liPick = liIndex;
			liIndex = CEsMessageFastList::Next(liIndex);

			lpMsgObj = CEsMessageFastList::GetEntry(liPick);

			if (lpMsgObj->muMsgID != EMSG_TIMER)
				continue;

			if (npTarget != NULL && lpMsgObj->mpMsgDestItr != npTarget)
				continue;

			if (lpMsgObj->GetInputDataSize() == sizeof(STEMS_TIMER) || nuTimerID != 0)	// 如果输入参数不合理，无需进一步判断了，下面删掉就是
			{
				lpTimer = (PSTEMS_TIMER)lpMsgObj->GetInputData();
				if (lpTimer != NULL)
					if (lpTimer->TimerID != nuTimerID)
						continue;
			}

			CEsMessageFastList::Remove(liPick);
			lpMsgObj->Release();
			liRemoved++;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}


	moLock.Leave();

	return liRemoved;
}
