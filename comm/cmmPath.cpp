/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


//////////////////////////////////////////////////////////////////////////
// 常用文件和目录转换工具函数
#include "stdafx.h"
#include "Windows.h"
#include "cmmPath.h"
#include "cmmstruct.h"
#include "Shlobj.h"

CFilePathName::CFilePathName()
{
	msiLength = 0;
	mszPathName[0]=UNICODE_NULL;
}

CFilePathName::CFilePathName(const class CFilePathName& src)
{
	*this = src;
}

CFilePathName::CFilePathName(const wchar_t* nszPathName)
{
	*this = nszPathName;
}


// 动态拷贝对象
CFilePathName* CFilePathName::Create(const wchar_t* nszPathName)
{
	CFilePathName* lpObj = new CFilePathName(nszPathName);
	return lpObj;
}

// 动态创建对象
CFilePathName* CFilePathName::Duplicate()
{
	CFilePathName* lpObj = new CFilePathName(*this);
	return lpObj;
}

// 当前目录
bool CFilePathName::SetByCurrentPath(void)
{
	int liLength;

	GetCurrentDirectory(MAX_PATH,mszPathName); 
	liLength = (int)wcslen(mszPathName);
	if(liLength+1 >= MAX_PATH || liLength <= 0)
	{
		mszPathName[0] = UNICODE_NULL;
		msiLength = 0;
		return false;
	}

	msiLength = (short)liLength;
	mszPathName[msiLength++] = L'\\';
	mszPathName[msiLength] = UNICODE_NULL;

	return true;
}

// 模块所在目录和文件名
bool CFilePathName::SetByModulePathName(void)
{
	int liLength;

	liLength = GetModuleFileName(GetModuleHandle(NULL),mszPathName,MAX_PATH);
	if(liLength >= MAX_PATH || liLength <= 0)
	{
		mszPathName[0] = UNICODE_NULL;
		msiLength = 0;
		return false;
	}

	msiLength = (short)liLength;

	return true;
}

// 获得当前用户的AppData\Roaming 目录 例如：C:\Users\UserName\AppData\Roaming
// 调用该函数后，路径最后带有"\"
bool CFilePathName::SetByUserAppData(void)
{
	bool lbRet = false;

	do 
	{
		//获取目录
		if(SHGetSpecialFolderPath(NULL,mszPathName,CSIDL_LOCAL_APPDATA,FALSE) == FALSE)
			break;

		int liLength = (int)wcslen(mszPathName);
		if(liLength+1 >= MAX_PATH || liLength <= 0)
		{
			mszPathName[0] = UNICODE_NULL;
			msiLength = 0;
			break;
		}

		msiLength = (short)liLength;
		mszPathName[msiLength++] = L'\\';
		mszPathName[msiLength] = UNICODE_NULL;

		lbRet = true;

	} while (false);
	
	return lbRet;
}

// 设置为目录，如果结尾没有'\\'就加上
void CFilePathName::AssurePath(void)
{
	if(msiLength>0 && mszPathName[msiLength-1]!=L'\\' && msiLength < MAX_PATH)
	{
		mszPathName[msiLength++] = L'\\';
		mszPathName[msiLength] = 0;
	}
}

// 附加目录或者文件名，返回false失败，可能的原因无法转换位置，或者结果超长
//	实用用法：取同层目录传入L".\\"；取上层目录传入"..\\'
bool CFilePathName::Transform(const wchar_t* nszTransTo,bool nbForceBePath)
{
	cmmStack<wchar_t*> loSeparators;

	// 首先扫描一遍当前数据
	USHORT i;

	for (i=0;i<msiLength;i++)
	{
		if(mszPathName[i]==UNICODE_NULL)
			break;
		if(mszPathName[i] == L'\\')
		{
			loSeparators.Push(&mszPathName[i]);
		}
	}

	// 处理转换请求
	for(i=0;i<MAX_PATH;i++)
	{
		if(nszTransTo[i]==L'.')
		{
			i++;
			if(nszTransTo[i]==L'.' && nszTransTo[i+1]==L'\\')
			{
				i++;

				loSeparators.Pop();
			}
		}

		if(nszTransTo[i]!=L'\\')
			break;
	}

	if(loSeparators.Size() <= 0)
		return false;// 错误的层次

	// 复制目录后面的内容
	short j = (short)(loSeparators.Top() - mszPathName)+1;
	for(;i<MAX_PATH && j<MAX_PATH;i++,j++)
	{
		if((mszPathName[j] = nszTransTo[i])==UNICODE_NULL)
			break;
	}
	if(nbForceBePath != false)
	{
		if(j>= MAX_PATH)
			return false;
		if(j >= 0 && mszPathName[j-1]!=L'\\')
		{
			mszPathName[j++] = L'\\';
			mszPathName[j] = UNICODE_NULL;
		}
	}

	msiLength = j;

	return true;
}
// 附加目录或者文件名，返回false失败，可能的原因无法转换位置，或者结果超长
//	实用用法：取同层目录传入L".\\"；取上层目录传入"..\\'
bool CFilePathName::Transform(const CFilePathName& roPath)
{
	return Transform(roPath.mszPathName);
}

void CFilePathName::UnlockBuffer()
{
	for (msiLength =0;msiLength < MAX_PATH;msiLength++)
	{
		if(mszPathName[msiLength] == UNICODE_NULL)
			break;
	}
}

// 清除
void CFilePathName::Clean(void)
{
	mszPathName[0] = UNICODE_NULL;
	msiLength = 0;
}

// 取文件名，如果当前保存的是一个目录，则返回L"\0"
const wchar_t* CFilePathName::GetFileName(
	int* npLength	// 返回名字的字符数，不包括'\0'结尾
	)const
{
	int i;
	for(i=msiLength-1;i>= 0;i--)
	{
		if(mszPathName[i] == L'\\')
			break;
	}
	if(i<0)
		i=0;	// 不包含目录
	else
		i++;

	if(npLength != NULL)
		*npLength = (msiLength-i);

	return mszPathName+i;
}

// 取扩展名，如果当前保存的不是一个文件，或者不具有扩展名，则返回L"\0"
const wchar_t* CFilePathName::GetExtName(void)const
{
	int i;
	for(i=msiLength-1;i>= 0;i--)
	{
		if(mszPathName[i] == L'\\')
		{
			i = -1;
			break;
		}
		if(mszPathName[i]==L'.')
			break;
	}
	if(i<0)
		i = msiLength;	// 刚好指向L'\0'
	else
		i++;

	return mszPathName+i;
}

// 复制
void CFilePathName::operator=(const class CFilePathName& src)
{
	msiLength = src.msiLength;
	if(msiLength > 0)
		RtlCopyMemory(mszPathName,src.mszPathName,(msiLength+1)*sizeof(wchar_t));
}

// 从字符串赋值
void CFilePathName::operator=(const wchar_t* nswSrc)
{
	for (msiLength =0;msiLength < MAX_PATH;msiLength++)
	{
		if((mszPathName[msiLength] = nswSrc[msiLength])==UNICODE_NULL)
			break;
	}
}


// 只复制全部目录，结果会保留'\\'
bool CFilePathName::CopyPath(const class CFilePathName& src)
{
	*this = src;
	return Transform(L".\\");
}
// 判断文件扩展名是否等于指定的类型，使用\0分割类型名，两个\0表示类型结束，返回值<0不相符合，否则返回的是相符合的类型编号，从0开始
int CFilePathName::ExtNameMatch(const wchar_t* nszFilter)const
{
	int liIndex;
	const wchar_t* lpExtName = GetExtName();

	for(liIndex=0;*nszFilter != UNICODE_NULL;nszFilter++,liIndex++)
	{
		if(_wcsicmp(lpExtName,nszFilter)==0)
			return liIndex;

		while(*nszFilter != UNICODE_NULL)
			nszFilter++;
	}

	return -1;
}

// 直接附加字符串
void CFilePathName::operator+=(const class CFilePathName& src)throw(...)
{
	if(msiLength+src.msiLength >= MAX_PATH)
		throw mszPathName;
	RtlCopyMemory(mszPathName+msiLength,src.mszPathName,(src.msiLength+1)*sizeof(wchar_t));
	msiLength += src.msiLength;
}

void CFilePathName::operator+=(const wchar_t* nszAppend)throw(...)
{
	for(;msiLength < MAX_PATH;msiLength++)
	{
		if((mszPathName[msiLength] = *nszAppend++)==UNICODE_NULL)
			break;
	}
	if(msiLength >= MAX_PATH)
		throw mszPathName;
}

bool CFilePathName::operator==(const class CFilePathName& right)
{
	if(msiLength != right.msiLength)
		return false;

	for (int i=0;i<msiLength;i++)
	{
		if(mszPathName[i] != right.mszPathName[i] && ((mszPathName[i]|0x20) != (right.mszPathName[i]|0x20) || (mszPathName[i]|0x20) < L'a' || (mszPathName[i]|0x20) > L'z'))
		{
			return false;
			break;
		}
	}

	return true;
}

bool CFilePathName::operator!=(const class CFilePathName& right)
{
	return !(*this==right);
}
