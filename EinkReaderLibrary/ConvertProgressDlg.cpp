#include "stdafx.h"
#include "ConvertProgressDlg.h"

#define FP_ID_PROGRESS_ICON 1
#define FP_ID_PROGRESS_TIMER 1

DEFINE_BUILTIN_NAME(ConvertProgressDlg)

ERESULT CConvertProgressDlg::OnElementCreate(IEinkuiIterator * npIterator)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;

	if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
		return result;

	mpIterator->ModifyStyles(EITR_STYLE_POPUP);

	result = ERESULT_SUCCESS;

	return result;
}

ULONG CConvertProgressDlg::InitOnCreate(IN IEinkuiIterator * npParent, IN ICfKey * npTemplete, IN ULONG nuEID)
{
	ERESULT result = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = nullptr;

	//首先调用父类的方法
	result = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
	if (result != ERESULT_SUCCESS)
		return result;

	mpIteratorProgressIcon = mpIterator->GetSubElementByID(FP_ID_PROGRESS_ICON);
	if (!mpIteratorProgressIcon) return result;

	result = ERESULT_SUCCESS;
	CMM_SAFE_RELEASE(lpSubKey);
	return result;
}

tuple<PDFConvert::ConvertResult, wstring> CConvertProgressDlg::DoModal(const wstring& sourceFileName)
{
	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->UpdateView(true);

	m_sourceFileName = sourceFileName;
	m_startTime = std::chrono::system_clock::now();
	PDFConvert::SubmitConvertTask(sourceFileName);

	mpIterator->SetTimer(FP_ID_PROGRESS_TIMER, MAXULONG32, 100, NULL);
	EinkuiGetSystem()->DoModal(mpIterator);

	auto result = std::make_tuple(m_result, m_targetFileName);
	mpIterator->Close();
	return result;
}

void CConvertProgressDlg::ExitModal(PDFConvert::ConvertResult result)
{
	m_result = result;
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

void CConvertProgressDlg::OnTimer(PSTEMS_TIMER npStatus)
{
	if (npStatus->TimerID != FP_ID_PROGRESS_TIMER) return;

	auto calculateIconPosition = [this]()->int{
		int seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - this->m_startTime).count() / 1000;
		return 50 + (seconds % 4) * 150;
	};

	int newPosition = calculateIconPosition();
	if (newPosition != m_currentIconPosition)
	{
		auto pos = mpIteratorProgressIcon->GetPosition();
		pos.x = newPosition;
		mpIteratorProgressIcon->SetPosition(pos);
		m_currentIconPosition = newPosition;
	}

	if (PDFConvert::IsConvertingCompleted())
	{
		m_targetFileName = PDFConvert::GetPDFFileFullPath();
		ExitModal(PDFConvert::GetConvertResult());
	}
}

ERESULT CConvertProgressDlg::ParseMessage(IEinkuiMessage * npMsg)
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

ERESULT CConvertProgressDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	return ERESULT_SUCCESS;
}
