/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "windows.h"
#include "Float.h"
#include "pdfStextQuadList.h"



//////////////////////////////////////////////////////////////////////////
// Annot List 类
DEFINE_BUILTIN_NAME(CpdfStextQuadsList)


CpdfStextQuadsList::CpdfStextQuadsList()
{
	//mAPoint.x = mAPoint.y = mBPoint.x = mBPoint.y = 0.0f;
}

CpdfStextQuadsList::~CpdfStextQuadsList()
{
	//mAPoint.x = 0;
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CpdfStextQuadsList::InitOnCreate(void)
{
	return EDERR_SUCCESS;
}

void CpdfStextQuadsList::GetAPoint(OUT ED_POINTF* pt)
{
	if (mQuads.Size() <= 0)
	{
		pt->x = pt->y = 0.0f;
		return;
	}
	pt->x = mQuads.Front().left;
	pt->y = (mQuads.Front().top + mQuads.Front().bottom) / 2.0f;	// A点取最高处Quad的高度的一半
}

void CpdfStextQuadsList::GetBPoint(OUT ED_POINTF* pt)
{
	if (mQuads.Size() <= 0)
	{
		pt->x = pt->y = 0.0f;
		return;
	}
	pt->x = mQuads.Back().right;
	pt->y = (mQuads.Back().top + mQuads.Back().bottom) / 2.0f;	// B点取最低处Quad的高度的一半位置
}

int32Eink CpdfStextQuadsList::GetQuadCount(void)
{
	return mQuads.Size();
}

void CpdfStextQuadsList::GetQuadBound(IN int32Eink index, OUT ED_RECTF_PTR quad)
{
	quad->left = mQuads.GetEntry(index).left;
	quad->top = mQuads.GetEntry(index).top;
	quad->right = mQuads.GetEntry(index).right;
	quad->bottom = mQuads.GetEntry(index).bottom;
}

const ED_RECTF* CpdfStextQuadsList::GetQuad(IN int32Eink index)
{
	if (index < 0 || index >= mQuads.Size())
	{
		return NULL;
	}
	return &mQuads[index];
}

//#include "math.h"
//#include "cmmString.h"
void CpdfStextQuadsList::AddQuad(const ED_RECTF& quad)
{
	//if (fabs(quad.left - 48.0f) <= 1.0f && fabs(quad.right - 260.0f) <= 1.0f && fabs(quad.top - 723.0f) <= 1.0f && fabs(quad.bottom - 736.0f) <= 1.0f)
	//	DebugBreak();
	//cmmStringW msg;
	//msg.format(L"[%.0f:%.0f] - [%.0f:%.0f]\n", quad.left,  quad.right,quad.top, quad.bottom);
	//OutputDebugString(msg.ptr());

	mQuads.Insert(-1, quad);
	//mBPoint.x = quad.right;
	//mBPoint.y = (quad.top + quad.bottom) / 2.0f;	// B点取最尾处Quad的高度的一半位置
	//if (mQuads.Size() <= 0)
	//{	// A点取最头处Quad的高度的一半位置
	//	mAPoint = mBPoint;
	//}

	/*
	modified by Ax. 2020.04.16
	//mAPoint.x = mAPoint.y = FLT_MAX;
	//mBPoint.x = mBPoint.y = 0;

	int i;
	// 按照top优先，插入列表
	for (i = 0; i < mQuads.Size(); i++)
	{
		// 首先判断是不是同行，主要判断区域对Y轴投影是否相交
		if(IsSameLine(mQuads[i],quad))
			continue;// 在同行，则继续查找

		// 不在同行，则插入在i对象行之前
		if (mQuads[i].top > quad.top)
			break;
	}

	// 插入对象
	mQuads.Insert(i, quad);

	// 重新计算A/B点
	int a, b;

	//a = b = 0;
	//for (i = 1; i < mQuads.Size(); i++)
	//{
	//	// 如果在同一行
	//	if (IsSameLine(mQuads[a], mQuads[i]))
	//	{
	//		// 判断是否在左边
	//		if (mQuads[a].left > mQuads[i].left)
	//			a = i;
	//	}
	//	else
	//	if (mQuads[i].top < mQuads[a].top)
	//	{	// 比a点所在的行要小，更换a点
	//		a = i;
	//	}

	//	// 如果在同一行
	//	if (IsSameLine(mQuads[b], mQuads[i]))
	//	{
	//		// 判断是否在右边
	//		if (mQuads[b].right < mQuads[i].right)
	//			b = i;
	//	}
	//	else
	//	if (mQuads[i].top > mQuads[b].top)
	//	{	// 比b点所在的行要大，更换b点
	//		b = i;
	//	}
	//}
	// 更新A/B点的值
	//mAPoint.x = mQuads[a].left;
	//mAPoint.y = (mQuads[a].top + mQuads[a].bottom) / 2.0f;	// A点取最高处Quad的高度的一半位置
	//mBPoint.x = mQuads[b].right;
	//mBPoint.y = (mQuads[b].top + mQuads[b].bottom) / 2.0f;	// B点取最低处Quad的高度的一半位置
	*/
}

void CpdfStextQuadsList::AddQuad(const fz_quad& quad)
{
	ED_RECTF quadRect;

	quadRect.left =		quad.ul.x;
	quadRect.top =		quad.ul.y;
	quadRect.right =	quad.lr.x;
	quadRect.bottom =	quad.lr.y;

	AddQuad(quadRect);
}

int32 CpdfStextQuadsList::DetectIntersection(IEdStextQuadList_ptr checkWith)
{
	int32 rev = -1;
	for (int i = 0; i < mQuads.Size(); i++)
	{
		rev = DetectIntersection(checkWith, mQuads[i]);
		if (rev > -1)
			break;
	}

	return rev;
}

int32 CpdfStextQuadsList::DetectIntersection(IEdStextQuadList_ptr quadList, const ED_RECTF& quad)
{
	int rev = -1;
	for (int i = 0; i < quadList->GetQuadCount(); i++)
	{
		if (HasIntersection(*quadList->GetQuad(i), quad) != false)
		{
			rev = i;
			break;
		}
	}

	return rev;
}

bool CpdfStextQuadsList::HasIntersection(const ED_RECTF& a, const ED_RECTF& b)
{
	float lfRight = min(a.right, b.right);
	float lfBottom = min(a.bottom, b.bottom);
	float lfLeft = max(a.left, b.left);
	float lfTop = max(a.top, b.top);

	return (lfRight > lfLeft && lfBottom > lfTop);
}

bool CpdfStextQuadsList::IsSameLine(const ED_RECTF& a, const ED_RECTF& b)
{
	int insect = min(a.bottom, b.bottom) - max(a.top, b.top);
	if (insect > 0 && (insect*2 > (a.bottom-a.top) || insect*2 > (b.bottom-b.top)))
		return true;

	return false;
}

bool CpdfStextQuadsList::Combination(IEdStextQuadList_ptr addition)
{
	//ED_RECTF quad;

	//for (int i = 0; i < src->GetQuadCount(); i++)
	//{
	//	src->GetQuadBound(i, &quad);
	//	AddQuad(quad);
	//}
	if (addition->GetQuadCount() <= 0)
		return false;

	// 首先查看附加对象的首位是否和我碰撞
	int impact = DetectIntersection(this, *addition->GetQuad(0));
	if (impact >= 0)
	{	// 有碰撞，说明在文字选择顺序上，附加对象首行等于或者迟于本对象首行
		if (impact == 0)	// 两个对象的第一分块碰撞，则需要判断谁在先
		{	// 既然碰撞，必然是同一行，则横坐标优先
			if (mQuads.Front().left <= addition->GetQuad(0)->left)
				impact = 1;	// 从一插入队列
		}

	}
	else
	{// 没有碰撞，说明在文字选择顺序上，本对象首行迟于附加对象首行
		//CMMASSERT(DetectIntersection(addition, mQuads.Front()) >= 0);
		if (DetectIntersection(addition, mQuads.Front()) < 0)
			return false;	// 出错的原因可能是pdf annot内置对象的quad记录不符合多列文字排版的先后规则，比如我们之前的bug，也会导致部分文件中存在这个错误

		impact = 0;
	}

	// 插入附加对象除最后一块，虽然，这个插入可能会导致内部数据并不对齐，但不重要，我们主要就是记录一下它们，并保证AB点有效就好
	for (int i = 0; i < addition->GetQuadCount() - 1; i++)
	{
		mQuads.Insert(impact + i, *addition->GetQuad(i));
	}

	// 检查附加对象尾部是否和我碰撞
	impact = DetectIntersection(this, *addition->GetQuad(addition->GetQuadCount() - 1));
	if (impact >= 0)
	{	// 有碰撞，说明在文字选择顺序上，附加对象尾行等于或者早于本对象尾行
		if (impact == mQuads.Size() - 1)	// 碰撞在我的最后一行，则需要判断谁在后
		{
			if (mQuads.Back().right <= addition->GetQuad(addition->GetQuadCount() - 1)->right)
				impact++;
		}
	}
	else
	{ // 没有碰撞，说明在文字选择顺序上，本对象尾行早于附加对象尾行
		//CMMASSERT(DetectIntersection(addition, mQuads.Back())>=0);
		if (DetectIntersection(addition, mQuads.Back()) < 0)
			return false;	// 出错的原因可能是pdf annot内置对象的quad记录不符合多列文字排版的先后规则，比如我们之前的bug，也会导致部分文件中存在这个错误
		impact = mQuads.Size();
	}
	mQuads.Insert(impact, *addition->GetQuad(addition->GetQuadCount() - 1));

	return true;
}