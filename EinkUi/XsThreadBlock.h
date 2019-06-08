/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

#include "cmmIDSet.h"


class CXuiThreadBlock{
public:
	CXuiThreadBlock(){
	};
	~CXuiThreadBlock(){};

	//增加一个阻点，返回唯一的阻点ID
	ULONG AddBlock();

	//获得阻点的状况，返回ERESULT_BLOCK表示此阻点尚未完成，其他值表示已经完成;
	ERESULT ReadBlockState(ULONG nuID);

	//设定阻点的状况
	void SetBlockState(ULONG nuID,ERESULT nuState);

	//删除一个阻点
	void RemoveBlock(ULONG nuID);

private:
	cmmIDSet<ERESULT> moBlocks;
	CExclusiveAccess moAccLock;
};


class CXuiModalState
{
public:
	IEinkuiIterator* mpTarget;
	ULONG muBlock;
	void operator=(const CXuiModalState& src){
		mpTarget = src.mpTarget;
		muBlock = src.muBlock;
	}
};

class CXuiModalStack
{
public:
	CXuiModalStack(){}
	~CXuiModalStack(){}

	void AddModal(const CXuiModalState& rModalState);

	bool GetTopModel(CXuiModalState& rState);

	void RemoveModal(IEinkuiIterator* npModelElement);

	// 返回的是存入的BlockID
	ULONG GetBlockIDOfModal(IEinkuiIterator* npModelElement);

private:
	cmmVector<CXuiModalState,16,16> moStack;
	CExclusiveAccess moAccLock;

	int FindModal(IEinkuiIterator* npModelElement);
};