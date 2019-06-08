/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmstruct.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvSpinButtonImp.h"


//定义键值
#define SB_KEY_DEFAULT_CTRL						L"DefaultCtrl"
#define SB_KEY_SPINBUTTON						L"SpinButton"
#define SB_KEY_DEFAULT_EDIT						L"DefaultEdit"
#define SB_KEY_DEFAULT_IMAGE_BUTTON_UP			L"DefaultImageButtonUp"
#define SB_KEY_DEFAULT_IMAGE_BUTTON_DOWN		L"DefaultImageButtonDown"
#define SB_KEY_DEFAULT_PICTURE_FRAME			L"DefaultPictureFrame"

#define SB_KEY_EDIT								L"Edit"
#define SB_KEY_ARROW							L"Arrow"
#define SB_KEY_UP_ARROW							L"UpArrow"
#define SB_KEY_DOWN_ARROW						L"DownArrow"
#define SB_KEY_BACK_IMAGE						L"BackImage"
#define SB_KEY_DEFAULT_VALUE					L"DefaultValue"

#define SB_KEY_WIDTH							L"Width"
#define SB_KEY_HEIGHT							L"Height"
#define SB_KEY_X								L"X"
#define SB_KEY_Y								L"Y"
#define SB_KEY_BACKGROUND						L"BackGround"


DEFINE_BUILTIN_NAME(SpinButton)


// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvSpinButton::CEvSpinButton() : 
	mpEdit(NULL),
	mpBtnUpArrow(NULL),
	mpBtnDownArrow(NULL),
	mpBkg(NULL)
{	
	miMinValue = SPINBUTTON_MIN_VALUE;
	miMaxValue = SPINBUTTON_MAX_VALUE;
}

CEvSpinButton::~CEvSpinButton() {}

ULONG CEvSpinButton::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID				// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//创建SpinButton的子控件
		IConfigFile * lpConfigFile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		if (lpConfigFile == NULL) break;
		ICfKey* lpRootKey = lpConfigFile->GetRootKey();
		if (lpRootKey == NULL) break;
		ICfKey* lpDefaultCtrlKey = lpRootKey->GetSubKey(SB_KEY_DEFAULT_CTRL);
		if (lpDefaultCtrlKey == NULL) break;
		ICfKey* lpSpinButtonKey = lpDefaultCtrlKey->GetSubKey(SB_KEY_SPINBUTTON);
		if (lpSpinButtonKey == NULL) break;

		//创建背景图
		ICfKey * lpBkgKey = lpSpinButtonKey->GetSubKey(SB_KEY_DEFAULT_PICTURE_FRAME);
		if(lpBkgKey)
		{
			mpBkg = CEvPictureFrame::CreateInstance(mpIterator, lpBkgKey, SB_ID_CTRL_BACKGROUND);			
		}

		//创建编辑框
		ICfKey * lpEditKey = lpSpinButtonKey->GetSubKey(SB_KEY_DEFAULT_EDIT);
		if(lpEditKey)
		{
			mpEdit = CEvEditImp::CreateInstance(mpIterator, lpEditKey, SB_ID_CTRL_EDIT);			
		}

		
		//创建上箭头
		ICfKey * lpButtonUpKey = lpSpinButtonKey->GetSubKey(SB_KEY_DEFAULT_IMAGE_BUTTON_UP);
		if(lpButtonUpKey)
		{
			mpBtnUpArrow = CEvImageButton::CreateInstance(mpIterator, lpButtonUpKey, SB_ID_CTRL_BUTTON_UP);			
		}

		//创建下箭头
		ICfKey * lpButtonDownKey = lpSpinButtonKey->GetSubKey(SB_KEY_DEFAULT_IMAGE_BUTTON_DOWN);
		if(lpButtonDownKey)
		{
			mpBtnDownArrow = CEvImageButton::CreateInstance(mpIterator, lpButtonDownKey, SB_ID_CTRL_BUTTON_DOWN);			
		}

		//mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL|EITR_STYLE_KEYBOARD);
		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvSpinButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		SetChildCtrlPara();

		//mpIterator->ModifyStyles(EITR_STYLE_POPUP);

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvSpinButton::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		case EEVT_IMAGEBUTTON_CLICK:
			{
				IEinkuiIterator* lpIter = npMsg->GetMessageSender();
				//SetCurrentValueByDisplay();
				int lnPreValue = GetCurrentValue();
				if (lpIter == mpBtnUpArrow->GetIterator())
				{
					++lnPreValue;
				}
				else if (lpIter == mpBtnDownArrow->GetIterator())
				{
					--lnPreValue;
				}

				if(lnPreValue <= miMaxValue && lnPreValue >= miMinValue)
				{
					SetCurrentValue(lnPreValue);

					// 通知父对象（因为主动去设置编辑框的值，不会反馈消息）
					PostMessageToParent(EEVT_SPINBUTTON_CONTENT_COMPLETION, lnPreValue);

				}

				break;
			}
			

		/*case EEVT_EDIT_CONTENT_MODIFIED:
			{
				SetCurrentValueByDisplay();
				break;
			}*/

		case EACT_SPINBUTTON_GET_CURRENT_VALUE:
			{
				int* lpValue = (int*)npMsg->GetOutputBuffer();
				//SetCurrentValueByDisplay();
				*lpValue = GetCurrentValue();

				break;
			}

		case EACT_SPINBUTTON_SET_CURRENT_VALUE:
			{
				const int* lpValue = (const int*)npMsg->GetInputData();
				SetCurrentValue(*lpValue);

				break;
			}

		case EEVT_EDIT_CONTENT_MODIFIED:
			{
				PostMessageToParent(EEVT_SPINBUTTON_CONTENT_MODIFING, CExMessage::DataInvalid);
				break;
			}

		case EEVT_EDIT_CONTENT_COMPLETION:				// 编辑框输入完成
			{
				wchar_t* lswContent = (wchar_t*)npMsg->GetInputData();
				if(NULL != lswContent)
				{
					int liValue = 0;
					if(0 == _wcsicmp(lswContent, L"-"))		// 只有一个符号
					{
						CExMessage::SendMessageWithText(npMsg->GetMessageSender(), mpIterator, EACT_EDIT_SET_TEXT, L"0");
					}
					else
						swscanf_s(lswContent, L"%d", &liValue);

					// 判断是否越界
					if(liValue < miMinValue || liValue > miMaxValue)
					{
						SetCurrentValue(liValue);
					}

					// 通知父对象
					PostMessageToParent(EEVT_SPINBUTTON_CONTENT_COMPLETION, liValue);
				}
				
				break;
			}

		case EACT_SPINBUTTON_SET_MIN_VALUE:			// 设置最小值
			{
				int* lpiMinValue = NULL;
				if(ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lpiMinValue))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				miMinValue = *lpiMinValue;
				// 如果当前值小于最小值，则设置为最小值
				if(mnCurrentValue < miMinValue)
				{
					SetCurrentValue(miMinValue);
				}
			}
			break;

		case EACT_SPINBUTTON_SET_MAX_VALUE:			// 设置最大值
			{
				int* lpiMaxValue = NULL;
				if(ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lpiMaxValue))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				miMaxValue = *lpiMaxValue;
				// 如果当前值大于最大值，则设置为最大值
				if(mnCurrentValue > miMaxValue)
				{
					SetCurrentValue(miMaxValue);
				}
			}
			break;

		default:
			luResult = ERESULT_NOT_SET;
			break;
		}

		if(luResult == ERESULT_NOT_SET)
		{
			luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		}

	} while (false);

	return luResult;
}

//禁用或启用
ERESULT CEvSpinButton::OnElementEnable(bool nbIsEnable)
{
	if (mpEdit) mpEdit->GetIterator()->SetEnable(nbIsEnable);
	if (mpBtnUpArrow) mpBtnUpArrow->GetIterator()->SetEnable(nbIsEnable);
	if (mpBtnDownArrow) mpBtnDownArrow->GetIterator()->SetEnable(nbIsEnable);

	if (nbIsEnable)
	{
		if (mpBkg) mpBkg->GetIterator()->SetAlpha(1.0f);
	}
	else
	{
		if (mpBkg) mpBkg->GetIterator()->SetAlpha(0.5f);
	}

	return ERROR_SUCCESS;
}

BOOL CEvSpinButton::SetChildCtrlPara()
{
	if (mpTemplete == NULL) return FALSE;

	if (mpBkg)
	{
		//设置背景
		IEinkuiIterator* pIter = mpBkg->GetIterator();
		ICfKey* lpKeyBkg = mpTemplete->GetSubKey(SB_KEY_BACK_IMAGE);

		if (lpKeyBkg)
		{
			/*wchar_t lszBkgImage[SPINBUTTON_BUF_SIZE] = {0};
			lpKeyBkg->QuerySubKeyValue(SB_KEY_BACKGROUND, lszBkgImage, SPINBUTTON_BUF_SIZE * sizeof(wchar_t));
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				pIter, EACT_PICTUREFRAME_CHANGE_PIC, lszBkgImage,
				(wcslen(lszBkgImage) + 1) * sizeof(wchar_t), NULL, 0);*/

			ULONG lPosX = lpKeyBkg->QuerySubKeyValueAsLONG(SB_KEY_X);
			ULONG lPosY = lpKeyBkg->QuerySubKeyValueAsLONG(SB_KEY_Y);
			pIter->SetPosition((FLOAT)lPosX, (FLOAT)lPosY);

			ULONG lWidth = lpKeyBkg->QuerySubKeyValueAsLONG(SB_KEY_WIDTH);
			ULONG lHeight = lpKeyBkg->QuerySubKeyValueAsLONG(SB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)lWidth, (FLOAT)lHeight);
		}
	}

	if (mpEdit)
	{
		//设置编辑框
		IEinkuiIterator* pIter = mpEdit->GetIterator();
		ICfKey* lpKeyEdit = mpTemplete->GetSubKey(SB_KEY_EDIT);

		if (lpKeyEdit)
		{
			ULONG lPosX = lpKeyEdit->QuerySubKeyValueAsLONG(SB_KEY_X);
			ULONG lPosY = lpKeyEdit->QuerySubKeyValueAsLONG(SB_KEY_Y);
			pIter->SetPosition((FLOAT)lPosX - 5.0f, (FLOAT)lPosY);

			ULONG lWidth = lpKeyEdit->QuerySubKeyValueAsLONG(SB_KEY_WIDTH);
			ULONG lHeight = lpKeyEdit->QuerySubKeyValueAsLONG(SB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)lWidth, (FLOAT)lHeight);
		}

		//设置只接收数字
		LONG lMode = 1;
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			pIter, EACT_EDIT_NUMBER_ONLY, 
			&lMode, sizeof(LONG), NULL, 0);

		//设置字数限制
		LONG lLimit = 4;
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			pIter, EACT_EDIT_SET_LENGTH_LIMIT, 
			&lLimit, sizeof(LONG), NULL, 0);
	}

	ICfKey* lpKeyArrow = mpTemplete->GetSubKey(SB_KEY_ARROW);
	if (lpKeyArrow == NULL) return FALSE;

	if (mpBtnUpArrow)
	{
		//设置上箭头按钮
		IEinkuiIterator* pIter = mpBtnUpArrow->GetIterator();
		ICfKey* lpKeyUpArrow = lpKeyArrow->GetSubKey(SB_KEY_UP_ARROW);
		if (lpKeyUpArrow)
		{
			ULONG lPosX = lpKeyUpArrow->QuerySubKeyValueAsLONG(SB_KEY_X);
			ULONG lPosY = lpKeyUpArrow->QuerySubKeyValueAsLONG(SB_KEY_Y);
			pIter->SetPosition((FLOAT)lPosX, (FLOAT)lPosY);

			ULONG lWidth = lpKeyUpArrow->QuerySubKeyValueAsLONG(SB_KEY_WIDTH);
			ULONG lHeight = lpKeyUpArrow->QuerySubKeyValueAsLONG(SB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)lWidth, (FLOAT)lHeight);
		}
	}

	if (mpBtnDownArrow)
	{
		//设置下箭头按钮
		IEinkuiIterator* pIter = mpBtnDownArrow->GetIterator();
		ICfKey* lpKeyDownArrow = lpKeyArrow->GetSubKey(SB_KEY_DOWN_ARROW);
		if (lpKeyDownArrow)
		{
			ULONG lPosX = lpKeyDownArrow->QuerySubKeyValueAsLONG(SB_KEY_X);
			ULONG lPosY = lpKeyDownArrow->QuerySubKeyValueAsLONG(SB_KEY_Y);
			pIter->SetPosition((FLOAT)lPosX, (FLOAT)lPosY);

			ULONG lWidth = lpKeyDownArrow->QuerySubKeyValueAsLONG(SB_KEY_WIDTH);
			ULONG lHeight = lpKeyDownArrow->QuerySubKeyValueAsLONG(SB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)lWidth, (FLOAT)lHeight);
		}
	}

	//获取默认值
	int lDefaultValue = 0;
	ICfKey* lpKeyDefaultValue = mpTemplete->GetSubKey(SB_KEY_DEFAULT_VALUE);
	if (lpKeyDefaultValue)	lpKeyDefaultValue->GetValue(&lDefaultValue, sizeof(int));
	SetCurrentValue(lDefaultValue);

	//// 设置按钮响应区域
	//if(mpUpperPicture)
	//{
	//	D2D1_SIZE_F ldfSize;
	//	ldfSize = mpUpperPicture->GetIterator()->GetSize();
	//	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpCurrentItemButton->GetIterator(),
	//		EACT_BUTTON_SET_ACTION_RECT, &ldfSize,  sizeof(D2D1_SIZE_F), NULL, 0);

	//}

	return TRUE;
}

BOOL CEvSpinButton::SetCurrentValue(const int nValue)
{
	BOOL lbResult = FALSE;
	int lnValue = nValue;

	if (nValue > miMaxValue)		lnValue = miMaxValue;
	if (nValue < miMinValue)		lnValue = miMinValue;
	
	mnCurrentValue = lnValue;

	// 如果是最大值，则禁用增加按钮
	if(mnCurrentValue == miMaxValue)
	{
		mpBtnUpArrow->GetIterator()->SetEnable(false);
	}
	else
	{
		if(false == mpBtnUpArrow->GetIterator()->IsEnable())
			mpBtnUpArrow->GetIterator()->SetEnable(true);
	}

	// 如果是最小值，则禁用减小按钮
	if(mnCurrentValue == miMinValue)
	{
		mpBtnDownArrow->GetIterator()->SetEnable(false);
	}
	else
	{
		if(false == mpBtnDownArrow->GetIterator()->IsEnable())
			mpBtnDownArrow->GetIterator()->SetEnable(true);
	}

	return UpdateEditView();
}

BOOL CEvSpinButton::SetCurrentValueByDisplay()
{
	BOOL lbResult = FALSE;

	wchar_t buf[SPINBUTTON_BUF_SIZE] = { 0 };
	
	if (mpEdit)
	{
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpEdit->GetIterator(), EACT_EDIT_GET_TEXT, NULL, 0,
			buf, SPINBUTTON_BUF_SIZE * sizeof(wchar_t));

		mnCurrentValue = _wtoi(buf);

		lbResult = TRUE;
	}


	return lbResult;
}

//更新编辑视图
BOOL CEvSpinButton::UpdateEditView()
{
	BOOL lbResult = FALSE;

	wchar_t buf[SPINBUTTON_BUF_SIZE] = { 0 };
	_itow_s(mnCurrentValue, buf, SPINBUTTON_BUF_SIZE, 10);

	if (mpEdit)
	{
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpEdit->GetIterator(), EACT_EDIT_SET_TEXT, 
			buf, SPINBUTTON_BUF_SIZE * sizeof(wchar_t), NULL, 0);

		lbResult = TRUE;
	}

	return lbResult;
} 

int	CEvSpinButton::GetCurrentValue()
{
	SetCurrentValueByDisplay();
	return mnCurrentValue;
}