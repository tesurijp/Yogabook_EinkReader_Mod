#include "StdAfx.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvListImp.h"
#include "EvPopupMenuImp.h"
#include "EvMenuItemImp.h"




DEFINE_BUILTIN_NAME(MenuItem)

CEvMenuItem::CEvMenuItem(void)
{
	meMemuItemType = ENUM_MENUITEM_TYPE_NORMAL;
	miCommandID = -1;
	miShotcutKey = -1;

	mpoFocusTextImage = NULL;
	mpoFocusHotKeyImage = NULL;
	mpoTextImage = NULL;
	mpoLeftImage = NULL;
	mpoRightImage = NULL;
	mpoSeparation = NULL;
	mpoCheckedImage = NULL;
	mpoHotKeyImage = NULL;
	mpoItemBgBmp = NULL;

	miItemFrameCount = -1;

	miNameAreaWidth = 0;
	miHotKeyAreaWidth = 0;

	mbIsChecked = false;
	mbIsShowExtendMenu = false;

	miExtendMenuID = -1;

	mpoMenuItemInfo = NULL;

	mpoSubPopupMenu = NULL;

	miDuration = 0;

	mbIsFocused = false;

	mbIsSubMenuShow = false;

	ZeroMemory(mswMenuTittle, MAX_TITTLE_LENGTH * sizeof(wchar_t));
	ZeroMemory(mswHotKey, MAX_HOTKEY_LENGTH * sizeof(wchar_t));
}


CEvMenuItem::~CEvMenuItem(void)
{
	CMM_SAFE_RELEASE(mpoTextImage);

	CMM_SAFE_RELEASE(mpoLeftImage);

	CMM_SAFE_RELEASE(mpoRightImage);

	CMM_SAFE_RELEASE(mpoSeparation);

	CMM_SAFE_RELEASE(mpoCheckedImage);

	CMM_SAFE_RELEASE(mpoHotKeyImage);

	CMM_SAFE_RELEASE(mpoFocusHotKeyImage);

	CMM_SAFE_RELEASE(mpoFocusTextImage);

	CMM_SAFE_DELETE(mpoMenuItemInfo);

	mpoItemBgBmp = NULL;
}


ULONG CEvMenuItem::InitOnCreate(
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

		leResult = ERESULT_SUCCESS;
	} while (false);


	return leResult;
}
//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvMenuItem::OnElementCreate(IEinkuiIterator* npIterator)
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

//ERESULT CEvMenuItem::OnElementDestroy()
//{
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	return lResult;
//}
//
//绘制消息
ERESULT CEvMenuItem::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);
		if (NULL == mpoMenuItemInfo)
			break;

		// 分割线
		if (ENUM_MENUITEM_TYPE_SEPARATION == meMemuItemType)
		{
			if (mpoSeparation == NULL)
				break;

			// 显示分割线
			npPaintBoard->DrawBitmap(D2D1::RectF((FLOAT)mpoMenuItemInfo->LeftWidth, 0,
				(FLOAT)(mpIterator->GetSizeX()), (FLOAT)mpIterator->GetSizeY()),
				mpoSeparation, ESPB_DRAWBMP_EXTEND);

			lResult = ERESULT_SUCCESS;
			break;
		}


		FLOAT lfL = mpIterator->GetSizeX();
		// 显示背景
		if (mpoItemBgBmp != NULL)
		{
			// 如果当前获得了鼠标焦点，则用第二帧做背景图
			FLOAT lfOrgX = (FLOAT)(false != mbIsFocused ? CExFloat::Round((FLOAT)mpoItemBgBmp->GetWidth() / miItemFrameCount) : 0);
			npPaintBoard->DrawBitmap(D2D1::RectF(0, 0, (FLOAT)mpIterator->GetSizeX(), (FLOAT)mpIterator->GetSizeY()),
				D2D1::RectF(lfOrgX, 0, CExFloat::Round(lfOrgX + mpoItemBgBmp->GetWidth() / miItemFrameCount), (FLOAT)(mpoItemBgBmp->GetHeight())),
				mpoItemBgBmp, ESPB_DRAWBMP_EXTEND);
		}

		// 非割线类型
		if (ENUM_MENUITEM_TYPE_SEPARATION != meMemuItemType)
		{
			// 显示名称
			if (NULL != mpoTextImage)
			{
				FLOAT lfX = (FLOAT)mpoMenuItemInfo->LeftWidth;
				FLOAT lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoTextImage->GetHeight()) / 2.0f);		// 垂直居中显示
				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY,
					(FLOAT)mpoTextImage->GetWidth() + lfX, (FLOAT)mpoTextImage->GetHeight() + lfY),
					false != mbIsFocused ? mpoFocusTextImage : mpoTextImage, ESPB_DRAWBMP_NEAREST);
			}

			// 显示热键
			if (NULL != mpoHotKeyImage)
			{
				FLOAT lfX = (FLOAT)(mpoMenuItemInfo->LeftWidth + miNameAreaWidth + mpoMenuItemInfo->MiddleWidth
					+ miHotKeyAreaWidth - mpoHotKeyImage->GetWidth());
				FLOAT lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoHotKeyImage->GetHeight()) / 2.0f);		// 垂直居中显示
				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY,
					(FLOAT)mpoHotKeyImage->GetWidth() + lfX, (FLOAT)mpoHotKeyImage->GetHeight() + lfY),
					false != mbIsFocused ? mpoFocusHotKeyImage : mpoHotKeyImage, ESPB_DRAWBMP_NEAREST);
			}



			// 下面要根据不同的类型显示不同的类容
			FLOAT lfX = 0.0f;
			FLOAT lfY = 0.0f;
			FLOAT lfRight = 0.0f;
			FLOAT lfBottom = 0.0f;
			// 有图标风格
			if (ENUM_MENUITEM_TYPE_ICON == meMemuItemType
				&& NULL != mpoLeftImage)
			{
				lfX = CExFloat::Round((mpoMenuItemInfo->LeftWidth - mpoMenuItemInfo->LeftIconWidth) / 2.0f);
				lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoMenuItemInfo->LeftIconHeight) / 2.0f);
				lfRight = (FLOAT)mpoMenuItemInfo->LeftIconWidth + lfX;
				lfBottom = (FLOAT)mpoMenuItemInfo->LeftIconHeight + lfY;
				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY, lfRight, lfBottom),
					mpoLeftImage, ESPB_DRAWBMP_EXTEND);
			}
			else if (ENUM_MENUITEM_TYPE_CHECK == meMemuItemType
				&& NULL != mpoLeftImage && NULL != mpoCheckedImage)
			{
				lfX = CExFloat::Round((mpoMenuItemInfo->LeftWidth - mpoMenuItemInfo->LeftIconWidth) / 2.0f);
				lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoMenuItemInfo->LeftIconHeight) / 2.0f);
				lfRight = (FLOAT)mpoMenuItemInfo->LeftIconWidth + lfX;
				lfBottom = (FLOAT)mpoMenuItemInfo->LeftIconHeight + lfY;

				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY, lfRight, lfBottom),
					false != mbIsChecked ? mpoCheckedImage : mpoLeftImage, ESPB_DRAWBMP_EXTEND);
			}
			else if (ENUM_MENUITEM_TYPE_EXTEND == meMemuItemType
				&& NULL != mpoRightImage)
			{
				lfX = (FLOAT)(mpoMenuItemInfo->LeftWidth + miNameAreaWidth + mpoMenuItemInfo->MiddleWidth + miHotKeyAreaWidth
					+ CExFloat::Round((mpoMenuItemInfo->RightWidth - mpoMenuItemInfo->RightIconWidth) / 2.0f));
				lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoMenuItemInfo->RightIconHeight) / 2.0f);
				lfRight = (FLOAT)mpoMenuItemInfo->RightIconWidth + lfX;
				lfBottom = (FLOAT)mpoMenuItemInfo->RightIconHeight + lfY;


				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY, lfRight, lfBottom),
					mpoRightImage, ESPB_DRAWBMP_EXTEND);
			}
			else if (ENUM_MENUITEM_TYPE_APPLE_EXTEND == meMemuItemType
				&& NULL != mpoRightImage)
			{
				lfX = (FLOAT)(mpoMenuItemInfo->LeftWidth + miNameAreaWidth + mpoMenuItemInfo->MiddleWidth
					+ miHotKeyAreaWidth - mpoMenuItemInfo->RightIconWidth);
				lfY = CExFloat::Round((mpIterator->GetSizeY() - mpoMenuItemInfo->RightIconHeight) / 2.0f);
				lfRight = (FLOAT)mpoMenuItemInfo->RightIconWidth + lfX;
				lfBottom = (FLOAT)mpoMenuItemInfo->RightIconHeight + lfY;

				npPaintBoard->DrawBitmap(D2D1::RectF(lfX, lfY, lfRight, lfBottom),
					mpoRightImage, ESPB_DRAWBMP_EXTEND);
			}

		}
		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

void CEvMenuItem::LoadResource()
{
	bool lbResult = false;
	do
	{
		if (NULL == mpTemplete)
			break;

		if (NULL == mpoMenuItemInfo)
			break;

		// 读取ID
		miCommandID = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM_COMMAND_ID, -1);

		// 读取类型
		int liType = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM_TYPE, 1);
		meMemuItemType = (ENUM_MENUITEM_TYPE)liType;

		if (ENUM_MENUITEM_TYPE_SEPARATION == meMemuItemType)			// 分割线
		{
			wchar_t* lswSeparationImagePath = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_SEPARATION);
			if (NULL != lswSeparationImagePath)
			{
				mpoSeparation = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lswSeparationImagePath, lswSeparationImagePath[0] != L'.' ? true : false);
				mpTemplete->ReleaseBuffer(&lswSeparationImagePath);

				// 如果是分割线，则要以分割线的高度作为项高度
				mpoMenuItemInfo->ItemHeight = (int)mpoSeparation->GetHeight();
			}

			lbResult = true;
			break;
		}

		// 其他类型共有属性
		FLOAT lfHeight = 0;
		if (NULL != mpoItemBgBmp)
		{
			lfHeight = (FLOAT)mpoItemBgBmp->GetHeight();
		}

		if (meMemuItemType == ENUM_MENUITEM_TYPE_ICON
			|| meMemuItemType == ENUM_MENUITEM_TYPE_CHECK)
		{
			// 读取左图标
			wchar_t* lswLeftImage = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_LEFT_ICON);
			if (NULL != lswLeftImage)
			{
				mpoLeftImage = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lswLeftImage, lswLeftImage[0] == L'.' ? false : true);
				mpTemplete->ReleaseBuffer(&lswLeftImage);

				if (-1 == mpoMenuItemInfo->LeftIconWidth)
				{
					mpoMenuItemInfo->LeftIconWidth = mpoLeftImage->GetWidth();
				}
				if (-1 == mpoMenuItemInfo->LeftIconHeight)
				{
					mpoMenuItemInfo->LeftIconHeight = mpoLeftImage->GetHeight();
				}
			}


			// 读取Checked态图标
			if (meMemuItemType == ENUM_MENUITEM_TYPE_CHECK)
			{
				wchar_t* lswCheckedImage = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_LEFT_CHECKED_ICON);
				if (NULL != lswCheckedImage)
				{
					mpoCheckedImage = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lswCheckedImage, lswCheckedImage[0] == L'.' ? false : true);
					mpTemplete->ReleaseBuffer(&lswCheckedImage);
				}
			}
		}
		else if (meMemuItemType == ENUM_MENUITEM_TYPE_APPLE_EXTEND ||
			meMemuItemType == ENUM_MENUITEM_TYPE_EXTEND)
		{
			// 读取右图标
			wchar_t* lswRightImage = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_RIGHT_ICON);
			if (NULL != lswRightImage)
			{
				mpoRightImage = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lswRightImage, lswRightImage[0] == L'.' ? false : true);

				mpTemplete->ReleaseBuffer(&lswRightImage);

				if (-1 == mpoMenuItemInfo->RightIconWidth)
				{
					mpoMenuItemInfo->RightIconWidth = mpoRightImage->GetWidth();
				}
				if (-1 == mpoMenuItemInfo->RightIconHeight)
				{
					mpoMenuItemInfo->RightIconHeight = mpoRightImage->GetHeight();
				}
			}

			// 读取弹出菜单ID
			miExtendMenuID = (int)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM_POPUPMENU_ID, -1);


			// 读取周期
			miDuration = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM_DURATION, DEFAULT_TIMER_DURATION);

		}

		// 读取菜单名称
		wchar_t* lswMenuText = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_TEXT);
		long llResult = LoadTextImage(lswMenuText);
		mpTemplete->ReleaseBuffer(&lswMenuText);
		if (-1 != llResult && llResult > lfHeight)
			lfHeight = (FLOAT)llResult;


		// 设置菜单热键
		wchar_t* lswHotKey = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_HOTKEY);
		if (NULL != lswHotKey && UNICODE_NULL != lswHotKey[0])
		{
			STCTL_MENUITEM_HOTKEY ldHotKeyInfo;
			RtlZeroMemory(&ldHotKeyInfo, sizeof(STCTL_MENUITEM_HOTKEY));
			StringCchCopyW(ldHotKeyInfo.HotKeyToShow, MAX_PATH, lswHotKey);
			mpTemplete->ReleaseBuffer(&lswHotKey);

			// VirtualKey
			ldHotKeyInfo.VirtualKey = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM_HOTKEY_VIRTUAL_KEY, -1);
			if (-1 != ldHotKeyInfo.VirtualKey)
			{
				// Ctrl
				ICfKey* lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_CTRL_KEY);
				if (NULL != lpoKey)
					ldHotKeyInfo.NeedCtrl = true;

				// Shift
				CMM_SAFE_RELEASE(lpoKey);
				lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_SHAFT_KEY);
				if (NULL != lpoKey)
					ldHotKeyInfo.NeedShift = true;

				// Alt
				CMM_SAFE_RELEASE(lpoKey);
				lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_ALT_KEY);
				if (NULL != lpoKey)
					ldHotKeyInfo.NeedAlt = true;

				CMM_SAFE_RELEASE(lpoKey);

				llResult = LoadHotkeyImage(&ldHotKeyInfo);
				if (-1 != llResult && llResult > lfHeight)
					lfHeight = (FLOAT)llResult;
			}
		}

		// 设置菜单项高度
		if (-1 == mpoMenuItemInfo->ItemHeight)
			mpoMenuItemInfo->ItemHeight = (int)lfHeight;

		lbResult = true;
	} while (false);


	if (false == lbResult)
	{
		PrintDebugString(L"MenuItem_LoadResource 失败.");
	}
}

// 加载子菜单列表
void CEvMenuItem::LoadSubPopupMenu()
{
	if (-1 == miExtendMenuID)
	{
		return;
	}

	// 先判断父窗口是否存在
	if (NULL == mpIterator->GetParent())
		return;

	// 查找子菜单模板，并创建
	ICfKey* lpoPopupMenuKey = mpTemplete->GetParentsKey();
	while (NULL != lpoPopupMenuKey->GetParentsKey())
		lpoPopupMenuKey = lpoPopupMenuKey->GetParentsKey();
	lpoPopupMenuKey = lpoPopupMenuKey->GetSubKey(L"PopupMenu");
	if (NULL == lpoPopupMenuKey)
		return;

	// 找寻出ID为miExtendMenuID的子菜单
	lpoPopupMenuKey = lpoPopupMenuKey->MoveToSubKey();
	while (NULL != lpoPopupMenuKey)
	{
		if (miExtendMenuID == lpoPopupMenuKey->QuerySubKeyValueAsLONG(TF_ID_POPUPMENU_MAIN_ID, -1))
			break;
		else
			lpoPopupMenuKey = lpoPopupMenuKey->MoveToNextKey();
	}

	if (NULL == lpoPopupMenuKey)
		return;

	mpoSubPopupMenu = CEvPopupMenu::CreateInstance(mpIterator, lpoPopupMenuKey, 0);
	if (NULL == mpoSubPopupMenu)
		return;

	mpoSubPopupMenu->GetIterator()->SetPosition(mpIterator->GetSizeX() - 5, 0);
	mpoSubPopupMenu->GetIterator()->SetVisible(false);
}


// 描述：
//		设置初始化结构体的指针（这里之所以说是设置指针，是为了表示不需要去释放）
void CEvMenuItem::SetMenuItemInfo(IN PST_MENUITEM_INFO npoMenuItemInfo)
{
	CMM_SAFE_DELETE(mpoMenuItemInfo);
	mpoMenuItemInfo = new ST_MENUITEM_INFO;
	// CheckMarx fix by zhuhl5
	memcpy_s(mpoMenuItemInfo, sizeof(ST_MENUITEM_INFO), npoMenuItemInfo, sizeof(ST_MENUITEM_INFO));

	LoadResource();

}

// 鼠标按下
ERESULT CEvMenuItem::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		// 鼠标左键弹起
		if (npInfo->ActKey & MK_LBUTTON
			&& false == npInfo->Presssed && false != mpIterator->IsEnable())
		{
			if (ENUM_MENUITEM_TYPE_NORMAL == meMemuItemType
				|| ENUM_MENUITEM_TYPE_ICON == meMemuItemType
				|| ENUM_MENUITEM_TYPE_CHECK == meMemuItemType)
			{
				// 向父窗口发送消息
				PostMessageToParent(EEVT_MENUITEM_CLICK, miCommandID);

				// 恢复正常显示状态
				mbIsFocused = false;
			}
		}

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

ERESULT CEvMenuItem::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EEVT_MENUITEM_CLICK:
	{
		this->GetIterator()->PostMessageToParent(npMsg);

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_MENUITEM_SET_CHECK_STATE:
	{
		if (ENUM_MENUITEM_TYPE_CHECK != meMemuItemType)
			break;

		if (npMsg->GetInputDataSize() != sizeof(bool))
			break;
		mbIsChecked = *(bool*)npMsg->GetInputData();

		// 读取菜单名称和热键
		wchar_t* lswMenuText = NULL;
		if (false == mbIsChecked)
		{
			lswMenuText = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_TEXT);
		}
		else
		{
			lswMenuText = (wchar_t*)mpTemplete->QuerySubKeyValueAsBuffer(TF_ID_MENUITEM_CHECKED_TEXT);
		}
		long llResult = LoadTextImage(lswMenuText);
		if (-1 == mpoMenuItemInfo->ItemHeight
			&& mpIterator->GetSizeY() < (FLOAT)llResult)
		{
			mpoMenuItemInfo->ItemHeight = llResult;
			mpIterator->SetSize(mpIterator->GetSizeX(), (FLOAT)llResult);
		}

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_MENUITEM_CHANGE_TEXT:
	{
		//if(npMsg->GetInputDataSize() != sizeof(wchar_t*) )
			//break;

		wchar_t* lswMenuText = (wchar_t*)npMsg->GetInputData();

		// 设置菜单名称
		long llResult = LoadTextImage(lswMenuText);
		if (-1 == llResult)
			break;
		if (-1 == mpoMenuItemInfo->ItemHeight
			&& mpIterator->GetSizeY() < (FLOAT)llResult)
		{
			mpoMenuItemInfo->ItemHeight = llResult;
			mpIterator->SetSize(mpIterator->GetSizeX(), (FLOAT)llResult);
		}

		luResult = ERESULT_SUCCESS;
	}
	break;

	case EACT_MENUITEM_CHANGE_HOTKEY:
	{
		PSTCTL_MENUITEM_HOTKEY lpdHotKeyInfo = NULL;
		if (ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lpdHotKeyInfo))
		{
			luResult = ERESULT_UNSUCCESSFUL;
			break;
		}

		// 设置热键
		long llResult = LoadHotkeyImage(lpdHotKeyInfo);
		if (-1 == llResult)
			break;
		if (-1 == mpoMenuItemInfo->ItemHeight
			&& mpIterator->GetSizeY() < (FLOAT)llResult)
		{
			mpoMenuItemInfo->ItemHeight = llResult;
			mpIterator->SetSize(mpIterator->GetSizeX(), (FLOAT)llResult);
		}

		luResult = ERESULT_SUCCESS;
	}
	break;

	//mpIterator->SetToolTip(L"ksdfjlsdjkf");

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

//定时器
void CEvMenuItem::OnTimer(
	PSTEMS_TIMER npStatus
)
{
	if (npStatus->TimerID == TIMER_ID_SHOW_POPUPMENU)
	{
		if (NULL != mpoSubPopupMenu)
		{
			// 隐藏的时候要确保当前鼠标不在该菜单项所在的PopupMenu上
			mpoSubPopupMenu->GetIterator()->SetVisible(mbIsSubMenuShow);
			EinkuiGetSystem()->UpdateView();
		}
	}
}

//鼠标进入或离开
void CEvMenuItem::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	if (ENUM_MENUITEM_TYPE_SEPARATION == meMemuItemType || false == mpIterator->IsEnable())
		return;

	// 对于所有非分割线类型的菜单项
	// 只要鼠标进入，则进入焦点态。
	if (0 != npState->State)
	{
		mbIsFocused = true;

		// 同时通知PopupMenu，去夺取其他项的焦点
		if (NULL != mpIterator->GetParent()
			&& false != mpIterator->GetParent()->GetElementObject()->IsKindOf(GET_BUILTIN_NAME(PopupMenu))
			)
		{
			PostMessageToParent(EEVT_MENUITEM_GET_FOCUS, CExMessage::DataInvalid);

			// 与此同时，若当前菜单项对应的PopupMenu是一个二级菜单，则需要设置该PopupMenu父窗口为获取焦点状态
			IEinkuiIterator* lpPopupMenuParent = mpIterator->GetParent()->GetParent();
			if (NULL != lpPopupMenuParent
				&& false != lpPopupMenuParent->GetElementObject()->IsKindOf(GET_BUILTIN_NAME(MenuItem))
				)
			{
				STEMS_STATE_CHANGE ldStateChange;
				ldStateChange.Related = NULL;
				ldStateChange.State = 1;

				// 发送消息给对应的父MenuItem，设定其为选中状态。
				EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
					lpPopupMenuParent, EMSG_MOUSE_FOCUS, &ldStateChange, sizeof(STEMS_STATE_CHANGE));
			}
		}
	}
	// 否则鼠标离开了并且不是被子孙所夺取时，都失去焦点
	else if (NULL != npState->Related && false == npState->Related->FindAncestor(mpIterator))
	{
		mbIsFocused = false;
	}

	// 如果当前菜单项为扩展类型
	if (ENUM_MENUITEM_TYPE_EXTEND == meMemuItemType || ENUM_MENUITEM_TYPE_APPLE_EXTEND == meMemuItemType)
	{
		// 确保子菜单已经创建
		if (NULL == mpoSubPopupMenu)
		{
			LoadSubPopupMenu();
			if (NULL == mpoSubPopupMenu)
				return;
		}

		// 当鼠标进入的时候
		if (0 != npState->State)
		{
			// 首先要关闭定时器，避免鼠标刚离开就进入时，开启的隐藏子菜单的定时器
			mpIterator->KillTimer(TIMER_ID_SHOW_POPUPMENU);

			// 如果子菜单是隐藏状态，则开启定时器用来显示子菜单。
			if (false == mpoSubPopupMenu->GetIterator()->IsVisible())
			{
				// 启动定时器，打开子菜单
				mbIsSubMenuShow = true;
				mpIterator->SetTimer(TIMER_ID_SHOW_POPUPMENU, 1, miDuration, NULL);
			}

		}
		else		// 鼠标离开的时候
		{
			// 首先启动定时器，以避免刚才进入该项时开启的显示子菜单的定时器还在运行
			mpIterator->KillTimer(TIMER_ID_SHOW_POPUPMENU);

			// 若夺取焦点的不是当前项的子孙，则启动定时器，隐藏二级菜单。
			if (NULL != npState->Related && false == npState->Related->FindAncestor(mpIterator))
			{
				mbIsSubMenuShow = false;
				// 子菜单是显示状态，
				if (false != mpoSubPopupMenu->GetIterator()->IsVisible())
					mpIterator->SetTimer(TIMER_ID_SHOW_POPUPMENU, 1, miDuration, NULL);
			}
			else			// 是被子孙夺取了焦点，则仍然显示选中状态
			{
				mbIsFocused = true;
			}
		}
	}

	EinkuiGetSystem()->UpdateView();
}

// 鼠标落点检测
ERESULT CEvMenuItem::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if (!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX() || rPoint.y < 0.0f || rPoint.y >= mpIterator->GetSizeY()))
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}

	return luResult;
}

//禁用或启用
ERESULT CEvMenuItem::OnElementEnable(bool nbIsEnable)
{
	CMM_SAFE_RELEASE(mpoTextImage);
	if (GetMenuItemType() != ENUM_MENUITEM_TYPE_SEPARATION)
	{
		//构建结构体
		STETXT_BMP_INIT ldInit;
		ZeroMemory(&ldInit, sizeof(STETXT_BMP_INIT));
		ldInit.FontName = mpoMenuItemInfo->FontName;
		ldInit.FontSize = mpoMenuItemInfo->FontSize;

		if (false != nbIsEnable)
		{
			ldInit.TextColor = mpoMenuItemInfo->FontColor;
		}
		else
		{
			ldInit.TextColor = mpoMenuItemInfo->FontDisableColor;
		}

		ldInit.Text = mswMenuTittle;
		CMM_SAFE_RELEASE(mpoTextImage);
		mpoTextImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldInit);

		if (mswHotKey[0] != UNICODE_NULL)
		{
			ldInit.Text = mswHotKey;
			CMM_SAFE_RELEASE(mpoHotKeyImage);
			mpoHotKeyImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldInit);
		}
	}

	return ERESULT_SUCCESS;
}

//鼠标悬停
ERESULT CEvMenuItem::OnMouseHover()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (GetMenuItemType() == ENUM_MENUITEM_TYPE_SEPARATION)
			break;

		STCTL_MENUITEM_MOUSE_HOVER ldInfo;
		ldInfo.CommandID = miCommandID;
		ldInfo.MenuItemIter = mpIterator;
		PostMessageToParent(EEVT_MENUITEM_MOUSE_HOVER, ldInfo);

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//快捷键消息
ERESULT CEvMenuItem::OnHotKey(const STEMS_HOTKEY* npHotKey)
{
	if (npHotKey->HotKeyID == mlHotKeyID)
	{
		// 模拟点击消息
		// 鼠标左键弹起
		if (false != mpIterator->IsEnable())
		{
			if (ENUM_MENUITEM_TYPE_NORMAL == meMemuItemType
				|| ENUM_MENUITEM_TYPE_ICON == meMemuItemType
				|| ENUM_MENUITEM_TYPE_CHECK == meMemuItemType)
			{
				// 向父窗口发送消息
				PostMessageToParent(EEVT_MENUITEM_CLICK, miCommandID);
			}
		}
	}

	return ERESULT_KEY_UNEXPECTED;
}


// 描述：
//		加载菜单项文字图片，调用这个函数后初始化两个参数mpoTextImage poFocusTextImage
// 返回值：
//		-1 表示失败，否则返回创建的图片中最大高度
//		注意：若传入空指针或者指向空字符的指针，则会清空文字图片，返回高度为0
long CEvMenuItem::LoadTextImage(IN wchar_t* nswMenuName)
{
	long lfHeight = -1;
	do
	{
		if (NULL == nswMenuName || UNICODE_NULL == nswMenuName[0])
		{
			CMM_SAFE_RELEASE(mpoTextImage);	//去掉原来的图片
			CMM_SAFE_RELEASE(mpoFocusTextImage);	//去掉原来的图片
			lfHeight = 0;
			break;
		}

		// 解析出快捷键
		wchar_t* lswShotcutKeyBegin = wcsstr(nswMenuName, L"(&");
		if (NULL != lswShotcutKeyBegin)
		{
			wchar_t* lswShotcutKeyEnd = wcsstr(lswShotcutKeyBegin, L")");
			if (NULL != lswShotcutKeyEnd)
			{
				int liShotcutKey = toupper(*(lswShotcutKeyEnd - 1));
				if (liShotcutKey >= L'A' && liShotcutKey <= L'Z')
				{
					miShotcutKey = liShotcutKey;

					// 构造显示内容
					*(++lswShotcutKeyBegin) = liShotcutKey;
					*(++lswShotcutKeyBegin) = L')';
					*lswShotcutKeyEnd = UNICODE_NULL;
				}
			}
		}

		// 创建菜单名称图片
		STETXT_BMP_INIT ldNormalInit;
		ZeroMemory(&ldNormalInit, sizeof(STETXT_BMP_INIT));
		ldNormalInit.FontName = mpoMenuItemInfo->FontName;
		ldNormalInit.FontSize = mpoMenuItemInfo->FontSize;
		ldNormalInit.Text = nswMenuName;
		ldNormalInit.TextColor = mpoMenuItemInfo->FontColor;

		if (mpIterator->GetSizeX() > 0.0f && mpIterator->GetSizeY() > 0.0f)
		{
			//只有设置了宽高才设置这几项
			ldNormalInit.Width = (DWORD)mpIterator->GetSizeX();
			ldNormalInit.Height = (DWORD)mpIterator->GetSizeY();
			ldNormalInit.Limit = STETXT_BMP_INIT::EL_FIXEDSIZE;
		}

		CMM_SAFE_RELEASE(mpoTextImage);	//去掉原来的图片
		mpoTextImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldNormalInit);
		BREAK_ON_NULL(mpoTextImage);
		miNameAreaWidth = mpoTextImage->GetWidth();

		if (lfHeight < (long)mpoTextImage->GetHeight())
		{
			lfHeight = (long)mpoTextImage->GetHeight();
		}

		ldNormalInit.TextColor = mpoMenuItemInfo->FontFocusColor;
		CMM_SAFE_RELEASE(mpoFocusTextImage);	//去掉原来的图片
		mpoFocusTextImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldNormalInit);
		StringCchCopyW(mswMenuTittle, MAX_TITTLE_LENGTH, ldNormalInit.Text);				// 保存菜单标题
		if (lfHeight < (long)mpoFocusTextImage->GetHeight())
		{
			lfHeight = (long)mpoFocusTextImage->GetHeight();
		}

		// 发送消息给父窗口，要求重新布局
		PostMessageToParent(EACT_POPUPMENU_RELAYOUT_MENUITEM, CExMessage::DataInvalid);

	} while (false);

	// 如果除了背景外没有可显示的内容，则隐藏该项
	if (NULL == mpoFocusTextImage && NULL == mpoTextImage
		&& NULL == mpoHotKeyImage && NULL == mpoFocusHotKeyImage
		&& NULL == mpoLeftImage && NULL == mpoCheckedImage
		&& NULL == mpoRightImage)
	{
		mpIterator->SetVisible(false);
	}

	return lfHeight;
}


// 描述：
//		加载菜单项热键图片，调用这个函数后初始化两个参数mpoHotKeyImage poFocusHotKeyImage
// 返回值：
//		-1 表示失败，否则返回创建的图片中最大高度
//		注意：若传入空指针或者指向空字符的指针，则会清空热键图片，返回高度为0
long CEvMenuItem::LoadHotkeyImage(IN PSTCTL_MENUITEM_HOTKEY npdHotKeyInfo)
{
	long lfHeight = -1;
	do
	{
		BREAK_ON_NULL(mpoTextImage);

		if (NULL == npdHotKeyInfo->HotKeyToShow || UNICODE_NULL == npdHotKeyInfo->HotKeyToShow[0])
		{
			CMM_SAFE_RELEASE(mpoHotKeyImage);
			CMM_SAFE_RELEASE(mpoFocusHotKeyImage);
			lfHeight = 0;
			break;
		}

		// 必须有虚拟键值
		if (0 == npdHotKeyInfo->VirtualKey)
			break;

		// 创建热键图片
		STETXT_BMP_INIT ldNormalInit;
		ZeroMemory(&ldNormalInit, sizeof(STETXT_BMP_INIT));
		ldNormalInit.FontName = mpoMenuItemInfo->FontName;
		ldNormalInit.FontSize = mpoMenuItemInfo->FontSize;
		ldNormalInit.TextColor = mpoMenuItemInfo->FontColor;
		ldNormalInit.Text = npdHotKeyInfo->HotKeyToShow;

		CMM_SAFE_RELEASE(mpoHotKeyImage);
		mpoHotKeyImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldNormalInit);

		miHotKeyAreaWidth = mpoHotKeyImage->GetWidth();

		if (lfHeight < (long)mpoHotKeyImage->GetHeight())
		{
			lfHeight = (long)mpoHotKeyImage->GetHeight();
		}

		// 创建焦点态热键图片
		ldNormalInit.TextColor = mpoMenuItemInfo->FontFocusColor;
		CMM_SAFE_RELEASE(mpoFocusHotKeyImage);
		mpoFocusHotKeyImage = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldNormalInit);

		// 记录菜单热键
		StringCchCopyW(mswHotKey, MAX_HOTKEY_LENGTH, ldNormalInit.Text);

		if (lfHeight < (long)mpoFocusHotKeyImage->GetHeight())
		{
			lfHeight = (long)mpoFocusHotKeyImage->GetHeight();
		}

		// 发送消息给父窗口，要求重新布局
		PostMessageToParent(EACT_POPUPMENU_RELAYOUT_MENUITEM, CExMessage::DataInvalid);

		// 新注册热键
		// 必须有虚拟键值
		mlHotKeyID = npdHotKeyInfo->VirtualKey;

		// Ctrl
		if (false != npdHotKeyInfo->NeedCtrl)
		{
			mlHotKeyID |= MENUITEM_HOTKEY_CTRL;
		}

		// Shift
		if (false != npdHotKeyInfo->NeedShift)
		{
			mlHotKeyID |= MENUITEM_HOTKEY_SHIFT;
		}

		// Alt
		if (false != npdHotKeyInfo->NeedAlt)
		{
			mlHotKeyID |= MENUITEM_HOTKEY_ATL;
		}

		// 向框架注册全局热键
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(mpIterator, mlHotKeyID,
			npdHotKeyInfo->VirtualKey, npdHotKeyInfo->NeedCtrl, npdHotKeyInfo->NeedShift, npdHotKeyInfo->NeedAlt, NULL);



		//// 注册备用热键，允许失败
		ICfKey* lpoKey = this->mpTemplete->OpenKey(TF_ID_MENUITEM_RESERVE_HOTKEY);
		if (NULL != lpoKey)
		{
			int liReverseVk = this->mpTemplete->QuerySubKeyValueAsLONG(TF_ID_MENUITEM__RESERVE_HOTKEY_VIRTUAL_KEY, -1);
			if (-1 != liReverseVk)
			{
				// Ctrl
				CMM_SAFE_RELEASE(lpoKey);
				lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_CTRL_KEY);
				bool lbIsNeedCtrl = NULL != lpoKey ? true : false;

				// Shift
				CMM_SAFE_RELEASE(lpoKey);
				lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_SHAFT_KEY);
				bool lbIsNeedShift = NULL != lpoKey ? true : false;

				// Alt
				CMM_SAFE_RELEASE(lpoKey);
				lpoKey = mpTemplete->OpenKey(TF_ID_MENUITEM_HOTKEY_ALT_KEY);
				bool lbIsNeedAlt = NULL != lpoKey ? true : false;

				// 向框架注册全局热键
				bool lbIsOk = EinkuiGetSystem()->GetElementManager()->RegisterHotKey(mpIterator, mlHotKeyID,
					liReverseVk, lbIsNeedCtrl, lbIsNeedShift, lbIsNeedAlt, NULL);

				lbIsNeedAlt = false;
			}

			CMM_SAFE_RELEASE(lpoKey);
		}



	} while (false);

	// 如果除了背景外没有可显示的内容，则隐藏该项
	if (NULL == mpoFocusTextImage && NULL == mpoTextImage
		&& NULL == mpoHotKeyImage && NULL == mpoFocusHotKeyImage
		&& NULL == mpoLeftImage && NULL == mpoCheckedImage
		&& NULL == mpoRightImage)
	{
		mpIterator->SetVisible(false);
	}

	return lfHeight;
}


// 描述：
//		隐藏级联菜单
void CEvMenuItem::HideCascadeMenu()
{
	if (NULL != mpoSubPopupMenu)
		mpoSubPopupMenu->GetIterator()->SetVisible(false);
}
