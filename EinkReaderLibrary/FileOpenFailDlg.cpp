#include "stdafx.h"
#include "FileOpenFailDlg.h"

DEFINE_BUILTIN_NAME(FileOpenFailDlg)

#define FP_ID_OK 1
#define FP_ID_ERROR_TEXT 4

ERESULT CFileOpenFailDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);
	mpIterator->SetTimer(100, 1, 10, NULL);

	result = ERESULT_SUCCESS;

	return result;
}

ULONG CFileOpenFailDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratorOk = mpIterator->GetSubElementByID(FP_ID_OK);
	if (!mpIteratorOk) return result;

	mpIteratorErrorText = mpIterator->GetSubElementByID(FP_ID_ERROR_TEXT);
	if (!mpIteratorErrorText) return result;

	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

void CFileOpenFailDlg::DoModal(bool * npbIsSuccess)
{
	mpbIsSucess = npbIsSuccess;

	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);
	EinkuiGetSystem()->DoModal(mpIterator);

	mpIterator->Close();
}

void CFileOpenFailDlg::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

void CFileOpenFailDlg::SetText(const wstring & text)
{
	m_text = text;
	CExMessage::SendMessageWithText(mpIteratorErrorText, mpIterator, EACT_LABEL_SET_TEXT, m_text.c_str(), nullptr, 0);
}

ERESULT CFileOpenFailDlg::ParseMessage(IEinkuiMessage * npMsg)
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

ERESULT CFileOpenFailDlg::OnCtlButtonClick(IEinkuiIterator * npSender)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	ULONG llBtnID = npSender->GetID();

	switch (llBtnID)
	{
	case FP_ID_OK:
	{
		ExitModal();
		break;
	}
	default:
		break;
	}

	result = ERESULT_SUCCESS;
	return result;
}

ERESULT CFileOpenFailDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
