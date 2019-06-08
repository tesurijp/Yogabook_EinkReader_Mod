/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

typedef CXuiMessage* PElMessage;
typedef cmmFastList<PElMessage ,128,64> CEsMessageFastList;

class CXuiMessageQueue : protected CEsMessageFastList{
	friend CXelManager;
public:
	CXuiMessageQueue(){
		miHistroy = 0;
	};
	~CXuiMessageQueue(){};

	// 插入本队列前部
	bool Push_Front(IEinkuiMessage* npMsg);

	// 插入本队列尾部
	bool Push_Back(IEinkuiMessage* npMsg);

	//// 插入本队列尾部，但执行Reduce运算
	//bool Push_Back_Reduce(IEinkuiMessage* npMsg);

	IEinkuiMessage* GetMessage(void);

	// 获取对象数
	int Size(void);

	// 清除全部消息
	void Clear(void);

	//// 分离消息，将符合条件的消息依次分离到一个新的消息队列中
	//void DispartMessage(
	//	IN ULONG nuMsgID,
	//	IN IEinkuiIterator* npItr,	// 消息的发送目标是此对象及其子对象
	//	OUT CXuiMessageQueue& rSaveTo	// 用于接收消息的其他队列，里面的内容应该被清空
	//	);

	// 获取特定消息
	IEinkuiMessage* GetMessage(
		IN ULONG nuMsgID,
		IN IEinkuiIterator* npItr	// 消息的发送目标是此对象及其子对象
		);

	//// 清除指定要求的全部消息
	//int RemoveMessages(
	//	unsigned short nusType,
	//	unsigned short nusMajNum,
	//	unsigned short nusMinNum,
	//	unsigned long nusMask	// combination of the values that they are defined below
	//	);

	// 清除掉指定的定时器消息
	int RemoveTimerMessage(
		const IEinkuiIterator* npTarget,	// null for all targets
		unsigned long nuTimerID		// 0 for all timer-message sending to the target
		);


private:
	CExclusiveAccess moLock;
	int miHistroy;



};

#define XSMSG_REMOVE_TYPE	1
#define XSMSG_REMOVE_MAJ	2
#define XSMSG_REMOVE_MIN	4


