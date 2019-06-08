/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "cmmstruct.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvButtonImp.h"
#include "EvCheckButtonImp.h"
#include "EvRadioButtonGroupImp.h"


//using namespace D2D1;
DEFINE_BUILTIN_NAME(RadioButtonGroup)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvRadioButtonGroup::CEvRadioButtonGroup()
{
	mpCheckedItem = NULL;
}

// 用于释放成员对象
CEvRadioButtonGroup::~CEvRadioButtonGroup()
{

}

ULONG CEvRadioButtonGroup::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//设置自己的类型
		mpIterator->ModifyStyles(/*EITR_STYLE_CONTROL|*/EITR_STYLE_DRAG);



		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvRadioButtonGroup::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//看一下有没有默认选中的，如果没有，就选中第一个
		bool lbChecked = false;
		for (int i=0;i<mpIterator->GetSubElementCount();i++)
		{
			if(CExMessage::SendMessage(mpIterator->GetSubElementByZOder(i),mpIterator,EACT_BUTTON_GET_CHECKED,CExMessage::DataInvalid,&lbChecked,sizeof(bool)) != ERESULT_SUCCESS)
				continue;	//不是CheckButton

			if(lbChecked != false)
			{
				if(mpCheckedItem == NULL)
				{
					mpCheckedItem = mpIterator->GetSubElementByZOder(i);
				}
				else
				{
					lbChecked = false;	//已经有选中的了，就取消后面的选中
					CExMessage::SendMessage(mpIterator->GetSubElementByZOder(i),mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);
				}
			}
		}

		if(mpCheckedItem == NULL)
		{
			lbChecked = true;	//没有选中的，就选中第一个对象
			CExMessage::SendMessage(mpIterator->GetSubElementByZOder(0),mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);
			mpCheckedItem = mpIterator->GetSubElementByZOder(0);
		}

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}


// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvRadioButtonGroup::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
	ULONG lulId = 0;
	IEinkuiIterator* lpItem = NULL;

	switch (npMsg->GetMessageID())
	{
	case EEVT_BUTTON_CLICK:
		{
			//组里有一个按键完成了一次单击
			OnItemClick(npMsg->GetMessageSender());
			luResult = ERESULT_MSG_SENDTO_NEXT;	//让消息继续往上发
			break;
		}
	case EACT_RBG_SET_SELECT:
		{
			//要求更换选中对象
			bool lbChecked = false;
			luResult = CExMessage::GetInputData(npMsg,lulId);
			if(luResult != ERESULT_SUCCESS)
				break;

			//取消选中
			if(mpCheckedItem != NULL)
				CExMessage::SendMessage(mpCheckedItem,mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);

			mpCheckedItem = mpIterator->GetSubElementByID(lulId);
			lbChecked = true;
			if(mpCheckedItem != NULL)	//选中
				CExMessage::SendMessage(mpCheckedItem,mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);

			EinkuiGetSystem()->UpdateView(true);

			luResult = ERESULT_SUCCESS;

			break;
		}
	case EACT_RBG_GET_SELECT:
		{
			//获取选中对象ID
			if(npMsg->GetOutputBufferSize() != sizeof(ULONG))
				break;

			ULONG* lpWarp = (ULONG*)npMsg->GetOutputBuffer();
			
			if (mpCheckedItem != NULL)
			{
				*lpWarp = mpCheckedItem->GetID();
				npMsg->SetOutputDataSize(sizeof(ULONG));
				luResult = ERESULT_SUCCESS;
			}
			else
			{
				luResult = ERESULT_UNSUCCESSFUL;
			}
			

			break;
		}
	case EACT_RBG_DISABLE:
		{
			//设置某项禁用
			luResult = CExMessage::GetInputData(npMsg,lulId);
			if(luResult != ERESULT_SUCCESS)
				break;

			lpItem = mpIterator->GetSubElementByID(lulId);
			BREAK_ON_NULL(lpItem);
			lpItem->SetEnable(false);
			EinkuiGetSystem()->UpdateView(true);
			luResult = ERESULT_SUCCESS;
			
			break;
		}
	case EACT_RBG_ENABLE:
		{
			//设置某项启用
			luResult = CExMessage::GetInputData(npMsg,lulId);
			if(luResult != ERESULT_SUCCESS)
				break;

			lpItem = mpIterator->GetSubElementByID(lulId);
			BREAK_ON_NULL(lpItem);
			lpItem->SetEnable(true);
			EinkuiGetSystem()->UpdateView(true);
			luResult = ERESULT_SUCCESS;

			break;
		}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//有一项完成了一次单击
ERESULT CEvRadioButtonGroup::OnItemClick(IEinkuiIterator* npItem)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	bool lbChecked = false;

	do 
	{
// 		if(mArrayGroup.Size() <= 0)
// 			break;	//没有对象

		

		if(npItem == mpCheckedItem)
		{
			lbChecked = true;
			CExMessage::SendMessage(mpCheckedItem,mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);

			PostMessageToParent(EEVT_RBG_SELECTED_ITEM_CLICK, npItem->GetID());
			break;	//本来就选中了
		}
		//取消选中
		if(mpCheckedItem != NULL)
			CExMessage::SendMessage(mpCheckedItem,mpIterator,EACT_BUTTON_SET_CHECKED,lbChecked);

		//改变选中记录
		mpCheckedItem = npItem;

		PostMessageToParent(EEVT_RBG_SELECTED_CHANGED,mpCheckedItem->GetID());

		luResult = ERESULT_SUCCESS;

	} while (false);

	return luResult;
}

//禁用或启用
ERESULT CEvRadioButtonGroup::OnElementEnable(bool nbIsEnable)
{
	for (int i=0;i<mpIterator->GetSubElementCount();i++)
	{
		mpIterator->GetSubElementByZOder(i)->SetEnable(nbIsEnable);
	}

	return ERESULT_SUCCESS;
}