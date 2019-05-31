/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"

#include "XelDataMgr.h"

DEFINE_BUILTIN_NAME(CXelDataMgr)
CXelDataMgr* CXelDataMgr::sm_Inst = NULL;


CXelDataMgr::CXelDataMgr()
{
	mb64Bit = FALSE;
	IsWow64Process(GetCurrentProcess(),&mb64Bit);
}

CXelDataMgr::~CXelDataMgr()
{
	// 进行资源释放
	if (mpMainKey != NULL)
		RegCloseKey(mpMainKey);

	for (int i = 0; i < moFactories.Size(); i++)
	{
		if(moFactories[i].pfnElementFact!=NULL)
			moFactories[i].pfnElementFact->Release();
	}
}


ULONG CXelDataMgr::InitOnCreate(const wchar_t* nswRegPath)
{
	LONG  luStatus;
	mpMainKey = NULL;

	if(mb64Bit == FALSE)
		luStatus = RegOpenKey(HKEY_LOCAL_MACHINE,nswRegPath,&mpMainKey);
	else
		luStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,nswRegPath,0,KEY_READ|KEY_WOW64_64KEY,&mpMainKey); 

	if (luStatus == ERROR_SUCCESS)
		return 0;
	else
		return 0x80000000;

}



CXelDataMgr* CXelDataMgr::SingleInstance(const wchar_t* nswRegPath)
{
	if (NULL == sm_Inst)
	{
		sm_Inst = CXelDataMgr::CreateInstance(nswRegPath);
	}
	
	return sm_Inst;
}


IElementFactory* __stdcall CXelDataMgr::HasFactoryBeenLoaded(
	const wchar_t* nswDllPath
	)
{

	//// 查看内存维护的DLL句柄表，看是否加载
	//FACTCLASSPAIR	lstFactPair;
	//IElementFactory* lpElementFact = NULL;

	//for (int liLoop = 0; liLoop < mFactClassPairVec.Size(); liLoop++)
	//{
	//	lstFactPair = mFactClassPairVec.GetEntry(liLoop);
	//	if (0 == _wcsicmp(nswDllPath, lstFactPair.mswDllPath))
	//	{
	//		lpElementFact = lstFactPair.pfnElementFact;
	//		break;
	//	}
	//}

	//return lpElementFact;

	CLelFactoryNode loToFind;
	IElementFactory* lpElementFact = NULL;

	loToFind.GenerateHashCode(nswDllPath,-1);

	for(int i=0;i< moFactories.Size();i++)
	{
		if(moFactories[i].muHash == loToFind.muHash && _wcsicmp(moFactories[i].mswDllPath,nswDllPath)==0)
		{
			lpElementFact = moFactories[i].pfnElementFact;
			break;
		}
	}

	return lpElementFact;
}


REGSTATUS __stdcall CXelDataMgr::IsRegisted( 
	const wchar_t* nswClsName,
	wchar_t* nswDllPath,
	IElementFactory** npElementFact
	)
{
	// 查看在配置文件中能否找到注册信息
	if (FindFactoryDll(nswClsName, nswDllPath) == FALSE)
		return Invalid;

	// 查看该DLL是否已经加载
	*npElementFact = HasFactoryBeenLoaded(nswDllPath);
	if(*npElementFact != NULL)
		return LOAD;
	else
		return UNLOAD;

}


BOOL __stdcall CXelDataMgr::ModLoadedList( 
	const wchar_t* nswDllPath, 
	HANDLE	nhDllHandle,
	IElementFactory* pfnElementFact,
	BOOL	niMode 
	)
{
	BOOL lbResult = FALSE;
	if(niMode != FALSE)
	{
		CLelFactoryNode loAdd;
		loAdd.mpDllHandle = nhDllHandle;
		loAdd.pfnElementFact = pfnElementFact;
		loAdd.SavePath(nswDllPath);

		lbResult = (moFactories.Insert(-1,loAdd)>=0);
	}
	else
	{
		for(int i=0;i<moFactories.Size();i++)
		{
			if(moFactories[i].mpDllHandle == nhDllHandle)
			{
				moFactories[i].FreePath();
				moFactories.RemoveByIndex(i);
				lbResult = TRUE;
				break;
			}
		}
	}

	return lbResult;


}


//void CXelDataMgr::RemoveFactory(IElementFactory* npElementFact)
//{
//	for (int i = 0; i < moFactories.Size(); i++)
//	{
//		if (moFactories[i].pfnElementFact == npElementFact)
//		{
//			moFactories.RemoveByIndex(i);
//			break;
//		}
//	}
//}

BOOL __stdcall CXelDataMgr::FindFactoryDll( 
	const wchar_t* nswClsName, 
	wchar_t* nswDllPath 
	)
{

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

	// 遍历注册表：HKLM\Software\Lenovo\Veriface5\PublicClasses\控件列表
	// 查看有无对象类的注册信息

	TCHAR	lszSubKeyName[MAX_PATH];   
	DWORD	luLen;                 
	TCHAR	lszKeyName[MAX_PATH] = TEXT("");  
	DWORD	luClassNameLen = MAX_PATH; 
	DWORD	luSubKeyNum=0;        
	DWORD	luSubKeyLen;          
	DWORD	luClassLen;           
	DWORD	luValueNum;              
	DWORD	luSubKeyNameLen;        
	DWORD	luValueLen;      
	DWORD	luSecurityDescriptor; 
	FILETIME lstFtLastWriteTime;    

	DWORD	liLoop, luStatus; 
	HKEY	lpSubKey;
	BOOL	lbFind = FALSE;

	TCHAR	lszKeyValue[MAX_VALUE_NAME]; 
	DWORD	luSubValueLen = MAX_VALUE_NAME; 
	DWORD	luValueType = REG_SZ;

	// 获取类名和数量
	luStatus = RegQueryInfoKey(
		mpMainKey,					// 键的句柄
		lszKeyName,					// 类名 
		&luClassNameLen,			// 类名长度 
		NULL,						// 保留
		&luSubKeyNum,               // 子键的数量
		&luSubKeyLen,				// 子键长度
		&luClassLen,				// 类长度
		&luValueNum,                // 子键键值数量
		&luSubKeyNameLen,			// 子键名长度
		&luValueLen,				// 键值长度
		&luSecurityDescriptor,		// 安全描述符
		&lstFtLastWriteTime);       // 最后写时间 

	// 列举子键    
	if (luSubKeyNum)
	{

		for (liLoop=0; liLoop<luSubKeyNum; liLoop++) 
		{ 
			luLen = MAX_KEY_LENGTH;
			luStatus = RegEnumKeyEx(mpMainKey, 
				liLoop,
				lszSubKeyName, 
				&luLen, 
				NULL, 
				NULL, 
				NULL, 
				&lstFtLastWriteTime); 
			if (luStatus == ERROR_SUCCESS) 
			{
				// 类名匹配,则继续取DLLPATH的路径
				if (_wcsicmp(lszSubKeyName, nswClsName) == 0)
				{
					if(mb64Bit == FALSE)
						luStatus = RegOpenKey(mpMainKey,lszSubKeyName,&lpSubKey);
					else
						luStatus = RegOpenKeyEx(mpMainKey,lszSubKeyName,0,KEY_READ|KEY_WOW64_64KEY,&lpSubKey); 

					if (luStatus != ERROR_SUCCESS)
						continue;

					lszKeyValue[0] = '\0'; 

					luStatus = RegQueryValueEx(lpSubKey,
						L"DllPath",
						NULL,
						&luValueType,
						(unsigned char*)lszKeyValue,
						&luSubValueLen);
					
					RegCloseKey(lpSubKey);
					
					if (luStatus == ERROR_SUCCESS)
					{
						wcscpy_s(nswDllPath,luSubValueLen, lszKeyValue);
						lbFind = TRUE;
						break;
					}

				}
			}// end if
		} // end for
	} // end if


	return lbFind;

}



HKEY CXelDataMgr::GetMainKey()
{

	return mpMainKey;

}