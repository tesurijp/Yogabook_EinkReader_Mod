/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "XgBitArray.h"


CBitArray::CBitArray(DWORD ndwBitCount) : mpBits(NULL), mdwByteCount(0), mdwBitCount(0)
{
	mdwBitCount = ndwBitCount;

	mdwByteCount = (mdwBitCount / 8) + ((mdwBitCount % 8) ? 1 : 0);

	mpBits = new BYTE[mdwByteCount];

	if(NULL == mpBits)
		mdwBitCount = 0;
}

CBitArray::~CBitArray()
{
	if(mpBits)
		delete []mpBits;
}

void CBitArray::SetBit(DWORD ndwOffset, bool nBit)
{
	if(ndwOffset >= mdwBitCount)
		return ;
	DWORD dwByteOff = ndwOffset / 8;
	BYTE cByteMask = (0x01 << (ndwOffset % 8));

	if(nBit)
		mpBits[dwByteOff] |= cByteMask;
	else 
		mpBits[dwByteOff] &= ~cByteMask;
}

bool CBitArray::GetBit(DWORD ndwOffset)
{
	if(ndwOffset >= mdwBitCount)
		return false;
	DWORD dwByteOff = ndwOffset / 8;
	BYTE cByteMask = (0x01 << (ndwOffset % 8));

	return (mpBits[dwByteOff] & cByteMask) ? true : false;
}
