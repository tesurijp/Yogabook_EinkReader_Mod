/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "math.h"




//////////////////////////////////////////////////////////////////////////
// 用于浮点数辅助运算
class CGmFloat
{
public:
	// 去掉浮点数的小数位，采用4舍5入
	static __inline float Round(float Org){
		return (float)floor(Org+0.49999f);
	}

	// 将浮点数的取值为最接近的‘.5’值，如1.3 -> 1.5 , 2.9 -> 2.5
	static __inline float HalfPixel(float Org){
		return (float)floor(Org)+0.5f;
	}

	// 将浮点数取值为最小整数减去0.4999，如1.3 -> 0.5 , 2.9 -> 1.5
	static __inline float UnderHalf(float Org){
		return (float)floor(Org)-0.49999f;
	}

	// 判断两个浮点数是否相等
	static __inline bool Equal(float X1,float X2,float Allow=0.0001f){
		if(fabs(X1 - X2)<Allow)
			return true;
		return false;
	}

	// 取最小的大于等于输入浮点数的整数值
	static __inline float Ceil(float Org){
		return (float)ceil(Org);
	}

	// 取最大的小于等于输入浮点数的整数值
	static __inline float Floor(float Org){
		return (float)floor(Org);
	}

	// 将浮点数转换为整型数，将执行四舍五入
	static __inline LONG ToLong(float Org){
		return static_cast<LONG>(Round(Org));
	}


};


//#define LW_REAL_EPSILON        1.192092896e-07F        /* FLT_EPSILON */
//
////////////////////////////////////////////////////////////////////////////
//// 矩形运算
//class CExRect
//{
//public:
//
//	static __inline bool IsEmptyArea(float nfWidth, float nfHeight)
//	{
////		return (nfWidth <= LW_REAL_EPSILON) || (nfHeight <= LW_REAL_EPSILON);
//		return (nfWidth >= -LW_REAL_EPSILON && nfWidth <= LW_REAL_EPSILON) || (nfHeight >= -LW_REAL_EPSILON && nfHeight <= LW_REAL_EPSILON);
//	}
//
//	// 求交集 Sets CRect equal to the intersection of two rectangles.
//	static __inline bool  Intersect(IN const D2D1_RECT_F& ndRectOne, IN const D2D1_RECT_F& ndRectTwo, OUT D2D1_RECT_F& ndIntersectRect)
//	{
//		
//		float lfRight = min(ndRectOne.right, ndRectTwo.right);
//		float lfBottom  = min(ndRectOne.bottom, ndRectTwo.bottom);
//		float lfLeft = max(ndRectOne.left, ndRectTwo.left);
//		float lfTop = max(ndRectOne.top, ndRectTwo.top);
//
//		ndIntersectRect.left = lfLeft;
//		ndIntersectRect.top = lfTop;
//		ndIntersectRect.right = lfRight;
//		ndIntersectRect.bottom = lfBottom;
//
//		return !CExRect::IsEmptyArea(ndIntersectRect.right - ndIntersectRect.left, ndIntersectRect.top - ndIntersectRect.bottom);
//
//	}
//
//	// 求并集 Sets CRect equal to the union of two rectangles.
//	static __inline bool Union(IN const D2D1_RECT_F& ndRectOne, IN const D2D1_RECT_F& ndRectTwo, OUT D2D1_RECT_F& ndUnionRect)
//	{
//		float lfRight = max(ndRectOne.right, ndRectTwo.right);
//		float lfBottom  = max(ndRectOne.bottom, ndRectTwo.bottom);
//		float lfLeft = min(ndRectOne.left, ndRectTwo.left);
//		float lfTop = min(ndRectOne.top, ndRectTwo.top);
//
//		ndUnionRect.left = lfLeft;
//		ndUnionRect.top = lfTop;
//		ndUnionRect.right = lfRight;
//		ndUnionRect.bottom = lfBottom;
//		
//		return !CExRect::IsEmptyArea(ndUnionRect.right - ndUnionRect.left, ndUnionRect.top - ndUnionRect.bottom);
//	}
//
//	// 调整矩形，使其顶点序列化
//	static __inline D2D1_RECT_F GetNormalizedRectangle(float x1, float y1, float x2, float y2)
//	{
//		if (x2 < x1)
//		{
//			float tmp = x2;
//			x2 = x1;
//			x1 = tmp;
//		}
//
//		if (y2 < y1)
//		{
//			float tmp = y2;
//			y2 = y1;
//			y1 = tmp;
//		}
//		return D2D1::RectF(x1, y1, x2, y2);
//	}
//
//	// 调整矩形，使其顶点序列化
//	static __inline D2D1_RECT_F GetNormalizedRectangle(const D2D1_RECT_F& ndRect)
//	{
//		return CExRect::GetNormalizedRectangle(ndRect.left, ndRect.top, ndRect.right, ndRect.bottom);
//	}
//
//	// 检测当前点是否落在矩形区域	
//	static __inline bool PtInRect(IN const D2D1_POINT_2F& ndPoint, IN const D2D1_RECT_F& ndRect)
//	{
//		D2D1_RECT_F ldRect = CExRect::GetNormalizedRectangle(ndRect);
//
//		return ndPoint.x >= ldRect.left && ndPoint.x < ldRect.right && ndPoint.y >= ldRect.top && ndPoint.y < ldRect.bottom;
//	}
//
//};

//////////////////////////////////////////////////////////////////////////
// 线段
class CGmLine
{
public:
	static bool IsSegmentCross(float p1x, float p1y, float p2x, float p2y, float q1x, float q1y, float q2x, float q2y) {
		// https://blog.csdn.net/qq826309057/article/details/70942061

		//快速排斥实验
		if ((p1x > p2x ? p1x : p2x) < (q1x < q2x ? q1x : q2x) ||
			(p1y > p2y ? p1y : p2y) < (q1y < q2y ? q1y : q2y) ||
			(q1x > q2x ? q1x : q2x) < (p1x < p2x ? p1x : p2x) ||
			(q1y > q2y ? q1y : q2y) < (p1y < p2y ? p1y : p2y))
		{
			return false;
		}
		//跨立实验
		if ((((p1x - q1x)*(q2y - q1y) - (p1y - q1y)*(q2x - q1x))*
			((p2x - q1x)*(q2y - q1y) - (p2y - q1y)*(q2x - q1x))) > 0 ||
			(((q1x - p1x)*(p2y - p1y) - (q1y - p1y)*(p2x - p1x))*
			((q2x - p1x)*(p2y - p1y) - (q2y - p1y)*(p2x - p1x))) > 0)
		{
			return false;
		}
		return true;
	}
};
