/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"
#include "Xsthreadblock.h"




//增加一个阻点，返回唯一的阻点ID
ULONG CXuiThreadBlock::AddBlock()
{
	ULONG luVlue;

	moAccLock.Enter();

	luVlue = moBlocks.SaveItem(ERESULT_BLOCK);

	moAccLock.Leave();

	return luVlue;
}

//获得阻点的状况，返回ERESULT_BLOCK表示此阻点尚未完成，其他值表示已经完成;
ERESULT CXuiThreadBlock::ReadBlockState(ULONG nuID)
{
	ERESULT luResult;
	
	moAccLock.Enter();
	luResult = moBlocks.GetItem(nuID,ERESULT_OBJECT_NOT_FOUND);
	moAccLock.Leave();

	return luResult;
}

//设定阻点的状况
void CXuiThreadBlock::SetBlockState(ULONG nuID,ERESULT nuState)
{
	moAccLock.Enter();
	moBlocks.UpdateItem(nuID,nuState);
	moAccLock.Leave();
}

//删除一个阻点
void CXuiThreadBlock::RemoveBlock(ULONG nuID)
{
	moAccLock.Enter();
	moBlocks.DeleteItem(nuID);
	moAccLock.Leave();
}



void CXuiModalStack::AddModal(const CXuiModalState& rModalState)
{
	moAccLock.Enter();
	moStack.Insert(0,rModalState);
	moAccLock.Leave();
}

bool CXuiModalStack::GetTopModel(CXuiModalState& rState)
{
	bool lbOK = false;
	moAccLock.Enter();
	if(moStack.Size() > 0)
	{
		rState = moStack[0];
		lbOK = true;
	}
	moAccLock.Leave();
	return lbOK;
}

// 返回的是存入的BlockID
void CXuiModalStack::RemoveModal(IEinkuiIterator* npModelElement)
{
	int liIndex;

	moAccLock.Enter();

	liIndex = FindModal(npModelElement);
	if(liIndex >= 0)
		moStack.RemoveByIndex(liIndex);

	moAccLock.Leave();
}

// 返回的是存入的BlockID
ULONG CXuiModalStack::GetBlockIDOfModal(IEinkuiIterator* npModelElement)
{
	int liIndex;
	ULONG luId=0;

	moAccLock.Enter();

	liIndex = FindModal(npModelElement);
	if(liIndex >= 0)
		luId = moStack[liIndex].muBlock;

	moAccLock.Leave();

	return luId;
}

int CXuiModalStack::FindModal(IEinkuiIterator* npModelElement)
{
	for (int i=0;i<moStack.Size();i++)
	{
		if(moStack[i].mpTarget == npModelElement)
		{
			return i;
		}
	}
	return -1;
}


