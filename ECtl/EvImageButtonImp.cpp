/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */



#include "stdafx.h"

#include "cmmstruct.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "XCtl.h"
#include "EvImageButtonImp.h"

//定义常量
#define BUF_SIZE			256

//定义键值

#define IB_KEY_DEFAULT_CTRL						L"DefaultCtrl"
#define IB_KEY_IMAGE_BUTTON						L"ImageButton"
#define IB_KEY_DEFAULT_PICTURE_FRAME			L"DefaultPictureFrame"

#define IB_KEY_STYLE							L"Style"
#define IB_KEY_SELECT_FRAME						L"SelectFrame"
#define IB_KEY_FRAME_COUNT						L"FrameCount"
#define IB_KEY_LEFT_PICTURE						L"LeftPicture"
#define IB_KEY_RIGHT_PICTURE					L"RightPicture"

#define IB_KEY_X								L"X"
#define IB_KEY_Y								L"Y"
#define IB_KEY_WIDTH							L"Width"
#define IB_KEY_HEIGHT							L"Height"
#define IB_KEY_BACKGROUND						L"BackGround"
#define IB_KEY_TIP								L"Tip"
#define IB_KEY_EXPANDABLE						L"Expandable"
//#define IB_KEY_PIC_PATH_LIST					L"PicPathList"
#define IB_KEY_MENU_ITEM_LIST					L"MenuItemList"
#define IB_KEY_MENU_ITEM_ID						L"Id"
#define IB_KEY_MENU_ITEM_PIC_PATH				L"PicPath"
	//消息相关
#define IB_KEY_MESSAGE_INFO							L"MessageInfo"
#define IB_KEY_MESSAGE_INFO_ID						L"ID"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE			L"MessageType"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_NONE		L"0"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_BOOL		L"1"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_INT		L"2"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_FLOAT		L"3"
#define IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_STRING		L"4"
	//颜色标志
#define IB_KEY_COLOR_FLAG							L"ColorFlag"
#define IB_KEY_COLOR_FLAG_true						L"1"
#define IB_KEY_COLOR_FLAG_FLASE						L"0"


//定义风格
#define IB_STYLE_UNCHECKABLE					L"0"
#define IB_STYLE_CHECKABLE						L"1"

#define IB_STYLE_UNEXPANDABLE					L"0"
#define IB_STYLE_EXPANDABLE						L"1"


//定义子控件
#define	IB_ID_EXPAND_CTRL						10		//扩展控件

DEFINE_BUILTIN_NAME(ImageButton)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvImageButton::CEvImageButton() :
	mpLeftPicture(NULL),
	mpRightPicture(NULL),
	mpBitmapSelectOrOver(NULL),
	mbIsMouseFocus(false),
	mbIsKeyboardFocus(false),
	mbIsPressed(false),
	mbChecked(false),
	mnStyle(-1),
	mbExpandable(false),
	mpIterExpandCtrl(NULL),
	mbShowExpandCtrl(false),
	mbHasColorFlag(false),
	mluRGBColor(0),
	mfRadio(1.0f),
	mbDrawShape(true)
	//mlCurrentPage(0),
	//mlPageCountMax(0)
{	
}

CEvImageButton::~CEvImageButton() 
{
}

ULONG CEvImageButton::InitOnCreate(
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

		//创建ImageButton的子控件
		IConfigFile * lpConfigFile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		if (lpConfigFile == NULL) break;
		ICfKey* lpRootKey = lpConfigFile->GetRootKey();
		if (lpRootKey == NULL) break;
		ICfKey* lpDefaultCtrlKey = lpRootKey->GetSubKey(IB_KEY_DEFAULT_CTRL);
		if (lpDefaultCtrlKey == NULL) break;
		ICfKey* lpImageButtonKey = lpDefaultCtrlKey->GetSubKey(IB_KEY_IMAGE_BUTTON);
		if (lpImageButtonKey == NULL) break;

		ICfKey * lpDefaultPictureFrameKey = lpImageButtonKey->GetSubKey(IB_KEY_DEFAULT_PICTURE_FRAME);
		if (lpDefaultPictureFrameKey == NULL) break;

		//	初始化UnificSetting
		//mpUnificSetting = GetUnificSetting();

		//创建左半部分PictureFrame
		mpLeftPicture = CEvPictureFrame::CreateInstance(mpIterator, lpDefaultPictureFrameKey, IB_ID_CTRL_LEFT_PICTURE);			

		//创建右半部分PictureFrame
		mpRightPicture = CEvPictureFrame::CreateInstance(mpIterator, lpDefaultPictureFrameKey, IB_ID_CTRL_RIGHT_PICTURE);			
		
		//mpLeftPicture->GetIterator()->ModifyStyles(NULL, EITR_STYLE_MOUSE);
		//mpLeftPicture->GetIterator()->ModifyStyles(NULL, EITR_STYLE_MOUSE);
		//mpIterator->ModifyStyles(NULL, EITR_STYLE_MOUSE);
		//mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL|EITR_STYLE_KEYBOARD);

		ICfKey* lpValue = mpTemplete->GetSubKey(IB_KEY_TIP); //Tip1
		wchar_t* lpszTip = NULL;

		do 
		{
			BREAK_ON_NULL(lpValue);
			LONG llLen = lpValue->GetValueLength();
			lpszTip = new wchar_t[llLen];
			BREAK_ON_NULL(lpszTip);

			lpValue->GetValue(lpszTip,lpValue->GetValueLength());
			
			mpIterator->SetToolTip(lpszTip);

		} while (false);

		CMM_SAFE_DELETE(lpszTip);
		CMM_SAFE_RELEASE(lpValue);
		//SetImageButtonEnable(false);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvImageButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		SetChildCtrlPara();
		//LoadResource();

		if (mbExpandable == true)
		{
			mpIterExpandCtrl = mpIterator->GetSubElementByID(IB_ID_EXPAND_CTRL);
			if (mpIterExpandCtrl)
			{
				mpIterator->ModifyStyles(EITR_STYLE_POPUP);
				mpIterExpandCtrl->SetVisible(mbShowExpandCtrl);
			}
		}
		
		lResult = ERESULT_SUCCESS;
	} while(false);

	return lResult;
}

//鼠标进入或离开
void CEvImageButton::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	if (npState->State != 0)
	{
		//鼠标进入
		if(mbIsMouseFocus == false) mbIsMouseFocus = true;
	}
	else
	{
		//鼠标移出
		if(mbIsMouseFocus != false) mbIsMouseFocus = false;

		if(mbIsPressed != false)  //如果鼠标移走了，就去掉按下状态
			mbIsPressed = false;
	}

	EinkuiGetSystem()->UpdateView();
}

//鼠标按下
ERESULT CEvImageButton::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);

		if(mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if((npInfo->ActKey & MK_LBUTTON) == 0)  //如果不是鼠标左键就不处理
			break;

		if ( (npInfo->Presssed == false) && (mbIsPressed == true) )
		{
			//鼠标抬起
			mbIsPressed = false;	

			if (mnStyle == 1) mbChecked = !mbChecked;		//设置按钮 选中 / 取消选中

			//到这里才算一次Click
			PostMessageToParent(EEVT_IMAGEBUTTON_CLICK, CExMessage::DataInvalid);

			if (mnStyle == 1)
			{
				if (mbChecked == true)
				{
					PostMessageToParent(EEVT_IMAGEBUTTON_CHECKED, CExMessage::DataInvalid);
				}
				else
				{
					PostMessageToParent(EEVT_IMAGEBUTTON_UNCHECKED, CExMessage::DataInvalid);
				}
			}

			if (mbExpandable == true)
			{
				if (mpIterExpandCtrl)
				{
					//mbShowExpandCtrl = ( (mbExpandable == true) ? false : true);
					if (mbShowExpandCtrl == true)
						mbShowExpandCtrl = false;
					else
						mbShowExpandCtrl = true;

					mpIterExpandCtrl->SetVisible(mbShowExpandCtrl);

				}
			}

			if (mMsgInfo.mnMsgParaType == TMPT_NONE)
			{
				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
				//	mpIterator->GetParent()->GetParent(), EEVT_TOOLBARITEM_CLICK, &mMsgInfo, sizeof(TOOLBAR_MSG));
				CExMessage::PostMessage(mpIterator->GetParent()->GetParent(),mpIterator,EEVT_TOOLBARITEM_CLICK,mMsgInfo,EMSG_POSTTYPE_FAST);
			}

			if (mMsgInfo.mnMsgParaType == TMPT_BOOL)
			{
				mMsgInfo.mbBool = mbChecked;

				//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
				//	mpIterator->GetParent()->GetParent(), EEVT_TOOLBARITEM_CLICK, &mMsgInfo, sizeof(TOOLBAR_MSG));
				CExMessage::PostMessage(mpIterator->GetParent()->GetParent(),mpIterator,EEVT_TOOLBARITEM_CLICK,mMsgInfo,EMSG_POSTTYPE_FAST);
			}

			/*if (mMsgInfo.mnMsgParaType == TMPT_INT)	
			{
				EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
					mpIterator->GetParent(), EEVT_TOOLBARITEM_CLICK, &mMsgInfo, sizeof(TOOLBAR_MSG));
			}*/
		}
		else
		{
			//鼠标按下
			mbIsPressed = true;
		}

		EinkuiGetSystem()->UpdateView();

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

// 鼠标落点检测
ERESULT CEvImageButton::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if(!(rPoint.x < 0.0f || rPoint.x >= mpIterator->GetSizeX()
		|| rPoint.y < 0.0f || rPoint.y >= mpIterator->GetSizeY()))
	{
		luResult = ERESULT_MOUSE_OWNERSHIP;
	}

	return luResult;
}

//绘制
ERESULT CEvImageButton::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mbIsMouseFocus == true)
		{
			ULONG lulMethod = ESPB_DRAWBMP_LINEAR;
			npPaintBoard->DrawBitmap(
				D2D1::RectF(0.0f, 0.0f, mpIterator->GetSize().width, mpIterator->GetSize().height),

				D2D1::RectF(
				(FLOAT)(mpBitmapSelectOrOver->GetWidth()) * 2.0f / 4.0f, 
				0.0f, 
				(FLOAT)(mpBitmapSelectOrOver->GetWidth()) * 3.0f / 4.0f, 
				(FLOAT)(mpBitmapSelectOrOver->GetHeight())
				),

				mpBitmapSelectOrOver,
				lulMethod
				);
		}

		if (mnStyle == 0)
		{
			if (mbIsPressed == true)
			{
				ULONG lulMethod = ESPB_DRAWBMP_LINEAR;
				npPaintBoard->DrawBitmap(
					D2D1::RectF(0.0f, 0.0f, mpIterator->GetSize().width, mpIterator->GetSize().height),

					D2D1::RectF(
					(FLOAT)(mpBitmapSelectOrOver->GetWidth()) * 3.0f / 4.0f, 
					0.0f, 
					(FLOAT)(mpBitmapSelectOrOver->GetWidth()), 
					(FLOAT)(mpBitmapSelectOrOver->GetHeight())
					),

					mpBitmapSelectOrOver,
					lulMethod
					);
			}
		}
		else if (mnStyle == 1)
		{
			if (mbChecked != false)
			{
				ULONG lulMethod = ESPB_DRAWBMP_LINEAR;
				npPaintBoard->DrawBitmap(
					D2D1::RectF(0, 0, mpIterator->GetSize().width, mpIterator->GetSize().height),

					D2D1::RectF(
					(FLOAT)(mpBitmapSelectOrOver->GetWidth()) * 3.0f / 4.0f, 
					0.0f, 
					(FLOAT)(mpBitmapSelectOrOver->GetWidth()), 
					(FLOAT)(mpBitmapSelectOrOver->GetHeight())
					),

					mpBitmapSelectOrOver,
					lulMethod
					);
			}
		}

		if ( (mbHasColorFlag != false) && (mbDrawShape != false) )
		{
			ID2D1SolidColorBrush *lpBrush;
			HRESULT hr = npPaintBoard->GetD2dRenderTarget()->CreateSolidColorBrush(
				//D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &lpBrush);
				//D2D1::ColorF((float)(GetRValue(mluRGBColor)), (float)(GetGValue(mluRGBColor)), (float)GetBValue(mluRGBColor)), &lpBrush);
				D2D1::ColorF(RGB(GetBValue(mluRGBColor), GetGValue(mluRGBColor), GetRValue(mluRGBColor)), 1.0f), &lpBrush);
			npPaintBoard->GetD2dRenderTarget()->FillRectangle(
				D2D1::RectF(5.0f, 20.0f, 20.0f, 24.0f), lpBrush);	
		}
	
		lResult = ERESULT_SUCCESS;
		
	} while(false);

	return lResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvImageButton::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
		case EEVT_MENUITEM_CLICK:			//扩展控件的菜单项被点击
			{	
				//if (mpVecExpandMenuItem.Size() != 0)
				{
					//读取ID
					if(npMsg->GetInputDataSize()!=sizeof(int) || npMsg->GetInputData()==NULL)
						luResult = ERESULT_WRONG_PARAMETERS;
					int lItemID = *(int*)(npMsg->GetInputData());

					wchar_t* pPicPath = NULL;
					for (int i = 0; i < mpVecExpandMenuItem.Size(); ++i)
					{
						if (mpVecExpandMenuItem[i].id == lItemID)
						{
							pPicPath = mpVecExpandMenuItem[i].pPicPath;

							if (pPicPath && (*pPicPath != '\0'))
							{	
								//更换按钮左半部分图片
								if (mpLeftPicture)
								{
									EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
										mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, pPicPath,
										(wcslen(pPicPath) + 1) * sizeof(wchar_t), NULL, 0);
								}
							}

							break;
						}
					}

					//发送toolbar子项点击消息
					if (mMsgInfo.mnMsgParaType == TMPT_INT)
					{
						//mMsgInfo.mpMsgBuf = (int*)(npMsg->GetInputData());
						mMsgInfo.mlInterge = (LONG)*(int*)(npMsg->GetInputData());

						//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
						//	mpIterator->GetParent()->GetParent(), EEVT_TOOLBARITEM_CLICK, &mMsgInfo, sizeof(TOOLBAR_MSG));

						//EinkuiGetSystem()->GetElementManager()->SimplePostMessage(
						//	mpIterator->GetParent(), EEVT_TOOLBARITEM_CLICK, &mMsgInfo, sizeof(TOOLBAR_MSG));

						CExMessage::PostMessage(mpIterator->GetParent()->GetParent(),mpIterator,EEVT_TOOLBARITEM_CLICK,mMsgInfo,EMSG_POSTTYPE_FAST);
						CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,EEVT_TOOLBARITEM_CLICK,mMsgInfo,EMSG_POSTTYPE_FAST);

					}

					mbShowExpandCtrl = false;
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

				if (lpActive->State == 0 && mpIterExpandCtrl->IsVisible() != false)			// 可见状态下失去激活状态，需要隐藏当前展开的菜单
				{
					mbShowExpandCtrl = false;
					if (mpIterExpandCtrl)
					{
						mpIterExpandCtrl->SetVisible(mbShowExpandCtrl);
					}
				}

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_SET_COLOR:
			{
				if(npMsg->GetInputDataSize() != sizeof(ULONG))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				
				mluRGBColor = *(ULONG*)npMsg->GetInputData();
				if((mluRGBColor & 0xFF000000) != 0)
					mbDrawShape = true;
				else
					mbDrawShape = false;

				EinkuiGetSystem()->UpdateView();

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_CHANGE_LEFT_IMAGE_BKG:
			{
				
				wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, lpValue,
					(wcslen(lpValue) + 1) * sizeof(wchar_t), NULL, 0);

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_CHANGE_LEFT_IMAGE_BKG_FULL_PATH:
			{
			
				wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

				EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
					mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC_FULLPATH, lpValue,
					(wcslen(lpValue) + 1) * sizeof(wchar_t), NULL, 0);

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_SET_CHECKED:
			{
				if(npMsg->GetInputDataSize() != sizeof(bool))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				// 获取输入数据
				bool* lpValue = (bool*)npMsg->GetInputData();
				SetChecked(*lpValue);

				luResult = ERESULT_SUCCESS;
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
				bool lbEnable = true;// pSetting->GetItemEnable(mMsgInfo.mnCtrlID);
				SetImageButtonEnable(lbEnable);

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EEVT_PANE_ITEM_SET_VALUE:
			{
				SetValue();

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_SET_RATIO:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();

				mfRadio = *lpValue;

				luResult = ERESULT_SUCCESS;
			}
			break;

		case EACT_IMAGEBUTTON_SET_ITEM_SELECTED:
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

		case EACT_IMAGEBUTTON_DRAW_SHAPE:
			{
				if(npMsg->GetInputDataSize() != sizeof(bool))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				// 获取输入数据
				bool* lpValue = (bool*)npMsg->GetInputData();
				mbDrawShape = *lpValue;
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


BOOL CEvImageButton::SetChildCtrlPara()
{
	if (mpLeftPicture && mpRightPicture)
	{
		ICfKey * lpLeftPictureKey		= mpTemplete->GetSubKey(IB_KEY_LEFT_PICTURE);
		ICfKey * lpRightPictureKey		= mpTemplete->GetSubKey(IB_KEY_RIGHT_PICTURE);

		if (lpLeftPictureKey)
		{
			ULONG lPosX = lpLeftPictureKey->QuerySubKeyValueAsLONG(IB_KEY_X);
			ULONG lPosY = lpLeftPictureKey->QuerySubKeyValueAsLONG(IB_KEY_Y);
			mpLeftPicture->GetIterator()->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lHeight = lpLeftPictureKey->QuerySubKeyValueAsLONG(IB_KEY_HEIGHT);
			ULONG lWidth = lpLeftPictureKey->QuerySubKeyValueAsLONG(IB_KEY_WIDTH);
			mpLeftPicture->GetIterator()->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));

			//换背景图
			wchar_t lpPicBackGround[BUF_SIZE] = {0};
			lpLeftPictureKey->QuerySubKeyValue(IB_KEY_BACKGROUND, lpPicBackGround, BUF_SIZE * sizeof(wchar_t));
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, lpPicBackGround,
				(wcslen(lpPicBackGround) + 1) * sizeof(wchar_t), NULL, 0);
		}

		if (lpRightPictureKey)
		{
			ULONG lPosX = lpRightPictureKey->QuerySubKeyValueAsLONG(IB_KEY_X);
			ULONG lPosY = lpRightPictureKey->QuerySubKeyValueAsLONG(IB_KEY_Y);
			mpRightPicture->GetIterator()->SetPosition((FLOAT)(lPosX), (FLOAT)(lPosY));

			ULONG lHeight = lpRightPictureKey->QuerySubKeyValueAsLONG(IB_KEY_HEIGHT);
			ULONG lWidth = lpRightPictureKey->QuerySubKeyValueAsLONG(IB_KEY_WIDTH);
			mpRightPicture->GetIterator()->SetSize((FLOAT)(lWidth), (FLOAT)(lHeight));

			//换背景图
			wchar_t lpPicBackGround[BUF_SIZE] = {0};
			lpRightPictureKey->QuerySubKeyValue(IB_KEY_BACKGROUND, lpPicBackGround, BUF_SIZE * sizeof(wchar_t));
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
				mpRightPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, lpPicBackGround,
				(wcslen(lpPicBackGround) + 1) * sizeof(wchar_t), NULL, 0);
		}

		//背景框
		wchar_t lpSelectFrame[BUF_SIZE] = {0};
		mpTemplete->QuerySubKeyValue(IB_KEY_SELECT_FRAME, lpSelectFrame, BUF_SIZE * sizeof(wchar_t));
		mpBitmapSelectOrOver = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lpSelectFrame);

		//配置文件读取控件风格
		wchar_t lpStyle[BUF_SIZE] = {0};
		mpTemplete->QuerySubKeyValue(IB_KEY_STYLE, lpStyle, BUF_SIZE * sizeof(wchar_t));
		if (wcscmp(lpStyle, IB_STYLE_UNCHECKABLE) == 0)
		{
			// UNCHECKABLE 风格
			mnStyle = 0;
		}
		else
		{
			// CHECKABLE 风格
			mnStyle = 1;
		}

		//配置文件读取扩展属性
		wchar_t lpExpandable[BUF_SIZE] = {0};
		mpTemplete->QuerySubKeyValue(IB_KEY_EXPANDABLE, lpExpandable, BUF_SIZE * sizeof(wchar_t));
		if (wcscmp(lpExpandable, IB_STYLE_EXPANDABLE) == 0)
		{			
			//支持扩展
			mbExpandable = true;
			
			//读取更换图片路径列表
			ICfKey* lpExpandMenuItemListKey = mpTemplete->GetSubKey(IB_KEY_MENU_ITEM_LIST);
			ICfKey* lpItemKey = NULL;

			if (lpExpandMenuItemListKey)
			{
				lpItemKey = lpExpandMenuItemListKey->MoveToSubKey();	
				int nCount = 0;
				while (lpItemKey != NULL)	//循环读取更换图片路径列表
				{
					ExpandMenuItem MenuItem;
					lpItemKey->QuerySubKeyValue(IB_KEY_MENU_ITEM_ID, &(MenuItem.id), sizeof(int));
					lpItemKey->QuerySubKeyValue(IB_KEY_MENU_ITEM_PIC_PATH, MenuItem.pPicPath, sizeof(wchar_t) * MAX_PATH);
						mpVecExpandMenuItem.Insert(nCount++, MenuItem);

					lpItemKey = lpItemKey->MoveToNextKey();
				}
			}
		}
		else
		{
			// 不支持扩展
			mbExpandable = false;
		}

		//配置文件读取消息属性
		ICfKey* lpMessageInfoKey = mpTemplete->GetSubKey(IB_KEY_MESSAGE_INFO);
		if (lpMessageInfoKey)
		{
			unsigned int		lnID = 0;
			wchar_t lpMessageType[BUF_SIZE] = {0};

			lpMessageInfoKey->QuerySubKeyValue(IB_KEY_MESSAGE_INFO_ID, &lnID, sizeof(unsigned int));
			lpMessageInfoKey->QuerySubKeyValue(IB_KEY_MESSAGE_INFO_MESSAGE_TYPE, lpMessageType, BUF_SIZE * sizeof(wchar_t));

			if (lnID != 0)
			{
				mMsgInfo.mnCtrlID = lnID;
			}

			if (wcscmp(lpMessageType, IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_NONE) == 0)
			{
				mMsgInfo.mnMsgParaType	= TMPT_NONE;
				//mMsgInfo.mpMsgBuf		= NULL;
			}
			else if (wcscmp(lpMessageType, IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_BOOL) == 0)
			{
				mMsgInfo.mnMsgParaType = TMPT_BOOL;
			}
			else if (wcscmp(lpMessageType, IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_INT) == 0)
			{
				mMsgInfo.mnMsgParaType = TMPT_INT;
			}
			else if (wcscmp(lpMessageType, IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_FLOAT) == 0)
			{
				mMsgInfo.mnMsgParaType = TMPT_FLOAT;
			}
			else if (wcscmp(lpMessageType, IB_KEY_MESSAGE_INFO_MESSAGE_TYPE_STRING) == 0)
			{
				mMsgInfo.mnMsgParaType = TMPT_STRING;
			}
			else
			{
				mMsgInfo.mnMsgParaType = TMPT_OTHERS;
			}
		}

		//配置文件读取颜色标记属性
		wchar_t lpColorFlag[BUF_SIZE] = {0};
		mpTemplete->QuerySubKeyValue(IB_KEY_COLOR_FLAG, lpColorFlag, BUF_SIZE * sizeof(wchar_t));
		if (wcscmp(lpColorFlag, IB_KEY_COLOR_FLAG_true) == 0)
		{
			mbHasColorFlag = true;
		}
		

		return true;
	}
	else
	{
		return false;
	}	
}


void CEvImageButton::SetChecked(bool nbChecked)
{
	mbChecked = nbChecked;

	if (true == nbChecked)
	{
		PostMessageToParent(EEVT_IMAGEBUTTON_CHECKED, CExMessage::DataInvalid);
	}
	else
	{
		PostMessageToParent(EEVT_IMAGEBUTTON_UNCHECKED, CExMessage::DataInvalid);
	}
}

bool CEvImageButton::SetValue()
{
	//if (NULL == mpUnificSetting) return false;

	if (false == mpIterator->IsEnable()) return false;

	//	读取值
	//eValueType ValueType;
	//wstring lwValue = mpUnificSetting->GetItemValue(mMsgInfo.mnCtrlID, &ValueType);
	wchar_t lwValue[] = L"1";
	if (1 == mnStyle)		// 可 check 风格
	{
		if (L"1" == lwValue)
		{
			mbChecked = true;
		}
		else if (L"0" == lwValue)
		{
			mbChecked = false;
		}
	}

	if (mbExpandable && mpIterExpandCtrl)
	{
		int lItemID = _wtoi(lwValue/*.c_str()*/);

		wchar_t* pPicPath = NULL;
		for (int i = 0; i < mpVecExpandMenuItem.Size(); ++i)
		{
			if (mpVecExpandMenuItem[i].id == lItemID)
			{
				pPicPath = mpVecExpandMenuItem[i].pPicPath;

				if (pPicPath && (*pPicPath != '\0'))
				{	
					//更换按钮左半部分图片
					if (mpLeftPicture)
					{
						EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
							mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, pPicPath,
							(wcslen(pPicPath) + 1) * sizeof(wchar_t), NULL, 0);
					}
				}

				break;
			}
		}
	}

	return true;
}

bool CEvImageButton::SetImageButtonEnable(bool nbEnable)
{
	if (nbEnable)
	{
		if (mpIterator) mpIterator->ModifyStyles(EITR_STYLE_MOUSE, NULL);
		if (mpLeftPicture)
		{
			IEinkuiIterator* iter = mpLeftPicture->GetIterator();
			if (iter)	iter->SetAlpha(1.0f);
		}
	}
	else
	{
		if (mpIterator) mpIterator->ModifyStyles(NULL, EITR_STYLE_MOUSE);
		if (mpLeftPicture)
		{
			IEinkuiIterator* iter = mpLeftPicture->GetIterator();
			if (iter)	iter->SetAlpha(0.5f);
		}
	}

	return true;
}

//元素参考尺寸发生变化
ERESULT CEvImageButton::OnElementResized(D2D1_SIZE_F nNewSize)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (mpLeftPicture)
			mpLeftPicture->GetIterator()->SetSize
			(mpIterator->GetSize().width - mpLeftPicture->GetIterator()->GetPositionX() * 2.0f * mfRadio,
			mpIterator->GetSize().height - mpLeftPicture->GetIterator()->GetPositionY() * 2.0f * mfRadio);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//禁用或启用
ERESULT CEvImageButton::OnElementEnable(bool nbIsEnable)
{
	SetImageButtonEnable(nbIsEnable);

	return ERROR_SUCCESS;
}

bool CEvImageButton::SetItemSelected(int nID)
{
	if (mbExpandable && mpIterExpandCtrl)
	{
		int lItemID = nID;

		wchar_t* pPicPath = NULL;
		for (int i = 0; i < mpVecExpandMenuItem.Size(); ++i)
		{
			if (mpVecExpandMenuItem[i].id == lItemID)
			{
				pPicPath = mpVecExpandMenuItem[i].pPicPath;

				if (pPicPath && (*pPicPath != '\0'))
				{	
					//更换按钮左半部分图片
					if (mpLeftPicture)
					{
						EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
							mpLeftPicture->GetIterator(), EACT_PICTUREFRAME_CHANGE_PIC, pPicPath,
							(wcslen(pPicPath) + 1) * sizeof(wchar_t), NULL, 0);
					}
				}

				break;
			}
		}
	}

	return true;
}