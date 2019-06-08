/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */
#pragma once
//管理APP显示内存
#include "cmmstruct.h"
#include "EinkIteAPI.h"

class CBufferMgr
{
public:
	CBufferMgr();
	~CBufferMgr();

	typedef struct _BUFFER_BLOCK
	{
		volatile bool IsFree;
		EI_BUFFER* EiBuffer;
		ULONG BufferOffset;
	}BUFFER_BLOCK,*PBUFFER_BLOCK;

	//设置内存起始地址及分配块大小
	//nulBufferSize为Buffer大小
	//nulWidth为每次要分配的大小的宽度
	//nulHeight为每次要分配的大小的高度度
	void Initialize(const BYTE* npBuffer,ULONG nulBufferSize, ULONG nulWidth,ULONG nulHeight);
	//获取Buffer,当不需要时需要调用FreeBuffer
	//正常返回Buffer地址，如果Buffer已经用完则返回NULL
	EI_BUFFER* GetBuffer();
	//释放Buffer
	bool FreeBuffer(const EI_BUFFER* npBuffer);
	//获取Buffer的offset
	ULONG GetBufferOffset(const EI_BUFFER* npBuffer);

private:
	//分配给自己的内存起始地址
	const BYTE* mpBufferBase;
	//BUFFER总大小
	ULONG mulSize;
	//分配块大小
	ULONG mulBlockSize;

	//Buffer池
	cmmVector<BUFFER_BLOCK*> mdBufferList;
};

