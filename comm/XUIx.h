/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
/*	FileName:	XuiX.h
	Comment:	本文件提供对Xui系统调用的简便封装
	History:
			Ax Created Nov.16,2011
			Pangxie modified to support Eink UI
*/
#include "math.h"
#include "Einkui.h"



// 下面的类用于提供便捷的访问XUI消息系统的方法，包括发送消息，和从消息获取数据
class CExMessage{
private:
	static IXelManager* gpElementManager;
	static __inline void GetElementManager(void);

	static __inline IEinkuiMessage* MakeUpMessage(IEinkuiIterator* npSender,bool nbSend,ULONG nuMsgID,const void* npInBuffer,int niInBytes,void* npOutBuffer,int niOutBytes);

public:
	static void* DataInvalid;


	// Post模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Post发送消息需要注意这个问题，防止地址到消息接收者出无法访问。
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
	template<typename DataType>
	static ERESULT PostMessage(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const DataType& rDataToPost,ULONG nuFast=EMSG_POSTTYPE_NORMAL);

	// Send模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Send模式发送消息时，这不会导致特殊问题
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
	template<typename DataType>
	static ERESULT SendMessage(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const DataType& rDateForSend,void* npBufferForReceive=NULL,int niBytesOfBuffer=0);

	// Post模式发送消息，并且携带一个字符串，注意，字符串一定要带有\0结尾
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
	static ERESULT PostMessageWithText(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const wchar_t* nswText,ULONG nuFast=EMSG_POSTTYPE_NORMAL);

	// Send模式发送消息，并且携带一个字符串，注意，字符串一定要带有\0结尾
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
	static ERESULT SendMessageWithText(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const wchar_t* nswText,void* npBufferForReceive=NULL,int niBytesOfBuffer=0);

	// Post模式发送消息，并且携带缓冲区数据
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
	static ERESULT PostMessageWithBuffer(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const void* npBuffer,int niBytes,ULONG nuFast=EMSG_POSTTYPE_NORMAL);

	// 没有必要提供Send模式发送缓冲区数据的方法，直接发送地址过去就好了。因为，当消息处理时，内存不会失效
	//ERESULT SendMessageWithBuffer(...);

	// 获取输入参数，成功获得将返回ERESULT_SUCCESS
	template<typename DataType>
	static ERESULT GetInputData(IEinkuiMessage* npMsg,DataType& rVariable);

	// 获取输入参数，成功获得将返回ERESULT_SUCCESS，如果得到的值与rInvalidValue相同，将返回ERESULT_WRONG_PARAMETERS
	template<typename DataType>
	static ERESULT GetInputData(IEinkuiMessage* npMsg,DataType& rVariable,const DataType& rInvalidValue);

	// 获取输入参数，当输入参数是一个结构体时，也可以直接获取访问它的指针；失败返回NULL
	template<typename DataType>
	static ERESULT GetInputDataBuffer(IEinkuiMessage* npMsg,DataType*& rBufferPointer);
};


void CExMessage::GetElementManager(void){
	if(gpElementManager == NULL)
	{
		gpElementManager = EinkuiGetSystem()->GetElementManager();
	}
}

IEinkuiMessage* CExMessage::MakeUpMessage(IEinkuiIterator* npSender,bool nbSend,ULONG nuMsgID,const void* npInBuffer,int niInBytes,void* npOutBuffer,int niOutBytes){

	GetElementManager();

	IEinkuiMessage* lpMsgIntf = gpElementManager->AllocateMessage();
	if(lpMsgIntf == NULL)
		return NULL;

	lpMsgIntf->SetMessageID(nuMsgID);
	if(npSender != NULL)
		lpMsgIntf->SetMessageSender(npSender);

	if(npInBuffer != NULL)
	{
		bool lbVolatile;

		// 如果输入数据量足够大，并且缓冲区不冲突，就不复制输入缓冲区了
		if(nbSend == false || niInBytes < 64 || npInBuffer == NULL || ( npOutBuffer != NULL &&
			((UCHAR*)npInBuffer+niInBytes) >= (UCHAR*)npOutBuffer &&
			(UCHAR*)npInBuffer			   < ((UCHAR*)npOutBuffer+niOutBytes)) )
			lbVolatile = true;
		else
			lbVolatile = false;

		lpMsgIntf->SetInputData(npInBuffer,niInBytes);
	}
	if(npOutBuffer != NULL)
		lpMsgIntf->SetOutputBuffer(npOutBuffer,niOutBytes);

	return lpMsgIntf;
}


// Post模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Post发送消息需要注意这个问题，防止地址到消息接收者出无法访问。
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
template<typename DataType>
ERESULT CExMessage::PostMessage(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const DataType& rDataToPost,ULONG nuFast)
{
	ERESULT luResult;
	IEinkuiMessage* lpMsgIntf;

	if((void*)&rDataToPost != (void*)&DataInvalid)
		lpMsgIntf = MakeUpMessage(npSender,false,nuMsgID,&rDataToPost,sizeof(rDataToPost),NULL,0);
	else
		lpMsgIntf = MakeUpMessage(npSender,false,nuMsgID,NULL,0,NULL,0);

	if(lpMsgIntf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luResult = gpElementManager->PostMessage(npReceiver,lpMsgIntf,nuFast);

	lpMsgIntf->Release();

	return luResult;
}

// Send模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Send模式发送消息时，这不会导致特殊问题
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
template<typename DataType>
ERESULT CExMessage::SendMessage(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const DataType& rDateForSend,void* npBufferForReceive,int niBytesOfBuffer)
{
	ERESULT luResult;
	IEinkuiMessage* lpMsgIntf;

	if((void*)&rDateForSend != (void*)&DataInvalid)
		lpMsgIntf = MakeUpMessage(npSender,true,nuMsgID,&rDateForSend,sizeof(rDateForSend),npBufferForReceive,niBytesOfBuffer);
	else
		lpMsgIntf = MakeUpMessage(npSender,true,nuMsgID,NULL,0,npBufferForReceive,niBytesOfBuffer);

	if(lpMsgIntf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luResult = gpElementManager->SendMessage(npReceiver,lpMsgIntf);

	lpMsgIntf->Release();

	return luResult;
}

// 获取输入参数，成功获得将返回ERESULT_SUCCESS
template<typename DataType>
ERESULT CExMessage::GetInputData(IEinkuiMessage* npMsg,DataType& rVariable)
{
	if(npMsg->GetInputDataSize() != sizeof(rVariable) || npMsg->GetInputData()==NULL)
		return ERESULT_WRONG_PARAMETERS;

	rVariable = *(DataType*)npMsg->GetInputData();

	return ERESULT_SUCCESS;
}

// 获取输入参数，成功获得将返回ERESULT_SUCCESS，如果得到的值与rInvalidValue相同，将返回ERESULT_WRONG_PARAMETERS
template<typename DataType>
ERESULT CExMessage::GetInputData(IEinkuiMessage* npMsg,DataType& rVariable,const DataType& rInvalidValue) 
{
	ERESULT luResult = CExMessage::GetInputData(npMsg,rVariable);

	if(luResult == ERESULT_SUCCESS && rVariable == rInvalidValue)
		return ERESULT_WRONG_PARAMETERS;

	return ERESULT_SUCCESS;
}

// 获取输入参数，当输入参数是一个结构体时，也可以直接获取访问它的指针；失败返回NULL
template<typename DataType>
ERESULT CExMessage::GetInputDataBuffer(IEinkuiMessage* npMsg,DataType*& rBufferPointer){
	if(npMsg->GetInputDataSize() != sizeof(*rBufferPointer) || npMsg->GetInputData()==NULL)
		return ERESULT_WRONG_PARAMETERS;

	rBufferPointer = (DataType*)npMsg->GetInputData();

	return ERESULT_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
// 用于显示Windows的消息对话框
DECLARE_BUILTIN_NAME(CExWinPromptBox)
class CExWinPromptBox : public cmmBaseObject<CExWinPromptBox,IBaseObject,GET_BUILTIN_NAME(CExWinPromptBox)>
{
public:
	//CExWinPromptBox(){}
	//~CExWinPromptBox(){}

	// 显示Windows的MessageBox，参数同Windows的完全一致；用法如：CExWinPromptBox::MessageBox(L"哈哈哈",L"Ax",MB_OK);
	static int MessageBox(const wchar_t* nswText,const wchar_t* nswTitle,UINT nuType);

	// 显示Windows的MessageBoxEx，参数同Windows的完全一致；
	static int MessageBoxEx(const wchar_t* nswText,const wchar_t* nswTitle,UINT nuType,WORD nsuLanguage);

	// 设置鼠标光标
	static HCURSOR SetCursor(HCURSOR nhCursor);

protected:
	struct ST_MESSAGEBOX{
		const wchar_t* Text;
		const wchar_t* Title;
		UINT Type;
		WORD Language;
		int Result;
	};

	ERESULT __stdcall MessageBoxCallBack(ULONG nuFlag,LPVOID npContext);
	ERESULT __stdcall MessageBoxExCallBack(ULONG nuFlag,LPVOID npContext);

	ERESULT __stdcall SetCursorCallBack(ULONG nuFlag,LPVOID npContext);


};


//////////////////////////////////////////////////////////////////////////
// 用于浮点数辅助运算
class CExFloat
{
public:
	// 去掉浮点数的小数位，采用4舍5入
	static __inline FLOAT Round(FLOAT Org){
		return (FLOAT)floor(Org+0.49999f);
	}

	// 将浮点数的取值为最接近的‘.5’值，如1.3 -> 1.5 , 2.9 -> 2.5
	static __inline FLOAT HalfPixel(FLOAT Org){
		return (FLOAT)floor(Org)+0.5f;
	}

	// 将浮点数取值为最小整数减去0.4999，如1.3 -> 0.5 , 2.9 -> 1.5
	static __inline FLOAT UnderHalf(FLOAT Org){
		return (FLOAT)floor(Org)-0.49999f;
	}

	// 判断两个浮点数是否相等
	static __inline bool Equal(FLOAT X1,FLOAT X2,FLOAT Allow=0.0001f){
		if(fabs(X1 - X2)<Allow)
			return true;
		return false;
	}

	// 取最小的大于等于输入浮点数的整数值
	static __inline FLOAT Ceil(FLOAT Org){
		return (FLOAT)ceil(Org);
	}

	// 取最大的小于等于输入浮点数的整数值
	static __inline FLOAT Floor(FLOAT Org){
		return (FLOAT)floor(Org);
	}

	// 将浮点数转换为整型数，将执行四舍五入
	static __inline LONG ToLong(FLOAT Org){
		return static_cast<LONG>(Round(Org));
	}


};


#define LW_REAL_EPSILON        1.192092896e-07F        /* FLT_EPSILON */

//////////////////////////////////////////////////////////////////////////
// 矩形运算
class CExRect
{
public:

	static __inline bool IsEmptyArea(float nfWidth, float nfHeight)
	{
//		return (nfWidth <= LW_REAL_EPSILON) || (nfHeight <= LW_REAL_EPSILON);
		return (nfWidth >= -LW_REAL_EPSILON && nfWidth <= LW_REAL_EPSILON) || (nfHeight >= -LW_REAL_EPSILON && nfHeight <= LW_REAL_EPSILON);
	}

	// 求交集 Sets CRect equal to the intersection of two rectangles.
	static __inline bool  Intersect(IN const D2D1_RECT_F& ndRectOne, IN const D2D1_RECT_F& ndRectTwo, OUT D2D1_RECT_F& ndIntersectRect)
	{
		
		float lfRight = min(ndRectOne.right, ndRectTwo.right);
		float lfBottom  = min(ndRectOne.bottom, ndRectTwo.bottom);
		float lfLeft = max(ndRectOne.left, ndRectTwo.left);
		float lfTop = max(ndRectOne.top, ndRectTwo.top);

		ndIntersectRect.left = lfLeft;
		ndIntersectRect.top = lfTop;
		ndIntersectRect.right = lfRight;
		ndIntersectRect.bottom = lfBottom;

		return !CExRect::IsEmptyArea(ndIntersectRect.right - ndIntersectRect.left, ndIntersectRect.top - ndIntersectRect.bottom);

	}

	// 求并集 Sets CRect equal to the union of two rectangles.
	static __inline bool Union(IN const D2D1_RECT_F& ndRectOne, IN const D2D1_RECT_F& ndRectTwo, OUT D2D1_RECT_F& ndUnionRect)
	{
		float lfRight = max(ndRectOne.right, ndRectTwo.right);
		float lfBottom  = max(ndRectOne.bottom, ndRectTwo.bottom);
		float lfLeft = min(ndRectOne.left, ndRectTwo.left);
		float lfTop = min(ndRectOne.top, ndRectTwo.top);

		ndUnionRect.left = lfLeft;
		ndUnionRect.top = lfTop;
		ndUnionRect.right = lfRight;
		ndUnionRect.bottom = lfBottom;
		
		return !CExRect::IsEmptyArea(ndUnionRect.right - ndUnionRect.left, ndUnionRect.top - ndUnionRect.bottom);
	}

	// 调整矩形，使其顶点序列化
	static __inline D2D1_RECT_F GetNormalizedRectangle(float x1, float y1, float x2, float y2)
	{
		if (x2 < x1)
		{
			float tmp = x2;
			x2 = x1;
			x1 = tmp;
		}

		if (y2 < y1)
		{
			float tmp = y2;
			y2 = y1;
			y1 = tmp;
		}
		return D2D1::RectF(x1, y1, x2, y2);
	}

	// 调整矩形，使其顶点序列化
	static __inline D2D1_RECT_F GetNormalizedRectangle(const D2D1_RECT_F& ndRect)
	{
		return CExRect::GetNormalizedRectangle(ndRect.left, ndRect.top, ndRect.right, ndRect.bottom);
	}

	// 检测当前点是否落在矩形区域	
	static __inline bool PtInRect(IN const D2D1_POINT_2F& ndPoint, IN const D2D1_RECT_F& ndRect)
	{
		D2D1_RECT_F ldRect = CExRect::GetNormalizedRectangle(ndRect);

		return ndPoint.x >= ldRect.left && ndPoint.x < ldRect.right && ndPoint.y >= ldRect.top && ndPoint.y < ldRect.bottom;
	}

};


//////////////////////////////////////////////////////////////////////////
// 用于多边形填充算法相关的函数
class CExFill{

public:

	// 判断点是否在多边形内
	static __inline bool PtInPolygon (
		D2D1_POINT_2F ndfPoint,			// 待检测的点
		D2D1_POINT_2F* npPolygonPoint,	// 多边形的各个顶点坐标（首尾点可以不一致）
		int niCount						// 多边形顶点个数
		)
	{
		// 方法：求解通过该点的水平线与多边形各边的交点

		int liCross = 0;

		for (int liLoop = 0; liLoop < niCount; liLoop++)
		{
			D2D1_POINT_2F p1 = npPolygonPoint[liLoop];
			D2D1_POINT_2F p2 = npPolygonPoint[(liLoop + 1) % niCount];

			// 求解 y=ndfPoint.y 与 p1p2 的交点

			// 如果平行则继续
			if ( p1.y == p2.y )		
				continue;

			// 交点是否在p1p2延长线上
			if ( ndfPoint.y < min(p1.y, p2.y) )		
				continue;

			// 交点是否在p1p2延长线上
			if ( ndfPoint.y >= max(p1.y, p2.y) )	
				continue;

			// 求交点的 X 坐标 
			// 基本思想是：相似三角形，利用边的比例关系，求出X边长度。继而求出X坐标

			float lfCrossCoordX = (float)(ndfPoint.y - p1.y) * (float)(p2.x - p1.x) / (float)(p2.y - p1.y) + p1.x;
			if ( lfCrossCoordX > ndfPoint.x )
				liCross++; // 只统计单边交点

		}

		// 单边交点为偶数时，则说明点在多边形之外
		return (liCross % 2 == 1);
	}


	// 判断点是否在椭圆内
	static __inline bool PtInEllipse(
		D2D1_POINT_2F ndfPoint,			// 待检测的点
		D2D1_RECT_F	ndfRect				// 椭圆所围成的最小矩形
		)
	{
		// 方法：
		// 给定椭圆f(x,y)= x^2/a^2 + y^2/b^2=1, 如果：f(x1,y1) <1，说明x1,y1在椭圆内部

		// 计算椭圆长短轴
		float lfShortAxis = (ndfRect.right-ndfRect.left)/2;
		float lfLongAxis = (ndfRect.bottom - ndfRect.top)/2;

		// 椭圆圆心
		D2D1_POINT_2F ldfCenterPoint = D2D1::Point2F(lfShortAxis + ndfRect.left, lfLongAxis + ndfRect.top); 

		// 计算待检测的点相对于椭圆中心的坐标
		D2D1_POINT_2F ldfCheckPoint = D2D1::Point2F(ldfCenterPoint.x - ndfPoint.x, ldfCenterPoint.y - ndfPoint.y);

		// 计算椭圆函数值
		float lfResult = (ldfCheckPoint.x * ldfCheckPoint.x)/(lfShortAxis*lfShortAxis)+
			(ldfCheckPoint.y*ldfCheckPoint.y)/(lfLongAxis*lfLongAxis);

		return lfResult<=1.0f;

	}


	// 判断点是否在线段上
	static __inline bool PtOnLine(
		D2D1_POINT_2F ndfPoint,			// 待检测的点
		D2D1_POINT_2F ndfStartPoint,	// 直线起点
		D2D1_POINT_2F ndfEndPoint		// 直线终点
		)
	{
		// 方法：
		// 1，用差乘计算，误差会引起很大的偏差；
		// 2，由于差乘法的偏差太大，遂改用测距离的方法

		// 计算线段长度
		float lfLineSegDistance = (FLOAT)sqrt((ndfEndPoint.x-ndfStartPoint.x)*(ndfEndPoint.x-ndfStartPoint.x) +
			(ndfEndPoint.y-ndfStartPoint.y)*(ndfEndPoint.y-ndfStartPoint.y)
			);

		// 计算点到线段两端的距离
		float lfDistToStart = (FLOAT)sqrt((ndfPoint.x-ndfStartPoint.x)*(ndfPoint.x-ndfStartPoint.x) +
			(ndfPoint.y-ndfStartPoint.y)*(ndfPoint.y-ndfStartPoint.y)
			);

		float lfDistToEnd = (FLOAT)sqrt((ndfPoint.x-ndfEndPoint.x)*(ndfPoint.x-ndfEndPoint.x) +
			(ndfPoint.y-ndfEndPoint.y)*(ndfPoint.y-ndfEndPoint.y)
			);

		// 求差值
		float lfDist = lfDistToStart + lfDistToEnd - lfLineSegDistance;
			
		// 误差纠正为0.1
		lfDist = lfDist - 0.1f;

		return lfDist < LW_REAL_EPSILON;

		// 差乘法，条件（带检测的点肯定在线段做围成的矩形内）
		//{
		//	// 生成两个矢量线段
		//	D2D1_POINT_2F ldfVector1 = D2D1::Point2F(ndfEndPoint.x - ndfStartPoint.x, ndfEndPoint.y - ndfStartPoint.y);
		//	D2D1_POINT_2F ldfVector2 = D2D1::Point2F(ndfPoint.x - ndfStartPoint.x, ndfPoint.y - ndfStartPoint.y);
		//
		//	// 计算差乘，判断点是否在直线段所在的直线上
		//	// 公式： V1(x1, y1) X V2(x2, y2) = x1y2 – y1x2
		//	float lfCrossProduct = ldfVector1.x * ldfVector2.y - ldfVector1.y * ldfVector2.x;

		//	return (lfCrossProduct >= -LW_REAL_EPSILON && lfCrossProduct <= LW_REAL_EPSILON);
		//}

	}


};



//////////////////////////////////////////////////////////////////////////
//用于坐标的转换
class CExPoint
{
public:
	//用于计算被放大图上的坐标对应于原图的坐标
	//rdSrcSize原图大小
	//rdDestSize当前的显示大小
	//rdPoint要转换的坐标
	//nuMethod使用的放大方式：1、ESPB_DRAWBMP_EXTEND（延展线方式） 2、其它值表示直接缩放 
	//rdExtendLine如果使用的是ESPB_DRAWBMP_EXTEND放大方式，需要给出延展线坐标
	static D2D1_POINT_2F BigToOldPoint(D2D1_SIZE_F& rdSrcSize,D2D1_SIZE_F& rdDestSize,D2D1_POINT_2F& rdPoint,ULONG nuMethod,D2D1_POINT_2F& rdExtendLine);
};


//////////////////////////////////////////////////////////////////////////
// 用于注册系统默认快捷键
class CExHotkey
{
public:
	static bool RegisterHotKey(
		IEinkuiIterator* npFocusOn,	// 关注此对象和它的子对象，NULL表示根对象
		IEinkuiIterator* npReceiver,	// 用来接收快捷键的对象
		ULONG nuSysHotkeyID		// 系统默认的ID，见lwUI文件IXelManager定义末尾的系统默认快捷键ID表
		);
};