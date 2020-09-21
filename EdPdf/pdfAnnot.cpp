/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "pdfannot.h"

//////////////////////////////////////////////////////////////////////////
// Annot类
DEFINE_BUILTIN_NAME(CpdfdAnnot)


CpdfdAnnot::CpdfdAnnot()
{
}

CpdfdAnnot::~CpdfdAnnot()
{

}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfdAnnot::InitOnCreate(void)
{

	return EDERR_SUCCESS;
}

uint32 CpdfdAnnot::SaveToAchive(buf_ptr buf, uint32 bufSize)
{
	return NULL;
}








//////////////////////////////////////////////////////////////////////////
// Annot List 类
DEFINE_BUILTIN_NAME(CpdfdAnnotList)


CpdfdAnnotList::CpdfdAnnotList()
{
}

CpdfdAnnotList::~CpdfdAnnotList()
{
	for (auto i : mAnnots)
	{
		i->Release();
	}
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfdAnnotList::InitOnCreate(void)
{
	return EDERR_SUCCESS;
}

int32Eink CpdfdAnnotList::GetCount(void)
{
	return (int32Eink)mAnnots.size();
}

IEdAnnot_ptr CpdfdAnnotList::GetAnnot(int32Eink index)
{
	mAnnots[index]->AddRefer();
	return mAnnots[index];
}

void CpdfdAnnotList::AddAnnot(IEdAnnot_ptr annot)
{
	annot->AddRefer();
	mAnnots.push_back(annot);
}

void CpdfdAnnotList::AddAnnots(vector<IEdAnnot_ptr> annots)
{
	for (auto i : annots)
	{
		i->AddRefer();
		mAnnots.push_back(i);
	}
}

