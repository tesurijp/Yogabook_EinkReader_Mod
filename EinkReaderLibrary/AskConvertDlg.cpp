#include "stdafx.h"
#include "AskConvertDlg.h"

DEFINE_BUILTIN_NAME(AskConvertDlg)

ERESULT CAskConvertDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);
	mpIterator->SetTimer(100, 1, 10, NULL);

	//CExMessage::SendMessage(mpIteratoraskswitch, mpIterator, EACT_BUTTON_SET_CHECKED, GetAskConvertStatus());

	result = ERESULT_SUCCESS;

	return result;
}

ULONG CAskConvertDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratoraskswitch = mpIterator->GetSubElementByID(PDF_COV_ASKSWITCH);
	if (!mpIteratoraskswitch) return result;


	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

PromptDialogAskResult CAskConvertDlg::DoModal()
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);
	EinkuiGetSystem()->DoModal(mpIterator);

	auto result = m_result;
	mpIterator->Close();
	return result;
}

void CAskConvertDlg::ExitModal(PromptDialogAskResult result)
{
	m_result = result;
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

/*bool CAskConvertDlg::GetAskConvertStatus()
{
	bool result = true;

	HKEY lhKey = NULL;
	DWORD ldwRes = 0;
	DWORD ldwValue = 1;
	DWORD ldwLen = sizeof(DWORD);

	ldwRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
	if (ldwRes == ERROR_SUCCESS)
	{
		RegQueryValueEx(lhKey, L"AskConvertOrNot", NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
	}

	if (ldwValue == 1)
	{
		//1: 不询问
		result = true;
	}
	else
	{
		//0： 询问
		result = false;
	}
	if (lhKey != NULL)
		RegCloseKey(lhKey);

	return result;
}*/

void CAskConvertDlg::NotAskConvert(bool nbIsShow)
{
	HKEY lhKey = NULL;
	DWORD ldwRes = 0;
	DWORD ldwValue = 0;
	if (nbIsShow)
	{
		//true ：不询问转换
		ldwValue = 1;
	}
	else
	{
		//false ：询问转换
		ldwValue = 0;
	}
	ldwRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
	if (ldwRes == ERROR_SUCCESS)
	{
		RegSetValueEx(lhKey, L"AskConvertOrNot", NULL, REG_DWORD, (BYTE*)&ldwValue, sizeof(DWORD));
	}

	if (lhKey != NULL)
		RegCloseKey(lhKey);
}

ERESULT CAskConvertDlg::ParseMessage(IEinkuiMessage * npMsg)
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

ERESULT CAskConvertDlg::OnCtlButtonClick(IEinkuiIterator * npSender)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	ULONG llBtnID = npSender->GetID();

	switch (llBtnID)
	{
	case PDF_COV_CONTINUE:
	{
		NotAskConvert(mbIsAsk);
		ExitModal(PromptDialogAskResult::Yes);
		break;
	}
	case PDF_COV_CANCEL:
	{
		ExitModal(PromptDialogAskResult::No);
		break;
	}
	case PDF_COV_ASKSWITCH:
	{
		CExMessage::SendMessage(npSender, mpIterator, EACT_BUTTON_GET_CHECKED, CExMessage::DataInvalid, &mbIsAsk, sizeof(bool));	
	}
	default:
		break;
	}

	result = ERESULT_SUCCESS;
	return result;
}

ERESULT CAskConvertDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
