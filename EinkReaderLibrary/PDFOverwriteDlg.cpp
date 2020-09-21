#include "stdafx.h"
#include "PDFOverwriteDlg.h"

DEFINE_BUILTIN_NAME(PDFOverwriteDlg)

#define FP_ID_OVERWRITE 1
#define FP_ID_CANCEL 2
#define FP_ID_OPEN_EXISTING 6

ERESULT CPDFOverwriteDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);
	mpIterator->SetTimer(100, 1, 10, NULL);

	result = ERESULT_SUCCESS;

	return result;
}

ULONG CPDFOverwriteDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratorOverwrite = mpIterator->GetSubElementByID(FP_ID_OVERWRITE);
	if (!mpIteratorOverwrite) return result;

	mpIteratorCancel = mpIterator->GetSubElementByID(FP_ID_CANCEL);
	if (!mpIteratorCancel) return result;

	mpIteratorOpenExisting = mpIterator->GetSubElementByID(FP_ID_OPEN_EXISTING);
	if (!mpIteratorOpenExisting) return result;

	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

OverwriteDialogResult CPDFOverwriteDlg::DoModal()
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);
	EinkuiGetSystem()->DoModal(mpIterator);

	auto result = m_result;
	mpIterator->Close();
	return result;
}

void CPDFOverwriteDlg::ExitModal(OverwriteDialogResult result)
{
	m_result = result;
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

ERESULT CPDFOverwriteDlg::ParseMessage(IEinkuiMessage * npMsg)
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

ERESULT CPDFOverwriteDlg::OnCtlButtonClick(IEinkuiIterator * npSender)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	ULONG llBtnID = npSender->GetID();

	switch (llBtnID)
	{
	case FP_ID_OVERWRITE:
	{
		ExitModal(OverwriteDialogResult::Overwrite);
		break;
	}
	case FP_ID_CANCEL:
	{
		ExitModal(OverwriteDialogResult::Cancel);
		break;
	}
	case FP_ID_OPEN_EXISTING:
	{
		ExitModal(OverwriteDialogResult::OpenExisting);
		break;
	}
	default:
		break;
	}

	result = ERESULT_SUCCESS;
	return result;
}

ERESULT CPDFOverwriteDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
