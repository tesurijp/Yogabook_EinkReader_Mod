/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

#include "time.h"


template<typename DataType>
class CIdsPaireNode{
public:
	ULONG ID;
	DataType mdUserData;

	void operator=(const class CIdsPaireNode& src) {
		ID = src.ID;
		mdUserData = src.mdUserData;
	}
};

template<typename DataType>
class CIdsPaireNodeCriterion	// 默认的判断准则
{
public:
	bool operator () (const CIdsPaireNode<DataType>& Obj1,const CIdsPaireNode<DataType>& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.ID < Obj2.ID);
	}
};


// ID的生成算法，仅生成离散的值，不考虑冲突
class CIdsIDGenerator	// 默认的判断准则
{
public:
	CIdsIDGenerator(){
		srand((unsigned int)time(0));
	}
	ULONG operator () (ULONG nuSeed)const
	{
		int liRand = rand();
		do 
		{
			nuSeed = ((ULONG)LOWORD(nuSeed)<<(liRand%24))|((ULONG)HIWORD(nuSeed)^(liRand&0xFFFF))+1;
		} while (nuSeed == 0);

		return nuSeed;
	}
};

//////////////////////////////////////////////////////////////////////////
// ID集，用于自动分配唯一ID，并保存与之对应的数据，可以指定自己的ID分配算法，也可以使用默认的
template<typename DataType,class IDGenerator=CIdsIDGenerator,int InitSize=16,int Increment=16>
class cmmIDSet
{
public:
	// 保存一项内容，返回新分配的ID，返回0表示失败
	ULONG SaveItem(const DataType& UserData){
		int liIndex;
		int i;
		CIdsPaireNode<DataType> loNode;
		union {
			const void* p;
			ULONG d;
		}ldValue;
		ldValue.p = &UserData;

		loNode.mdUserData = UserData;
		loNode.ID = loIDGenerator(ldValue.d);

		i = 0;
		liIndex = -1;
		do 
		{
			liIndex = moSet.Insert(loNode);
			if(liIndex >= 0)
				break;
			loNode.ID = loIDGenerator(loNode.ID);
		} while (++i < 10000);
		
		if(liIndex >=0)
			return moSet[liIndex].ID;

		return 0;
	}

	// 保存一项内容，指定使用的ID，如果该ID已经存在，返回0失败，否则返回传入的ID
	ULONG SaveItem(const DataType& UserData,ULONG nuID){
		CIdsPaireNode<DataType> loNode;

		if(nuID == 0)
			return 0;

		loNode.mdUserData = UserData;
		loNode.ID = nuID;

		if(moSet.Insert(loNode)>=0)
			return nuID;
		return false;
	}

	// 修改一项内容
	bool UpdateItem(ULONG nuID,const DataType& UserData){
		int liIndex;
		CIdsPaireNode<DataType> loNode;

		loNode.ID = nuID;
		
		liIndex = moSet.Find(loNode);

		if(liIndex < 0)
			return false;

		moSet[liIndex].mdUserData = UserData;

		return true;
	}

	// 删除一项内容
	bool DeleteItem(ULONG nuID){
		CIdsPaireNode<DataType> loNode;

		loNode.ID = nuID;

		return moSet.Remove(loNode);
	};

	// 获得一项内容
	DataType& GetItem(ULONG nuID,DataType& Default)	{
		int liIndex;
		CIdsPaireNode<DataType> loNode;

		loNode.ID = nuID;

		liIndex = moSet.Find(loNode);

		if(liIndex < 0)
			return Default;

		return moSet[liIndex].mdUserData;
	}

	// 获得一项内容
	const DataType& GetItem(ULONG nuID,const DataType& Default)	{
		DataType ldBridge = Default;
		return GetItem(nuID,ldBridge);
	}

	void Clear(void) {
		moSet.Clear();
	}

	//获取数据数量
	int Size(void){
		return moSet.Size();
	}

	//根据索引返回对象
	DataType& GetItemByIndex(LONG nlIndex,DataType& Default){
		if(nlIndex < 0 || nlIndex > Size())
			return Default;

		return moSet[nlIndex].mdUserData;
	}

	//根据索引返回对象
	const DataType& GetItemByIndex(LONG nlIndex,const DataType& Default){
		DataType ldBridge = Default;

		return GetItemByIndex(nlIndex,ldBridge);
	}

	//根据数据获取该数据对应的ID
	ULONG GetIdByItem(DataType UserData,ULONG Default){
		int liIndex=0;
		for (liIndex=0;liIndex<Size();liIndex++)
		{
			if(moSet[liIndex].mdUserData == UserData)
				break;	//找到了
		}

		if(liIndex > 0 && liIndex < Size())
			return moSet[liIndex].ID;
		else
			return Default;
	}

protected:
	cmmSequence<CIdsPaireNode<DataType>,CIdsPaireNodeCriterion<DataType>,InitSize,Increment> moSet;
	IDGenerator loIDGenerator;
};


