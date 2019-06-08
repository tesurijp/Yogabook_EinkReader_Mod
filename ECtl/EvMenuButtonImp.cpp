/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "EvButtonImp.h"
#include "EvListImp.h"
#include "EvPopupMenuImp.h"
#include "EvMenuButtonImp.h"
#include "EvButtonImp.h"


DEFINE_BUILTIN_NAME(MenuButton)

CEvMenuButton::CEvMenuButton(void)
{
	mpoPopupMenu = NULL;
	mpoButton = NULL;

	
}


CEvMenuButton::~CEvMenuButton(void)
{
}

ULONG CEvMenuButton::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针ss
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		leResult = ERESULT_SUCCESS;
	}while(false);


	return leResult;
}
//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvMenuButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		LoadResource();

		// 设置按钮响应区域
		if(NULL != mpoButton)
		{
			D2D1_SIZE_F ldfSize;
			ldfSize = mpoButton->GetIterator()->GetSize();
			CExMessage::PostMessage(mpoButton->GetIterator(), mpIterator, EACT_BUTTON_SET_ACTION_RECT, ldfSize);
		}

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//ERESULT CEvMenuButton::OnElementDestroy()
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	return lResult;
//}
//
//绘制消息
ERESULT CEvMenuButton::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,(FLOAT)mpBgBitmap->GetWidth(),(FLOAT)mpBgBitmap->GetHeight()),
			mpBgBitmap,ESPB_DRAWBMP_LINEAR);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

void CEvMenuButton::LoadResource()
{
	// 创建Button
	ICfKey* lpBtnKey = mpTemplete->GetSubKey(TF_ID_MENUBUTTON_BUTTON);
	if(NULL != lpBtnKey && NULL == mpoButton)
	{
		mpoButton = CEvButton::CreateInstance(mpIterator, lpBtnKey, ID_OF_MENUBUTTON_BUTTON);
		if(NULL != mpoButton)			// 设置该MenuButton大小即为按钮的大小
		{
			mpIterator->SetSize(mpoButton->GetIterator()->GetSize());
		}
	}
	// 获取PopupMenuID
	miPopupMenuID = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUBUTTON_POPUPMENU_ID, -1);

	LoadSubPopupMenu();
}

ERESULT CEvMenuButton::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EEVT_BUTTON_CLICK:					// 按钮被按下
		{
			OnBtnClick();

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_MENUITEM_MOUSE_HOVER:
		{
			mpIterator->PostMessageToParent(npMsg);
			luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_MENUITEM_CLICK:					// 有子对象的菜单项被点击
		{
			this->GetIterator()->PostMessageToParent(npMsg);

			BREAK_ON_NULL(mpoButton);
			// 有子菜单项被点击时，取消按钮的Checked状态
			bool lbIsCheck = false;
			luResult = CExMessage::PostMessage(mpoButton->GetIterator(), mpIterator, EACT_BUTTON_SET_CHECKED, lbIsCheck);
		}
		break;

	case EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE:
		{
			if(npMsg->GetInputDataSize() != sizeof(bool))
				break;

			bool lbIsVisible = *(bool*)npMsg->GetInputData();
			if(NULL != mpoPopupMenu)
				mpoPopupMenu->GetIterator()->SetVisible(lbIsVisible);
			bool lbIsChecked = false;
			if(false != lbIsVisible)
			{
				lbIsChecked = true;
			}
			if(NULL != mpoButton)
			{
				// 发送消息给按钮，使之进入Checked状态
				luResult = CExMessage::PostMessage(mpoButton->GetIterator(), mpIterator, EACT_BUTTON_SET_CHECKED, lbIsChecked);
			}
			else
				luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_BUTTON_MOUSE_IN:
	case EEVT_BUTTON_MOUSE_OUT:
		{
			STEMS_STATE_CHANGE ldStrteChange;
			ldStrteChange.State = npMsg->GetMessageID() == EEVT_BUTTON_MOUSE_IN? 1 : 0;
			ldStrteChange.Related = npMsg->GetMessageSender();
			OnMouseFocus(&ldStrteChange);

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_MENUBUTTON_INSERT_MENUITEM:
		{
			if(npMsg->GetInputDataSize() != sizeof(STCTL_MENUBUTTON_INSERT_MENUITEM)
				|| npMsg->GetInputData() == NULL)
				break;

			STCTL_MENUBUTTON_INSERT_MENUITEM ldInfo = *(PSTCTL_MENUBUTTON_INSERT_MENUITEM)npMsg->GetInputData();
			IEinkuiIterator* lpoIterPopupMenu = GetPopupMenuByUniqueID(ldInfo.UniquePopupMenuId);
			if(NULL == lpoIterPopupMenu)
				break;

			if(ERESULT_SUCCESS != CExMessage::PostMessage(lpoIterPopupMenu, mpIterator,
				EACT_POPUPMENU_INSERT_MENUITEM_BY_CREATE, ldInfo.PopupMenuInfo))
				break;

			luResult = ERESULT_SUCCESS;
		}
		break;

	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		//luResult = ERESULT_UNEXPECTED_MESSAGE;	// 这儿没有基类，派生本类时，删除本句；
	}

	return luResult;
}

void CEvMenuButton::LoadSubPopupMenu()
{
	if(-1 == miPopupMenuID)
		return;

	// 查找子菜单模板，并创建
	ICfKey* lpoPopupMenuKey = mpTemplete->GetParentsKey();
	while(NULL != lpoPopupMenuKey->GetParentsKey())
		lpoPopupMenuKey = lpoPopupMenuKey->GetParentsKey();
	lpoPopupMenuKey = lpoPopupMenuKey->GetSubKey(L"PopupMenu");
	if(NULL == lpoPopupMenuKey)
		return;

	// 找寻出ID为miPopupMenuID的子菜单
	lpoPopupMenuKey = lpoPopupMenuKey->MoveToSubKey();
	while(NULL != lpoPopupMenuKey)
	{
		if(miPopupMenuID == lpoPopupMenuKey->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_MAIN_ID, -1))
			break;
		else
			lpoPopupMenuKey = lpoPopupMenuKey->MoveToNextKey();
	}

	if(NULL == lpoPopupMenuKey)
		return;

	mpoPopupMenu = CEvPopupMenu::CreateInstance(mpIterator, lpoPopupMenuKey, ID_OF_MENUBUTTON_POPUPMENU);
	if(NULL == mpoPopupMenu)
		return;

	mpoPopupMenu->GetIterator()->SetVisible(false);

	SetPopupMenuPosition();
}

void CEvMenuButton::OnBtnClick()
{
	// 在按钮下方弹出子菜单

	// 由隐藏状态到显示状态
	bool lbIsShow = false;
	if(NULL != mpoPopupMenu)
	{
		if(false == mpoPopupMenu->GetIterator()->IsVisible())	
		{
			mpoPopupMenu->GetIterator()->SetVisible(true);
			lbIsShow = true;
		}
		else			// 由显示状态到隐藏状态
		{
			mpoPopupMenu->GetIterator()->SetVisible(false);
		}
	}

	// 发送消息给父窗口，表示菜单按钮被点击
	this->PostMessageToParent(EEVT_MENUBUTTON_CLICK, lbIsShow);
}

//禁用或启用
ERESULT CEvMenuButton::OnElementEnable(bool nbIsEnable)
{
	if(false == nbIsEnable)
	{
		if(NULL != mpoButton)
			mpoButton->GetIterator()->SetEnable(false);
		if(NULL != mpoPopupMenu)
			mpoPopupMenu->GetIterator()->SetEnable(false);
	}
	else
	{
		if(NULL != mpoButton)
			mpoButton->GetIterator()->SetEnable(true);
		if(NULL != mpoPopupMenu)
			mpoPopupMenu->GetIterator()->SetEnable(true);
	}

	return ERROR_SUCCESS;
}

void CEvMenuButton::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	if(0 != npState->State)	// 进入
	{
		// 如果当前子菜单已经显示，则不需要处理
		if(NULL != mpoPopupMenu && false != mpoPopupMenu->GetIterator()->IsVisible())
		{
			return;
		}

		// 询问是否已经有其他菜单项显示
		bool lbIsShow = false;
		this->SendMessageToParent(EACT_MENUBAR_ANY_SUBMENU_VISIBLE, CExMessage::DataInvalid, &lbIsShow, sizeof(bool));


		// 如果已经有菜单弹出，则鼠标移入该菜单按钮时应该弹出菜单，同时发送消息给父窗口，隐藏之前弹出的菜单
		if(false != lbIsShow)
		{
			// 发送消息给父窗口，隐藏之前的菜单
			this->PostMessageToParent(EACT_MENUBAR_HIDE_LAST_SUBMENU, CExMessage::DataInvalid);
			if(NULL != mpoPopupMenu)
			{
				// 发送消息给按钮，使之进入Checked状态
				bool lbIsChecked = true;
				CExMessage::PostMessage(mpoButton->GetIterator(), mpIterator, EACT_BUTTON_SET_CHECKED, lbIsChecked);

				mpoPopupMenu->GetIterator()->SetVisible(true);
			}
		}

	}
}


// 描述：
//		设置好弹出菜单的位置
void CEvMenuButton::SetPopupMenuPosition()
{
	if(NULL == mpoPopupMenu)
		return;

	// 计算要弹出子菜单的位置（按钮的左下角向右，或者按钮的右下角向左）
	//int liX = 0, liY = 0;

	// 看右下角是否有足够空间
	//EinkuiGetSystem()->GetElementManager()->GetDesktop()->GetSize();

	if(NULL != mpoButton)
		mpoPopupMenu->GetIterator()->SetPosition(mpoButton->GetIterator()->GetPositionX() - 6, mpoButton->GetIterator()->GetSizeY());
}

// 描述：
//		获取该MenuButton下指定UniqueID的PopupMenu（包括子孙的）
IEinkuiIterator* CEvMenuButton::GetPopupMenuByUniqueID(
	IN UINT niUniqueID
	)
{
	IEinkuiIterator* lpoResult = NULL;
	do 
	{
		BREAK_ON_NULL(mpoPopupMenu);
		lpoResult = mpoPopupMenu->GetPopupMenuByMainID(niUniqueID);

	} while (false);
	return lpoResult;
}


