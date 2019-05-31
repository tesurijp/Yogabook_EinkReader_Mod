/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */



#include "stdafx.h"
#include <CommDlg.h>		//调色板
#include "cmmstruct.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvToolBar.h"
#include "CommandIDDef.h"

#include "..\DesignerEnviromentMgr\DesignerEnviromentMgr.h"
using namespace LT_ENVIROMENT_MGR;
#pragma comment(lib,"designerEnvironmentmgr.lib")

//定义子Pane ID

#define ID_PANE_A	1
#define ID_PANE_B	2
#define ID_PANE_C	3
#define ID_PANE_D	4
#define ID_PANE_E	5
#define ID_PANE_F	6


DEFINE_BUILTIN_NAME(ToolBar)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvToolBar::CEvToolBar() :
	mbChangeFontColor(false),
	mbChangeFontBkgColor(false)
{	
}

CEvToolBar::~CEvToolBar() 
{
}

ULONG CEvToolBar::InitOnCreate(
	IN IXuiIterator* npParent,	// 父对象指针
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

		//mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL|EITR_STYLE_KEYBOARD);
		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvToolBar::OnElementCreate(IXuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//LoadResource();

		//mpIterator->ModifyStyles(EITR_STYLE_TOPDRAW);

		SetCtrlDefaultValue();

		lResult = ERESULT_SUCCESS;
	} while(false);

	return lResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvToolBar::ParseMessage(IXuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		case EEVT_TOOLBARITEM_CLICK:
			{
			//	if(npMsg->GetInputDataSize()!=sizeof(TOOLBAR_MSG) || npMsg->GetInputData()==NULL)
			//		luResult = ERESULT_WRONG_PARAMETERS;
			//	TOOLBAR_MSG MsgInfo = *(TOOLBAR_MSG*)(npMsg->GetInputData());

				TOOLBAR_MSG* lpInfoInput;
				TOOLBAR_MSG ldInfo;
				luResult = CExMessage::GetInputDataBuffer(npMsg,lpInfoInput);
				if(luResult != ERESULT_SUCCESS)
					break;
				RtlCopyMemory(&ldInfo,lpInfoInput,sizeof(ldInfo));
				ProcessMsg(&ldInfo);

				break;
			//}

		//case EEVT_UPDATE_UI:
		//	{
		//		int lID = 0;

		//		lID = 0x00200100;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_A),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);

		//		lID = 0x00200200;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_B),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);

		//		lID = 0x00200300;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_C),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);

		//		lID = 0x00200400;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_D),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);

		//		lID = 0x00200500;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_E),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);

		//		lID = 0x00200600;
		//		CExMessage::PostMessage(
		//			mpIterator->GetSubElementByID(ID_PANE_F),mpIterator,EEVT_UPDATE_PANE, lID,EMSG_POSTTYPE_REDUCE);
		//		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
		//			mpIterator->GetParent(), EEVT_TOOLBARITEM_CLICK, &MsgInfo, sizeof(TOOLBAR_MSG)
		//			, NULL, 0);

		//		break;
			}

		case EEVT_UPDATE_UI:
			{
				int lID = 0;

				lID = 0x00200100;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_A), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);

				lID = 0x00200200;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_B), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);
				
				lID = 0x00200300;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_C), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);
				
				lID = 0x00200400;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_D), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);

				lID = 0x00200500;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_E), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);
			
				lID = 0x00200600;
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpIterator->GetSubElementByID(ID_PANE_F), EEVT_UPDATE_PANE, &lID, sizeof(int), NULL, 0);
			}
			break;

		default:
			luResult = ERESULT_NOT_SET;
		}

		if(luResult == ERESULT_NOT_SET)
		{
			luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		}

	} while (false);

	return luResult;
}


//绘制消息
ERESULT CEvToolBar::OnPaint(IXuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
			mpBgBitmap,
			ESPB_DRAWBMP_EXTEND);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

bool CEvToolBar::ProcessMsg(TOOLBAR_MSG* npMsgInfo)
{
	switch (npMsgInfo->mnCtrlID)
	{
	case ID_TOOLBAR_FONT_COLOR_SELECT:
		{
			OpenChooseFontColorWnd();
			mulFontColor |= 0xFF000000;
			npMsgInfo->muColor = mulFontColor;
			//颜色选择控件，转换消息类型为整数
			npMsgInfo->mnMsgParaType = TMPT_INT;

			if (mbChangeFontColor)
			{
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,
					EEVT_TOOLBARITEM_CLICK, *npMsgInfo, EMSG_POSTTYPE_FAST);
			}
		}
		break;
	case ID_TOOLBAR_FONT_BACK_COLOR_SELECT:
		{
			if (npMsgInfo->mlInterge == 1)
			{
				//无颜色
				mluFontBkgColor = 0;
				EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
					mpIterator->GetSubElementByID(3)->GetSubElementByID(15), 
					EACT_IMAGEBUTTON_SET_COLOR, &mluFontBkgColor, sizeof(ULONG));	

				mbChangeFontBkgColor = true;
			}
			else if (npMsgInfo->mlInterge == 2)
			{
				OpenChooseFontBkgColorWnd();
				mluFontBkgColor |= 0xFF000000;	//如果有背景色的话
			}

			if (mbChangeFontBkgColor)
			{
				npMsgInfo->mnMsgParaType = TMPT_INT;
				npMsgInfo->muColor = mluFontBkgColor;
				//颜色选择控件，转换消息类型为整数
				CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,
					EEVT_TOOLBARITEM_CLICK, *npMsgInfo, EMSG_POSTTYPE_FAST);
			}
		}
		break;

	default:
		CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,
			EEVT_TOOLBARITEM_CLICK, *npMsgInfo, EMSG_POSTTYPE_FAST);
		break;
	}

	return true;
}

UINT_PTR CALLBACK CEvToolBar::CCHookProc(          
	HWND hdlg,
	UINT uiMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{

	switch(uiMsg)
	{
	case WM_INITDIALOG:
		{
			// 获取主窗口大小
			D2D1_SIZE_F ldMainFrameSize = EinkuiGetSystem()->GetElementManager()->GetDesktop()->GetSize();
			// 设置窗口位置
			::SetWindowPos(hdlg, NULL, (int)ldMainFrameSize.width/4, (int)ldMainFrameSize.height/4, 0, 0, SW_SHOWNORMAL/*SWP_SHOWWINDOW*/);

			break;
		}
	default:
		break;

	}

	return 0;
}

ERESULT __stdcall CEvToolBar::ChooseFontColorWnd(ULONG nuFlag,LPVOID npContext)
{
#define MAX_CUSTOM_COLOR_NUM 16

	ERESULT lhr = ERESULT_SUCCESS;

	CHOOSECOLOR		cc;
	COLORREF		crCustColors[16];

	// 通过环境变量管理器，获取自定义颜色
	IEnviromentMgr* lpEnvMgr = GetEnviromentMgr();
	sDesignerEnviroment* lpdDesignEnv = lpEnvMgr->GetDesignerEnviroment();
	if (NULL == lpdDesignEnv) return lhr;

	for (int liLoop = 0; liLoop < MAX_CUSTOM_COLOR_NUM; liLoop++)
		crCustColors[liLoop] = lpdDesignEnv->muCustColorArr[liLoop];

	cc.lStructSize		= sizeof(CHOOSECOLOR);
	cc.hwndOwner		= EinkuiGetSystem()->GetMainWindow();
	cc.hInstance		= NULL;
	cc.rgbResult		= RGB(0x80, 0x80, 0x80);
	cc.lpCustColors		= crCustColors;
	cc.Flags			= CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
	cc.lCustData		= 0;
	cc.lpfnHook			= (LPCCHOOKPROC)CEvToolBar::CCHookProc;
	cc.lpTemplateName	= NULL;

	BOOL lbOK = ChooseColor(&cc);

	if (TRUE == lbOK)
	{
		mulFontColor = cc.rgbResult;
		mbChangeFontColor = true;

		mulFontColor |= 0xFF000000;
		EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
			mpIterator->GetSubElementByID(3)->GetSubElementByID(16), 
			EACT_IMAGEBUTTON_SET_COLOR, &mulFontColor, sizeof(ULONG));

		// 重新设置自定义颜色到环境变量管理器
		for (int liLoop = 0; liLoop < MAX_CUSTOM_COLOR_NUM; liLoop++)
			lpdDesignEnv->muCustColorArr[liLoop] = crCustColors[liLoop];
		// 调用保存
		lpEnvMgr->SaveEnviroment();
	}
	else
	{
		mbChangeFontColor = false;
	}

	return lhr;
}
BOOL CEvToolBar::OpenChooseFontColorWnd()
{
	ERESULT lhr = EinkuiGetSystem()->CallBackByWinUiThread(this, 
		(ERESULT (__stdcall IBaseObject::*)(ULONG nuFlag,LPVOID npContext))&CEvToolBar::ChooseFontColorWnd,
		0, NULL);

	return TRUE;
}

ERESULT __stdcall CEvToolBar::ChooseFontBkgColorWnd(ULONG nuFlag,LPVOID npContext)
{
	ERESULT lhr = ERESULT_SUCCESS;

	CHOOSECOLOR		cc;
	COLORREF		crCustColors[16];

	cc.lStructSize		= sizeof(CHOOSECOLOR);
	cc.hwndOwner		= EinkuiGetSystem()->GetMainWindow();
	cc.hInstance		= NULL;
	cc.rgbResult		= RGB(0x80, 0x80, 0x80);
	cc.lpCustColors		= crCustColors;
	cc.Flags			= CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
	cc.lCustData		= 0;
	cc.lpfnHook			= (LPCCHOOKPROC)CEvToolBar::CCHookProc;
	cc.lpTemplateName	= NULL;

	BOOL lbOK = ChooseColor(&cc);

	if (TRUE == lbOK)
	{
		mluFontBkgColor = cc.rgbResult;
		mbChangeFontBkgColor = true;

		mluFontBkgColor |= 0xFF000000;
		EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
			mpIterator->GetSubElementByID(3)->GetSubElementByID(15), 
			EACT_IMAGEBUTTON_SET_COLOR, &mluFontBkgColor, sizeof(ULONG));
	}
	else
	{
		mbChangeFontBkgColor = false;
	}
	

	return lhr;
}
BOOL CEvToolBar::OpenChooseFontBkgColorWnd()
{
	ERESULT lhr = EinkuiGetSystem()->CallBackByWinUiThread(this, 
		(ERESULT (__stdcall IBaseObject::*)(ULONG nuFlag,LPVOID npContext))&CEvToolBar::ChooseFontBkgColorWnd,
		0, NULL);

	return TRUE;
}

BOOL CEvToolBar::SetCtrlDefaultValue()
{

	return TRUE;
}
