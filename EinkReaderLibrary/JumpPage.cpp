/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ElementImp.h"
#include "JumpPage.h"
#include "MsgDefine.h"
#include <stdio.h>
#include <Shlobj.h>
#include <math.h>


DEFINE_BUILTIN_NAME(JumpPage)

CJumpPage::CJumpPage()
{
	mszInputNumber[0] = UNICODE_NULL;
	miInputPage = 0;
}

CJumpPage::~CJumpPage(void)
{

	
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CJumpPage::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);

	

	
		mpIterator->SetTimer(1, 1, 10, NULL);

		//mpIterator->SetPosition((),mpIterator->GetPositionY());
		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

ULONG CJumpPage::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用EUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用父类的方法
		leResult = 	CXuiElement::InitOnCreate(npParent, npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

	

		mpIteratorCurrentPage = mpIterator->GetSubElementByID(4);
		BREAK_ON_NULL(mpIteratorCurrentPage);
		mpIteratorCurrentPage->SetPosition(60.0f, mpIteratorCurrentPage->GetPositionY());

		mpIteratorPageMax = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIteratorPageMax);
		mpIteratorPageMax->SetVisible(false);

		mpIteratorPageMaxText = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIteratorPageMaxText);
		mpIteratorPageMaxText->SetVisible(false);

		mpIteratorInput = mpIterator->GetSubElementByID(5);
		BREAK_ON_NULL(mpIteratorInput);

		mpIteratorBackspace = mpIterator->GetSubElementByID(JP_ID_BT_BACKSPACE);
		BREAK_ON_NULL(mpIteratorBackspace);
		mpIteratorBackspace->SetVisible(false);

		mpIteratorCur = mpIterator->GetSubElementByID(20);
		BREAK_ON_NULL(mpIteratorCur);
		
	
		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	return leResult;
}

//设置当前页码
void CJumpPage::SetCurrentPage(int niPage, int niMaxPage)
{
	if (niPage <= 0)
		niPage = 1;
	if (niMaxPage <= 0)
		niMaxPage = 1;

	miMaxPage = niMaxPage;


	wchar_t lszString[MAX_PATH] = { 0 };
	CExMessage::SendMessage(mpIteratorCurrentPage, mpIterator, EACT_LABEL_GET_TEXT, CExMessage::DataInvalid, &lszString, MAX_PATH);
	swprintf_s(lszString, MAX_PATH, L"%s : %d/%d", lszString, niPage, niMaxPage);
	CExMessage::SendMessageWithText(mpIteratorCurrentPage, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

	//FLOAT lfPosX = mpIterator->GetSizeX()- mpIteratorCurrentPage->GetSizeX() - 60;
	//mpIteratorCurrentPage->SetPosition(lfPosX, mpIteratorCurrentPage->GetPositionY());

	//设置页码范围
	mpIteratorPageMaxText->SetPosition(mpIteratorPageMax->GetPositionX() + mpIteratorPageMax->GetSizeX() + 10.0f, mpIteratorPageMaxText->GetPositionY()-4.0f);
	swprintf_s(lszString, MAX_PATH, L"(1-%d)", niMaxPage);
	CExMessage::SendMessageWithText(mpIteratorPageMaxText, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);
}

void CJumpPage::DoModal()
{
	do 
	{
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);
		

		mpIterator->Close();
		//mpIterator->Release();

	} while (false);
}

void CJumpPage::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator,0);
	
}

//消息处理函数
ERESULT CJumpPage::ParseMessage(IEinkuiMessage* npMsg)
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

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//定时器
void CJumpPage::OnTimer(PSTEMS_TIMER npStatus)
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
}
using namespace std;
//处理输入的数字
void CJumpPage::InputNumber(ULONG nulNumber)
{
	wchar_t lszInput[MAX_PATH];

	do 
	{
		if(wcslen(mszInputNumber) <= 0 && nulNumber == 0)
			break; //第一个数字不能是0

		swprintf_s(lszInput, MAX_PATH, L"%s%d", mszInputNumber, nulNumber);
		int liNumber = _wtoi(lszInput);
		if(liNumber > miMaxPage)
			break; //超过有效范围了

		wcscpy_s(mszInputNumber, MAX_PATH, lszInput);
		CExMessage::SendMessageWithText(mpIteratorInput, mpIterator, EACT_LABEL_SET_TEXT, mszInputNumber, NULL, 0);

		mpIteratorCur->SetPosition(mpIteratorInput->GetPositionX()+ mpIteratorInput->GetSizeX(), mpIteratorCur->GetPositionY());
		miInputPage = liNumber;

	} while (false);
}

void CJumpPage::DeleteNumber()
{
	do 
	{
		if (wcslen(mszInputNumber) <= 0)
		{
			miInputPage = 0;
			break; //没有数字了
		}
			
		*(mszInputNumber + wcslen(mszInputNumber) - 1) = UNICODE_NULL;
		CExMessage::SendMessageWithText(mpIteratorInput, mpIterator, EACT_LABEL_SET_TEXT, mszInputNumber, NULL, 0);
		mpIteratorCur->SetPosition(mpIteratorInput->GetPositionX() + mpIteratorInput->GetSizeX(), mpIteratorCur->GetPositionY());

		int liNumber = _wtoi(mszInputNumber);
		miInputPage = liNumber;

	} while (false);
}

//清空当前输入
void CJumpPage::ClearNumber()
{
	do
	{
		mszInputNumber[0] = UNICODE_NULL;
		CExMessage::SendMessageWithText(mpIteratorInput, mpIterator, EACT_LABEL_SET_TEXT, mszInputNumber, NULL, 0);
		mpIteratorCur->SetPosition(90.0f, mpIteratorCur->GetPositionY());

		miInputPage = 0;

	} while (false);
}

//按钮单击事件
ERESULT CJumpPage::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case JP_ID_BT_CANCAL:
		{
			ExitModal();
			break;
		}
		case JP_ID_BT_BACKSPACE:
		{
			//删除一位数字
			ClearNumber();
			mpIteratorBackspace->SetVisible(false);
			break;
		}
		case JP_ID_BT_OK:
		{
			//跳转执行
			if (miInputPage > 0)
			{
				PostMessageToParent(EEVT_ER_PAGE_JUMP, miInputPage);
			}
			
			ExitModal();

			break;
		}
		case JP_ID_BT_ZERO:
		case JP_ID_BT_ONE:
		case JP_ID_BT_TWO:
		case JP_ID_BT_THREE:
		case JP_ID_BT_FOR:
		case JP_ID_BT_FIVE:
		case JP_ID_BT_SIX:
		case JP_ID_BT_SEVEN:
		case JP_ID_BT_EIGHT:
		case JP_ID_BT_NINE:
		{
			//输入页码，如果是一个是0则忽略
			InputNumber(llBtnID - 10);
			if(miInputPage > 0)
				mpIteratorBackspace->SetVisible(true);

			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//元素参考尺寸发生变化
ERESULT CJumpPage::OnElementResized(D2D1_SIZE_F nNewSize)
{
	/*EI_SIZE ldPaintSize;
	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

	D2D1_POINT_2F ldParentPos = mpIterator->GetParent()->GetPosition();
	FLOAT lfWidth = mpIterator->GetSizeX();
	FLOAT lfX = (ldPaintSize.w - lfWidth) / 2;
	lfX = lfX - ldParentPos.x;
	mpIterator->SetPosition(lfX, mpIterator->GetPositionY());*/

	return ERESULT_SUCCESS;
}

