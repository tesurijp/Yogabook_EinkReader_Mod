/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef MAP_LIST_H_
#define MAP_LIST_H_

#include "MapDefine.h"
#include "cmmstruct.h"

// 从字符串的前四个字符提取HASH值
class CHashFront
{
public:
	ULONG operator()(const wchar_t* nswString)
	{

		ULONG luHash = 0;
		int liLen = wcslen(nswString);
		wchar_t lcCrt;

		liLen = liLen>4?4:liLen;	// 如果liLen>4,则取4个字节
		
		for (int liIndex = 0; liIndex < liLen; liIndex++)
		{
			if (nswString[liIndex] == 0)
				break;

			// 生成HASH值
			lcCrt = nswString[liIndex];
			if((lcCrt|0x20) >= L'a' && (lcCrt|0x20) <= L'z')
					lcCrt |= 0x20;
			luHash += lcCrt;
			luHash = (luHash<<1) | ((luHash&0x010000)>>16);

		}

		return luHash;
	}

};

// 从字符串的最后四个字符提取HASH值
class CHashEnd
{
public:
	ULONG operator()(const wchar_t* nswString)
	{
		ULONG luHash = 0;
		int liLen = wcslen(nswString);
		wchar_t lcCrt;

		liLen = (liLen-4)>0?(liLen-4):0;	// 如果liLen>4,则取4个字节

		for (int liIndex = wcslen(nswString)-1; liIndex >= liLen; liIndex--)
		{
			if (nswString[liIndex] == 0)
				break;

			// 生成HASH值
			lcCrt = nswString[liIndex];
			if((lcCrt|0x20) >= L'a' && (lcCrt|0x20) <= L'z')
				lcCrt |= 0x20;
			luHash += lcCrt;
			luHash = (luHash<<1) | ((luHash&0x010000)>>16);

		}

		return luHash;
	}

};

// 从字符串的全部字节内容上计算HASH值
class CHashFull
{
public:
	ULONG operator()(const wchar_t* nswString)
	{
		ULONG luHash = 0;
		int liLen = wcslen(nswString);
		wchar_t lcCrt;
		
		if (liLen > 255 || liLen < 0)
			liLen = 255;

		for (int liIndex = 0; liIndex < liLen; liIndex++)
		{
			if (nswString[liIndex] == 0)
				break;

			// 生成HASH值
			lcCrt = nswString[liIndex];
			if((lcCrt|0x20) >= L'a' && (lcCrt|0x20) <= L'z')
				lcCrt |= 0x20;
			luHash += lcCrt;
			luHash = (luHash<<1) | ((luHash&0x010000)>>16);

		}

		return luHash;
	}

};


// 字符串与函数地址在内存中的pair结构体
template<typename DataType>
class CPairNode{
public:
	ULONG		muHash;
	//ULONG		muStringLen;
	const wchar_t*	mswString;
//	AFX_MAPCALL		pfnFuncAddr;
	DataType mdUserData;

	__inline void operator=(const class CPairNode& src) {
		muHash = src.muHash;
		mswString = src.mswString;
//		pfnFuncAddr = src.pfnFuncAddr;
		mdUserData = src.mdUserData;
		//muStringLen = src.muStringLen;
	}

	CPairNode()
	{
		muHash = 0;
		//muStringLen = 0;
		mswString = NULL;
//		pfnFuncAddr = NULL;
	}

	bool SavePath(const wchar_t* nswString){
		if(nswString == NULL || nswString[0]==UNICODE_NULL)
			return true;
		int liSize = (int)wcslen(nswString)*sizeof(wchar_t);
		wchar_t* lpStr = new wchar_t[liSize];
		if(lpStr == NULL)
			return false;

		RtlCopyMemory(lpStr,nswString,liSize*sizeof(wchar_t));
		mswString = lpStr;

		return true;
	}
	void FreePath(void)	// 当一个对象从容器对象中删除后，才能调用本方法释放掉额外的内存
	{
		if((mswString)!=NULL)
		{
			delete mswString;
			mswString =NULL;
		}
	}

};

template<typename DataType>
class CMapNodeCriterion	// 默认的判断准则
{
public:
	bool operator () (const CPairNode<DataType>& Obj1,const CPairNode<DataType>& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.muHash < Obj2.muHash || (Obj1.muHash == Obj2.muHash && _wcsicmp(Obj1.mswString,Obj2.mswString)<0));
	}
};



// 该类维护pair集合，pair{string, funcAddr}
// 主要用途，给定一个string,能快速的查找出对应的函数地址
// DataType不能放过过大的数据机构，比如大于16个字节；总的数据量也不宜过大，最好在1千个单元以内


template<typename DataType,class CHashValue>
class CMapList
{
public:
	CMapList(){};
	~CMapList(){};

public:
	// 添加一对pair数据
	// 若失败，则返回false,失败的原因可能是因为已经存在，或者内存分配错误
	bool AddList(const wchar_t* nswString,DataType UserData);

	// 删除一对pair数据
	bool DelList(const wchar_t* nswString);

	// 更新一对pair数据
	bool UpdataList(const wchar_t* nswString,DataType UserData);

	// 查找指定的节点记录是否存在
	// 返回-1，表示不存在；其他值，表示找到的元素节点序号
	int LookupRecord(const wchar_t* nswString);

	//// 通过给定的nswString,返回对应的函数地址
	//AFX_MAPCALL GetFuncAddr(const wchar_t* nswString);

	// 获得用户数据，如果不存在将返回设置的默认值
	DataType GetUserData(const wchar_t* nswString,DataType Default);

	// 获得映射条目总数
	int GetCount(void){
		return moData.Size();
	}

	// 获得niIndex指定的条目的映射字符串，0 <= niIndex < GetCount()
	const wchar_t* GetMappedString(int niIndex){
		return moData[niIndex].mswString;
	}

	// 获得niIndex指定的条目的映射内容，0 <= niIndex < GetCount()
	DataType GetDataByIndex(int niIndex){
		return moData[niIndex].mdUserData;
	}

protected:

private:

	cmmSequence<CPairNode<DataType>,CMapNodeCriterion<DataType>> moData;	

	
};


template<typename DataType,class CHashValue>
int CMapList<DataType,CHashValue>::LookupRecord(
	const wchar_t* nswString
	)
{
	CPairNode<DataType> loPair;
	CHashValue	loHashValue;

	loPair.muHash = loHashValue.operator()(nswString);
	loPair.mswString = nswString;

	return moData.Find(loPair);
	// 	CHashValue loHashValue;
	// 	ULONG	luHash;
	// 	int		liIndex = -1;
	// 
	// 
	// 	luHash = loHashValue.operator()(nswString);
	// 
	// 	for (int liLoop = 0; liLoop< moData.Size(); liLoop++)
	// 	{
	// 		if (luHash == moData[liLoop].muHash && _wcsicmp(nswString, moData[liLoop].mswString)==0)
	// 		{
	// 			liIndex = liLoop;
	// 			break;
	// 		}
	// 	}
	// 
	// 	return liIndex;
}

template<typename DataType,class CHashValue>
bool CMapList<DataType,CHashValue>::AddList(
	const wchar_t* nswString,
	DataType UserData
	//AFX_MAPCALL npFuncAddr
	)
{

	CPairNode<DataType>	loPairNode;
	CHashValue	loHashValue;


	// 首先判断，该节点记录是否存在,>=0表示存在，则直接返回
	if(LookupRecord(nswString)>= 0)
		return false;

	loPairNode.muHash = loHashValue.operator()(nswString);
	//loPairNode.muStringLen = wcslen(nswString);
	//loPairNode.pfnFuncAddr = npFuncAddr;
	loPairNode.mdUserData = UserData;
	loPairNode.SavePath(nswString);

	return (moData.Insert(loPairNode)>=0);

}


template<typename DataType,class CHashValue>
bool CMapList<DataType,CHashValue>::DelList(
	const wchar_t* nswString
	)
{

	int liIndex = LookupRecord(nswString);
	if (liIndex < 0)
		return false;	// 不需要删除

	moData[liIndex].FreePath();
	moData.RemoveByIndex(liIndex);

	return true;
}


template<typename DataType,class CHashValue>
bool CMapList<DataType,CHashValue>::UpdataList(
	const wchar_t* nswString, 
	DataType UserData
	//	AFX_MAPCALL npFuncAddr
	)
{
	int liIndex = LookupRecord(nswString);
	if (liIndex < 0)	// 表示不存在该元素，则直接添加
		return AddList(nswString, UserData);	

	CPairNode<DataType>	loPairNode;
	CHashValue	loHashValue;

	// 存在已知元素，则更改记录
	moData[liIndex].muHash = loHashValue.operator()(nswString);// ???是否不需要修改
	//moData[liIndex].muStringLen = wcslen(nswString);
	//moData[liIndex].pfnFuncAddr = npFuncAddr;
	moData[liIndex].mdUserData = UserData;
	moData[liIndex].FreePath();
	moData[liIndex].SavePath(nswString);

	return true;
}


// 获得用户数据，如果不存在将返回设置的默认值
template<typename DataType,class CHashValue>
DataType CMapList<DataType,CHashValue>::GetUserData(const wchar_t* nswString,DataType Default)
{
	int liIndex = LookupRecord(nswString);
	if (liIndex < 0)	// 表示不存在该元素
		return Default;

	return moData[liIndex].mdUserData;
}



//// template是内部链接，声明和定义实现必须在一个单元文件里
//#include "mapList.cpp"
//

#endif

