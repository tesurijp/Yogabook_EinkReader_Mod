/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */




template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::bplustree()
{
#ifdef KERNEL_CODE
	KeInitializeSpinLock(&SpinLock);
	SavedIrql = PASSIVE_LEVEL;
#else
	InitializeCriticalSection(&CriticalSection);
#endif//KERNEL_CODE

	mpRoot = NULL;
	mEndItr.mpCrtLeaf = NULL;
	mEndItr.miCrtIndex = 0;

	Clear();
}


template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::~bplustree()
{
	ReleaseEntriesInNode(mpRoot);
#ifndef KERNEL_CODE
	DeleteCriticalSection(&CriticalSection);
#endif
	mpRoot = NULL;
}


// 获取访问权限
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
inline void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::EnterExclusiveAccess(void)const{
#ifdef KERNEL_CODE
	KeAcquireSpinLock((PKSPIN_LOCK)&SpinLock,(PKIRQL)&SavedIrql);
#else
	EnterCriticalSection((LPCRITICAL_SECTION)&CriticalSection);
#endif//KERNEL_CODE

}
// 释放访问权限
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
inline void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::LeaveExclusiveAccess(void)const{
#ifdef KERNEL_CODE
	KeReleaseSpinLock((PKSPIN_LOCK)&SpinLock,SavedIrql);
#else
	LeaveCriticalSection((LPCRITICAL_SECTION)&CriticalSection);
#endif//KERNEL_CODE

}


// 释放一个分支上的所有下级节点
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::ReleaseEntriesInNode(LPBPNODE npNode)
{
	if(npNode->mbIsLeaf == false)	// 不是叶节点,需要释放下属内容
	{
		bplustree_node<LPBPNODE,ObjsInNode>* lpBranch;
		lpBranch = (bplustree_node<LPBPNODE,ObjsInNode>*)npNode;

		for (int i=0;i<lpBranch->miEntryCount;i++)
		{
			ReleaseEntriesInNode(lpBranch->GetEntry(i));
		}
	}
	else
	{
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpLeaf = (bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*)npNode;
		for(int i=0;i<lpLeaf->miEntryCount;i++)
		{
			lpLeaf->GetEntry(i).~CBPlusTreeObjectClass();
		}
	}
	delete npNode;
}

// 插入，返回成功与否，失败的原因可能是存在相同的值或者内存分配失败
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Insert(
	CBPlusTreeObjectClass& nrObject		// 待插入的对象，注意，该对象的内容可能会应对应类别的 = 运算符的行为而修改
	)
{
	int liIndex;
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpLeaf;

	lpLeaf = FindInternal(nrObject,liIndex);
	if(lpLeaf ==NULL || liIndex < 0)
	{// Insert to the first leaf
		lpLeaf = mpFirstLeaf;
	}
	else
	if(mCriterion(nrObject,lpLeaf->GetEntry(liIndex))==false && mCriterion(lpLeaf->GetEntry(liIndex),nrObject)==false)
		return false;	// 该数值已经存在

	return InsertToLeaf(lpLeaf,nrObject);
}
// 插入，返回成功与否，失败的原因可能是存在相同的值或者内存分配失败，同上面的区别是，可以用常量做参数，当然，这个参数应该能够支持'='运算符，否则无法编译通过
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Insert(
	const CBPlusTreeObjectClass& nrObject		// 待插入的对象，注意，该对象的内容可能会应对应类别的 = 运算符的行为而修改
	)
{
	CBPlusTreeObjectClass loConvert = nrObject;
	return Insert(loConvert);
}

// 删除
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Remove(
	const CBPlusTreeObjectClass& nrObject // 查找到关键值意义上相同的对象，把它删除掉
	)
{
	bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> itr;
	itr = Find(nrObject);
	return Remove(itr);
}
// 删除
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Remove(
	bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode>&  nrIterator // 查找到关键值意义上相同的对象，把它删除掉
	)
{
	if(nrIterator != End())
		return DeleteFromLeaf(nrIterator.mpCrtLeaf,nrIterator.miCrtIndex);
	return false;
}

// 清楚全部内容
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Clear(void)
{
	// 删除全部
	if(mpRoot != NULL)
		ReleaseEntriesInNode(mpRoot);

	// 重新初始化
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpFirstLeaf;

	lpFirstLeaf = new bplustree_node<CBPlusTreeObjectClass,ObjsInNode>;
	if(lpFirstLeaf == NULL)
		return;

	lpFirstLeaf->mpLastSibling = NULL;
	lpFirstLeaf->mpNextSibling = NULL;
	lpFirstLeaf->mpParents = NULL;
	lpFirstLeaf->mpLowestObject = NULL;
	lpFirstLeaf->mbIsLeaf = true;

	mpRoot = lpFirstLeaf;
	mpFirstLeaf = mpLastLeaf = lpFirstLeaf;
	miTotalLeaves = 1;
	miTotalNodes = 0;
	miTotalObject = 0;
}

// 查询，对查询结果的访问，受一定的限制；如果获得结果后，再次调用过插入或删除功能，将可能导致刚才获得的结果对象失效，所以，请避免在查询后和访问查询结果对象前，调用B+树的插入和删除算法
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Find(
	const CBPlusTreeObjectClass& nrToFind, // 查找到关键值意义上相同的对象，提供用来做参数的对象，可以只设置关键值比较所需要的成员
	iterator* npEqualOrBelow=NULL	// 返回最接近查找对象的，等于或者仅比查找对象小的对象
	)const
{
	int liIndex;
	iterator loFind;
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpLeaf;

	lpLeaf = FindInternal(nrToFind,liIndex);
	if(lpLeaf !=NULL && liIndex >= 0 )
	{
		if(npEqualOrBelow != NULL)
		{
			npEqualOrBelow->mpCrtLeaf = lpLeaf;
			npEqualOrBelow->miCrtIndex = liIndex;
		}

		if(mCriterion(nrToFind,lpLeaf->GetEntry(liIndex))==false && mCriterion(lpLeaf->GetEntry(liIndex),nrToFind)==false)
		{
			loFind.mpCrtLeaf = lpLeaf;
			loFind.miCrtIndex = liIndex;
			return loFind;
		}
	}
	else
	if(npEqualOrBelow != NULL)
		*npEqualOrBelow = mEndItr;

	return mEndItr;
}

// 取关键值最小的对象
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::Begin(void)const
{
	iterator loFind;
	if(mpFirstLeaf->miEntryCount > 0)
	{
		loFind.mpCrtLeaf = (bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*)mpFirstLeaf;
		loFind.miCrtIndex = 0;
		return loFind;
	}
	return mEndItr;
}

// 取关键值最大的对象
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::ReverseBegin(void)const
{
	iterator loFind;
	if(mpLastLeaf->miEntryCount > 0)
	{
		loFind.mpCrtLeaf = (bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*)mpLastLeaf;
		loFind.miCrtIndex = mpLastLeaf->miEntryCount - 1;
		return loFind;
	}
	return mEndItr;
}

// 表示无效的对象
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
inline bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::End(void)const
{
	return mEndItr;
}

// 直接取某个值，注意，如果niIndex超出全部数值范围，将返回无效的数据或导致异常；另外，这个函数实质上不是真正的直接读取，当索引值超过一块叶节点的数据量时，将会逐块跳跃，效率需要考虑；故，如果是希望顺序遍历一串连续数据，请直接使用iterator的++和--来操作
template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
inline CBPlusTreeObjectClass& bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::operator[] (int niIndex)
{
	bplustree_iterator<CBPlusTreeObjectClass,Criterion,ObjsInNode> itr = Begin();
	itr += niIndex;
	return *itr;
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::VerifyTree(LPBPNODE npNode)const
{
	int liIndex;
	if(npNode == NULL)
		npNode = mpRoot;

	if(npNode->mbIsLeaf != false)
	{
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpLeaf = (bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*)npNode;
		for (liIndex=0;liIndex<lpLeaf->miEntryCount-1;liIndex++)
		{
			if(mCriterion(lpLeaf->GetEntry(liIndex),lpLeaf->GetEntry(liIndex+1))==false)
				break;
		}
		BPASSERT(liIndex >= lpLeaf->miEntryCount-1);
		BPASSERT(lpLeaf->mpLowestObject == &lpLeaf->GetEntry(0));
		BPASSERT(lpLeaf->mpLastSibling==NULL || lpLeaf->mpLastSibling->miEntryCount <= ObjsInNode);
		BPASSERT(lpLeaf->mpLastSibling==NULL || lpLeaf->mpLastSibling->mpNextSibling == lpLeaf);
		BPASSERT(lpLeaf->mpNextSibling == NULL || lpLeaf->mpNextSibling->mpLastSibling == lpLeaf);
		BPASSERT(lpLeaf->mpNextSibling == NULL || lpLeaf->mpNextSibling->miEntryCount<= ObjsInNode && mCriterion(lpLeaf->GetEntry(lpLeaf->miEntryCount-1),lpLeaf->mpNextSibling->GetEntry(0))!= false);
	}
	else
	{
		bplustree_node<LPBPNODE,ObjsInNode>* lpBranch = (bplustree_node<LPBPNODE,ObjsInNode>*)npNode;
		for (liIndex=0;liIndex<lpBranch->miEntryCount;liIndex++)
		{
			BPASSERT(lpBranch->GetEntry(liIndex)->mpParents == lpBranch);
		}

		for (liIndex=0;liIndex<lpBranch->miEntryCount-1;liIndex++)
		{
			if(mCriterion(*(CBPlusTreeObjectClass*)(lpBranch->GetEntry(liIndex)->mpLowestObject),*(CBPlusTreeObjectClass*)(lpBranch->GetEntry(liIndex+1)->mpLowestObject))==false)
				break;
		}
		BPASSERT(liIndex >= lpBranch->miEntryCount-1);
		BPASSERT(lpBranch->mpLowestObject == lpBranch->GetEntry(0)->mpLowestObject);

		for (liIndex=0;liIndex<lpBranch->miEntryCount;liIndex++)
			VerifyTree(lpBranch->GetEntry(liIndex));
	}
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>\
ULONG bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::GetSizeOfLeaf(void)const
{
	return sizeof(bplustree_node<CBPlusTreeObjectClass,ObjsInNode>);
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
ULONG bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::GetSizeOfBranch(void)const
{
	return sizeof(bplustree_node<LPBPNODE,ObjsInNode>);
}



// internal procedues

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::FindInternal(
	const CBPlusTreeObjectClass& nrToFind,	// 查找的参照对象
	int& Index	// 返回对象的索引值
	)const
{
	int liIndex = -1;
	LPBPNODE lpNode = mpRoot;
	BPASSERT(mpRoot!=NULL);

	while (lpNode != NULL)
	{
		if(lpNode->mbIsLeaf != false)
		{
			liIndex = FindInLeaf((bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*)lpNode,nrToFind);
			break;
		}
		liIndex = FindInBranch((bplustree_node<LPBPNODE,ObjsInNode>*)lpNode,nrToFind);
		if(liIndex < 0)
			break;
		lpNode = ((bplustree_node<LPBPNODE,ObjsInNode>*)lpNode)->GetEntry(liIndex);
	}

	if(liIndex < 0)
		return NULL;

	BPASSERT(liIndex < lpNode->miEntryCount);
	Index = liIndex;
	return (bplustree_node<CBPlusTreeObjectClass,ObjsInNode>*) lpNode;
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
int bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::FindInBranch(	// 在某个枝节点中寻找最大的小于或等于目标的节点
	bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
	const CBPlusTreeObjectClass& nrToFind	// 查找的参照对象
	)const
{
	int liTop,liBottom,liCrt;
	CBPlusTreeObjectClass* lpLowest;
	liTop = npBranch->miEntryCount -1 ;
	liBottom = 0;

	BPASSERT(npBranch->miEntryCount >0 || mCriterion(nrToFind,*((CBPlusTreeObjectClass*)(npBranch->mpLowestObject)))==false);

	while(liBottom <= liTop)
	{
		// 折半查找
		liCrt = (liTop + liBottom)/2;
		// 取当前位置对应节点的最小值
		lpLowest = (CBPlusTreeObjectClass*)((npBranch->GetEntry(liCrt))->mpLowestObject);

		// 不小于当前值
		if(mCriterion(nrToFind,*lpLowest) == false)
		{
			// 是否相等
			if(mCriterion(*lpLowest,nrToFind) == false)
				return liCrt;
			liBottom = liCrt+1;
		}
		else
		{
			liTop = liCrt-1;
		}
	}

	return liTop;
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
int bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::FindInLeaf(	// 在某个枝节点中寻找最大的小于或等于目标的节点
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
	const CBPlusTreeObjectClass& nrToFind	// 查找的参照对象
	)const
{
	int liTop,liBottom,liCrt;
	liTop = npLeaf->miEntryCount -1 ;
	liBottom = 0;

	BPASSERT(npLeaf->mpLowestObject==NULL || mCriterion(nrToFind,*((CBPlusTreeObjectClass*)(npLeaf->mpLowestObject)))==false || npLeaf == mpFirstLeaf);

	while(liBottom <= liTop)
	{
		// 折半查找
		liCrt = (liTop + liBottom)/2;

		// 不小于当前值
		if(mCriterion(nrToFind,npLeaf->GetEntry(liCrt)) == false)
		{
			// 是否相等
			if(mCriterion(npLeaf->GetEntry(liCrt),nrToFind) == false)
				return liCrt;
			liBottom = liCrt+1;
		}
		else
		{
			liTop = liCrt-1;
		}
	}

	return liTop;
}


template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::InsertToLeaf(
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
	CBPlusTreeObjectClass& nrObject
	)
{
	int liIndex;
	bool lbReval = false;

	BPASSERT(npLeaf!=NULL);

	// 首先检索将会挡在哪个位置
	liIndex = FindInLeaf(npLeaf,nrObject)+1;	// 如果比最小的对象都要小，则会返回-1

	// 当前树叶是否已经可以放下这个内容
	if(npLeaf->miEntryCount < ObjsInNode)
	{
		npLeaf->Insert(nrObject,liIndex);
		if(liIndex == 0)	// 需要更新全部的最小值索引
		{
			npLeaf->mpLowestObject = &npLeaf->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);
		}
		lbReval = true;
	}
	else	// 前一个节点是否能够放下
	if(npLeaf->mpLastSibling!=NULL && npLeaf->mpLastSibling->miEntryCount < ObjsInNode)
	{	
		// 待插入的数据是不是小于本节点的所有对象
		if(liIndex <= 0)
		{
			// 是的，将它直接插入到前一个节点去，因为调用这个算法时，同上都是将新数据插入到比他小的节点上，所以，这个地方通常不会到达，也就是说，此地目前无效
			npLeaf->mpLastSibling->Insert(nrObject,npLeaf->mpLastSibling->miEntryCount);
		}
		else
		{
			// 移动最小的一个数据到前面去
			npLeaf->mpLastSibling->Insert(npLeaf->GetEntry(0),npLeaf->mpLastSibling->miEntryCount);
			// 删除掉移走的对象
			npLeaf->Remove(0);
			// 将新数据插入到当前节点，因为移走了一个对象，所以，刚才计算的位置需要减去1
			liIndex--;
			npLeaf->Insert(nrObject,liIndex);
			
			// 需要更新全部的最小值索引
			npLeaf->mpLowestObject = &npLeaf->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);
		}
		lbReval = true;
	}
	else	// 后一个节点是否可以放下
	if(npLeaf->mpNextSibling != NULL && npLeaf->mpNextSibling->miEntryCount < ObjsInNode)
	{
		// 待插入的数据是不是大于本节点的所有对象
		if(liIndex >= ObjsInNode)
		{
			// 是的，将它直接插入到后一个节点去
			npLeaf->mpNextSibling->Insert(nrObject,0);
			npLeaf->mpNextSibling->mpLowestObject = &npLeaf->mpNextSibling->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpNextSibling->mpParents);
		}
		else
		{
			// 移走最后的一个数据
			npLeaf->mpNextSibling->Insert(npLeaf->GetEntry(ObjsInNode-1),0);
			npLeaf->mpNextSibling->mpLowestObject = &npLeaf->mpNextSibling->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpNextSibling->mpParents);

			npLeaf->Remove(ObjsInNode-1);
			// 将新数据插入
			npLeaf->Insert(nrObject,liIndex);
			if(liIndex == 0)	// 需要更新全部的最小值索引
			{
				npLeaf->mpLowestObject = &npLeaf->GetEntry(0);
				UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);
			}
		}
		lbReval = true;
	}
	else	// 只能否分裂本节点了
	{
		// 分裂为两个
		bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpNewLeaf = npLeaf->Split(ObjsInNode/2);

		if(lpNewLeaf == NULL)
			return false;	// 失败返回

		if(liIndex <= ObjsInNode/2)
		{
			lpNewLeaf->Insert(nrObject,liIndex);
		}
		else
		{
			liIndex -= ObjsInNode/2;
			npLeaf->Insert(nrObject,liIndex);
		}
		lpNewLeaf->mpLowestObject = &lpNewLeaf->GetEntry(0);
		npLeaf->mpLowestObject = & npLeaf->GetEntry(0);
		UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);

		// 将新叶节点插入到上级的分支节点上
		if(npLeaf->mpParents == NULL)	//  到达根节点
			lbReval = NewLevel(lpNewLeaf,npLeaf);
		else
			lbReval = InsertToBranch((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents,lpNewLeaf);
		//如果分列前的节点是最小叶节点，修改最小叶节点记录
		if(mpFirstLeaf == npLeaf)
			mpFirstLeaf = lpNewLeaf;
		miTotalLeaves++;

	}

	if(lbReval != false)
		miTotalObject++;
	return lbReval;
}


template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::InsertToBranch(
	bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
	LPBPNODE npNewNode
	)
{
	int liIndex;
	bool lbReval = false;

	BPASSERT(npBranch!=NULL);

	// 首先检索将会挡在哪个位置
	liIndex = FindInBranch(npBranch,*((CBPlusTreeObjectClass*)npNewNode->mpLowestObject))+1;	// 如果比最小的对象都要小，则会返回-1

	// 当前树叶是否已经可以放下这个内容
	if(npBranch->miEntryCount < ObjsInNode)
	{
		npBranch->Insert(npNewNode,liIndex);
		npNewNode->mpParents = npBranch;	// 更新父节点
		if(liIndex == 0)	// 需要更新全部的最小值索引
		{
			UpdateLowestValue(npBranch);
		}
		lbReval = true;
	}
	else	// 前一个节点是否能够放下
	if(npBranch->mpLastSibling!=NULL && npBranch->mpLastSibling->miEntryCount < ObjsInNode)
	{	
		// 待插入的数据是不是小于本节点的所有对象
		if(liIndex <= 0)
		{
			// 是的，将它直接插入到前一个节点去
			npBranch->mpLastSibling->Insert(npNewNode,npBranch->mpLastSibling->miEntryCount);
			npNewNode->mpParents = npBranch->mpLastSibling;
		}
		else
		{
			// 移动最小的一个数据到前面去
			npBranch->mpLastSibling->Insert(npBranch->GetEntry(0),npBranch->mpLastSibling->miEntryCount);
			npBranch->GetEntry(0)->mpParents = npBranch->mpLastSibling;
			// 删除掉移走的对象
			npBranch->Remove(0);
			// 将新数据插入到当前节点，因为移走了一个对象，所以，刚才计算的位置需要减去1
			liIndex--;
			npBranch->Insert(npNewNode,liIndex);
			npNewNode->mpParents = npBranch;	// 更新父节点

			// 需要更新全部的最小值索引
			UpdateLowestValue(npBranch);
		}
		lbReval = true;
	}
	else	// 后一个节点是否可以放下
	if(npBranch->mpNextSibling != NULL && npBranch->mpNextSibling->miEntryCount < ObjsInNode)
	{
		// 待插入的数据是不是大于本节点的所有对象
		if(liIndex >= ObjsInNode)
		{
			// 是的，将它直接插入到后一个节点去，同插入树叶的函数的向前节点借位类似，这个地方也不会被调用。因为，新节点都是产生于旧节点向前分裂，插入上级节点时，都是插入到比他小的节点中
			// 不会出现插入的数据大于目前全部数据的可能，这儿目前无用
			npBranch->mpNextSibling->Insert(npNewNode,0);
			npNewNode->mpParents = npBranch->mpNextSibling;
			UpdateLowestValue(npBranch->mpNextSibling);
		}
		else
		{
			// 移走最后的一个数据
			npBranch->mpNextSibling->Insert(npBranch->GetEntry(ObjsInNode-1),0);
			npBranch->GetEntry(ObjsInNode-1)->mpParents = npBranch->mpNextSibling;
			UpdateLowestValue(npBranch->mpNextSibling);

			npBranch->Remove(ObjsInNode-1);
			// 将新数据插入
			npBranch->Insert(npNewNode,liIndex);
			npNewNode->mpParents = npBranch;
			if(liIndex == 0)	// 需要更新全部的最小值索引
			{
				UpdateLowestValue(npBranch);
			}
		}
		lbReval = true;
	}
	else	// 只能否分裂本节点了
	{
		// 分裂为两个
		bplustree_node<LPBPNODE,ObjsInNode>* lpNewBranch = npBranch->Split(ObjsInNode/2);

		if(lpNewBranch == NULL)
			return false;	// 失败返回

		if(liIndex <= ObjsInNode/2)
		{
			lpNewBranch->Insert(npNewNode,liIndex);
			npNewNode->mpParents = lpNewBranch;
		}
		else
		{
			liIndex -= ObjsInNode/2;
			npBranch->Insert(npNewNode,liIndex);
			npNewNode->mpParents = npBranch;
		}
		for(liIndex=0;liIndex<lpNewBranch->miEntryCount;liIndex++)
			lpNewBranch->GetEntry(liIndex)->mpParents = lpNewBranch;

		UpdateLowestValue(lpNewBranch);
		UpdateLowestValue(npBranch);

		// 将新节点插入到上级的分支节点上
		if(npBranch->mpParents == NULL)	//  到达根节点
			lbReval = NewLevel(lpNewBranch,npBranch);
		else
			lbReval = InsertToBranch((bplustree_node<LPBPNODE,ObjsInNode>*)npBranch->mpParents,lpNewBranch);
		miTotalNodes++;
	}

	return lbReval;
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::DeleteFromLeaf(
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npLeaf,
	const int niIndex
	)
{
	bool lbReval = true;
	// 首先删除掉这个数据
	npLeaf->GetEntry(niIndex).~CBPlusTreeObjectClass();
	npLeaf->Remove(niIndex);
	if(niIndex == 0 && npLeaf->miEntryCount > 0)
	{
		npLeaf->mpLowestObject = &npLeaf->GetEntry(0);
		UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);
	}

	// 检查该节点是否包含不少于容量一半的数据
	if(npLeaf->miEntryCount < ObjsInNode/2 && npLeaf != mpRoot)
	{
		if(npLeaf->mpLastSibling != NULL && npLeaf->mpLastSibling->miEntryCount <= ObjsInNode/2) // 同前一个节点合并
		{
			bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpAhead = npLeaf->mpLastSibling;
			lpAhead->Combine(npLeaf);
			lbReval = DeleteFromBranch((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents,npLeaf);
			if(npLeaf == mpLastLeaf)
				mpLastLeaf = lpAhead;

			delete npLeaf;
			miTotalLeaves--;
		}
		else
		if(npLeaf->mpNextSibling != NULL && npLeaf->mpNextSibling->miEntryCount <= ObjsInNode/2) // 同后一个节点合并
		{
			bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpBehind = npLeaf->mpNextSibling;
			npLeaf->Combine(lpBehind);
			lbReval = DeleteFromBranch((bplustree_node<LPBPNODE,ObjsInNode>*)lpBehind->mpParents,lpBehind);
			if(lpBehind == mpLastLeaf)
				mpLastLeaf = npLeaf;
			delete lpBehind;
			miTotalLeaves--;
		}
		else
		if(npLeaf->mpLastSibling != NULL && npLeaf->mpLastSibling->miEntryCount > ObjsInNode/2) // 从前面节点借一个数据
		{
			npLeaf->Insert(npLeaf->mpLastSibling->GetEntry(npLeaf->mpLastSibling->miEntryCount-1),0);
			npLeaf->mpLastSibling->Remove(npLeaf->mpLastSibling->miEntryCount-1);
			npLeaf->mpLowestObject = &npLeaf->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpParents);
		}
		else
		if(npLeaf->mpNextSibling != NULL && npLeaf->mpNextSibling->miEntryCount > ObjsInNode/2) // 从后面节点借一个数据
		{
			npLeaf->Insert(npLeaf->mpNextSibling->GetEntry(0),npLeaf->miEntryCount);
			npLeaf->mpNextSibling->Remove(0);
			npLeaf->mpNextSibling->mpLowestObject = &npLeaf->mpNextSibling->GetEntry(0);
			UpdateLowestValue((bplustree_node<LPBPNODE,ObjsInNode>*)npLeaf->mpNextSibling->mpParents);
		}
		else
		{
			// 不可能到达
			BPASSERT(0);
		}
	}

	if(lbReval != false)
		miTotalObject--;
	return lbReval;
}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::DeleteFromBranch(
	bplustree_node<LPBPNODE,ObjsInNode>* npBranch,
	LPBPNODE npValue	// 值等于npValue的数据对象
	)
{
	int liIndex;
	bool lbReval = true;

	// 首先找到这个值所在的位置
	for (liIndex = 0;liIndex < npBranch->miEntryCount;liIndex++)
	{
		if(npBranch->GetEntry(liIndex) == npValue)
			break;
	}
	if(liIndex >= npBranch->miEntryCount)
		return false;	// 出错了

	// 删除掉这个数据
	npBranch->Remove(liIndex);
	if(liIndex == 0 && npBranch->miEntryCount > 0)
	{
		UpdateLowestValue(npBranch);
	}

	// 检查该节点是否包含不少于容量一半的数据
	if(npBranch->miEntryCount < ObjsInNode/2)
	{
		// 判断是否是根节点
		if(npBranch == mpRoot)
		{
			if(npBranch->miEntryCount <= 1)
			{
				// 去掉根节点，直接将下层节点提上来
				LPBPNODE lpLower = npBranch->GetEntry(0);
				lpLower->mpParents = NULL;
				delete mpRoot;
				mpRoot = lpLower;
				miTotalNodes--;
			}
			// else 无改变
		}
		else
		if(npBranch->mpLastSibling != NULL && npBranch->mpLastSibling->miEntryCount <= ObjsInNode/2) // 同前一个节点合并
		{
			bplustree_node<LPBPNODE,ObjsInNode>* lpAhead = npBranch->mpLastSibling;
			lpAhead->Combine(npBranch);
			for (liIndex = 0;liIndex< lpAhead->miEntryCount;liIndex++)
			{
				if(lpAhead->GetEntry(liIndex)->mpParents != lpAhead)
					lpAhead->GetEntry(liIndex)->mpParents = lpAhead;
			}
			lbReval = DeleteFromBranch((bplustree_node<LPBPNODE,ObjsInNode>*)npBranch->mpParents,npBranch);
			delete npBranch;
			miTotalNodes--;
		}
		else
		if(npBranch->mpNextSibling != NULL && npBranch->mpNextSibling->miEntryCount <= ObjsInNode/2) // 同后一个节点合并
		{
			bplustree_node<LPBPNODE,ObjsInNode>* lpBehind = npBranch->mpNextSibling;
			npBranch->Combine(lpBehind);
			for (liIndex = 0;liIndex< npBranch->miEntryCount;liIndex++)
			{
				if(npBranch->GetEntry(liIndex)->mpParents != npBranch)
					npBranch->GetEntry(liIndex)->mpParents = npBranch;
			}
			lbReval = DeleteFromBranch((bplustree_node<LPBPNODE,ObjsInNode>*)lpBehind->mpParents,lpBehind);
			delete lpBehind;
			miTotalNodes--;
		}
		else
		if(npBranch->mpLastSibling != NULL && npBranch->mpLastSibling->miEntryCount > ObjsInNode/2) // 从前面节点借一个数据
		{
			npBranch->Insert(npBranch->mpLastSibling->GetEntry(npBranch->mpLastSibling->miEntryCount-1),0);
			npBranch->GetEntry(0)->mpParents = npBranch;
			npBranch->mpLastSibling->Remove(npBranch->mpLastSibling->miEntryCount-1);
			UpdateLowestValue(npBranch);
		}
		else
		if(npBranch->mpNextSibling != NULL && npBranch->mpNextSibling->miEntryCount > ObjsInNode/2) // 从后面节点借一个数据
		{
			npBranch->Insert(npBranch->mpNextSibling->GetEntry(0),npBranch->miEntryCount);
			npBranch->GetEntry(npBranch->miEntryCount-1)->mpParents = npBranch;
			npBranch->mpNextSibling->Remove(0);
			UpdateLowestValue(npBranch->mpNextSibling);
		}
		else
		{
			// 不可能到达
			BPASSERT(0);
		}
	}

	return lbReval;
}


template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
inline void bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::UpdateLowestValue(	// 更新由该节点至上相关节点的最小值对象索引
	bplustree_node<LPBPNODE,ObjsInNode>* npBranch
	)
{
	while(npBranch != NULL)
	{
		if(npBranch->mpLowestObject == npBranch->GetEntry(0)->mpLowestObject)
			break;
		npBranch->mpLowestObject = npBranch->GetEntry(0)->mpLowestObject;
		npBranch = (bplustree_node<LPBPNODE,ObjsInNode>*)npBranch->mpParents;
	}
}


template<class CBPlusTreeObjectClass,int ObjsInNode>
void bplustree_node<CBPlusTreeObjectClass,ObjsInNode>::Insert(CBPlusTreeObjectClass& nrObject,int niIndex)	//　插入一个对象到指定位置
{
	int liBehind = miEntryCount>=ObjsInNode?ObjsInNode-1:miEntryCount;
	// 将其后的对象全部后移一格
	while(liBehind > niIndex)
	{
		mObjectsArray[liBehind] = mObjectsArray[liBehind-1];
		liBehind--;
	}
	// 将对象放入
	mObjectsArray[niIndex] = nrObject;
	miEntryCount++;
}

template<class CBPlusTreeObjectClass,int ObjsInNode>
void bplustree_node<CBPlusTreeObjectClass,ObjsInNode>::Remove(int niIndex)	// 删除指定位置的对象
{
	int liBehind = niIndex+1;

	// 将其后的对象全部前移一格
	while(liBehind < miEntryCount)
	{
		mObjectsArray[liBehind-1] = mObjectsArray[liBehind];
		liBehind++;
	}
	miEntryCount--;
}

template<class CBPlusTreeObjectClass,int ObjsInNode>
bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* bplustree_node<CBPlusTreeObjectClass,ObjsInNode>::Split(int niCount)		// 将本节点分为两个，并移动niCount个数据到前一个节点，返回的对象就是新节点，新节点位于本节点前并且同本节点将前后链接好，但，不会设置它的上级节点
{
	int liIndex;
	bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* lpAhead = new bplustree_node<CBPlusTreeObjectClass,ObjsInNode>;
	if(lpAhead == NULL)
		return NULL;

	BPASSERT(niCount < miEntryCount);

	lpAhead->mpParents = NULL;
	lpAhead->mbIsLeaf = mbIsLeaf;
	lpAhead->mpLastSibling = mpLastSibling;
	lpAhead->mpNextSibling = this;
	mpLastSibling = lpAhead;
	if(lpAhead->mpLastSibling != NULL)
		lpAhead->mpLastSibling->mpNextSibling = lpAhead;

	// 复制数据
	for(liIndex=0;liIndex<niCount;liIndex++)
	{
		lpAhead->mObjectsArray[liIndex] = mObjectsArray[liIndex];
	}
	lpAhead->miEntryCount = liIndex;
	lpAhead->mpLowestObject = &lpAhead->mObjectsArray[0];

	for(;liIndex<miEntryCount;liIndex++)
	{
		mObjectsArray[liIndex-niCount] = mObjectsArray[liIndex];
	}
	miEntryCount -= niCount;

	return lpAhead;
}

template<class CBPlusTreeObjectClass,int ObjsInNode>
void bplustree_node<CBPlusTreeObjectClass,ObjsInNode>::Combine(bplustree_node<CBPlusTreeObjectClass,ObjsInNode>* npWith)		// 将目标节点的数据合并到本节点，并且修改前后链接指针，本对象必须是目标对象的前对象
{
	int liIndex;
	BPASSERT(mpNextSibling == npWith && (miEntryCount+npWith->miEntryCount) <= ObjsInNode);

	// 复制数据
	for(liIndex=0;liIndex<npWith->miEntryCount;liIndex++)
	{
		mObjectsArray[liIndex+miEntryCount] = npWith->mObjectsArray[liIndex];
	}
	miEntryCount += npWith->miEntryCount;

	mpNextSibling = npWith->mpNextSibling;
	if(mpNextSibling != NULL)
		mpNextSibling->mpLastSibling = this;

}

template<class CBPlusTreeObjectClass,class Criterion,int ObjsInNode>
bool bplustree<CBPlusTreeObjectClass,Criterion,ObjsInNode>::NewLevel(	// 在根节点位置，新增加一层；实际行为就是一个根分裂为两个，并且增加一个新的根节点
	LPBPNODE npNodeAhead,
	LPBPNODE npNodeBehind
	)
{
	bplustree_node<LPBPNODE,ObjsInNode>* lpNewRoot = new bplustree_node<LPBPNODE,ObjsInNode>;
	if(lpNewRoot == NULL)
		return false;
	lpNewRoot->Insert(npNodeAhead,0);
	lpNewRoot->Insert(npNodeBehind,1);
	lpNewRoot->mpLowestObject = npNodeAhead->mpLowestObject;
	lpNewRoot->mpLastSibling = NULL;
	lpNewRoot->mpNextSibling = NULL;
	lpNewRoot->mpParents = NULL;
	lpNewRoot->mbIsLeaf = false;
	mpRoot = lpNewRoot;
	npNodeAhead->mpParents = mpRoot;
	npNodeBehind->mpParents = mpRoot;

	miTotalNodes++;
	return true;
}
