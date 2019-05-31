/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "cmmstruct.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "XCtl.h"

#include "SmCuveImp.h"

DEFINE_BUILTIN_NAME(Cuve)

CSmCuveImp::CSmCuveImp()
{
	mpLineBrush = NULL;
}

CSmCuveImp::~CSmCuveImp()
{

}

ULONG CSmCuveImp::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(npTemplete == NULL)
			return ERESULT_UNSUCCESSFUL;

		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//mpIterator->ModifyStyles(EITR_STYLE_LAZY_UPDATE);


		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CSmCuveImp::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	do
	{
		luResult = CXuiElement::OnElementCreate(npIterator);
		if(luResult != ERESULT_SUCCESS)
			break;

		mlBaseLine = mpTemplete->QuerySubKeyValueAsLONG(L"BaseLine",(LONG)mpIterator->GetSizeY() - 1);
		mfInterval = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(L"Interval",6);
		if(mfInterval <= 0.0f)
			mfInterval = 6.0f;
		mlMaxPt = ((LONG)mpIterator->GetSizeX() + (LONG)mfInterval - 1)/(LONG)mfInterval;

		mpIterator->SetVisibleRegion(D2D1::RectF(0.0f,0.0f,mpIterator->GetSizeX(),mpIterator->GetSizeY()));

		luResult = ERESULT_SUCCESS;

	}while(false);

	return luResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CSmCuveImp::OnElementDestroy()
{
	CMM_SAFE_RELEASE(mpLineBrush);

	CXuiElement::OnElementDestroy();

	return ERESULT_SUCCESS;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CSmCuveImp::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	if(npMsg->GetMessageID() == EACT_CUVE_ADD_POINT)
	{
		if(npMsg->GetInputDataSize() < sizeof(LONG))
		{
			return ERESULT_WRONG_PARAMETERS;
		}

		LONG llPercent = *(LONG*)npMsg->GetInputData();
		if(llPercent < 0)
			llPercent = 0;

		llPercent = ((100-llPercent)*mlBaseLine)/100;
		moPointArr.Insert(-1,(FLOAT)llPercent);

		if(moPointArr.Size() > mlMaxPt)
			moPointArr.RemoveByIndex(0);
	}
	else
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类

	return luResult;
}

//绘制消息
ERESULT CSmCuveImp::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	FLOAT lfX = mpIterator->GetSizeX();

	// 下面建立画刷
	if(mpLineBrush ==NULL)
	{
		mpLineBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush,D2D1::ColorF(D2D1::ColorF::Green,1.0f));
	}

	for (int i=moPointArr.Size()-1;i>0;i--)
	{
		npPaintBoard->DrawLine(D2D1::Point2F(lfX,moPointArr[i]),D2D1::Point2F(lfX-mfInterval,moPointArr[i-1]),mpLineBrush);
		lfX -= mfInterval;
	}

	return ERESULT_SUCCESS;
}

