#include "stdafx.h"
#include "CYesNoPromptDlg.h"

DEFINE_BUILTIN_NAME(YesNoPromptDlg)

#define FP_ID_YES 1
#define FP_ID_NO 2

ERESULT CYesNoPromptDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);
	mpIterator->SetTimer(100, 1, 10, NULL);

	result = ERESULT_SUCCESS;

	return result;
}

ULONG CYesNoPromptDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratorYes = mpIterator->GetSubElementByID(FP_ID_YES);
	if (!mpIteratorYes) return result;

	mpIteratorNo = mpIterator->GetSubElementByID(FP_ID_NO);
	if (!mpIteratorNo) return result;

	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

PromptDialogResult CYesNoPromptDlg::DoModal()
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);
	EinkuiGetSystem()->DoModal(mpIterator);

	PromptDialogResult result = m_result;

	mpIterator->Close();

	return result;
}

void CYesNoPromptDlg::ExitModal(PromptDialogResult result)
{
	m_result = result;
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

ERESULT CYesNoPromptDlg::ParseMessage(IEinkuiMessage * npMsg)
{
	ERESULT result = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_MODAL_ENTER:
	{
		//// 创建要弹出的对话框
		result = ERESULT_SUCCESS;
		break;
	}
	default:
		result = ERESULT_NOT_SET;
		break;
	}

	if(result == ERESULT_NOT_SET)
	{
		result = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return result;
}

ERESULT CYesNoPromptDlg::OnCtlButtonClick(IEinkuiIterator * npSender)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	ULONG llBtnID = npSender->GetID();

	switch (llBtnID)
	{
	case FP_ID_YES:
	{
		ExitModal(PromptDialogResult::Yes);
		break;
	}
	case FP_ID_NO:
	{
		ExitModal(PromptDialogResult::No);
		break;
	}
	default:
		break;
	}

	result = ERESULT_SUCCESS;
	return result;
}

ERESULT CYesNoPromptDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
