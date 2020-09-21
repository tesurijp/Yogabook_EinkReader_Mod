


#include "stdafx.h"
#include "cmmstruct.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvToolPane.h"

#pragma warning (disable:4995) 


DEFINE_BUILTIN_NAME(ToolPane)

#define ID_TOOLBAR_ITEM_FONT		1		//	字体 （组合框）
#define ID_TOOLBAR_ITEM_FONT_SIZE	2		//	字号	（组合框）
#define ID_TOOLBAR_ITEM_SPACING		4		//	间距	（组合框）

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvToolPane::CEvToolPane() :
	mpUnificSetting(NULL)
{	
}

CEvToolPane::~CEvToolPane() 
{
}

ULONG CEvToolPane::InitOnCreate(
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

		//	初始化UnificSetting
		mpUnificSetting = GetUnificSetting();

		//mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL|EITR_STYLE_KEYBOARD);
		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvToolPane::OnElementCreate(IXuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//LoadResource();
		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		lResult = ERESULT_SUCCESS;
	} while(false);

	return lResult;
}

ERESULT CEvToolPane::OnCtlButtonClick(IXuiIterator* npSender)
{
	ULONG luCtrlId = npSender->GetID();

	ERESULT luResult = ERESULT_SUCCESS;

	/*switch (luCtrlId)
	{


	}*/

	return luResult;
}


//装载配置资源
//ERESULT CEvToolPane::LoadResource()
//{
//
//	ERESULT leResult = ERESULT_UNSUCCESSFUL;
//
//	ICfKey* lpValue = NULL;
//
//	do 
//	{
//		//获取帧信息
//		mlMaxFrame = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LIST_BACKIAMGE_FRAME_COUNT,1);
//
//		if (mlMaxFrame > 0 && mpBgBitmap != NULL)
//		{
//			//计算每帧大小
//			UINT luiWidth = mpBgBitmap->GetWidth();
//			UINT luiHeight = mpBgBitmap->GetHeight();
//			mlCurrentIndex = 0;
//
//			mpIterator->SetSize(float(luiWidth / mlMaxFrame),(float)luiHeight);
//		}
//		//获取显示模式
//
//		//获取是否不需要显示滚动条等属性
//
//		leResult = ERESULT_SUCCESS;
//
//	} while (false);
//
//	return leResult;
//}

//绘制
//ERESULT CEvToolPane::OnPaint(IXuiPaintBoard* npPaintBoard)
//{
//
//	ERESULT lResult = ERESULT_UNSUCCESSFUL;
//
//	do
//	{
//		BREAK_ON_NULL(npPaintBoard);
//
//		if (mpBgBitmap != NULL)
//		{
//			float lfX = mpIterator->GetSizeX() * mlCurrentIndex;
//
//			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
//				D2D1::RectF(lfX,0,lfX + mpIterator->GetSizeX(),mpIterator->GetSizeY()),
//				mpBgBitmap,
//				0
//				);
//		}
//		 
//	}while(false);
//
//	return lResult;
//}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvToolPane::ParseMessage(IXuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		//case EEVT_COMBOBOX_LIST_ITEM_CLICK_COMPLEX:
		//	{
		//		//TOOLBAR_MSG MsgInfo = *(TOOLBAR_MSG*)(npMsg->GetInputData());
		//
		//		//EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
		//		//	mpIterator->GetParent(), EEVT_TOOLBARITEM_CLICK, &MsgInfo, sizeof(TOOLBAR_MSG)
		//		//	, NULL, 0);
		//		TOOLBAR_MSG* lpMsgInfo;

		//		luResult = CExMessage::GetInputDataBuffer(npMsg,lpMsgInfo);
		//		if(luResult != ERESULT_SUCCESS)
		//			break;
		//		CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EEVT_TOOLBARITEM_CLICK,*lpMsgInfo,EMSG_POSTTYPE_FAST);
		//	}
		//	break;

		case EEVT_UPDATE_PANE:
			{
				int* value = (int*)npMsg->GetInputData();
				BREAK_ON_NULL(value);

				SetPaneItemValue(*value);
			}
			break;

		case EEVT_IMAGEBUTTON_CLICK:
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

bool CEvToolPane::SetPaneItemValue(int lID)
{
	//	依据配置文件的子元素ID

	if (NULL == mpUnificSetting) return false;

	IXuiIterator* lpItem = NULL;

	int i = 1;
	bool lbDirty = false;
	
	while(lpItem = mpIterator->GetSubElementByID(i++))
	{
		int lnID = 0;

		//取ID
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			lpItem, EEVT_GET_UNIFIC_SETTING_ID, NULL, 0, &lnID, sizeof(int));

		if (0 !=lnID)
		{
			//判断dirty
			if (lbDirty = mpUnificSetting->GetDirty(lnID))
			{
				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					lpItem, EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);

				//清除修改标记
				mpUnificSetting->SetDirty(lnID, false);
			}
		}
	}

	/*switch (lID)
	{
	case 0x00200100:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200101))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200102))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200103))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(3), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200104))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(4), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);


	}
	break;

	case 0x00200200:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200201))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200202))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200203))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(3), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200204))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(4), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200205))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(5), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200206))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(6), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200207))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(7), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200208))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(8), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	}
	break;

	case 0x00200300:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200301))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200302))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200303))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(3), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200304))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(4), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200305))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(5), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200306))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(6), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200307))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(7), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200308))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(8), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200309))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(9), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x0020030a))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(10), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x0020030b))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(11), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x0020030c))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(12), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	}
	break;

	case 0x00200400:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200401))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200402))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200403))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(3), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200404))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(4), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	}
	break;

	case 0x00200500:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200501))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200502))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	}
	break;

	case 0x00200600:
	{
	if (lbDirty = mpUnificSetting->GetDirty(0x00200601))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(1), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200602))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(2), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200603))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(3), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200604))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(4), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200605))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(5), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	if (lbDirty = mpUnificSetting->GetDirty(0x00200606))
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
	mpIterator->GetSubElementByID(6), EEVT_PANE_ITEM_SET_VALUE, NULL, 0, NULL, 0);
	}
	break;
	}*/

	return true;
}