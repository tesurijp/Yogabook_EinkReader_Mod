#include "StdAfx.h"
#include "Einkui.h"

#include "ElementImp.h"
#include "cmmstruct.h"
#include "EvMenuItemImp.h"
#include "EvPopupMenuImp.h"

#include "assert.h"


DEFINE_BUILTIN_NAME(PopupMenu)
CEvPopupMenu::CEvPopupMenu(void)
{
	miMainID = -1;

	mpoMenuItemInfo = NULL;

	miItemBgFrameCount = -1;

	mbIsManagerMenuItemEnable = true;
}


CEvPopupMenu::~CEvPopupMenu(void)
{
	CMM_SAFE_RELEASE(mpoItemBgBitmap);
	CMM_SAFE_DELETE(mpoMenuItemInfo);
}

ULONG CEvPopupMenu::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//首先调用基类
		leResult = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
		if (leResult != ERESULT_SUCCESS)
			break;
		LoadResource();

		leResult = ERESULT_SUCCESS;
	} while (false);


	return leResult;
}
//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvPopupMenu::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//ERESULT CEvPopupMenu::OnElementDestroy()
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	// 	do
//	// 	{
//	// 
//	// 		lResult = ERESULT_SUCCESS;
//	// 
//	// 	}while(false);
//
//	return lResult;
//}

//绘制消息
ERESULT CEvPopupMenu::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0, 0, (FLOAT)mpIterator->GetSizeX(), (FLOAT)mpIterator->GetSizeY()), mpBgBitmap, ESPB_DRAWBMP_EXTEND);

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

void CEvPopupMenu::LoadResource()
{
	bool lbResult = false;
	do
	{
		// 获取MainID
		miMainID = (int)this->mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_MAIN_ID, -1);
		// 如果没有MainId,则不加载
		if (-1 == miMainID)
			break;

		// 这里为了使得所有创建好的MenuItem项显示出来的宽度都一致，需要寻找最宽的MenuItem，
		// 因为需要再创建好所有的MenuItem之后才能知道最宽的宽度，而创建MenuItem又需要很多初始化信息，
		// 因此这里的MenuItemInfo提供了公共的menuItem初始化信息，每个MenuItem本身复制一份该信息，并根据自身特殊性进行修改
		// 创建好了之后再设置好所有MenuItem的宽度

		// 初始化ItemInfo结构体
		InitMenuItemInfo();
		if (NULL == mpoMenuItemInfo)
			break;

		// 加载好背景图
		// ItemBackGround
		wchar_t* lswBackgGround = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_POPUPMENU_ITEM_BACKGROUND);
		if (NULL != lswBackgGround && UNICODE_NULL != lswBackgGround[0])
		{
			mpoItemBgBitmap = EinkuiGetSystem()->GetAllocator()->LoadImageFile(
				lswBackgGround, L'.' == lswBackgGround[0] ? false : true);
			mpTemplete->ReleaseBuffer(&lswBackgGround);
		}

		// ItemFrameCount 
		miItemBgFrameCount = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ITEM_FRAMECOUNT, DEFAULT_ITEM_FRAMECOUNT);
		if (NULL != mpoItemBgBitmap)
		{
			mpoItemBgBitmap->SetExtendLine(mpoItemBgBitmap->GetWidth() / miItemBgFrameCount / 2, mpoItemBgBitmap->GetHeight() / 2);
		}


		// 创建所有MenuItem
		ICfKey* lpMenuItemKey = mpTemplete->GetSubKey(TF_ID_POPUPMENU_MENU_ITEM);
		BREAK_ON_NULL(lpMenuItemKey);
		CEvMenuItem* lpoMenuItem = NULL;
		lpMenuItemKey = lpMenuItemKey->MoveToSubKey();
		while (lpMenuItemKey != NULL)	//循环创建所有子元素
		{
			lpoMenuItem = CEvMenuItem::CreateInstance(mpIterator, lpMenuItemKey);
			if (NULL == lpoMenuItem)			// 遇到创建失败的，则跳出
				break;

			lpoMenuItem->GetIterator()->SetSize((FLOAT)lpMenuItemKey->QuerySubKeyValueAsLONG(L"Width", 0), (FLOAT)lpMenuItemKey->QuerySubKeyValueAsLONG(L"Height", 0));
			lpoMenuItem->SetBgBitmapPtr(mpoItemBgBitmap, miItemBgFrameCount);

			lpoMenuItem->SetMenuItemInfo(mpoMenuItemInfo);			// 提供菜单项信息，用来加载菜单项资源

			// 将该项加入到向量末尾
			if (moMenuItemVec.Insert(-1, lpoMenuItem) < 0)
				break;

			lpMenuItemKey = lpMenuItemKey->MoveToNextKey();	//打开下一个子元素键
		}

		ReLayoutMenuItem();

		lbResult = true;
	} while (false);

	if (false == lbResult)
	{
		PrintDebugString(L"PopupMenu_LoadResource 失败。");
	}
}

// 初始化MenuItemInfo
void CEvPopupMenu::InitMenuItemInfo()
{
	if (NULL == mpTemplete)
		return;

	CMM_SAFE_DELETE(mpoMenuItemInfo);

	mpoMenuItemInfo = new ST_MENUITEM_INFO;
	memset(mpoMenuItemInfo, 0, sizeof(ST_MENUITEM_INFO));

	// LeftWidth
	mpoMenuItemInfo->LeftWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_LEFT_WIDTH, DEFAULT_LEFT_WIDTH);

	// LeftIconWidth
	mpoMenuItemInfo->LeftIconWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_LEFT_ICON_WIDTH, -1);

	// LeftIconHeight
	mpoMenuItemInfo->LeftIconHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_LEFT_ICON_HEIGHT, -1);

	// RightWidth
	mpoMenuItemInfo->RightWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_RIGHT_WIDTH, DEFAULT_RIGHT_WIDTH);

	// RightIconWidth
	mpoMenuItemInfo->RightIconWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_RIGHT_ICON_WIDTH, -1);

	// RightIconHeight
	mpoMenuItemInfo->RightIconHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_RIGHT_ICON_HEIGHT, -1);

	// MiddleWidth
	mpoMenuItemInfo->MiddleWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_MIDDLE_WIDTH, DEFAULT_MIDDLE_WIDTH);

	// FontName
	mpTemplete->QuerySubKeyValue(TF_ID_POPUPMENU_FONT_NAME, mpoMenuItemInfo->FontName, sizeof(mpoMenuItemInfo->FontName) * sizeof(wchar_t));

	// FontSize
	mpoMenuItemInfo->FontSize = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_FONT_SIZE, DEFAULT_FONT_SIZE);

	// FontColor
	mpoMenuItemInfo->FontColor = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_FONT_COLOR, DEFAULT_FONT_COLOR);

	// FontDisabledColor
	mpoMenuItemInfo->FontDisableColor = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_FONT_DISABLE_COLOR, DEFAULT_FONT_DISABLE_COLOR);

	// FontFocusColor
	mpoMenuItemInfo->FontFocusColor = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_FONT_FOCUS_COLOR, DEFAULT_FONT_FOCUS_COLOR);

	// ItemHeight
	mpoMenuItemInfo->ItemHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ITEM_HEIGHT, -1);
}

ERESULT CEvPopupMenu::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_POPUPMENU_IS_MANAGER_MENUITEM_ENABLE:
	{
		bool* lpbIsSet = NULL;
		if (ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lpbIsSet))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		mbIsManagerMenuItemEnable = *lpbIsSet;

		luResult = ERESULT_SUCCESS;

		break;
	}
	case EEVT_MENUITEM_CLICK:
	{
		this->GetIterator()->PostMessageToParent(npMsg);

		// 当子菜单项被点击时，要隐藏掉自身
		mpIterator->SetVisible(false);

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EEVT_MENUITEM_MOUSE_HOVER:
	{
		mpIterator->PostMessageToParent(npMsg);
		luResult = ERESULT_SUCCESS;
	}
	break;

	case EEVT_MENUITEM_GET_FOCUS:				// 有子菜单项获得了焦点
	{
		IEinkuiIterator* lpIterMenuItem = npMsg->GetMessageSender();
		// 必须是当前PopupMenu的子项，并且是MenuItem
		if (NULL != lpIterMenuItem)
			OnMenuItemGetFocused(lpIterMenuItem);

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_INSERT_MENUITEM_BY_CREATE:			// 插入新的菜单项
	{
		if (npMsg->GetInputDataSize() != sizeof(STCTL_POPUPMENU_MENUITEMINFO)
			|| npMsg->GetInputData() == NULL)
			break;

		STCTL_POPUPMENU_MENUITEMINFO ldInfo = *(PSTCTL_POPUPMENU_MENUITEMINFO)npMsg->GetInputData();
		if (false == OnInsertMenuItem(ldInfo))
			break;

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_INSERT_MENUITEM_BY_EXIST:
	{
		PSTCTL_POPUPMENU_MENUITEMINSERT lpdMenuItemInserted = NULL;
		if (ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lpdMenuItemInserted))
		{
			luResult = ERESULT_UNSUCCESSFUL;
			break;
		}
		BREAK_ON_FALSE(lpdMenuItemInserted->MenuItem->GetElementObject()->GlobleVerification(L"MenuItem"));

		CEvMenuItem* lpoMenuItem = (CEvMenuItem*)lpdMenuItemInserted->MenuItem->GetElementObject();

		moMenuItemVec.Insert(lpdMenuItemInserted->Index, lpoMenuItem);

		lpoMenuItem->SetBgBitmapPtr(mpoItemBgBitmap, miItemBgFrameCount);

		lpoMenuItem->SetMenuItemInfo(mpoMenuItemInfo);

		// 重新计算PopupMenu布局
		ReLayoutMenuItem();

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_RELAYOUT_MENUITEM:			// 重新布局
	{
		ReLayoutMenuItem();

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_DELETE_MENUITEM_BY_COMMANDID:
	{
		if (npMsg->GetInputDataSize() != sizeof(UINT) || NULL == npMsg->GetInputData())
			break;

		if (false == DeleteItemByCommandID(*(UINT*)npMsg->GetInputData()))
			break;

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_DELETE_MENUITEM_BY_INDEX:
	{
		if (npMsg->GetInputDataSize() != sizeof(int) || NULL == npMsg->GetInputData())
			break;

		if (false == DeleteItemByIndex(*(int*)npMsg->GetInputData()))
			break;

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_GET_MENUITEM_BY_COMMANDID:
	{
		if (npMsg->GetInputDataSize() != sizeof(UINT) || NULL == npMsg->GetInputData()
			|| npMsg->GetOutputBufferSize() != sizeof(IEinkuiIterator*) || npMsg->GetOutputBuffer() == NULL)
			break;

		IEinkuiIterator** lpIter = (IEinkuiIterator**)npMsg->GetOutputBuffer();
		UINT luCommandID = *(UINT*)npMsg->GetInputData();
		(*lpIter) = GetItemByCommandID(luCommandID);
		BREAK_ON_NULL(*lpIter);

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_POPUPMENU_GET_MENUITEM_BY_INDEX:
	{
		if (npMsg->GetInputDataSize() != sizeof(UINT) || NULL == npMsg->GetInputData()
			|| npMsg->GetOutputBufferSize() != sizeof(IEinkuiIterator*) || npMsg->GetOutputBuffer() == NULL)
			break;

		IEinkuiIterator** lpIter = (IEinkuiIterator**)npMsg->GetOutputBuffer();
		UINT luIndex = *(UINT*)npMsg->GetInputData();
		(*lpIter) = GetItemByIndex(luIndex);
		BREAK_ON_NULL(*lpIter);

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EMSG_ELEMENT_ACTIVATED:
	{
		//激活状态改变
		STEMS_ELEMENT_ACTIVATION* lpActive = NULL;
		luResult = CExMessage::GetInputDataBuffer(npMsg, lpActive);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lpActive->State == 0 && mpIterator->IsVisible() != false)			// 可见状态下失去激活状态，
		{
			mpIterator->SetVisible(false);
		}

		luResult = ERESULT_SUCCESS;
	}
	break;


	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if (luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}


// 描述：
//		处理当有子菜单项获得焦点
void CEvPopupMenu::OnMenuItemGetFocused(
	IN IEinkuiIterator* npIterMenuItem
)
{
	// 发送消息给所有其他菜单项，失去焦点
	STEMS_STATE_CHANGE ldStateChange;
	ldStateChange.Related = npIterMenuItem;
	ldStateChange.State = 0;
	for (int i = 0; i < moMenuItemVec.Size(); ++i)
	{
		IEinkuiIterator* lpMenuItem = moMenuItemVec.GetEntry(i)->GetIterator();
		if (NULL != lpMenuItem && npIterMenuItem != lpMenuItem)
		{
			EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
				lpMenuItem, EMSG_MOUSE_FOCUS, &ldStateChange, sizeof(STEMS_STATE_CHANGE));
		}
	}
}

ERESULT CEvPopupMenu::OnElementShow(bool nbIsShow)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (false != nbIsShow && false != mbIsManagerMenuItemEnable)			// 显示的时候，查看所有子项的可用状态
		{
			//// 通知所有子菜单项
			//IUnificSetting* lpoUnificSetting = GetUnificSetting();
			//BREAK_ON_NULL(lpoUnificSetting);
			//for(int i = 0; i < moMenuItemVec.Size(); ++i)
			//{
			//	moMenuItemVec.GetEntry(i)->GetIterator()->SetEnable(lpoUnificSetting->GetItemEnable(moMenuItemVec.GetEntry(i)->GetCommandID()));
			//}
		}
		else							// 隐藏的时候要隐藏级联菜单
		{
			for (int i = 0; i < moMenuItemVec.Size(); ++i)
			{
				moMenuItemVec.GetEntry(i)->HideCascadeMenu();
			}
		}



		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

// 描述：
//		获取给定MainID的PopupMenu(包括MenuItem所弹出来的PopupMenu)
// 返回值：
//		成功返回对应ID的迭代器，失败返回NULL
IEinkuiIterator* CEvPopupMenu::GetPopupMenuByMainID(
	IN UINT niUinqueID
)
{
	IEinkuiIterator* lpoResult = NULL;
	do
	{
		if (niUinqueID == miMainID)
		{
			lpoResult = mpIterator;
			break;
		}

		for (int i = 0; i < moMenuItemVec.Size(); ++i)
		{
			if (moMenuItemVec.GetEntry(i)->GetMenuItemType() == CEvMenuItem::ENUM_MENUITEM_TYPE_APPLE_EXTEND
				|| moMenuItemVec.GetEntry(i)->GetMenuItemType() == CEvMenuItem::ENUM_MENUITEM_TYPE_EXTEND)
			{
				CEvPopupMenu* lpoPopupMenu = moMenuItemVec.GetEntry(i)->GetPopupMenu();
				if (NULL != lpoPopupMenu)
				{
					lpoResult = lpoPopupMenu->GetPopupMenuByMainID(niUinqueID);
					if (NULL != lpoResult)
						break;
				}
			}
		}

	} while (false);
	return lpoResult;
}

// 描述：
//		插入新的菜单项
bool CEvPopupMenu::OnInsertMenuItem(
	IN STCTL_POPUPMENU_MENUITEMINFO ndMenuInfo
)
{
	bool lbResult = false;
	do
	{
		if (ndMenuInfo.Type == (UINT)CEvMenuItem::ENUM_MENUITEM_TYPE_NORMAL)
		{
			CEvMenuItem* lpoMenuItem = CEvMenuItem::CreateInstance(mpIterator, NULL, ndMenuInfo.MenuItemId);
			BREAK_ON_NULL(lpoMenuItem);

			moMenuItemVec.Insert(ndMenuInfo.Index, lpoMenuItem);

			lpoMenuItem->GetIterator()->SetSize((FLOAT)mpTemplete->QuerySubKeyValueAsLONG(L"MenuItem/Width", 0), (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(L"MenuItem/Height", 0));
			lpoMenuItem->SetCommandID(ndMenuInfo.UniqueMenuItemId);

			lpoMenuItem->SetBgBitmapPtr(mpoItemBgBitmap, miItemBgFrameCount);

			lpoMenuItem->SetMenuItemType((CEvMenuItem::ENUM_MENUITEM_TYPE)ndMenuInfo.Type);

			lpoMenuItem->SetMenuItemInfo(mpoMenuItemInfo);

			CExMessage::SendMessage(lpoMenuItem->GetIterator(), mpIterator, EACT_MENUITEM_CHANGE_TEXT, ndMenuInfo.MenuText);

			if (ndMenuInfo.HotKeyInfo != NULL)
			{
				CExMessage::SendMessage(lpoMenuItem->GetIterator(), mpIterator, EACT_MENUITEM_CHANGE_HOTKEY, *ndMenuInfo.HotKeyInfo);
			}

			// 重新计算PopupMenu布局
			ReLayoutMenuItem();
		}
		else
			break;

		lbResult = true;
	} while (false);
	return lbResult;
}


// 描述：
//		重新计算菜单项的布局
void CEvPopupMenu::ReLayoutMenuItem()
{
	if (0 == moMenuItemVec.Size())
	{
		return;
	}
	int liMaxNameWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_TEXT_WIDTH, 0);				// 最大名称宽度
	int liMaxHotKeyWidth = 0;			// 最大热键宽度
	for (int i = 0; i < this->moMenuItemVec.Size(); ++i)
	{
		// 获取创建的MenuItem的各部分的最大宽度
		if (liMaxNameWidth < moMenuItemVec.GetEntry(i)->GetNameAreaWidth())
		{
			liMaxNameWidth = moMenuItemVec.GetEntry(i)->GetNameAreaWidth();
		}
		if (liMaxHotKeyWidth < moMenuItemVec.GetEntry(i)->GetHotKeyAreaWidth())
		{
			liMaxHotKeyWidth = moMenuItemVec.GetEntry(i)->GetHotKeyAreaWidth();
		}
	}

	// 获取MenuItem显示的位置
	D2D1_RECT_F ldAlignRect;
	ldAlignRect.left = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ALIGN_LEFT, 0);
	ldAlignRect.right = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ALIGN_RIGHT, 0);
	ldAlignRect.top = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ALIGN_TOP, 0);
	ldAlignRect.bottom = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_ALIGN_BOTTOM, 0);


	// 设置好MenuItem的信息
	IEinkuiIterator* lpIter = NULL;
	float lfItemHeightCount = ldAlignRect.top;
	for (int i = 0; i < this->moMenuItemVec.Size(); ++i)
	{
		// 设置好菜单名称和热键区域宽度
		moMenuItemVec.GetEntry(i)->SetNameAreaWidth(liMaxNameWidth);
		moMenuItemVec.GetEntry(i)->SetHotKeyAreaWidth(liMaxHotKeyWidth);

		// 让菜单项自己更新尺寸
		moMenuItemVec.GetEntry(i)->UpdateSize();

		if (false != moMenuItemVec.GetEntry(i)->GetIterator()->IsVisible())
		{
			// 设置好该项的位置
			moMenuItemVec.GetEntry(i)->GetIterator()->SetPosition(ldAlignRect.left,
				lfItemHeightCount);

			lfItemHeightCount += moMenuItemVec.GetEntry(i)->GetIterator()->GetSizeY();
		}
	}

	// 设置好PopupMenu的大小
	FLOAT lfWidth = ldAlignRect.left + moMenuItemVec.GetEntry(0)->GetIterator()->GetSizeX() + ldAlignRect.right;
	FLOAT lfHeight = lfItemHeightCount + ldAlignRect.bottom;
	mpIterator->SetSize(lfWidth, lfHeight);
}

// 描述：
//		根据CommandID删除菜单项
bool CEvPopupMenu::DeleteItemByCommandID(
	IN UINT niCommandID
)
{
	bool lbResult = false;
	do
	{
		int i = 0;
		for (; i < moMenuItemVec.Size(); ++i)
		{
			if (moMenuItemVec.GetEntry(i)->GetCommandID() == niCommandID)
			{
				break;
			}
		}

		if (i == moMenuItemVec.Size())
			break;

		IEinkuiIterator* lpoIter = moMenuItemVec.GetEntry(i)->GetIterator();
		if (false == moMenuItemVec.RemoveByIndex(i))
			break;

		if (NULL != lpoIter)
		{
			lpoIter->Close();
		}


		ReLayoutMenuItem();

		lbResult = true;
	} while (false);
	return lbResult;

}

// 描述：
//		根据索引删除菜单项,传入-1删除全部。
bool CEvPopupMenu::DeleteItemByIndex(
	IN int niIndex
)
{
	bool lbResult = false;
	do
	{
		if (niIndex >= 0 &&
			niIndex < moMenuItemVec.Size())
		{
			IEinkuiIterator* lpoIter = moMenuItemVec.GetEntry(niIndex)->GetIterator();
			if (false == moMenuItemVec.RemoveByIndex(niIndex))
				break;

			if (NULL != lpoIter)
			{
				lpoIter->Close();
			}
		}
		else if (-1 == niIndex)
		{
			for (int i = 0; i < moMenuItemVec.Size(); ++i)
			{
				moMenuItemVec.GetEntry(i)->GetIterator()->Close();
			}
			moMenuItemVec.Clear();
		}
		else
			break;

		ReLayoutMenuItem();

		lbResult = true;
	} while (false);
	return lbResult;
}

// 描述：
//		根据CommandID获取菜单项
IEinkuiIterator* CEvPopupMenu::GetItemByCommandID(
	IN UINT niCommandID
)
{
	IEinkuiIterator* lpoIter = NULL;
	do
	{
		for (int i = 0; i < moMenuItemVec.Size(); ++i)
		{
			if (moMenuItemVec.GetEntry(i)->GetCommandID() == niCommandID)
			{
				lpoIter = moMenuItemVec.GetEntry(i)->GetIterator();
				break;
			}
		}

	} while (false);
	return lpoIter;
}



// 描述：
//		根据索引获取菜单项
IEinkuiIterator* CEvPopupMenu::GetItemByIndex(
	IN UINT niIndex
)
{
	IEinkuiIterator* lpoIter = NULL;
	do
	{
		if (niIndex < (UINT)moMenuItemVec.Size())
			lpoIter = moMenuItemVec.GetEntry(niIndex)->GetIterator();

	} while (false);
	return lpoIter;
}