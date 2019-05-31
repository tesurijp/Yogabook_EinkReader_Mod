/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvButtonImp.h"

#include "EvPpButtonImp.h"

DEFINE_BUILTIN_NAME(PingpongButton)

CEvPingpongButton::CEvPingpongButton()
{
	miDirection = -1;
}

CEvPingpongButton::~CEvPingpongButton()
{

}



ULONG CEvPingpongButton::InitOnCreate( 
	IN IEinkuiIterator* npParent, 
	IN ICfKey* npTemplete, 
	IN ULONG nuEID/* =MAXULONG32 */ 
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{

		// 先调用基类函数
		leResult = CEvButton::InitOnCreate(npParent, npTemplete, nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		BREAK_ON_NULL(npTemplete);
		// 通过配置项，获取按钮方向，即垂直按钮还是水平按钮
		miDirection = npTemplete->QuerySubKeyValueAsLONG(L"Direction", 0);

		CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_HAND);

	} while (false);

	return leResult;
}


//元素拖拽
ERESULT CEvPingpongButton::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		// 水平按钮
		if (miDirection == 1)
		{
			// 从左向右移动,偏移量为正值，当偏移量>(按钮宽度/2)时，状态置位
			if(npInfo->Offset.x > (CEvButton::GetIterator()->GetSize().width/2))
				CEvButton::SetChecked(true);

			// 从右向左 偏移量为负值
			if(npInfo->Offset.x < -(CEvButton::GetIterator()->GetSize().width/2))
				CEvButton::SetChecked(false);
		}
		// 垂直按钮
		else if (miDirection == 2)
		{
			// 从下往上移动，偏移量为正值
			if (npInfo->Offset.y > (CEvButton::GetIterator()->GetSize().height/2))
				CEvButton::SetChecked(false);


			// 从上往下移动，偏移量为负值
			if (npInfo->Offset.y < -(CEvButton::GetIterator()->GetSize().height/2))
				CEvButton::SetChecked(true);

		}

		leResult = ERESULT_SUCCESS;

	} while (false);


			 
	return leResult;
}

//拖拽开始,nulActKey哪个鼠标按钮按下进行拖拽
ERESULT CEvPingpongButton::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if(mpIterator->IsEnable() == false)
			break;	//如果是禁用状态就不响应

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//拖拽结束,nulActKey哪个鼠标按钮按下进行拖拽
ERESULT CEvPingpongButton::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if(npInfo->CurrentPos.x < 0 || npInfo->CurrentPos.y < 0 || npInfo->CurrentPos.x > mpIterator->GetSizeX() || npInfo->CurrentPos.y > mpIterator->GetSizeY())
			PostMessageToParent(EEVT_BUTTON_CLICK,CExMessage::DataInvalid); //算做一次单击,否则父类Button就会发这个消息了，这里就不需要发了

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}
