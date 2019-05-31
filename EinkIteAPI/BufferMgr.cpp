/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */
#include "stdafx.h"
#include "BufferMgr.h"


CBufferMgr::CBufferMgr()
{
	mpBufferBase = NULL;
	mulSize = 0;
	mulBlockSize = 0;
}


CBufferMgr::~CBufferMgr()
{
	for (int i = 0; i < mdBufferList.Size(); i++)
		if (mdBufferList[i] != NULL)
			delete mdBufferList[i];
}

//设置内存起始地址及分配块大小
//nulBufferSize为Buffer大小
//nulBlockSize为每次要分配的大小
void CBufferMgr::Initialize(const BYTE* npBuffer, ULONG nulBufferSize, ULONG nulWidth, ULONG nulHeight)
{
	do 
	{
		mpBufferBase = npBuffer;
		mulSize = nulBufferSize;
		mulBlockSize = nulWidth*nulHeight;

		//开始分配内存块
		ULONG lulOffset = 0;

		while ((lulOffset+ mulBlockSize) < nulBufferSize)
		{
			//起始地址
			BYTE* lpBufferBase = (BYTE*)npBuffer + lulOffset;

			//转换成对应结构体
			EI_BUFFER* lpEiBuffer = (EI_BUFFER*)lpBufferBase;
			lpEiBuffer->ulBufferSize = mulBlockSize;
			lpEiBuffer->ulWidth = nulWidth;
			lpEiBuffer->ulHeight = nulHeight;

			BUFFER_BLOCK* lpdBufferBlock = new BUFFER_BLOCK();
			lpdBufferBlock->EiBuffer = lpEiBuffer;
			lpdBufferBlock->IsFree = true;
			lpdBufferBlock->BufferOffset = lulOffset;

			mdBufferList.Insert(-1, lpdBufferBlock);

			//偏移要加上结构体大小
			lulOffset += mulBlockSize + sizeof(EI_BUFFER) + 1;
		}

	} while (false);
	
}

//获取Buffer,当不需要时需要调用FreeBuffer
//正常返回Buffer地址，如果Buffer已经用完则返回NULL
EI_BUFFER* CBufferMgr::GetBuffer()
{
	EI_BUFFER* lpRetBuffer = NULL;

	do 
	{
		for (int i=0;i<mdBufferList.Size();i++)
		{
			BUFFER_BLOCK* lpdBufferBlock = mdBufferList.GetEntry(i);
			if (lpdBufferBlock->IsFree != false)
			{
				lpdBufferBlock->IsFree = false;
				lpRetBuffer = (EI_BUFFER*)lpdBufferBlock->EiBuffer;
				break;
			}
		}

	} while (false);

	return lpRetBuffer;
}

//释放Buffer
bool CBufferMgr::FreeBuffer(const EI_BUFFER* npBuffer)
{
	bool lbRet = false;

	do
	{
		for (int i = 0; i < mdBufferList.Size(); i++)
		{
			BUFFER_BLOCK* lpdBufferBlock = mdBufferList.GetEntry(i);
			if (lpdBufferBlock->EiBuffer == npBuffer)
			{
				lpdBufferBlock->IsFree = true;

				break;
			}
		}

		lbRet = true;

	} while (false);

	return lbRet;
}

//获取Buffer的offset
ULONG CBufferMgr::GetBufferOffset(const EI_BUFFER* npBuffer)
{
	ULONG lbRet = 0;

	do
	{
		for (int i = 0; i < mdBufferList.Size(); i++)
		{
			BUFFER_BLOCK* lpdBufferBlock = mdBufferList.GetEntry(i);
			if (lpdBufferBlock->EiBuffer == npBuffer)
			{
				lbRet = lpdBufferBlock->BufferOffset;

				break;
			}
		}

	} while (false);

	return lbRet;
}