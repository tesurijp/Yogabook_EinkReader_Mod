/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _LW_BITARRAY
#define _LW_BITARRAY

class CBitArray {
	DWORD mdwBitCount;
	PBYTE mpBits;
	DWORD mdwByteCount;
public:
	CBitArray(DWORD ndwBitCount);
	~CBitArray();

	void SetBit(DWORD ndwOffset, bool nBit);
	bool GetBit(DWORD ndwOffset);
};


#endif