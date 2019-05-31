/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ZoomControlToolbarTxt.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(ZoomControlToolbarTxt)


CZoomControlToolbarTxt::CZoomControlToolbarTxt(void)
{
	mdwFontSizeIndex = 2;
}


CZoomControlToolbarTxt::~CZoomControlToolbarTxt(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CZoomControlToolbarTxt::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CZoomControlToolbarTxt::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄
		
		mpIterBtAdd = mpIterator->GetSubElementByID(ZCT_BT_ADD);
		BREAK_ON_NULL(mpIterBtAdd);

		mpIterBtSub = mpIterator->GetSubElementByID(ZCT_BT_SUB);
		BREAK_ON_NULL(mpIterBtSub);

		initData();

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

void CZoomControlToolbarTxt::SetFontsize(DWORD ldwFontSizeIndex)
{
	mdwFontSizeIndex = ldwFontSizeIndex;
}

//按钮单击事件
ERESULT CZoomControlToolbarTxt::OnCtlButtonClick(IEinkuiIterator* npSender)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case ZCT_BT_ADD:
		{
			//放大
			SetLevel(true);

			break;
		}
		case ZCT_BT_SUB:
		{
			//缩小
			SetLevel(false);

			break;
		}
		
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//初始化自己
void CZoomControlToolbarTxt::initData(void)
{
	//mpIterBtSub->SetEnable(mdwFontSizeIndex <= 0?false:true);

	//mpIterBtAdd->SetEnable(mdwFontSizeIndex >= ZCT_FONTSIZE_LEVEL-1 ? false : true);
}

//设置放大级别
void CZoomControlToolbarTxt::SetLevel(bool nbIsAdd)
{
	do
	{
		if (nbIsAdd == false)
		{
			//降低
			if (mdwFontSizeIndex <= 0)
				break; //已经是最低了

			--mdwFontSizeIndex;
			/*if (--mdwFontSizeIndex <= 0)
				mpIterBtSub->SetEnable(false);*/

			mpIterBtAdd->SetEnable(true);
		}
		else
		{
			//增加
			if (mdwFontSizeIndex >= ZCT_FONTSIZE_LEVEL - 1)
				break; //已经是最高了

			++mdwFontSizeIndex;
			/*if (++mdwFontSizeIndex >= ZCT_FONTSIZE_LEVEL - 1)
				mpIterBtAdd->SetEnable(false);*/

			mpIterBtSub->SetEnable(true);
		}

		PostMessageToParent(EEVT_ER_SET_TXT_ZOOM, mdwFontSizeIndex);

	} while (false);
}

//消息处理函数
ERESULT CZoomControlToolbarTxt::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_MODAL_ENTER:
	{
		//// 创建要弹出的对话框
		//mpIterator->SetVisible(true);
		luResult = ERESULT_SUCCESS;
		break;
	}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if (luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//定时器
void CZoomControlToolbarTxt::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CZoomControlToolbarTxt::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CZoomControlToolbarTxt::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}