/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "MapDefine.h"
#include "cmmstruct.h"
#include "mapList.h"


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
		return AddList(nswString, npFuncAddr);	

	CPairNode	loPairNode;
	CHashValue	loHashValue;

	// 存在已知元素，则更改记录
	moData[liIndex].muHash = loHashValue.operator()(nswString);// ???是否不需要修改
	//moData[liIndex].muStringLen = wcslen(nswString);
	//moData[liIndex].pfnFuncAddr = npFuncAddr;
	moData[liIndex].mdUseData = UserData;
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


//template<typename DataType,class CHashValue>
//AFX_MAPCALL CMapList<DataType,CHashValue>::GetFuncAddr(
//	const wchar_t* nswString
//	)
//{
//	CHashValue loHashValue;
//	ULONG	luHash;
//	bool	lbIsFind = false;
//
//
//	luHash = loHashValue.operator()(nswString);
//
//
//	for (int liLoop = 0; liLoop< moData.Size(); liLoop++)
//	{
//		if (luHash == moData[liLoop].muHash && _wcsicmp(nswString, moData[liLoop].mswString)==0)
//		{
//			return moData[liLoop].pfnFuncAddr;
//		}
//	}
//
//
//	return NULL;
//}