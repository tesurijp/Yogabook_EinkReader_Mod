/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmstruct.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvComboBoxImp.h"


//定义键值
#define CB_KEY_DEFAULT_CTRL						L"DefaultCtrl"
#define CB_KEY_COMBOBOX							L"ComboBox"

#define CB_KEY_DEFAULT_EDIT						L"DefaultEdit"
#define CB_KEY_DEFAULT_LIST						L"DefaultList"
#define CB_KEY_DEFAULT_BUTTON					L"DefaultButton"
#define CB_KEY_DEFAULT_DROP_BUTTON				L"DefaultDropButton"
#define CB_KEY_DEFAULT_IMAGE_BUTTON				L"DefaultImageButton"
#define CB_KEY_DEFAULT_PICTURE_FRAME			L"DefaultPictureFrame"
//#define CB_KEY_DEFAULT_UNDER_PICTURE_FRAME		L"DefaultUnderPictureFrame"

#define CB_KEY_UPPER_PICTURE					L"UpperPicture"
//#define CB_KEY_UNDER_PICTURE					L"UnderPicture"
#define CB_KEY_CURRENT_ITEM_BUTTON				L"CurrentItemButton"
#define CB_KEY_CURRENT_ITEM_EDIT				L"CurrentItemEdit"
#define CB_KEY_DROP_DOWN_BUTTON					L"DropDownButton"
//#define CB_KEY_LIST								L"List"

#define CB_KEY_STYLE							L"Style"

#define CB_KEY_X								L"X"
#define CB_KEY_Y								L"Y"
#define CB_KEY_WIDTH							L"Width"
#define CB_KEY_HEIGHT							L"Height"
#define CB_KEY_BACKGROUND						L"BackGround"
#define CB_KEY_LIST_HEIGHT						L"ListHeight"
//#define CB_KEY_DEFAULT_VALUE_LIST				L"DefaultValueList"
#define CB_KEY_DEFAULT_CURRENT_VALUE			L"DefaultCurrentValue"

#define CB_KEY_CHILDREN							L"Children"
#define CB_KEY_MENUITEM							L"MenuItem"
#define CB_KEY_MENUITEM_ID						L"ID"
#define CB_KEY_MENUITEM_TEXT					L"Text"

//输入框相关
#define CB_KEY_ONLY_ACCEPT_NUM					L"OnlyAcceptNum"
#define CB_KEY_MAX_INPUT_NUM					L"MaxInputNum"
#define CB_KEY_MIN_INPUT_NUM					L"MinInputNum"
#define CB_KEY_MAX_INPUT_LEN					L"MaxInputLen"

//控件风格
#define CB_STYLE_EDITABLE						L"0"
#define CB_STYLE_UNEDITABLE						L"1"


//定义子控件
#define	CB_ID_POPMENU							10		//弹出菜单

//消息相关
#define CB_KEY_MESSAGE_INFO							L"MessageInfo"
#define CB_KEY_MESSAGE_INFO_ID						L"ID"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE			L"MessageType"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_NONE		L"0"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_BOOL		L"1"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_INT		L"2"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_FLOAT		L"3"
#define CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_STRING		L"4"


DEFINE_BUILTIN_NAME(ComboBox)


// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvComboBox::CEvComboBox() : 
			mpUpperPicture(NULL),
			mpCurrentItemEdit(NULL),
			mpCurrentItemButton(NULL),
			mpDropDownButton(NULL),
			//mpCurItem(NULL),
			mpIterPopMenu(NULL),
			mpComboBoxKey(NULL),

			mnStyle(-1),

			mbOnlyAcceptNum(false),
			mnMaxInputNum(0),
			mnMinInputNum(0)
{	
	//mfMaxWidth = 0.0f;
}

CEvComboBox::~CEvComboBox()
{
	CMM_SAFE_RELEASE(mpComboBoxKey);
}

ULONG CEvComboBox::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID				// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpRootKey = NULL;
	ICfKey* lpDefaultCtrlKey = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//创建ComboBox的子控件
		IConfigFile * lpConfigFile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		if (lpConfigFile == NULL) break;
		lpRootKey = lpConfigFile->GetRootKey();
		if (lpRootKey == NULL) break;
		lpDefaultCtrlKey = lpRootKey->GetSubKey(CB_KEY_DEFAULT_CTRL);
		if (lpDefaultCtrlKey == NULL) break;
		mpComboBoxKey = lpDefaultCtrlKey->GetSubKey(CB_KEY_COMBOBOX);
		if (mpComboBoxKey == NULL) break;

		//创建编辑区域的背景（上背景）
		ICfKey * lpUpperPictureKey = mpComboBoxKey->GetSubKey(CB_KEY_DEFAULT_PICTURE_FRAME);
		if(lpUpperPictureKey)
		{
			mpUpperPicture = CEvPictureFrame::CreateInstance(mpIterator, lpUpperPictureKey, COMBO_ID_CTRL_UPPER_PICTURE);			
		}

		//创建组合框中当前项（编辑模式）
		ICfKey * lpCurrentItemEditKey = mpComboBoxKey->GetSubKey(CB_KEY_DEFAULT_EDIT);
		if(lpCurrentItemEditKey)
		{
			mpCurrentItemEdit = CEvEditImp::CreateInstance(mpIterator, lpCurrentItemEditKey, COMBO_ID_CTRL_CURRENT_ITEM_EDIT);			
		}

		//创建组合框中当前项（非编辑模式）
		ICfKey * lpCurrentItemButtonKey = mpComboBoxKey->GetSubKey(CB_KEY_DEFAULT_DROP_BUTTON);
		if(lpCurrentItemButtonKey)
		{
			mpCurrentItemButton = CEvButton::CreateInstance(mpIterator, lpCurrentItemButtonKey, COMBO_ID_CTRL_CURRENT_ITEM_BUTTON);	
		}

		//创建组合框中的下拉按钮
		ICfKey * lpDropDownButtonKey = mpComboBoxKey->GetSubKey(CB_KEY_DEFAULT_IMAGE_BUTTON);
		if(lpDropDownButtonKey)
		{
			mpDropDownButton = CEvImageButton::CreateInstance(mpIterator, lpDropDownButtonKey, COMBO_ID_CTRL_DROP_DOWN_BUTTON);			
		}

		//配置文件读取消息属性
		ICfKey* lpMessageInfoKey = mpTemplete->GetSubKey(CB_KEY_MESSAGE_INFO);
		if (lpMessageInfoKey)
		{
			unsigned int		lnID = 0;
			wchar_t lpMessageType[BUF_SIZE] = {0};

			lpMessageInfoKey->QuerySubKeyValue(CB_KEY_MESSAGE_INFO_ID, &lnID, sizeof(unsigned int));
			lpMessageInfoKey->QuerySubKeyValue(CB_KEY_MESSAGE_INFO_MESSAGE_TYPE, lpMessageType, BUF_SIZE * sizeof(wchar_t));

			if (lnID != 0)
			{
				mMsgInfo.mnCtrlID = lnID;
			}

			if (wcscmp(lpMessageType, CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_NONE) == 0)
			{
				mMsgInfo.mnMsgParaType	= COMBOBOX_TMPT_NONE;
				mMsgInfo.mpMsgBuf		= NULL;
			}
			else if (wcscmp(lpMessageType, CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_BOOL) == 0)
			{
				mMsgInfo.mnMsgParaType = COMBOBOX_TMPT_BOOL;
			}
			else if (wcscmp(lpMessageType, CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_INT) == 0)
			{
				mMsgInfo.mnMsgParaType = COMBOBOX_TMPT_INT;
			}
			else if (wcscmp(lpMessageType, CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_FLOAT) == 0)
			{
				mMsgInfo.mnMsgParaType = COMBOBOX_TMPT_FLOAT;
			}
			else if (wcscmp(lpMessageType, CB_KEY_MESSAGE_INFO_MESSAGE_TYPE_STRING) == 0)
			{
				mMsgInfo.mnMsgParaType = COMBOBOX_TMPT_STRING;
			}
			else
			{
				mMsgInfo.mnMsgParaType = COMBOBOX_TMPT_OTHERS;
			}
		}

		////	初始化UnificSetting
		//mpUnificSetting = GetUnificSetting();

		
		//创建List
		ICfKey * lpListKey = mpComboBoxKey->GetSubKey(CB_KEY_DEFAULT_LIST);
		if(lpListKey != NULL)
		{
			CEvList* lpIterList = CEvList::CreateInstance(mpIterator, lpListKey,0);		
			BREAK_ON_NULL(lpIterList);
			mpIterList = lpIterList->GetIterator();
		}

		//mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL|EITR_STYLE_KEYBOARD);
		leResult = ERESULT_SUCCESS;

	} while (false);


	CMM_SAFE_RELEASE(lpRootKey);
	CMM_SAFE_RELEASE(lpDefaultCtrlKey);
	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvComboBox::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		SetChildCtrlPara();

		SetDefaultValueList();

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);

		mpIterPopMenu = mpIterator->GetSubElementByID(CB_ID_POPMENU);
		if (mpIterPopMenu != NULL)
		{
			//mpIterPopMenu->SetPosition(mpIterPopMenu->GetPositionX() - 5.0f, mpIterPopMenu->GetPositionY());
			
			if (mpIterList != NULL)
			{
				mpIterList->SetPosition(-5.0f,mpIterPopMenu->GetPositionY());
				//mpIterPopMenu->SetPosition(0.0f, 0.0f);
				mpIterList->SetSize(mpIterPopMenu->GetSizeX(),(float)mpTemplete->QuerySubKeyValueAsLONG(CB_KEY_LIST_HEIGHT,(LONG)mpIterPopMenu->GetSizeY()));
				EinkuiGetSystem()->GetElementManager()->SetParentElement(mpIterList,mpIterPopMenu);	//把菜单放到List里
				CExMessage::SendMessage(mpIterList,mpIterator,EACT_LIST_ADD_ELEMENT,mpIterPopMenu);
				mpIterList->SetVisible(false);
			}
			//mpIterPopMenu->SetVisible(false);
		}
		if (NULL != mpIterPopMenu)
		{
			bool lbEnable = false;
			CExMessage::SendMessageW(mpIterPopMenu, mpIterator,
				EACT_POPUPMENU_IS_MANAGER_MENUITEM_ENABLE,
				lbEnable);
		}

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

ERESULT CEvComboBox::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ULONG luCtrlId = npSender->GetID();

	ERESULT luResult = ERESULT_SUCCESS;

	switch (luCtrlId)
	{
	case COMBO_ID_CTRL_CURRENT_ITEM_BUTTON:
	//case COMBO_ID_CTRL_DROP_DOWN_BUTTON:
		{
// 			if (mpIterPopMenu)
// 			{
// 				mpIterPopMenu->SetVisible(mpIterPopMenu->IsVisible() ? false : true);
// 			}
			if(mpIterList != NULL && mpIterPopMenu != NULL)
			{
				mpIterList->SetVisible(mpIterList->IsVisible()?false:true);
				mpIterPopMenu->SetVisible(mpIterList->IsVisible());

			}

			break;
		}
	default:
		{
			////列表项被点击
			//if (luCtrlId >= COMBO_ID_CTRL_OTHERS)
			//{
			//	wchar_t* lswBuf = NULL;
			//	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(npSender,
			//		EACT_BUTTON_GETTEXT, NULL, 0, &lswBuf, sizeof(wchar_t*));
			//	mpCurItem = lswBuf;

			//	if (mnStyle == 0)
			//	{
			//		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			//			mpCurrentItemEdit->GetIterator(),
			//			EACT_EDIT_SET_TEXT,
			//			(void*)lswBuf,
			//			(wcslen(lswBuf) + 1) * sizeof(wchar_t), NULL, 0);
			//	}
			//	else if (mnStyle == 1)
			//	{
			//		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			//			mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON),
			//			EACT_BUTTON_SETTEXT,
			//			(void*)lswBuf,
			//			(wcslen(lswBuf) + 1) * sizeof(wchar_t), NULL, 0);
			//	}
			//}
		}
	}

	return luResult;
}

//禁用或启用
ERESULT CEvComboBox::OnElementEnable(bool nbIsEnable)
{
	SetComboBoxEnable(nbIsEnable);

	return ERROR_SUCCESS;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvComboBox::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		case EEVT_IMAGEBUTTON_CLICK:
// 			if (mpIterPopMenu)
// 			{
// 				mpIterPopMenu->SetVisible(mpIterPopMenu->IsVisible() ? false : true);
// 			}
			if(mpIterList != NULL && mpIterPopMenu != NULL)
			{
				mpIterList->SetVisible(mpIterList->IsVisible()?false:true);
				mpIterPopMenu->SetVisible(mpIterList->IsVisible());

			}
			break;

		case EMSG_ELEMENT_ACTIVATED:
			{
				//激活状态改变
				STEMS_ELEMENT_ACTIVATION* lpActive;
				luResult = CExMessage::GetInputDataBuffer(npMsg,lpActive);
				if(luResult != ERESULT_SUCCESS)
					break;

// 				if (lpActive->State == 0 && mpIterPopMenu->IsVisible() != false)			// 可见状态下失去激活状态，需要隐藏当前展开的菜单
// 				{
// 					bool lbShow = false; 
// 					mpIterPopMenu->SetVisible(lbShow);
// 				}

				if (lpActive->State == 0 && mpIterList->IsVisible() != false)			// 可见状态下失去激活状态，需要隐藏当前展开的菜单
				{
					bool lbShow = false; 
					mpIterList->SetVisible(lbShow);
				}
			}
			break;

		case EEVT_MENUITEM_CLICK:			//扩展控件的菜单项被点击
			{	
				//读取ID
				if(npMsg->GetInputDataSize()!=sizeof(int) || npMsg->GetInputData()==NULL)
					luResult = ERESULT_WRONG_PARAMETERS;
				int lItemID = *(int*)(npMsg->GetInputData());
				PostMessageToParent(EEVT_COMBOBOX_LIST_ITEM_CLICK, lItemID);

				if (mpVecComboMenuItem.Size() != 0)
				{
					int i = 0;
					for (i = 0; i <mpVecComboMenuItem.Size(); i++)
					{
						if (mpVecComboMenuItem[i].mnID == lItemID)
						{
							PostMessageToParent(EEVT_COMBOBOX_ITEM_CLICK_WITH_TEXT, mpVecComboMenuItem[i].mpText);		// 通知父对象 add by colin
							PostMessageToParent(EEVT_COMBOBOX_ITEM_CLICK_WITH_INDEX, i);

							if (mnStyle == 0)
							{
								EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
									mpCurrentItemEdit->GetIterator(),
									EACT_EDIT_SET_TEXT,
									(void*)mpVecComboMenuItem[i].mpText,
									BUF_SIZE * sizeof(wchar_t), NULL, 0);
							}
							else if (mnStyle == 1)
							{
								EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
									mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON),
									EACT_BUTTON_SETTEXT,
									(void*)mpVecComboMenuItem[i].mpText,
									BUF_SIZE * sizeof(wchar_t), NULL, 0);

								
							}
							break;
						}
					}

					if(mpIterList != NULL && mpIterPopMenu != NULL)
					{
						mpIterList->SetVisible(false);
						mpIterPopMenu->SetVisible(false);
					}

					mToolbarMsgInfo.mnCtrlID = mMsgInfo.mnCtrlID;
					mToolbarMsgInfo.mnMsgParaType = (TOOLBAR_MSG_PARA_TYPE)mMsgInfo.mnMsgParaType;

					if(mMsgInfo.mnMsgParaType == COMBOBOX_TMPT_INT)
					{
						int lnVal = _wtoi(mpVecComboMenuItem[i].mpText);
						mToolbarMsgInfo.mlInterge = lnVal;
					}
					else if(mMsgInfo.mnMsgParaType == COMBOBOX_TMPT_FLOAT)
					{
						FLOAT lfVal = (FLOAT)_wtof(mpVecComboMenuItem[i].mpText);
						mToolbarMsgInfo.mfFloat = lfVal;
					}
					else if (mMsgInfo.mnMsgParaType == COMBOBOX_TMPT_STRING)
					{
						memset(mToolbarMsgInfo.mswString, 0, MAX_PATH);

						wcscpy_s(mToolbarMsgInfo.mswString, MAX_PATH, mpVecComboMenuItem[i].mpText);
						//mToolbarMsgInfo.mswString = mpVecComboMenuItem[i].mpText;
					}

					CExMessage::PostMessage(mpIterator->GetParent()->GetParent(),
						mpIterator, EEVT_TOOLBARITEM_CLICK, mToolbarMsgInfo, EMSG_POSTTYPE_FAST);
				}
			}
			break;
		case EACT_COMBOBOX_ADD_ITEM:
			{
				//if(npMsg->GetInputDataSize() != sizeof(wchar_t*))
				//{
				//	luResult = ERESULT_WRONG_PARAMETERS;
				//	break;
				//}
				//wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();
				//wchar_t* lpValue = *(wchar_t**)npMsg->GetInputData();

				InsertItem((wchar_t*)npMsg->GetInputData());

				break;
			}

		case EACT_COMBOBOX_DELETE_ITEM_BY_INDEX:		// 删除项 add by colin
			{
				if(npMsg->GetInputDataSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				int* lpiInex = (int*)npMsg->GetInputData();
				BREAK_ON_FALSE(DeleteItem(*lpiInex));

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_COMBOBOX_GET_CURRENT_ITEM_TEXT:			// 获取当前选中项的文本	add by colin
			{
				// 获取当前选中项文本内容
				IEinkuiIterator* lpoCrtItemIter = NULL;
				wchar_t lswBuffer[BUF_SIZE] = {0};
				if (mnStyle == 0)
				{
					lpoCrtItemIter = mpCurrentItemEdit->GetIterator();
					BREAK_ON_NULL(lpoCrtItemIter);
					luResult = CExMessage::SendMessage(lpoCrtItemIter,mpIterPopMenu,EACT_EDIT_GET_TEXT, NULL, npMsg->GetOutputBuffer(), npMsg->GetOutputBufferSize());
				}
				else if (mnStyle == 1)
				{
					lpoCrtItemIter = mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON);
					BREAK_ON_NULL(lpoCrtItemIter);
					luResult = CExMessage::SendMessage(lpoCrtItemIter,mpIterator,EACT_BUTTON_GETTEXT, NULL, npMsg->GetOutputBuffer(), npMsg->GetOutputBufferSize());
				}

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_COMBOBOX_SET_ITEM_SELECTED_BY_INDEX:		// 根据索引设置选项 add by colin
			{
				// 获取选中项索引
				ULONG* lpulItemIndex = NULL;
				if(ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg,lpulItemIndex))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				BREAK_ON_FALSE(lpulItemIndex);

				if((ULONG)mpVecComboMenuItem.Size() > (*lpulItemIndex))
				{
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_MENUITEM_CLICK, mpVecComboMenuItem[*lpulItemIndex].mnID);
				}

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EEVT_PANE_ITEM_SET_VALUE:
			{
				SetValue();
			}
			break;

		case EEVT_GET_UNIFIC_SETTING_ID:
			{
				if(npMsg->GetOutputBufferSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				int* lpValue = (int*)npMsg->GetOutputBuffer();
				
				*lpValue = mMsgInfo.mnCtrlID;

				//IUnificSetting* pSetting = GetUnificSetting();
				//if (NULL == pSetting) break;
				bool lbEnable = true;//pSetting->GetItemEnable(mMsgInfo.mnCtrlID);
				SetComboBoxEnable(lbEnable);
			}
			break;

		case EACT_COMBOBOX_SET_ITEM_SELECTED:
			{
				if(npMsg->GetInputDataSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				int* lpValue = (int*)npMsg->GetInputData();

				SetItemSelected(*lpValue);

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EEVT_EDIT_CONTENT_COMPLETION:
			{
				wchar_t* lswContent = (wchar_t*)npMsg->GetInputData();
				int lnNum = _wtoi(lswContent);
				if (lnNum < mnMinInputNum) 
				{
					lnNum = mnMinInputNum;
					wchar_t lwBuf[BUF_SIZE] = {0};
					_itow_s(lnNum, lwBuf, 10);
					CExMessage::SendMessageWithText(mpCurrentItemEdit->GetIterator(), 
						mpIterator, EACT_EDIT_SET_TEXT, lwBuf, NULL, 0);
				}

				if (lnNum > mnMaxInputNum) 
				{
					lnNum = mnMaxInputNum;
					wchar_t lwBuf[BUF_SIZE] = {0};
					_itow_s(lnNum, lwBuf, 10);
					CExMessage::SendMessageWithText(mpCurrentItemEdit->GetIterator(), 
						mpIterator, EACT_EDIT_SET_TEXT, lwBuf, NULL, 0);
				}
				//wchar_t lwBuf[BUF_SIZE] = {0};
				//_itow_s(lnNum, lwBuf, 10);

				mToolbarMsgInfo.mnCtrlID = mMsgInfo.mnCtrlID;
				mToolbarMsgInfo.mnMsgParaType = (TOOLBAR_MSG_PARA_TYPE)mMsgInfo.mnMsgParaType;
				//memset(mToolbarMsgInfo.mswString, 0, MAX_PATH);
				//wcscpy_s(mToolbarMsgInfo.mswString, MAX_PATH, lwBuf);
				mToolbarMsgInfo.mlInterge = lnNum;

				CExMessage::PostMessage(mpIterator->GetParent()->GetParent(),
					mpIterator, EEVT_TOOLBARITEM_CLICK, mToolbarMsgInfo, EMSG_POSTTYPE_FAST);
			}
			break;

		case EACT_COMBOBOX_SET_ENABLE:
			{
				if(npMsg->GetInputDataSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				int* lpID = (int*)npMsg->GetInputData();

				SetItemEnable(*lpID);
			}
			break;

		case EACT_COMBOBOX_SET_DISABLE:
			{
				if(npMsg->GetInputDataSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				int* lpID = (int*)npMsg->GetInputData();

				SetItemDisable(*lpID);
			}
			break;

		case EEVT_EDIT_KEYBOARD_FOCUS:
			{
				if(npMsg->GetInputDataSize() != sizeof(LONG))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				LONG* lpFocus = (LONG*)npMsg->GetInputData();
				if ( (*lpFocus != 0) && (mpCurrentItemEdit != NULL) )
				{
					_STCTL_EDIT_SELECTION sel;
					sel.SelBegin = 0;
					sel.SelCount = -1;
					CExMessage::PostMessageW(mpCurrentItemEdit->GetIterator(),
						mpIterator, EACT_EDIT_SET_SELECTION, sel);
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

//插入新的一项
BOOL CEvComboBox::InsertItem(wchar_t* lswName)
{
	BOOL lbRet = FALSE;

	do 
	{
		BREAK_ON_NULL(lswName);

		_STCTL_POPUPMENU_MENUITEMINFO lsPopItem = {0};
		lsPopItem.Type = 1;
		lsPopItem.Index = -1;
		lsPopItem.UniqueMenuItemId = mpVecComboMenuItem.Size()+1;
		lsPopItem.MenuItemId = mpVecComboMenuItem.Size()+1;
		wcscpy_s(lsPopItem.MenuText,lswName);

		if(CExMessage::SendMessage(mpIterPopMenu,mpIterator,EACT_POPUPMENU_INSERT_MENUITEM_BY_CREATE,lsPopItem) != ERESULT_SUCCESS)
			break;

		//重新设置List的大小
		mpIterList->SetSize(mpIterPopMenu->GetSizeX(),(float)mpTemplete->QuerySubKeyValueAsLONG(CB_KEY_LIST_HEIGHT,(LONG)mpIterPopMenu->GetSizeY()));
		//mpIterPopMenu->SetPosition(0.0f,0.0f);
		//CExMessage::SendMessage(mpIterList,mpIterator,EACT_LIST_RECACULATE,mpIterPopMenu);

		ComboMenuItem ldMenuItem;
		ldMenuItem.mnID = lsPopItem.MenuItemId;
		wcscpy_s(ldMenuItem.mpText,BUF_SIZE,lswName);
		mpVecComboMenuItem.Insert(-1, ldMenuItem);

		//// 在非编辑模式下，如果当前只有刚插入的一项，则要设置该项为默认选中的项  add by colin
		//if(mnStyle == 1 && mpVecComboMenuItem.Size() == 1)
		//{
		//	CExMessage::PostMessage(mpIterator, mpIterator, EEVT_MENUITEM_CLICK, ldMenuItem.mnID);
		//}

		lbRet = TRUE;

	} while (FALSE);
	
	return lbRet;
}

// 删除指定项，小于0表示删除全部。 ————Add by colin
bool CEvComboBox::DeleteItem(IN int niIndexToDel)
{
	bool lbResult = false;
	do 
	{
		if(mpVecComboMenuItem.Size() > 0)
		{
			if(niIndexToDel >= mpVecComboMenuItem.Size())
				break;

			// 删除项
			CExMessage::SendMessage(mpIterPopMenu, mpIterPopMenu, EACT_POPUPMENU_DELETE_MENUITEM_BY_INDEX, niIndexToDel);

			// 获取当前选中项文本内容
			IEinkuiIterator* lpoCrtItemIter = NULL;
			wchar_t lswBuffer[BUF_SIZE] = {0};
			if (mnStyle == 0)
			{
				lpoCrtItemIter = mpCurrentItemEdit->GetIterator();
				BREAK_ON_NULL(lpoCrtItemIter);
				if(ERESULT_SUCCESS != CExMessage::SendMessage(lpoCrtItemIter,mpIterPopMenu,EACT_EDIT_GET_TEXT, NULL, (void*)&lswBuffer, sizeof(wchar_t) * BUF_SIZE))
					break;
			}
			else if (mnStyle == 1)
			{
				lpoCrtItemIter = mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON);
				BREAK_ON_NULL(lpoCrtItemIter);
				CExMessage::SendMessage(lpoCrtItemIter,mpIterator,EACT_BUTTON_GETTEXT, NULL, (void*)&lswBuffer,BUF_SIZE * sizeof(wchar_t));
			}
			else
				break;

			// 删除记录项
			if(niIndexToDel < 0)		// 删除全部
			{
				wchar_t lswEmpty[] = L"";
				if(mnStyle == 0)
					CExMessage::SendMessageWithText(lpoCrtItemIter,mpIterator,EACT_EDIT_SET_TEXT, lswEmpty);
				else if(mnStyle == 1)
					CExMessage::SendMessageWithText(lpoCrtItemIter,mpIterator,EACT_BUTTON_SETTEXT, lswEmpty);

				mpVecComboMenuItem.Clear();

				// 将PopmMenu设置成刚好没有List的大小
				mpIterPopMenu->SetSize(mpIterPopMenu->GetSizeX(), mpIterList->GetSizeY());
			}
			else		// 删除某项
			{
				// 如果要删除的项的内容和当前选中项的内容相同，则切换一项
				if(_wcsicmp(lswBuffer, mpVecComboMenuItem[niIndexToDel].mpText) == 0)
				{
					// 如果不是最后一项，则使用下一项，否则使用第一项
					int liNextItemIndex = (niIndexToDel + 1) < mpVecComboMenuItem.Size()? (niIndexToDel + 1) : 0;
					if(mnStyle == 0)
						CExMessage::SendMessageWithText(lpoCrtItemIter,mpIterator,EACT_EDIT_SET_TEXT, mpVecComboMenuItem[liNextItemIndex].mpText);
					else if(mnStyle == 1)
						CExMessage::SendMessageWithText(lpoCrtItemIter,mpIterator,EACT_BUTTON_SETTEXT, NULL, mpVecComboMenuItem[liNextItemIndex].mpText);
					else
						break;
				}

				mpVecComboMenuItem.RemoveByIndex(niIndexToDel);
			}


			// 重置List的大小
			CExMessage::SendMessage(mpIterList,mpIterator,EACT_LIST_RECACULATE,CExMessage::DataInvalid);

		}
		

		lbResult = true;
	} while (false);

	return lbResult;
}

BOOL CEvComboBox::SetChildCtrlPara()
{
	if (mpTemplete == NULL) return FALSE;

	if (mpUpperPicture)
	{
		//设置上背景
		ICfKey* pKeyUpperPicture = mpTemplete->GetSubKey(CB_KEY_UPPER_PICTURE);
		if(NULL != pKeyUpperPicture)		// 键值存在时再更改，否则仍然使用默认的  ——add by colin
		{
			IEinkuiIterator* pIter = mpUpperPicture->GetIterator();
			wchar_t lpUpperPicture[BUF_SIZE] = {0};
			pKeyUpperPicture->QuerySubKeyValue(CB_KEY_BACKGROUND, lpUpperPicture, BUF_SIZE * sizeof(wchar_t));
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				pIter, EACT_PICTUREFRAME_CHANGE_PIC, lpUpperPicture,
				(wcslen(lpUpperPicture) + 1) * sizeof(wchar_t), NULL, 0);

			ULONG lPosX = pKeyUpperPicture->QuerySubKeyValueAsLONG(CB_KEY_X);
			ULONG lPosY = pKeyUpperPicture->QuerySubKeyValueAsLONG(CB_KEY_Y);
			pIter->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lWidth = pKeyUpperPicture->QuerySubKeyValueAsLONG(CB_KEY_WIDTH);
			ULONG lHeight = pKeyUpperPicture->QuerySubKeyValueAsLONG(CB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));
		}
	}

	if (mpCurrentItemEdit)
	{
		//设置组合框中当前项（编辑模式）
		ICfKey* pKeyCurrentItemEdit = mpTemplete->GetSubKey(CB_KEY_CURRENT_ITEM_EDIT);
		if(NULL != pKeyCurrentItemEdit)		// 键值存在时再更改，否则仍然使用默认的  ——add by colin
		{
			IEinkuiIterator* pIter = mpCurrentItemEdit->GetIterator();
			ULONG lPosX = pKeyCurrentItemEdit->QuerySubKeyValueAsLONG(CB_KEY_X);
			ULONG lPosY = pKeyCurrentItemEdit->QuerySubKeyValueAsLONG(CB_KEY_Y);
			pIter->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lWidth = pKeyCurrentItemEdit->QuerySubKeyValueAsLONG(CB_KEY_WIDTH);
			ULONG lHeight = pKeyCurrentItemEdit->QuerySubKeyValueAsLONG(CB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));
		}
	}

	if (mpCurrentItemButton)
	{
		//设置组合框中当前项（非编辑模式）
		ICfKey* pKeyCurrentItemButton = mpTemplete->GetSubKey(CB_KEY_CURRENT_ITEM_BUTTON);
		if(NULL != pKeyCurrentItemButton)	// 键值存在时再更改，否则仍然使用默认的  ——add by colin
		{
			IEinkuiIterator* pIter = mpCurrentItemButton->GetIterator();

			ULONG lPosX = pKeyCurrentItemButton->QuerySubKeyValueAsLONG(CB_KEY_X);
			ULONG lPosY = pKeyCurrentItemButton->QuerySubKeyValueAsLONG(CB_KEY_Y);
			pIter->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lWidth = pKeyCurrentItemButton->QuerySubKeyValueAsLONG(CB_KEY_WIDTH);
			ULONG lHeight = pKeyCurrentItemButton->QuerySubKeyValueAsLONG(CB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));

			D2D1_RECT_F ldfRect;
			ldfRect.left = 0.0f;
			ldfRect.top = 0.0f;
			ldfRect.right = (FLOAT)(lWidth);
			ldfRect.bottom = (FLOAT)(lHeight);

			pIter->SetVisibleRegion(ldfRect);
		}


	}
	
	if (mpDropDownButton)
	{
		//设置组合框中的下拉按钮
		ICfKey* pKeyDropDownButton = mpTemplete->GetSubKey(CB_KEY_DROP_DOWN_BUTTON);
		if(NULL != pKeyDropDownButton)	// 键值存在时再更改，否则仍然使用默认的  ——add by colin
		{
			IEinkuiIterator* pIter = mpDropDownButton->GetIterator();

			ULONG lPosX = pKeyDropDownButton->QuerySubKeyValueAsLONG(CB_KEY_X);
			ULONG lPosY = pKeyDropDownButton->QuerySubKeyValueAsLONG(CB_KEY_Y);
			pIter->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lWidth = pKeyDropDownButton->QuerySubKeyValueAsLONG(CB_KEY_WIDTH);
			ULONG lHeight = pKeyDropDownButton->QuerySubKeyValueAsLONG(CB_KEY_HEIGHT);
			pIter->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));
		}
	}

	// 设置按钮响应区域
	if(mpUpperPicture)
	{
		D2D1_SIZE_F ldfSize;
		ldfSize = mpUpperPicture->GetIterator()->GetSize();
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpCurrentItemButton->GetIterator(),
			EACT_BUTTON_SET_ACTION_RECT, &ldfSize,  sizeof(D2D1_SIZE_F), NULL, 0);

	}
	
	//配置文件读取控件风格
	wchar_t lpStyle[BUF_SIZE] = {0};
	mpTemplete->QuerySubKeyValue(CB_KEY_STYLE, lpStyle, BUF_SIZE * sizeof(wchar_t));
	if (wcscmp(lpStyle, CB_STYLE_EDITABLE) == 0)
	{
		//可编辑风格
		mnStyle = 0;
		mpCurrentItemButton->GetIterator()->SetVisible(false);

		//读取编辑参数
		ICfKey* pKeyCurrentItemEdit = mpTemplete->GetSubKey(CB_KEY_CURRENT_ITEM_EDIT);
		if (pKeyCurrentItemEdit != NULL)
		{
			wchar_t lpBuf[BUF_SIZE] = {0};
			pKeyCurrentItemEdit->QuerySubKeyValue(CB_KEY_ONLY_ACCEPT_NUM, lpBuf, BUF_SIZE * sizeof(wchar_t));
			if (wcscmp(lpBuf, L"1") == 0)
			{
				mbOnlyAcceptNum = true;

				ULONG lulAcceptNumOnly = 1;
				CExMessage::SendMessage(mpCurrentItemEdit->GetIterator(), 
					mpIterator, EACT_EDIT_NUMBER_ONLY, lulAcceptNumOnly, NULL, 0);
				// 读取字数限制
				memset(lpBuf, 0, sizeof(wchar_t) * BUF_SIZE);
				pKeyCurrentItemEdit->QuerySubKeyValue(CB_KEY_MAX_INPUT_LEN, 
					lpBuf, BUF_SIZE * sizeof(wchar_t));
				ULONG lulLengthLimit = _wtoi(lpBuf);
				CExMessage::SendMessage(mpCurrentItemEdit->GetIterator(), 
					mpIterator, EACT_EDIT_SET_LENGTH_LIMIT, lulLengthLimit, NULL, 0);
				// 最大数值
				memset(lpBuf, 0, sizeof(wchar_t) * BUF_SIZE);
				pKeyCurrentItemEdit->QuerySubKeyValue(CB_KEY_MAX_INPUT_NUM, 
					lpBuf, BUF_SIZE * sizeof(wchar_t));
				mnMaxInputNum = _wtoi(lpBuf);
				// 最小数值
				memset(lpBuf, 0, sizeof(wchar_t) * BUF_SIZE);
				pKeyCurrentItemEdit->QuerySubKeyValue(CB_KEY_MIN_INPUT_NUM, 
					lpBuf, BUF_SIZE * sizeof(wchar_t));
				mnMinInputNum = _wtoi(lpBuf);
			}
		}
	}
	else
	{
		//不可编辑风格
		mnStyle = 1;
		mpCurrentItemEdit->GetIterator()->SetVisible(false);
	}

	//设置菜单项列表
	ICfKey*  lpChildrenKey = NULL;
	ICfKey*	lpMenuKey = NULL;
	ICfKey*	lpMenuItemKey = NULL;

	lpChildrenKey = mpTemplete->GetSubKey(CB_KEY_CHILDREN);
	if (lpChildrenKey)		lpMenuKey = lpChildrenKey->GetSubKey(CB_ID_POPMENU);
	if (lpMenuKey)			lpMenuItemKey = lpMenuKey->GetSubKey(CB_KEY_MENUITEM);
	if (lpMenuItemKey)
	{
		ICfKey* lpItemKey = lpMenuItemKey->MoveToSubKey();	
		int nCount = 0;
		while (lpItemKey != NULL)	//循环读取菜单项信息
		{
 			ComboMenuItem MenuItem;
 			lpItemKey->QuerySubKeyValue(CB_KEY_MENUITEM_ID, &(MenuItem.mnID), sizeof(int));
 			lpItemKey->QuerySubKeyValue(CB_KEY_MENUITEM_TEXT, MenuItem.mpText, sizeof(wchar_t) * BUF_SIZE);
			mpVecComboMenuItem.Insert(nCount++, MenuItem);

			lpItemKey = lpItemKey->MoveToNextKey();
		}
	}
	
	return TRUE;
}

BOOL CEvComboBox::SetDefaultValueList()
{
	//设置默认值
	ICfKey* lpDefaultCurrentValueKey = mpTemplete->GetSubKey(CB_KEY_DEFAULT_CURRENT_VALUE);
	if (lpDefaultCurrentValueKey)
	{
		wchar_t lswBuf[BUF_SIZE] = { 0 };
		lpDefaultCurrentValueKey->GetValue(lswBuf, sizeof(wchar_t) * BUF_SIZE);

		if (mnStyle == 0)
		{
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				mpCurrentItemEdit->GetIterator(),
				EACT_EDIT_SET_TEXT,
				(void*)lswBuf,
				(wcslen(lswBuf) + 1) * sizeof(wchar_t), NULL, 0);
		}
		else if (mnStyle == 1)
		{
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON),
				EACT_BUTTON_SETTEXT,
				(void*)lswBuf,
				(wcslen(lswBuf) + 1) * sizeof(wchar_t), NULL, 0);
		}
	}

	return TRUE;
}

bool CEvComboBox::SetValue()
{
	//if (NULL == mpUnificSetting) return false;

	if (false == mpIterator->IsEnable()) return false;

	//	读取值
	//eValueType ValueType;
	//wstring lwValue = mpUnificSetting->GetItemValue(mMsgInfo.mnCtrlID, &ValueType);
	wchar_t lwValue[] = L"1";
	//int lnID = _wtoi(lwID.c_str());

	//// 设置当前选择值
	//if (mpCurrentItemEdit)
	//{
	//	
	//}

	//if (mpCurrentItemButton)
	//{
	//	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	//		mpCurrentItemButton->GetIterator(), EACT_BUTTON_SETTEXT, lwID.c_str(),
	//		(lwID.size() + 1) * sizeof(wchar_t), NULL, 0);
	//}

	if (mnStyle == 0)
	{
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				mpCurrentItemEdit->GetIterator(),
				EACT_EDIT_SET_TEXT,
				lwValue/*.c_str()*/,
				(wcslen(lwValue)/*lwValue.size()*/ + 1) * sizeof(wchar_t), 
				NULL, 
				0);
	}
	else if (mnStyle == 1)
	{
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpCurrentItemButton->GetIterator(),
			EACT_BUTTON_SETTEXT,
			lwValue/*.c_str()*/,
			(wcslen(lwValue)/*lwValue.size()*/ + 1) * sizeof(wchar_t), 
			NULL, 
			0);
	}

	return true;
}

bool CEvComboBox::SetComboBoxEnable(bool lbEnalbe)
{
	IEinkuiIterator* iter = NULL;

	if (mpUpperPicture) 
	{
		iter = mpUpperPicture->GetIterator();
		if (iter)	iter->SetEnable(lbEnalbe);
	}
	if (mpCurrentItemEdit) 
	{
		iter = mpCurrentItemEdit->GetIterator();
		if (iter)	iter->SetEnable(lbEnalbe);
	}
	if (mpCurrentItemButton) 
	{
		iter = mpCurrentItemButton->GetIterator();
		if (iter)	iter->SetEnable(lbEnalbe);
	}
	if (mpDropDownButton) 
	{
		iter = mpDropDownButton->GetIterator();
		if (iter)	iter->SetEnable(lbEnalbe);
	}

	//if (mpIterPopMenu) mpIterPopMenu->SetVisible(false);

	if(mpIterList != NULL)
		mpIterList->SetVisible(false);

	if (lbEnalbe)
	{
		iter = mpUpperPicture->GetIterator();
		if (iter) iter->SetAlpha(1.0f);
	}
	else
	{
		iter = mpUpperPicture->GetIterator();
		if (iter) iter->SetAlpha(1.0f);
	}

	return true;
}

bool CEvComboBox::SetItemSelected(int nID)
{
	int lItemID = nID;

	if (mpVecComboMenuItem.Size() != 0)
	{
		int i = 0;
		for (i = 0; i < mpVecComboMenuItem.Size(); i++)
		{
			if (mpVecComboMenuItem[i].mnID == lItemID)
			{
				if (mnStyle == 0)
				{
					EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
						mpCurrentItemEdit->GetIterator(),
						EACT_EDIT_SET_TEXT,
						(void*)mpVecComboMenuItem[i].mpText,
						BUF_SIZE * sizeof(wchar_t), NULL, 0);
				}
				else if (mnStyle == 1)
				{
					EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
						mpIterator->GetSubElementByID(COMBO_ID_CTRL_CURRENT_ITEM_BUTTON),
						EACT_BUTTON_SETTEXT,
						(void*)mpVecComboMenuItem[i].mpText,
						BUF_SIZE * sizeof(wchar_t), NULL, 0);
				}
				break;
			}
		}
	}

	return true;
}

bool CEvComboBox::SetItemEnable(int nID)
{
	IEinkuiIterator* lpoPopupMenu = NULL;
	ULONG lulCommandID = (ULONG)nID;

	if (NULL == mpIterPopMenu) return false;

	if(ERESULT_SUCCESS == CExMessage::SendMessageW(mpIterPopMenu, 
		mpIterator, EACT_POPUPMENU_GET_MENUITEM_BY_COMMANDID, lulCommandID, &lpoPopupMenu, sizeof(IEinkuiIterator*)))
	{
		if (NULL != lpoPopupMenu)
		{
			lpoPopupMenu->SetEnable(true);
		}
	}

	return true;
}

bool CEvComboBox::SetItemDisable(int nID)
{
	IEinkuiIterator* lpoPopupMenu = NULL;	
	ULONG lulCommandID = (ULONG)nID;

	if (NULL == mpIterPopMenu) return false;

	if(ERESULT_SUCCESS == CExMessage::SendMessageW(mpIterPopMenu, 
		mpIterator, EACT_POPUPMENU_GET_MENUITEM_BY_COMMANDID, lulCommandID, &lpoPopupMenu, sizeof(IEinkuiIterator*)))
	{
		if (NULL != lpoPopupMenu)
		{
			lpoPopupMenu->SetEnable(false);
		}
	}
	
	return false;
}