/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _MAP_DEFINE_
#define _MAP_DEFINE_

__interface IXsElement;
__interface IEinkuiIterator;
__interface ICfKey;


// 定义供测试的函数指针类型

// 函数指针类型，对应函数调用形式为：CEvXXX::CreateInstance(npParent, npTemplete,nuEID)
typedef IXsElement* (_cdecl *AFX_MAPCALL )(IEinkuiIterator*, ICfKey*, ULONG);

template <typename Type>
class CMapCallFP{
public:
	typedef Type* (_cdecl *CUSTOM_MAPCALL )(IEinkuiIterator*, ICfKey*, ULONG);
	union FP_Convert{
		AFX_MAPCALL Normal;
		CUSTOM_MAPCALL Custom;
	};
	__inline static AFX_MAPCALL Custom2Normal(CUSTOM_MAPCALL npFun){
		//FP_Convert ldPointer;
		//ldPointer.Custom = npFun;
		//return ldPointer.Normal;
		return ((FP_Convert*)&npFun)->Normal;
	}
	__inline static CUSTOM_MAPCALL Normal2Custom(AFX_MAPCALL npFun){
		//FP_Convert ldPointer;
		//ldPointer.Normal = npFun;
		//return ldPointer.Custom;
		return ((FP_Convert*)&npFun)->Custom;
	}
};

#endif