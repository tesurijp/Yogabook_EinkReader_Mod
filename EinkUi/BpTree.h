/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */

/*++

Module Name:

    BpTree.h

Abstract:

    This module implements the bplustree struct and operations.

--*/

#ifndef _BPTREE_H_
#define _BPTREE_H_

#ifdef KERNEL_CODE

#include "ntddk.h"

#else

#include "Windows.h"

#endif//KERNEL_CODE


/*/////////////////////////////////////////////////////////////////////////
// B+树模板使用说明
///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
这个模板是用来干嘛的？
	本模板类帮助使用者将数据存储到B+树中，方便数据管理，并且大幅度提高查询
效率。 B+树采用大数据节点，可以减少零散内存分配。


//////////////////////////////////////////////////////////////////////////
保存什么到B+树中？
	可以保存用户自己定义的C++类对象，或者是C++类对象的指针。

1. 保存对象本身
	在这种方式下，B+树的节点中将保存有完整的用户对象。通过对象类重载的‘=’运算符，
将调用B+树的插入算法试提供的外部分配的对象赋值给B+树内部分配的对象。
	采用这种方法的好处很多，对于大量的小型数据，可以减少小型数据本身的内存分配。

2. 保存对象的指针
	B+树节点中保存的只是对象的指针，这会稍微增加查询计算的时间。但，对于
存储位置必须在外部的对象，这是唯一的方法，比如该对象是系统GDI数据对象等。

//////////////////////////////////////////////////////////////////////////
比较标准类
	使用本B+树模板，必须提供一个比较标准类，该类实现对两个存放类型的比较。请参考
本文件中的CBpTreeCriterion。

//////////////////////////////////////////////////////////////////////////
被保存对象
	如果B+树保存的是对象本身，该对象类型需要重载‘=’运算符，如果需要自动释放对象内
的外部资源，需要添加一些如引用计数之类的方法，可以参考demo程序的相关部分
	
//////////////////////////////////////////////////////////////////////////
// B+树的使用
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 运用高级篇
//////////////////////////////////////////////////////////////////////////
1. 提供最简化构造函数，提高效率
2. 为对象提供一个关联的关键值
3. 当排序用关键值不唯一时，如何建立B+树




/////////////////////////////////////////////////////////////////////////*/



class CBpTreeCriterion	// 默认的判断准则
{
public:
	bool operator () (const int& Obj1,const int& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1 < Obj2);
	}
	bool operator () (const void* Obj1,const void* Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1 < Obj2);
	}
};

class bplustree_node_base{
public:
	bplustree_node_base(){
		miEntryCount = 0;
	}
	~bplustree_node_base(){}
	bool mbIsLeaf;	// 表示这是叶子节点即最底层的节点
	int	miEntryCount;	// 内容数
	bplustree_node_base* mpParents;	// 指向父节点
	void* mpLowestObject;	// 指向最小对象，它可能处于本块中，也可能处于某级下属子块中
};
typedef bplustree_node_base* LPBPNODE;

#pragma pack(4)

template<class CBPlusTreeObjectClass,int ObjsInNode>
class bplustree_node : public bplustree_node_base
{
public:
	bplustree_node(){}
	~bplustree_node(){}
	CBPlusTreeObjectClass& operator [](int niIndex)
	{	// 为了提高效率，这儿不验证下标是否越界
		return mObjectsArray[niIndex];
	}
	CBPlusTreeObjectClass&  GetEntry(int niIndex)
	{
		return mObjectsArray[niIndex];
	}
	void Insert(CBPlusTreeObjectClass& nrObject,int niIndex);	//　插入一个对象到指定位置
	void Remove(int niIndex);	// 删除指定位置的对象
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* Split(int niCount);		// 将本节点分为两个，并移动niCount个数据到前一个节点，返回的对象就是新节点，新节点位于本节点前并且同本节点将前后链接好，但，不会设置它的上级节点
	void Combine(bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npWith);		// 将目标节点的数据合并到本节点，并且修改前后链接指针，本对象必须是目标对象的前对象

	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* mpLastSibling;
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* mpNextSibling;
protected:
	//bplustree_node
	CBPlusTreeObjectClass mObjectsArray[ObjsInNode];	// 存放对象的数组
};

#pragma pack()

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
class bplustree_iterator;


//////////////////////////////////////////////////////////////////////////
// 第一个参数给出的是对象的类型，第二个参数给出的是判断准则，第三个参数给出的是B+树的每一个节点中可以容纳的最大对象数，这个值绝对不能小于3
template<class CBPlusTreeObjectClass,class Criterion=CBpTreeCriterion,int ObjsInNode=30>
class bplustree 
{
protected:
	LPBPNODE mpRoot;	// 注意，可能是叶节点，也可能不是
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* mpFirstLeaf;
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* mpLastLeaf;
	int miTotalLeaves;
	int miTotalNodes;
	int miTotalObject;

#ifdef KERNEL_CODE
	KSPIN_LOCK SpinLock;
	KIRQL SavedIrql;
#else
	CRITICAL_SECTION CriticalSection;
#endif//KERNEL_CODE


public:
	bplustree();
	~bplustree();
	typedef bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> iterator;

	//////////////////////////////////////////////////////////////////////////
	// 标准接口																//
	//////////////////////////////////////////////////////////////////////////

	// 获取访问权限
	void EnterExclusiveAccess(void)const;
	// 释放访问权限
	void LeaveExclusiveAccess(void)const;

	// 取得树中的数据对象数目
	int GetCount(void)const{ return miTotalObject;}

	// 插入，返回成功与否，失败的原因可能是存在相同的值或者内存分配失败
	bool Insert(
		CBPlusTreeObjectClass& nrObject		// 待插入的对象，注意，该对象的内容可能会应对应类别的 = 运算符的行为而修改
		);
	// 插入，返回成功与否，失败的原因可能是存在相同的值或者内存分配失败，同上面的区别是，可以用常量做参数，当然，这个参数应该能够支持'='运算符，否则无法编译通过
	bool Insert(
		const CBPlusTreeObjectClass& nrObject		// 待插入的对象，注意，该对象的内容可能会应对应类别的 = 运算符的行为而修改
		);

	// 删除
	bool Remove(
		const CBPlusTreeObjectClass& nrObject // 查找到关键值意义上相同的对象，把它删除掉
		);

	// 删除
	bool Remove(
		bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>&  nrIterator // 查找到关键值意义上相同的对象，把它删除掉
		);

	// 查询，对查询结果的访问，受一定的限制；如果获得结果后，再次调用过插入或删除功能，将可能导致刚才获得的结果对象失效，所以，请避免在查询后和访问查询结果对象前，调用B+树的插入和删除算法
	iterator Find(
		const CBPlusTreeObjectClass& nrToFind, // 查找到关键值意义上相同的对象，提供用来做参数的对象，可以只设置关键值比较所需要的成员
		iterator* npEqualOrBelow=NULL	// 返回最接近查找对象的，等于或者仅比查找对象小的对象
		)const;

	// 取关键值最小的对象
	iterator Begin(void)const;
	// 取关键值最大的对象
	iterator ReverseBegin(void)const;
	// 表示无效的对象
	inline iterator End(void)const;
	// 直接取某个值，注意，如果niIndex超出全部数值范围，将返回无效的数据；另外，这个函数实质上不是真正的直接读取，当索引值超过一块叶节点的数据量时，将会逐块跳跃，效率需要考虑；故，如果是希望顺序遍历一串连续数据，请直接使用iterator的++和--来操作
	inline CBPlusTreeObjectClass& operator[] (int niIndex);

	// 清楚全部内容
	inline void Clear(void);

	void VerifyTree(LPBPNODE npNode)const;
	ULONG GetSizeOfLeaf(void)const;
	ULONG GetSizeOfBranch(void)const;

private:
	iterator mEndItr;
	Criterion mCriterion;

	// 释放一个分支上的所有下级节点
	void ReleaseEntriesInNode(
		LPBPNODE npNode
		);
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* FindInternal(
		const CBPlusTreeObjectClass& nrToFind,	// 查找的参照对象
		int& Index	// 返回对象的索引值
		)const;
	int FindInBranch(	// 在某个枝节点中寻找最大的小于或等于目标的节点
		bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
		const CBPlusTreeObjectClass& nrToFind	// 查找的参照对象
		)const;
	int FindInLeaf(	// 在某个叶节点中寻找最大的小于或等于目标的节点
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
		const CBPlusTreeObjectClass& nrToFind	// 查找的参照对象
		)const;
	bool InsertToLeaf(
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
		CBPlusTreeObjectClass& nrObject
		);
	bool InsertToBranch(
		bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
		LPBPNODE npNewNode
		);
	inline void UpdateLowestValue(	// 更新由该节点至上相关节点的最小值对象索引
		bplustree_node<LPBPNODE,ObjsInNode>* npBranch
		);
	bool NewLevel(	// 在根节点位置，新增加一层；实际行为就是一个根分裂为两个，并且增加一个新的根节点
		LPBPNODE npNodeAhead,
		LPBPNODE npNodeBehind
		);
	bool DeleteFromLeaf(
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
		const int niIndex
		);
	bool DeleteFromBranch(
		bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
		LPBPNODE npValue	// 值等于npValue的数据对象
		);

};

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
class bplustree_iterator
{
	friend class bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>;
public:
	bplustree_iterator(){
		mpCrtLeaf = NULL;
		miCrtIndex = 0;
	}
	bplustree_iterator(const bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>& src){
		mpCrtLeaf = src.mpCrtLeaf;
		miCrtIndex = src.miCrtIndex;
	}
	CBPlusTreeObjectClass& operator*() {	// 返回的对象，不要随意使用‘=’运算符赋值给别处，这很有可能导致这个对象内容发生转移，请确认能够安全使用‘=’运算符
		return mpCrtLeaf->GetEntry(miCrtIndex);
	}

	CBPlusTreeObjectClass* operator->() {	// return pointer to class object
		return &mpCrtLeaf->GetEntry(miCrtIndex);
	}
	void operator++() {
		if(miCrtIndex >= mpCrtLeaf->miEntryCount-1)
		{
			mpCrtLeaf = mpCrtLeaf->mpNextSibling;
			miCrtIndex = 0;
		}
		else
			miCrtIndex++;
	}
	void operator++(int) { // postfix
		if(miCrtIndex >= mpCrtLeaf->miEntryCount-1)
		{
			mpCrtLeaf = mpCrtLeaf->mpNextSibling;
			miCrtIndex = 0;
		}
		else
			miCrtIndex++;
	}

	void operator--(){
		if(miCrtIndex <= 0)
		{
			mpCrtLeaf = mpCrtLeaf->mpLastSibling;
			if(mpCrtLeaf != NULL)
				miCrtIndex = mpCrtLeaf->miEntryCount - 1;
		}
		else
			miCrtIndex--;
	}
	void operator--(int){ //postfix
		if(miCrtIndex <= 0)
		{
			mpCrtLeaf = mpCrtLeaf->mpLastSibling;
			if(mpCrtLeaf != NULL)
				miCrtIndex = mpCrtLeaf->miEntryCount - 1;
		}
		else
			miCrtIndex--;
	}
	void operator+= (int niOff) {

		if(mpCrtLeaf == NULL)
			return;

		miCrtIndex+= niOff;

		while(miCrtIndex >= mpCrtLeaf->miEntryCount)
		{
			miCrtIndex -= mpCrtLeaf->miEntryCount;
			mpCrtLeaf = mpCrtLeaf->mpNextSibling;
			if(mpCrtLeaf == NULL)
			{
				miCrtIndex = 0;
				return;
			}
		}
	}

	void operator-= (int niOff) {

		if(mpCrtLeaf == NULL)
			return;

		miCrtIndex -= niOff;

		while(miCrtIndex < 0)
		{
			mpCrtLeaf = mpCrtLeaf->mpLastSibling;
			if(mpCrtLeaf == NULL)
			{
				miCrtIndex = 0;
				return;
			}
			miCrtIndex += mpCrtLeaf->miEntryCount;
		}
	}

	bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> operator+ (int niOff)const {
		bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> loReturn = *this;
		loReturn += niOff;
		return loReturn;
	}

	bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> operator- (int niOff)const {
		bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> loReturn = *this;
		loReturn -= niOff;
		return loReturn;
	}

	void operator =(const bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>& src){
		mpCrtLeaf = src.mpCrtLeaf;
		miCrtIndex = src.miCrtIndex;
	}

	bool operator==(const bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>& cmp) const
	{
		if(mpCrtLeaf == cmp.mpCrtLeaf && miCrtIndex == cmp.miCrtIndex)
			return true;
		return false;
	}
	bool operator!=(const bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>& cmp) const
	{
		if(mpCrtLeaf != cmp.mpCrtLeaf || miCrtIndex != cmp.miCrtIndex)
			return true;
		return false;
	}
		
protected:
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* mpCrtLeaf;
	int miCrtIndex;
	//bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>* mpOwnerOfMe;
};



#if defined(DBG)||defined(_DEBUG)	// >>>>>>>>>>>>>>>>>>>

#ifdef KERNEL_CODE
#define BPASSERT(_X) {if(!(_X))DbgBreakPoint();}
#else
#define BPASSERT(_X) {if(!(_X))RaiseException(0x888888,0,0,0);}
#endif

#else
#define BPASSERT


#endif//  define(DBG) || define(_DEBUG) <<<<<<<<<<<<<<

#include "BpTree.cpp"





#endif//_BPTREE_H_
