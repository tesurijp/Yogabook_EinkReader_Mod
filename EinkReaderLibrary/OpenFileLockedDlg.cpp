#include "stdafx.h"
#include "OpenFileLockedDlg.h"

#define FP_ID_RETRY 3
#define FP_ID_CANCEL 4

DEFINE_BUILTIN_NAME(OpenFileLockedDlg)

ERESULT COpenFileLockedDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);

	result = ERESULT_SUCCESS;

	return result;
}

ULONG COpenFileLockedDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratorRetry = mpIterator->GetSubElementByID(FP_ID_RETRY);
	if (!mpIteratorRetry) return result;

	mpIteratorCancel = mpIterator->GetSubElementByID(FP_ID_RETRY);
	if (!mpIteratorCancel) return result;

	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

bool COpenFileLockedDlg::DoModal()
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);

	EinkuiGetSystem()->DoModal(mpIterator);

	auto result = m_result;
	mpIterator->Close();
	return result;
}

void COpenFileLockedDlg::ExitModal(bool result)
{
	m_result = result;
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

ERESULT COpenFileLockedDlg::ParseMessage(IEinkuiMessage * npMsg)
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

ERESULT COpenFileLockedDlg::OnCtlButtonClick(IEinkuiIterator * npSender)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	ULONG llBtnID = npSender->GetID();

	switch (llBtnID)
	{
	case FP_ID_RETRY:
	{
		ExitModal(true);
		break;
	}
	case FP_ID_CANCEL:
	{
		ExitModal(false);
		break;
	}
	default:
		break;
	}

	result = ERESULT_SUCCESS;
	return result;
}

ERESULT COpenFileLockedDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
