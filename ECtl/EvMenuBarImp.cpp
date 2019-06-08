/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "EvMenuBarImp.h"
#include "EvButtonImp.h"
#include "EvListImp.h"
#include "EvPopupMenuImp.h"
#include "EvMenuButtonImp.h"


DEFINE_BUILTIN_NAME(MenuBar)

CEvMenuBar::CEvMenuBar(void)
{
	mbIsSubMenuVisible = false;
	mpoLastShowMenuButton = NULL;
}


CEvMenuBar::~CEvMenuBar(void)
{

}


ULONG CEvMenuBar::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
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

		LoadResource();

		leResult = ERESULT_SUCCESS;
	}while(false);


	return leResult;
}
//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvMenuBar::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);


		//// 注册Alt + F4 热键
		//mulHotKeyID = VK_F4;
		//mulHotKeyID |= 0x00000100;

		//// 向框架注册全局热键
		//EinkuiGetSystem()->GetElementManager()->RegisterHotKey(mpIterator, mulHotKeyID, VK_F4, false, false, true, NULL);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//ERESULT CEvMenuBar::OnElementDestroy()
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	// 	do
//	// 	{
//	// 		//CDropTargetMgr::UnregisterDropWindow(EinkuiGetSystem()->GetMainWindow(),mpDropTargetManager);
//	// 
//	// 		lResult = ERESULT_SUCCESS;
//	// 
//	// 	}while(false);
//
//	return lResult;
//}
//
//绘制消息
ERESULT CEvMenuBar::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
			mpBgBitmap,ESPB_DRAWBMP_LINEAR);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

void CEvMenuBar::LoadResource()
{

}

ERESULT CEvMenuBar::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EEVT_MENUITEM_CLICK:			// 有子对象的菜单项被点击
		{
			this->GetIterator()->PostMessageToParent(npMsg);

			// 取消弹出菜单状态
			mbIsSubMenuVisible = false;
			if(NULL != mpoLastShowMenuButton)
			{
				CExMessage::PostMessage(mpoLastShowMenuButton, mpIterator, EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE, false);
				mbIsSubMenuVisible = false;
				mpoLastShowMenuButton = NULL;
			}
			

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_MENUITEM_MOUSE_HOVER:
		{
			mpIterator->PostMessageToParent(npMsg);
			luResult = ERESULT_SUCCESS;
		}
		break;

	case EEVT_MENUBUTTON_CLICK:					// MenuButton被点击
		{
			if(npMsg->GetInputDataSize() != sizeof(bool))
				break;

			mbIsSubMenuVisible = *(bool*)npMsg->GetInputData();

			// 弹出子菜单,则记下当前弹出的子菜单
			if(false != mbIsSubMenuVisible)
			{
				mpoLastShowMenuButton = npMsg->GetMessageSender();
				if(NULL == mpoLastShowMenuButton)
					break;

				if(false == mpoLastShowMenuButton->GetElementObject()->IsKindOf(GET_BUILTIN_NAME(MenuButton)))
				{
					mpoLastShowMenuButton = NULL;
					break;
				}
			}
			else
			{
				if(NULL != mpoLastShowMenuButton)
				{
					CExMessage::PostMessage(mpoLastShowMenuButton, mpIterator, EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE, false);
					mbIsSubMenuVisible = false;
					mpoLastShowMenuButton = NULL;
				}
			}

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EACT_MENUBAR_ANY_SUBMENU_VISIBLE:		// 询问是否有子菜单被展开	
		{
			if(npMsg->GetOutputBufferSize() != sizeof(bool))
			{
				break;
			}

			// 获取输出缓冲的地址
			bool* lpbIsShow = (bool*)npMsg->GetOutputBuffer();
			// 存放内容到输出缓冲
			*lpbIsShow = mbIsSubMenuVisible;

			npMsg->SetOutputDataSize(sizeof(bool));

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EACT_MENUBAR_HIDE_LAST_SUBMENU:		
		{
			if(NULL != mpoLastShowMenuButton)
			{
				// 发送消息隐藏子菜单项
				bool lbIsVisible = false;
				CExMessage::PostMessage(mpoLastShowMenuButton, mpIterator, EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE, lbIsVisible);
				mpoLastShowMenuButton = npMsg->GetMessageSender();
				if(NULL == mpoLastShowMenuButton)
					break;

				if(false == mpoLastShowMenuButton->GetElementObject()->IsKindOf(GET_BUILTIN_NAME(MenuButton)))
				{
					mpoLastShowMenuButton = NULL;
					break;
				}
			}
			luResult = ERESULT_SUCCESS;
		}
		break;
	case EMSG_ELEMENT_ACTIVATED:
		{
			//激活状态改变
			STEMS_ELEMENT_ACTIVATION* lpActive;
			luResult = CExMessage::GetInputDataBuffer(npMsg,lpActive);
			if(luResult != ERESULT_SUCCESS)
				break;

			if (lpActive->State == 0 && mpIterator->IsVisible() != false)			// 可见状态下失去激活状态，需要隐藏当前展开的菜单
			{
				if(NULL != mpoLastShowMenuButton)
				{
					CExMessage::PostMessage(mpoLastShowMenuButton, mpIterator, EEVT_MENUBUTTON_SET_SUBMENU_VISIBLE, false);
					mbIsSubMenuVisible = false;
				}
			}

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EACT_MENUBAR_INSERT_NEW_MENUITEM:
		{
			if(npMsg->GetInputDataSize() != sizeof(STCTL_MENUBAR_INSERT_MENUITEM)
				|| npMsg->GetInputData() == NULL)
				break;

			STCTL_MENUBAR_INSERT_MENUITEM ldInfo = *(PSTCTL_MENUBAR_INSERT_MENUITEM)npMsg->GetInputData();
			IEinkuiIterator* lpoIterMenuButton = mpIterator->GetSubElementByID(ldInfo.MenuButtonID);
			if(NULL == lpoIterMenuButton)
				break;

			if(ERESULT_SUCCESS != CExMessage::PostMessage(lpoIterMenuButton, mpIterator, EEVT_MENUBUTTON_INSERT_MENUITEM, ldInfo.MenuButtonInfo))
				break;

			luResult = ERESULT_SUCCESS;
		}
		break;

	case EACT_MENUBAR_GET_POPUPMENU_BY_COMMANDID:
		{
			ULONG *lplCommandID = NULL;
			if(ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lplCommandID)
				|| sizeof(IEinkuiIterator*) != npMsg->GetOutputBufferSize())
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			IEinkuiIterator** lpIter = (IEinkuiIterator**)npMsg->GetOutputBuffer();
			(*lpIter) = GetPopupMenuByCommandID(*lplCommandID);

			if(NULL == *lpIter)
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

//按钮单击事件
ERESULT CEvMenuBar::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		LONG llCommandID = 0;
		if(npSender->GetID() == TF_ID_MENUBAR_BTN_MIN)		// 最小化
		{
			llCommandID = 0x00000001;
		}
		else if(npSender->GetID() == TF_ID_MENUBAR_BTN_CLOSE)		// 关闭
		{
			llCommandID = 0x00000002;
		}
		else
			break;

		EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
			mpIterator->GetParent(), EEVT_MENUITEM_CLICK, &llCommandID, sizeof(long));

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//元素参考尺寸发生变化
ERESULT CEvMenuBar::OnElementResized(D2D1_SIZE_F nNewSize)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		// 设置好两个按钮的位置
		IEinkuiIterator* lpoMiniBtn = mpIterator->GetSubElementByID(TF_ID_MENUBAR_BTN_MIN);
		if(NULL != lpoMiniBtn)
		{
			lpoMiniBtn->SetPosition(mpIterator->GetSizeX() - 57, lpoMiniBtn->GetPositionY());

			IEinkuiIterator* lpoCloseBtn = mpIterator->GetSubElementByID(TF_ID_MENUBAR_BTN_CLOSE);
			if(NULL != lpoCloseBtn)
				lpoCloseBtn->SetPosition(lpoMiniBtn->GetPositionX() + lpoMiniBtn->GetSizeX(), lpoMiniBtn->GetPositionY());
		}

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

// 获取指定CommandID的PopupMenu
IEinkuiIterator* CEvMenuBar::GetPopupMenuByCommandID(IN ULONG niCommandID)
{
	IEinkuiIterator* lpoIter = NULL;
	do 
	{
		IEinkuiIterator* lpoTempIter = NULL;
		for(int i = 0; i < mpIterator->GetSubElementCount(); i++)
		{
			lpoTempIter = mpIterator->GetSubElementByZOder(i);
			if(NULL != lpoTempIter && false != lpoTempIter->GetElementObject()->GlobleVerification(L"MenuButton"))
			{
				lpoTempIter = ((CEvMenuButton*)lpoTempIter->GetElementObject())->GetPopupMenuByUniqueID(niCommandID);
				if(NULL != lpoTempIter)
				{
					lpoIter = lpoTempIter;
					break;
				}
			}
		}


		return lpoIter;
	} while (false);
	return lpoIter;
}

////快捷键消息
//ERESULT CEvMenuBar::OnHotKey(const STEMS_HOTKEY* npHotKey)
//{
//	if(npHotKey->HotKeyID == mulHotKeyID)		// 关闭
//	{
//		LONG llCommandID = 0x00000002;
//		EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
//			mpIterator->GetParent(), EEVT_MENUITEM_CLICK, &llCommandID, sizeof(long));
//	}
//
//	return ERESULT_KEY_UNEXPECTED;
//}

