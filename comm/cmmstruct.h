/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


/*++

Do not use this code until allowed by Ensurebit rights. This code is only allowed for Ensurebit Internal Developers.

Module Name:

    cmmstruct.h

Abstract:

    This module implements vector,sequence,multi-sequence,stack and queue.

--*/

#ifndef _CMMSTRUCT_H_
#define _CMMSTRUCT_H_

#if defined(KERNEL_CODE)||defined(NATIVE_CODE)
#include "ntddk.h"
#else
#include "Windows.h"
#endif

#if defined(DBG)||defined(_DEBUG)	// >>>>>>>>>>>>>>>>>>>

#if defined(KERNEL_CODE)||defined(NATIVE_CODE)
#define CMMASSERT(_X) {if(!(_X))DbgBreakPoint();}
#else
#define CMMASSERT(_X) {if(!(_X))RaiseException(0x888888,0,0,0);}
#endif
#else
#define CMMASSERT

#endif		//  define(DBG) || define(_DEBUG) <<<<<<<<<<<<<<





/////////////////////////////////////////////////////////////////////////*/
// 矢量数组模板，注意，InitSize长度的初始数组将直接分配在本模板生成的对象本身
// 所有的内容对象，必定是存在一块连续存储区中，所以，考虑到每次插入删除导致的大量内存复制，不适合用于元素过多的目的和存储过于大的元素（大于256字节），单字长的元素少于1000，大字长的少于100

template<class CCmmContent,int InitSize=16,int Increment=16>
class cmmVector
{
public:
	cmmVector(){
		miCapacity = InitSize;
		mpArray = moInitArray;
		miSize = 0;
	}
	~cmmVector(){
		if(mpArray != moInitArray && mpArray != NULL)
		{
			delete []mpArray;
			mpArray = NULL;
		}
	}

	// 返回成员对象
	CCmmContent& operator [](int niIndex){
		return GetEntry(niIndex);
	}

	// 返回成员对象
	CCmmContent&  GetEntry(int niIndex){
		CMMASSERT(niIndex >= 0 && niIndex<miSize);

		return mpArray[niIndex];
	}

	// 返回当前的全部成员的存储区
	CCmmContent* GetBuffer(void){
		return mpArray;
	}

	// 清除全部成员
	void Clear(void){
		if(mpArray != moInitArray)
			delete []mpArray;
		miCapacity = InitSize;
		mpArray = moInitArray;
		miSize = 0;
		//for (int i=0;i<InitSize;i++)
		//{	不用显式初始化它们，这些存储区目前是未用状态，一点朝它们赋值，拷贝构造函数就会辈调用
		//	new(&moInitArray[i])CCmmContent();
		//}
	}

	// 返回有效成员数目
	int Size(void){
		return miSize;
	}

	// 返回存储容量
	int Capacity(void){
		return miCapacity;
	}

	// 返回第一个成员对象
	CCmmContent& Front(void){
		CMMASSERT(miSize>0);
		return mpArray[0];
	}

	// 返回最后一个成员对象
	CCmmContent& Back(void){
		CMMASSERT(miSize>0);
		return mpArray[miSize-1];
	}

	// 插入一个成员对象，返回实际插入的位置
	int Insert(
		IN int niIndex,	// -1 or one above Size() indicate to append element, others is the location
		IN CCmmContent& nrObject
		);

	// 插入一个成员对象，返回实际插入的位置
	int Insert(
		IN int niIndex,	// -1 or one above Size() indicate to append element, others is the location
		IN const CCmmContent& nrObject
		){
		CCmmContent loNew = nrObject;
		return Insert(niIndex,loNew);
	}

	// 删除一个成员对象，通过制定成员对象的位置
	bool RemoveByIndex(
		IN int niIndex	// -1 or one above Size() indicate the last element, others is the location
		);

	// 插入一个同类的矢量
	int Insert(
		IN int niIndex,	// -1 or one above Size() indicate to append element, others is the location
		IN const cmmVector<CCmmContent, InitSize, Increment>& srcVector
	);


protected:
	int miCapacity;
	int miSize;
	CCmmContent* mpArray;
	CCmmContent  moInitArray[InitSize+1];

	bool PrepareBuffer(bool nbIncrease){
		if(nbIncrease != false)
		{
			if(miSize+1 > miCapacity)
				return ExpandCapacity(true);
		}
		else
		{
			// 如果当前有效数据成员的数量小于总容量的3/4，并且小于总容量减去3/2倍增长量时，收缩
			if(miSize-1 < miCapacity - Increment - Increment/2 && miSize < (miCapacity*3)/4 && mpArray != moInitArray)
				return ExpandCapacity(false);
		}
		return true;
	}
	bool ExpandCapacity(bool nbIncrease,int ExpandTo=0);

};

template<class CCmmContent,int InitSize,int Increment>
bool cmmVector<CCmmContent,InitSize,Increment>::ExpandCapacity(bool nbIncrease, int ExpandTo)
{
	int liNewCap = (nbIncrease!=false)?miCapacity+Increment:miCapacity-Increment;

	if(ExpandTo != 0)
		liNewCap = ((ExpandTo + Increment - 1) / Increment)*Increment;

	CCmmContent* lpNewArray;
	if(liNewCap > InitSize)
	{
		lpNewArray = new CCmmContent[liNewCap];
	}
	else
	{
		CMMASSERT(mpArray != moInitArray);

		lpNewArray = moInitArray;
		liNewCap = InitSize;
		// 按理说需要对这些单元调用默认构造函数，但，有可能目标对象类没有提供下面的重载函数，所以，是不能支持这种调用构造函数的操作的
		// *** 仅为了配合placement new而提供的默认构造函数，Release Building中无任何存在
		// *** void* __cdecl operator new(size_t nSize,void* npObj){return npObj;}
		// *** 防止编译器出现警告，实现与上一个函数配对的析构函数，并不会调用它
		// *** void __cdecl operator delete(void* nObj,void* npObj){}
		//for (int i=0;i<InitSize;i++)
		//{
		//	new(&moInitArray[i])CCmmContent();
		//}
	}

	if(lpNewArray == NULL)
		return false;

	for(int liIndex=0;liIndex < miSize;liIndex++)
	{
		lpNewArray[liIndex] = mpArray[liIndex];
	}

	if(mpArray != moInitArray)
		delete []mpArray;

	mpArray = lpNewArray;
	miCapacity = liNewCap;

	return true;
}

template<class CCmmContent,int InitSize,int Increment>
int cmmVector<CCmmContent,InitSize,Increment>::Insert(
	IN int niIndex,	// -1 or one above Size() indicate to append element, others is the location
	IN CCmmContent& nrObject
	)
{
	if(niIndex < 0 || niIndex >= miSize)
		niIndex = miSize;
	if(PrepareBuffer(true) == false)
		return -1;

	for(int liIndex=miSize;liIndex > niIndex; liIndex--)
	{
		mpArray[liIndex] = mpArray[liIndex-1];
	}
	mpArray[niIndex] = nrObject;
	miSize++;

	return niIndex;
}
template<class CCmmContent,int InitSize,int Increment>
bool cmmVector<CCmmContent,InitSize,Increment>::RemoveByIndex(
	IN int niIndex	// -1 or one above Size() indicate the last element, others is the location
	)
{
	if(miSize == 0)
		return false;

	if(niIndex < 0 || niIndex >= miSize)
		niIndex = miSize-1;

	if(PrepareBuffer(false) == false)
		return false;

	// 将其后的对象全部前移一格
	for(int liBehind = niIndex;liBehind < miSize-1;liBehind++)
	{
		mpArray[liBehind] = mpArray[liBehind+1];
	}
	miSize--;

	return true;
}

// 插入一个同类的矢量
template<class CCmmContent, int InitSize, int Increment>
int cmmVector<CCmmContent, InitSize, Increment>::Insert(
	IN int niIndex,	// -1 or one above Size() indicate to append element, others is the location
	IN const cmmVector<CCmmContent, InitSize, Increment>& srcVector
)
{
	int newSize = miSize + srcVector.miSize;
	if (ExpandCapacity(1,newSize) == false)
		return 0;

	if (niIndex < 0 || niIndex >= miSize)
		niIndex = miSize;

	for (int liIndex = miSize-1,liDst = newSize-1; liIndex >= niIndex; liIndex--,liDst--)
	{
		mpArray[liDst] = mpArray[liIndex];
	}

	for (int liIndex = 0; liIndex < srcVector.miSize; liIndex++)
	{
		mpArray[liIndex+niIndex] = srcVector.mpArray[liIndex];
	}

	miSize = newSize;

	return miSize;
}

//////////////////////////////////////////////////////////////////////////
// 快速链表内部数据结构，快速链定义在下面
template<class CCmmContent>
class cmmNodeOfFastList
{
public:
	cmmNodeOfFastList(){}
	~cmmNodeOfFastList(){}

	cmmNodeOfFastList<CCmmContent>& operator =(cmmNodeOfFastList<CCmmContent>& src){
		moData = src.moData;
		LastNode = src.LastNode;
		NextNode = src.NextNode;
		return *this;
	}

#pragma pack(4)
	LONG LastNode;	// 指向前一个节点，链表的首节点将会指向自己；Free node设置为-1
	LONG NextNode;
#pragma pack()
	CCmmContent moData;
};

//////////////////////////////////////////////////////////////////////////
// 快速链表，不适合存放过大的内容（32字节），不适合存放过多的节点（多余32K）；主要用于存放小对象（注意指针也是小对象），频繁插入删除，较少便利查询的情况；无论是插入、删除还是查询都要优于普通链表结构。
// 原理：每个链表节点并不是独立分配的内存，它们都连续存放在一个cmmVector数组中，当某个节点被删除后，仅仅是把它从链表上摘掉，内存对象仍然在cmmVector中没有释放，这样就可以避免
// 链表增删节点导致的大量内存分配操作；当查找一个对象时，快速链表将在数组中遍历查找，速度会优于普通链表；
// 注意：本模板类的大多数函数使用的Index索引是用于获取和指定某个节点的，并不是代表某个节点在链表中的前后次序，小于0表示失败或者错误的索引
template<class CCmmContent,int InitSize=16,int Increment=16>
class cmmFastList:protected cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>
{
public:
	cmmFastList(){
		miValidNodes = 0;
		miFirstFree = -1;
		CMMASSERT(Increment != 0);

		cmmNodeOfFastList<CCmmContent> loNode;
		loNode.NextNode = loNode.LastNode = 0;
		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::Insert(0,loNode);
	}
	~cmmFastList(){}

	// 插入一个节点，返回值是插入节点的索引，这个索引是存储在内部数组中的位置，并不是链表中的前后次序；返回值小于等于0表示失败
	int Insert(
		int niNodeRef,	// 插入到这个节点前后，这个值必须是某次获得的有效节点的索引
		CCmmContent& nrObject,
		bool nbBefore=false	// 插入到前或者后
		);

	// 插入一个节点，返回值是插入节点的索引，这个索引是存储在内部数组中的位置，并不是链表中的前后次序；返回值小于等于0表示失败
	int Insert(
		int niNodeRef,	// 插入到这个节点前后，这个值必须是某次获得的有效节点的索引
		const CCmmContent& nrObject,
		bool nbBefore=false	// 插入到前或者后
		){
		CCmmContent loNew = nrObject;
		return Insert(niNodeRef,loNew,nbBefore);
	}

	// 删除一个节点
	bool Remove(
		int niNodeRef
		);

	// 清除全部节点
	bool Clear(){return false;}
	 
	// 获得链表的有效成员数
	int Size(){
		return miValidNodes;
	}

	// 获得链表的第一个节点的索引，然后通过[]运算符或者GetEntry访问目标节点的内容
	int Front(void){
		return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(0).NextNode;
	}

	// 获得链表的最后一个节点的索引，然后使用[]运算符或者GetEntry访问目标节点的内容
	int Back(void){
		return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(0).LastNode;
	}

	// 获得下一个节点
	int Next(int niNodeRef);

	// 获得前一个节点
	int Previous(int niNodeRef);

	// 查找某个节点，如果要使用这个方法，CCmmContent必须提供operator ==运算符支持，对于基本类型系统提供了默认的支持，而自定义的类这需要重载这个运算符
	// 注意：某个CCmmContent对象在链表中可能不唯一，获得下一个等价对象，请继续Find
	int Find(
		const CCmmContent& nrObject,
		int niOver= 0	// 从这个索引对象之后(不包括niOver本身所指)查找，默认从第一位开始
		);

	// 返回成员对象
	CCmmContent& operator [](
		int niIndex	// 成员索引值，注意：这个值不是成员在链表中的排序，而是存储在数组中的序号；它必须是通过Front Back Next Previous等函数获得的值，而不能值用户自己随意设定的
		){
		return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niIndex).moData;
	}

	// 返回成员对象
	CCmmContent&  GetEntry(
		int niIndex	// 成员索引值，注意：这个值不是成员在链表中的排序，而是存储在数组中的序号；它必须是通过Front Back Next Previous等函数获得的值，而不能值用户自己随意设定的
		){
		CMMASSERT(niIndex >= 0 && niIndex<miSize);
		return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niIndex).moData;
	}
	
protected:
	int miValidNodes;
	int miFirstFree;

	// 保存一个闲置的存储单元
	bool SaveFreeNode(int niNodeRef){
		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).NextNode = miFirstFree;
		miFirstFree = niNodeRef;
		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode = -1;	// 表示它已经被删除了

		if(miValidNodes < miCapacity/2 && mpArray != moInitArray)
			return RecoverDataBuffer();

		return true;
	}

	// 恢复Buffer为小尺寸
	bool RecoverDataBuffer(){
		cmmNodeOfFastList<CCmmContent>* lpNewArray;
		int liNewCap = ((miValidNodes+Increment-1)/Increment)*Increment;	// 取最接近的倍数

		if(liNewCap > InitSize)
		{
			lpNewArray = new cmmNodeOfFastList<CCmmContent>[liNewCap];
		}
		else
		{
			lpNewArray = moInitArray;
			liNewCap = InitSize;
		}

		if(lpNewArray == NULL)
			return false;

		int liSrc = 0;	// 我们的链表头也要复制过去
		int liDst = 0;

		do
		{
			lpNewArray[liDst].moData = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(liSrc).moData;
			lpNewArray[liDst].LastNode = liDst-1;
			lpNewArray[liDst].NextNode = liDst+1;

			liSrc = Next(liSrc);
			liDst++;
		}while(liSrc > 0);
		CMMASSERT(miValidNodes == liDst-1); // 链表头不能计算其中

		miFirstFree = -1;

		lpNewArray[0].LastNode = liDst-1;
		lpNewArray[liDst-1].NextNode = 0;

		miSize = miValidNodes+1;

		if(mpArray != moInitArray)
			delete []mpArray;

		mpArray = lpNewArray;
		miCapacity = liNewCap;

		return true;
	}

};

template<class CCmmContent,int InitSize,int Increment>
int cmmFastList<CCmmContent,InitSize,Increment>::Insert(
	int niNodeRef,	// 插入到这个节点前后，这个值必须是某次获得的有效节点的索引
	CCmmContent& nrObject,
	bool nbBefore	// 插入到前或者后
	)
{
	int liSet;

	// 首先确认插入的参考位置是否正确
	if(niNodeRef < 0 || niNodeRef >= miSize)
		return 0;	// 返回失败

	if(niNodeRef >= 0 && cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode < 0)
		return 0; // 这不是有效链表节点的索引，返回失败

	cmmNodeOfFastList<CCmmContent> loNode;
	loNode.moData = nrObject;

	// 确认是否存在未使用的节点
	if(miFirstFree < 0)
	{
		// 目前没有未使用的节点，直接在基类对象增加一个记录
		liSet = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::Insert(-1,loNode);
		if(liSet < 0)
			return 0;	//失败
	}
	else
	{
		liSet = miFirstFree;
		miFirstFree = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(liSet).NextNode;
	}

	if(nbBefore != false)
	{	// 插入到参考位置前
		loNode.LastNode = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode;
		loNode.NextNode = niNodeRef;

		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(liSet) = loNode;

		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode = liSet;
		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(loNode.LastNode).NextNode = liSet;

	}
	else
	{
		// 插入到参考位置后
		loNode.LastNode = niNodeRef;
		loNode.NextNode = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).NextNode;

		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(liSet) = loNode;

		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(loNode.NextNode).LastNode = liSet;
		cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).NextNode = liSet;
	}
	
	miValidNodes++;
	return liSet;
}

template<class CCmmContent,int InitSize,int Increment>
bool cmmFastList<CCmmContent,InitSize,Increment>::Remove(int niNodeRef)
{
	if(niNodeRef <= 0 || niNodeRef >= miSize)
		return false;

	if(cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode < 0)	// 已经删除了
		return true;

	cmmNodeOfFastList<CCmmContent> loCrt = cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef);
	cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(loCrt.LastNode).NextNode = loCrt.NextNode;
	cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(loCrt.NextNode).LastNode = loCrt.LastNode;

	// 将这个失效的单元保存到闲置列表中，并且尝试缩小总存储空间
	miValidNodes--;
	return SaveFreeNode(niNodeRef);
}

template<class CCmmContent,int InitSize,int Increment>
int cmmFastList<CCmmContent,InitSize,Increment>::Next(int niNodeRef)
{
	// niNodeRef < 0 与下一个函数不同，这儿允许用0来获得链表首节点
	if(niNodeRef < 0 || niNodeRef >= miSize || cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode < 0)
		return 0;

	return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).NextNode;
}

template<class CCmmContent,int InitSize,int Increment>
int cmmFastList<CCmmContent,InitSize,Increment>::Previous(int niNodeRef)
{
	if(niNodeRef <= 0 || niNodeRef >= miSize || cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode < 0) 
		return 0;

	return cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niNodeRef).LastNode;
}

template<class CCmmContent,int InitSize,int Increment>
int cmmFastList<CCmmContent,InitSize,Increment>::Find(
	const CCmmContent& nrObject,
	int niOver	// 从这个索引对象之后查找，默认从第一位开始
	)
{
	if (niOver <= 0)
		niOver = Front();

	while(niOver > 0)
	{
		if(cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niOver).moData == nrObject && cmmVector<cmmNodeOfFastList<CCmmContent>,InitSize,Increment>::GetEntry(niOver).LastNode >= 0)
			break;

		niOver = Next(niOver);
	}

	if(niOver >= miSize)
		niOver = 0;

	return niOver;
}


//////////////////////////////////////////////////////////////////////////
// 排序数组用排序比较类
class CSequenceCriterion	// 默认的判断准则
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
//////////////////////////////////////////////////////////////////////////
// 排序数组，不适合放置过大的内容（128个字节），不适合放置过多的对象，数组将保持存储区为最大容量时的容量
template<class CCmmContent,class Criterion=CSequenceCriterion,int InitSize=16,int Increment=16>
class cmmSequence : protected cmmVector<CCmmContent,InitSize,Increment>
{
public:
	cmmSequence(){}
	~cmmSequence(){}

	// 查找一个等值的已保存元素，返回相等效的结构内对象的索引，如果返回为-1则没有找到
	int Find(const CCmmContent& nrFind)
	{
		return InternalFind(nrFind,NULL);
	}

	// 插入，返回-1表示失败，否则表示插入的位置
	int Insert(const CCmmContent& nrKeyObj)
	{
		CCmmContent loContent = nrKeyObj;
		return Insert(loContent);
	}
	// 插入，返回-1表示失败，否则表示插入的位置
	int Insert(CCmmContent& nrKeyObj)
	{
		int liLower;
		int liIndex = InternalFind(nrKeyObj,&liLower);
		if(liIndex >= 0)
			return -1;

		liLower++;
		
		return cmmVector<CCmmContent,InitSize,Increment>::Insert(liLower,nrKeyObj);
	}
	// 添加一个元素到最尾部，只有确定这个元素的排序结果在尾部才能这么调用，否则会弄坏整个排序队列，本函数通常只用于从保存到文件的结构中装回整个结构，返回值是本元素插入的位置
	int Push_Back(const CCmmContent& nrKeyObj)
	{
		CCmmContent loContent = nrKeyObj;
		return Push_Back(loContent);
	}
	// 添加一个元素到最尾部，只有确定这个元素的排序结果在尾部才能这么调用，否则会弄坏整个排序队列，本函数通常只用于从保存到文件的结构中装回整个结构，返回值是本元素插入的位置
	int Push_Back(CCmmContent& nrKeyObj)
	{
		return cmmVector<CCmmContent,InitSize,Increment>::Insert(-1,nrKeyObj);
	}

	// 删除，返回成功与否，niIndex待删除的节点的索引
	bool RemoveByIndex(int niIndex)
	{
		return cmmVector<CCmmContent,InitSize,Increment>::RemoveByIndex(niIndex);
	}

	// 删除，返回成功与否，
	bool Remove(const CCmmContent& nrKeyObj)
	{
		return RemoveByIndex(Find(nrKeyObj));
	}

	CCmmContent& operator [](int niIndex)
	{
		return GetEntry(niIndex);
	}

	void Clear(void){
		cmmVector<CCmmContent,InitSize,Increment>::Clear();
	}

	// 获取对象数
	int Size(void)
	{
		return cmmVector<CCmmContent,InitSize,Increment>::Size();
	}

	// 测试整个数组是否是有序的，执行速度很慢，仅用于调试和阶段性检测
	bool SequenceTest(void){
		int liLower = 0;
		bool lbReval = true;

		while(liLower+1 < Size())
		{
			if(mCriterion(GetEntry(liLower),GetEntry(liLower+1)) == false)
			{
				lbReval = false;
				break;
			}
			liLower++;
		}
		return lbReval;
	}
protected:
	int InternalFind(const CCmmContent& nrFind,int* npLower)
	{
		int liTop,liBottom,liCrt;
		liTop = miSize-1;
		liBottom = 0;

		while(liBottom <= liTop)
		{
			// 折半查找
			liCrt = (liTop + liBottom)/2;

			// 不小于当前值
			if(mCriterion(nrFind,GetEntry(liCrt)) == false)
			{
				// 是否相等
				if(mCriterion(GetEntry(liCrt),nrFind) == false)
					return liCrt;
				liBottom = liCrt+1;
			}
			else
				liTop = liCrt-1;
		}
		if(npLower != NULL)
			*npLower = liTop;
		return -1;
	}
	Criterion mCriterion;
};

//////////////////////////////////////////////////////////////////////////
// 多元排序数组，能够放置多个排序条件下相等的元素。不适合放置过大的内容（128个字节），不适合放置过多的对象，数组将保持存储区为最大容量时的容量
template<class CCmmContent,class Criterion=CSequenceCriterion,int InitSize=16,int Increment=16>
class cmmMultiSequence : public cmmSequence<CCmmContent,Criterion,InitSize,Increment>
{
public:
	// 查找第一个等价的已保存元素，返回相等效的结构内对象的索引，如果返回为-1则没有找到
	// 获取后续的等价元素，直接使用索引向后加一枚举并自行判断，或者调用下面的NextEquivalent
	int Find(const CCmmContent& nrFind)
	{
		int liReval = InternalFind(nrFind,NULL);
		while(liReval > 0)
		{
			if(mCriterion(nrFind,GetEntry(liReval-1)) != false || mCriterion(GetEntry(liReval-1),nrFind)!=false)
				break;	// 如果前一个已经不等价，则退出循环
			liReval--;
		}
		return liReval;
	}

	// 插入，返回-1表示失败，否则表示插入的位置，nbAhead!=false时，将该对象插入到等价的对象之前，否则插入在等价对象之后
	int Insert(const CCmmContent& nrKeyObj,bool nbAhead=false)
	{
		CCmmContent loContent = nrKeyObj;
		return Insert(loContent,nbAhead);
	}
	// 插入，返回-1表示失败，否则表示插入的位置，nbAhead!=false时，将该对象插入到等价的对象之前，否则插入在等价对象之后
	int Insert(CCmmContent& nrKeyObj,bool nbAhead=false)
	{
		int liIndex;

		liIndex = Find(nrKeyObj);
		if(liIndex < 0)	// 如果没有相等的值，才去找比它小的值
		{
			InternalFind(nrKeyObj,&liIndex);
			liIndex++;
		}
		else
		{
			if(nbAhead == false)
			{
				// 找到相同值之后的插入位置
				while(NextEquivalent(liIndex++)>=0);
			}
		}

		return cmmVector<CCmmContent,InitSize,Increment>::Insert(liIndex,nrKeyObj);
	}

	// 取下一个等价的元素，返回-1表示没有了
	int NextEquivalent(int niCurrent)
	{
		if(niCurrent < 0 || niCurrent+1 >= miSize)
			return -1;

		if(mCriterion(GetEntry(niCurrent),GetEntry(niCurrent+1)) == false && mCriterion(GetEntry(niCurrent+1),GetEntry(niCurrent))==false)
			return niCurrent+1;
		return -1;
	}

	// 删除全部等价的元素，返回成功与否，
	bool Remove(const CCmmContent& nrKeyObj)
	{
		int liReval = Find(nrKeyObj);
		while(liReval >= 0 )
		{
			if(Remove(liReval)==false)
				return false;

			liReval = Find(nrKeyObj);
		}

		return true;
	}
	// 删除，返回成功与否，niIndex待删除的节点的索引
	bool RemoveByIndex(int niIndex)
	{
		return cmmSequence<CCmmContent,Criterion,InitSize,Increment>::RemoveByIndex(niIndex);
	}
};


//////////////////////////////////////////////////////////////////////////
// 堆栈结构
template<class CCmmContent,int InitSize=16,int Increment=16>
class cmmStack :protected cmmVector<CCmmContent,InitSize,Increment>
{
public:
	cmmStack(){}
	~cmmStack(){}

	bool Push(const CCmmContent& nrObject){
		return cmmVector<CCmmContent,InitSize,Increment>::Insert(-1,nrObject)>=0;
	}
	CCmmContent& Top(void){
		return cmmVector<CCmmContent,InitSize,Increment>::Back();
	}
	void Pop(void){
		cmmVector<CCmmContent,InitSize,Increment>::RemoveByIndex(-1);
	}
	int Size(void) {
		return cmmVector<CCmmContent,InitSize,Increment>::Size();
	}
	// 清除全部成员
	void Clear(void){
		cmmVector<CCmmContent,InitSize,Increment>::Clear();
	}

	// 返回成员对象
	CCmmContent& operator [](int niIndex){
		return cmmVector<CCmmContent,InitSize,Increment>::GetEntry(niIndex);
	}

	// 返回成员对象
	CCmmContent&  GetEntry(int niIndex){
		return cmmVector<CCmmContent,InitSize,Increment>::GetEntry(niIndex);
	}

};


//////////////////////////////////////////////////////////////////////////
// 队列，不适合存放过多(大于32K)的成员，也不适合存放过大(大于16个字节，任何指针类型都只有4字节)的对象
template<class CCmmContent,int InitSize=16,int Increment=16>
class cmmQueue:protected cmmFastList<CCmmContent,InitSize,Increment> {
public:
	cmmQueue(){};
	~cmmQueue(){};
	bool Push_Front(const CCmmContent& nrObject){
		return Insert(cmmFastList<CCmmContent,InitSize,Increment>::Front(),nrObject,true)>=0;
	}
	bool Push_Back(const CCmmContent& nrObject){
		return Insert(cmmFastList<CCmmContent,InitSize,Increment>::Back(),nrObject)>=0;
	}
	void Pop_Front(void){
		Remove(cmmFastList<CCmmContent,InitSize,Increment>::Front());
	}
	void Pop_Back(void) {
		Remove(cmmFastList<CCmmContent,InitSize,Increment>::Back());
	}
	CCmmContent& Front(void){
		return cmmFastList<CCmmContent,InitSize,Increment>::GetEntry(cmmFastList<CCmmContent,InitSize,Increment>::Front());
	}
	CCmmContent& Back(void){
		return cmmFastList<CCmmContent,InitSize,Increment>::GetEntry(cmmFastList<CCmmContent,InitSize,Increment>::Back());
	}
	// 获取对象数
	int Size(void){
		return cmmFastList<CCmmContent,InitSize,Increment>::Size();
	}
	// 清除全部成员
	void Clear(){
		cmmFastList<CCmmContent,InitSize,Increment>::Clear();
	}
};


//////////////////////////////////////////////////////////////////////////
// 属性表，通过4个字节的索引，获取属性表中的属性值
// 4字节索引采用英文字母加数字的方式，如：'Att1'，每个属性可以保存的属性值长度最大为1K
// InitSize是初始设置的存储区大小，如果存放一个属性，需要6个字节的头加上属性值长度；任何时候都需要6个字节的截至符
template<int InitSize=16,int Increment=30>
class cmmAttributes:protected cmmVector<unsigned char,InitSize,Increment>
{
public:
	cmmAttributes()
	{
		memset(cmmVector<unsigned char,InitSize,Increment>::GetBuffer(), 0x0, cmmVector<unsigned char,InitSize,Increment>::Size());
	}
	~cmmAttributes(){}
public:

	template<typename Type>
	// 设置一个属性，如果失败可能是属性已经存在或者内存访问失败
	bool SetAttribute(
		ULONG nuIndex,	// 索引，必须是用字符串形式提供，如：'Att1'
		const Type& rToSave,	// 保存为一个ULONG的值
		bool nbOverwrite=false	// 如果存在是否改写，设置为true，存在相同索引的属性就改写它；为false，存在相同索引就返回失败
		){
			return SetAttributeItem(nuIndex,(unsigned char*)&rToSave,sizeof(Type));
	}

	// 删除一个属性，如果失败可能是未存在或者内存访问失败
	bool DeleteAttribute(
		ULONG nuIndex	// 索引，必须是用字符串形式提供，如：'Att1'
		);	


	// 设置一个属性，如果失败可能是属性已经存在或者内存访问失败
	bool SetAttributeItem(
		ULONG nuIndex,	// 索引，必须是用字符串形式提供，如：'Att1'
		unsigned char* npBuffer,	// 保存为一个Buffer的值
		int niValueSize,
		bool nbOverwrite=false	// 如果存在是否改写，设置为true，存在相同索引的属性就改写它；为false，存在相同索引就返回失败
		);


	template<typename Type>
	// 获得一个属性，返回成功与否
	bool GetAttribute(
		ULONG nuIndex,	// 索引，必须是用字符串形式提供，如：'Att1'
		Type& nrValue
		){
			if(GetAttributeItem(nuIndex,(unsigned char*)&nrValue,sizeof(Type))!=sizeof(Type))
				return false;

			return true;
	}

	// 获得一个属性，返回获得的字节数，小于等于0表示失败或者不存在该属性
	int GetAttributeItem(
		ULONG nuIndex,	// 索引，必须是用字符串形式提供，如：'Att1'
		unsigned char* npBuffer,
		int niBufSize	// 缓冲区的大小
		);


};


template<int InitSize,int Increment>
bool cmmAttributes<InitSize, Increment>::DeleteAttribute(
	ULONG nuIndex		// 索引，必须是用字符串形式提供，如：'Att1'
	)	
{
#define ATTRIBUTE_ZERO_LEN	sizeof(ULONG)

	unsigned char* lpDataBuffer = NULL;		// 获得存储属性表的缓冲区
	ULONG	luAttrNum = 0;					// 属性个数
	USHORT	lsValueLen = 0;					// 已存在的属性值的总长度
	USHORT	lsAttrValueLen = 0;
	int		liHitIndex = 0;					// 命中索引
	int		luDataSize = 0;					// 属性列表总的缓冲区长度
	bool	lbIsHit = false;
	int		liBase;
	char	lcData;

	lpDataBuffer = cmmVector<unsigned char,InitSize,Increment>::GetBuffer();
	luDataSize = cmmVector<unsigned char,InitSize,Increment>::Size();

	// 解析获得属性列表
	for (int liLoop = 0; liLoop < luDataSize; )
	{
		if ('0000' == *(ULONG*)&lpDataBuffer[liLoop])
			break;

		if (nuIndex == *(ULONG*)&lpDataBuffer[liLoop])
		{	
			liHitIndex = luAttrNum;
			lbIsHit = true;
		}
		luAttrNum++;
		liLoop += 4;
	}

	// 不存在指定属性，直接返回
	if (!lbIsHit)
		return false;

	// 获取指定属性的值长度,首先获取到的是高位数，所以采用倒序取值的方法
	liBase = luAttrNum*sizeof(ULONG) + (liHitIndex+1)*sizeof(USHORT) + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop < sizeof(USHORT); liLoop++)
	{
		liBase--; lcData = '0';

		lcData = cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase);
		lsAttrValueLen = lsAttrValueLen<<(liLoop*8);
		lsAttrValueLen |= lcData;

	}
	if (lsAttrValueLen <= 0)
		return false;


	// 解析liHitIndex属性之前的属性内容的长度		
	liBase = luAttrNum*sizeof(ULONG) + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop< liHitIndex; liLoop++)
	{
		USHORT lsTmpLen = 0;
		// 将2字节拼成一个USHORT,这里需要注意little-endian规则
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+1);
		lsTmpLen = lsTmpLen<<8;
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+0);

		lsValueLen += lsTmpLen;
	}

	// 删除指定属性的值,基类数组会自动前移，所以在同一索引上连续删除即可
	liBase = luAttrNum*sizeof(ULONG) + luAttrNum*sizeof(USHORT) + lsValueLen + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop < lsAttrValueLen; liLoop++)
	{
		cmmVector<unsigned char,InitSize,Increment>::RemoveByIndex(liBase);
	}

	// 删除存储长度的内存单元
	liBase = luAttrNum*sizeof(ULONG) + liHitIndex*sizeof(USHORT)+ ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop < sizeof(USHORT); liLoop++)
	{
		cmmVector<unsigned char,InitSize,Increment>::RemoveByIndex(liBase);
	}

	// 删除属性索引
	liBase = liHitIndex*sizeof(ULONG);
	for (int liLoop = 0; liLoop < sizeof(ULONG); liLoop++)
	{
		cmmVector<unsigned char,InitSize,Increment>::RemoveByIndex(liBase);
	}

	return true;

}


template<int InitSize,int Increment>
bool cmmAttributes<InitSize, Increment>::SetAttributeItem(
	ULONG nuIndex,				// 索引，必须是用字符串形式提供，如：'Att1'
	unsigned char* npBuffer,	// 保存为一个Buffer的值
	int niValueSize,
	bool nbOverwrite=false		// 如果存在是否改写，设置为true，存在相同索引的属性就改写它；为false，存在相同索引就返回失败
	)	
{

#define ATTRIBUTE_ZERO_LEN	sizeof(ULONG)

	unsigned char* lpDataBuffer = NULL;		// 获得存储属性表的缓冲区
	ULONG	luAttrNum = 0;					// 属性个数
	USHORT	lsValueLen = 0;					// 已存在的属性值的总长度
	USHORT	lsAttrValueLen = 0;
	int		luDataSize = 0;					// 属性列表总的缓冲区长度
	bool	lbIsHit = false;
	int		liBase;
	char	lcData;


	lpDataBuffer = cmmVector<unsigned char,InitSize,Increment>::GetBuffer();
	luDataSize = cmmVector<unsigned char,InitSize,Increment>::Size();


	// 解析获得属性列表
	for (int liLoop = 0; liLoop < luDataSize; )
	{
		if ('0000' == *(ULONG*)&lpDataBuffer[liLoop])
			break;

		if (nuIndex == *(ULONG*)&lpDataBuffer[liLoop])
			lbIsHit = true;

		luAttrNum++;
		liLoop += 4;
	}

	// 已经存在指定属性，则进行更新
	if (lbIsHit)
	{
		// 先删除，然后新建，相当于更新
		DeleteAttribute(nuIndex);
		luAttrNum--;
	}


	liBase = luAttrNum*sizeof(ULONG) +ATTRIBUTE_ZERO_LEN;
	// 解析获得已存在的属性对应的属性内容的长度		
	for (int liLoop = 0; liLoop< (int)luAttrNum; liLoop++)
	{
		USHORT lsTmpLen = 0;
		// 将2字节拼成一个USHORT,这里需要注意little-endian规则
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+1);
		lsTmpLen = lsTmpLen<<8;
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+0);

		lsValueLen += lsTmpLen;
	}


	liBase = luAttrNum*sizeof(ULONG);
	// 查看是否是第一个属性，如果是，则首先填充尾部0
	if (liBase == 0)	
	{
		liBase += 4;
		for (int liLoop = 0; liLoop < sizeof(ULONG); liLoop++)
		{
			lcData= '0';
			cmmVector<unsigned char,InitSize,Increment>::Insert(liBase, lcData);
			liBase++;
		}
	}
	// 插入新的属性值	
	liBase = luAttrNum*sizeof(ULONG);	
	for (int liLoop = 0; liLoop < sizeof(ULONG); liLoop++)
	{
		lcData= '0';
		lcData= (UCHAR)(nuIndex>>(liLoop*sizeof(char)*8));
		cmmVector<unsigned char,InitSize,Increment>::Insert(liBase, lcData);
		liBase++;
	}
	luAttrNum++;

	liBase = luAttrNum*sizeof(ULONG) + (luAttrNum-1)*sizeof(USHORT) + ATTRIBUTE_ZERO_LEN;
	// 插入新属性的长度值
	for (int liLoop = 0; liLoop < sizeof(USHORT); liLoop++)
	{
		lcData= '0';
		lcData= niValueSize>>(liLoop*sizeof(char)*8);
		cmmVector<unsigned char,InitSize,Increment>::Insert(liBase, lcData);
		liBase++;
	}


	liBase = luAttrNum*sizeof(ULONG) + luAttrNum*sizeof(USHORT) + ATTRIBUTE_ZERO_LEN + lsValueLen;
	// 插入新值的内容
	for (int liLoop = 0; liLoop < niValueSize; liLoop++)
	{
		cmmVector<unsigned char,InitSize,Increment>::Insert(liBase, npBuffer[liLoop]);
		liBase++;
	}

	return true;

}



template<int InitSize,int Increment>
int cmmAttributes<InitSize, Increment>::GetAttributeItem(
	ULONG nuIndex,				// 索引，必须是用字符串形式提供，如：'Att1'
	unsigned char* npBuffer,
	int niBufSize				// 缓冲区的大小
	)
{

#define ATTRIBUTE_ZERO_LEN	sizeof(ULONG)

	unsigned char* lpDataBuffer = NULL;		// 获得存储属性表的缓冲区
	ULONG	luAttrNum = 0;					// 属性个数
	USHORT	lsValueLen = 0;					// 已存在的属性值的总长度
	USHORT	lsAttrValueLen = 0;
	int		liHitIndex = 0;					// 命中索引
	int		luDataSize = 0;					// 属性列表总的缓冲区长度
	bool	lbIsHit = false;
	int		liBase;
	char	lcData;

	lpDataBuffer = cmmVector<unsigned char,InitSize,Increment>::GetBuffer();
	luDataSize = cmmVector<unsigned char,InitSize,Increment>::Size();

	// 解析获得属性列表
	for (int liLoop = 0; liLoop < luDataSize; )
	{
		if ('0000' == *(ULONG*)&lpDataBuffer[liLoop])
			break;

		if (nuIndex == *(ULONG*)&lpDataBuffer[liLoop])
		{	
			liHitIndex = luAttrNum;
			lbIsHit = true;
		}
		luAttrNum++;
		liLoop += 4;
	}

	// 不存在指定属性，直接返回
	if (!lbIsHit)
		return false;

	// 获取指定属性的值长度,首先获取到的是高位数，所以采用倒序取值的方法
	liBase = luAttrNum*sizeof(ULONG) + (liHitIndex+1)*sizeof(USHORT) + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop < sizeof(USHORT); liLoop++)
	{
		liBase--; lcData= '0';
		
		lcData = cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase);
		lsAttrValueLen = lsAttrValueLen<<(liLoop*8);
		lsAttrValueLen |= lcData;
	}
	if (lsAttrValueLen <= 0)
		return 0;


	// 解析liHitIndex属性之前的属性内容的长度		
	liBase = luAttrNum*sizeof(ULONG) + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop< liHitIndex; liLoop++)
	{
		USHORT lsTmpLen = 0;
		// 将2字节拼成一个USHORT,这里需要注意little-endian规则
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+1);
		lsTmpLen = lsTmpLen<<8;
		lsTmpLen |= cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase + liLoop*sizeof(USHORT)+0);

		lsValueLen += lsTmpLen;
	}

	// 获取指定属性的值
	liBase = luAttrNum*sizeof(ULONG) + luAttrNum*sizeof(USHORT) + lsValueLen + ATTRIBUTE_ZERO_LEN;
	for (int liLoop = 0; liLoop < lsAttrValueLen; liLoop++)
	{
		npBuffer[liLoop] = cmmVector<unsigned char,InitSize,Increment>::GetEntry(liBase);
		liBase++;
	}

	return lsAttrValueLen;

}

#endif//_CMMSTRUCT_H_
