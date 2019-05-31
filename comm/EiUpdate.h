/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "cmmstruct.h"

#pragma pack(1)
class CEiLineDiff
{
public:
	unsigned short Begin;
	unsigned short End;
	CEiLineDiff(){}
	void operator=(const CEiLineDiff& src) {
		this->Set(src.Begin, src.End);
	}
	CEiLineDiff(unsigned short nuBegin, unsigned short nuEnd) {
		Set(nuBegin, nuEnd);
	}
	void Set(unsigned short nuBegin, unsigned short nuEnd) {
		Begin = nuBegin;
		End = nuEnd;
	}
};
#pragma pack()

class CEiLineDiffVector : private cmmVector<CEiLineDiff,1,1500>
{
public:
	CEiLineDiffVector();

	// 设置虚拟成员，仅在Eink 分辨率变化时调用
	bool AddDummy(int niTotal) {

		InterlockedExchange(&mlLock, 1);

		while(miSize < niTotal)
		{
			if (miCapacity > niTotal)
			{
				miSize = niTotal;
			}
			else
			{
				// 扩充存储
				CEiLineDiff loDummy(0xFFFF,0);
				Insert(-1, loDummy);
			}
		}

		InterlockedExchange(&mlLock, 0);
		return true;
	}
	
	int Size(void) {
		// 简单锁死
		while (mlLock != 0)
			Sleep(1);

		return cmmVector<CEiLineDiff, 1, 1500>::Size();
	}

	// 返回成员对象
	CEiLineDiff& operator [](int niIndex) {

		// 简单锁死
		while (mlLock != 0)
			Sleep(1);

		return cmmVector<CEiLineDiff, 1, 1500>::operator[](niIndex);
	}

	void Reset(void) {
		for (int i = 0; i < miSize; i++)
		{
			GetEntry(i).Set(0xFFFF, 0);
		}
	}

private:
	volatile LONG mlLock;
};

class CEiRowBlock
{
public:
	unsigned short x1;
	unsigned short y1;
	unsigned short x2;	// 指向结束位置
	unsigned short y2;	// 指向结束位置
	unsigned short Lines;
	void operator=(const CEiRowBlock& src) {
		x1 = src.x1;
		x2 = src.x2;
		y1 = src.y1;
		y2 = src.y2;
		Lines = src.Lines;
	}
	void Add(const CEiRowBlock& ToAdd) {
		if (ToAdd.x1 < x1)
			x1 = ToAdd.x1;
		if (ToAdd.x2 > x2)
			x2 = ToAdd.x2;
		if (ToAdd.y1 < y1)
			y1 = ToAdd.y1;
		if (ToAdd.y2 > y2)
			y2 = ToAdd.y2;
		Lines = y2 - y1;
	}
	void Add(const CEiLineDiff& ToAdd) {
		if (ToAdd.Begin < x1)
			x1 = ToAdd.Begin;
		if (ToAdd.End > x2)
			x2 = ToAdd.End;
		y2++;
		Lines = y2 - y1;
	}
};

typedef cmmVector<CEiRowBlock> CEiBlocks;


class CEiUpdate
{
public:
	CEiUpdate();
	~CEiUpdate();

	void SetPanel(ULONG nuWith, ULONG nuHeight) {
		muTotalPixels = nuWith*nuHeight;
		moDiffLines.AddDummy(nuHeight);
		moDiffLines.Reset();
	}

	void Reset(void) {
		moDiffLines.Reset();
	}

	// 这个函数多线程安全 
	void SaveLineDiff(int niLine, unsigned long nuBegin, unsigned long nuEnd) {
		if(moDiffLines[niLine].Begin > (unsigned short)nuBegin)
			moDiffLines[niLine].Begin = (unsigned short)nuBegin;
		if(moDiffLines[niLine].End < (unsigned short)nuEnd)
			moDiffLines[niLine].End =  (unsigned short)nuEnd;
	}

	int GetDiffBlocks(CEiBlocks& nrBlocks);

private:
	CEiLineDiffVector moDiffLines;
	ULONG muTotalPixels;

	// 找到所有差异块，一个差异块即连续不相等行
	void FoundDiffBlocks(CEiBlocks& nrDiffBlocks);

	// 计算面积最小的分块融合方案，为了简单，最多只合并为两块
	void CombineBlocks(CEiBlocks& nrDiffBlocks, CEiBlocks& nrComBlocks);
};

