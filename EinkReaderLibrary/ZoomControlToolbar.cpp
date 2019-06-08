/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ZoomControlToolbar.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(ZoomControlToolbar)

float defaultRatios[] = {1.0f,1.26f,1.90f,2.54f,3.18f,4.78f,6.38f,9.58f,12.78f,19.18f,25.58f,35.18f,41.18f,68.18f,83.18f,115.18f,179.18f,0.0f};




CZoomControlToolbar::CZoomControlToolbar(void)
{
	mpIterBili = NULL;

	mlCurrentZoomLevel = 0;
	mfZoom.Insert(-1, 1.0f);
	//miMaxRatioInx = 0;
	miFatRatioInx = 0;
}


CZoomControlToolbar::~CZoomControlToolbar(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CZoomControlToolbar::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CZoomControlToolbar::InitOnCreate(
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
		mpIterBili = mpIterator->GetSubElementByID(110);
		BREAK_ON_NULL(mpIterBili);
		
		mpIterBtAdd = mpIterator->GetSubElementByID(ZC_BT_ADD);
		BREAK_ON_NULL(mpIterBtAdd);

		mpIterBtSub = mpIterator->GetSubElementByID(ZC_BT_SUB);
		BREAK_ON_NULL(mpIterBtSub);

		mpIterBtDefault = mpIterator->GetSubElementByID(ZC_BT_DEFAULT);
		BREAK_ON_NULL(mpIterBtDefault);

		initData();

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置显示比例
void CZoomControlToolbar::SetString(ULONG nulLevel)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	swprintf_s(lszString, MAX_PATH, L"%d%s", int(mfZoom[nulLevel] * 100),L"%");
	CExMessage::SendMessageWithText(mpIterBili, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);
}

//按钮单击事件
ERESULT CZoomControlToolbar::OnCtlButtonClick(IEinkuiIterator* npSender)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case ZC_BT_DEFAULT:
		{
			//恢复为100%显示
			SendMessageToParent(EEVT_ER_SET_ZOOM, mfZoom[miFatRatioInx],NULL,0);
			initData();

			break;
		}
		case ZC_BT_ADD:
		{
			//放大
			SetLevel(true);

			break;
		}
		case ZC_BT_SUB:
		{
			//缩小
			SetLevel(false);

			break;
		}
		case ZC_BT_CLOSE:
		{
			//退出缩放模式
			PostMessageToParent(EEVT_ER_ENTER_ZOOM, false);
			initData();

			break;
		}
		case ZC_BT_SNAP:
		{
			//截屏
			PostMessageToParent(EEVT_ER_ENTER_SNAPSHOT, false);

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
void CZoomControlToolbar::initData(void)
{
	mlCurrentZoomLevel = miFatRatioInx;
	mpIterBtSub->SetEnable(true);
	mpIterBtAdd->SetEnable(true);
	SetString(mlCurrentZoomLevel);

	//置灰
	mpIterBtDefault->SetEnable(false);
}

void CZoomControlToolbar::SetFatRatio(float fatRatio)
{
	//mfFatRatio = fatRatio;
	bool fatSaved = false;
	mfZoom.Clear();
	for (int i=0;defaultRatios[i]>0.0f;i++)
	{
		/*if (defaultRatios[i] > fatRatio*4.0f)
			break;*/

		if (i > miFatRatioInx + 4)
			break; //撑满屏幕后，再放大4次

		if (fatSaved == false && CExFloat::Equal(defaultRatios[i], fatRatio, 0.1f))	// 相差不大，直接替换
		{
			mfZoom.Insert(-1, fatRatio);
			miFatRatioInx = i;
			fatSaved = true;
		}
		else
			mfZoom.Insert(-1, defaultRatios[i]);

		if (fatSaved == false && defaultRatios[i] < fatRatio && defaultRatios[i + 1] > fatRatio) 
		{ 
			fatSaved = true;

			//判断一下，是离前面的近，还是离后面的近
			if ((fatRatio - mfZoom.GetEntry(i)) > (defaultRatios[i + 1] - fatRatio))
			{
				miFatRatioInx = i + 1;
				mfZoom.Insert(-1, fatRatio);

				i++; //否则会增加一个倍数相近的
			}
			else
			{
				miFatRatioInx = i;
				mfZoom.GetEntry(i) = fatRatio;
			}
		}
		//miMaxRatioInx = i;
	}
	SendMessageToParent(EEVT_ER_SET_ZOOM, mfZoom[miFatRatioInx], NULL, 0);
	initData();
}

//设置放大级别
void CZoomControlToolbar::SetLevel(bool nbIsAdd)
{
	do
	{
		if (nbIsAdd == false)
		{
			//降低
			if (mlCurrentZoomLevel <= 0)
				break; //已经是最低了

			if (--mlCurrentZoomLevel <= 0)
				mpIterBtSub->SetEnable(false);

			mpIterBtAdd->SetEnable(true);
		}
		else
		{
			//增加
			if (mlCurrentZoomLevel >= mfZoom.Size()-1)
				break; //已经是最高了

			if (++mlCurrentZoomLevel >= mfZoom.Size()-1)
				mpIterBtAdd->SetEnable(false);

			mpIterBtSub->SetEnable(true);
		}

		SendMessageToParent(EEVT_ER_SET_ZOOM, mfZoom[mlCurrentZoomLevel],NULL,0);
		SetString(mlCurrentZoomLevel);

		if (mlCurrentZoomLevel == miFatRatioInx)
			mpIterBtDefault->SetEnable(false);// CExMessage::SendMessageWithText(mpIterBtDefault, mpIterator, EACT_BUTTON_CHANGE_PIC, L".\\Pic\\zoom100%_ic_disable.png");
		else
			mpIterBtDefault->SetEnable(true);

	} while (false);
}

//消息处理函数
ERESULT CZoomControlToolbar::ParseMessage(IEinkuiMessage* npMsg)
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
void CZoomControlToolbar::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CZoomControlToolbar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CZoomControlToolbar::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}