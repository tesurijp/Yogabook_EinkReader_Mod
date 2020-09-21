/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "cmmstruct.h"
#include "Einkui.h"

#include "math.h"
#include "ElementImp.h"
#include "XCtl.h"

#include "Dwrite.h"
#include "EvLabelImp.h"

DEFINE_BUILTIN_NAME(Label)

CEvLabelImp::CEvLabelImp()
{
	mpForeBrush=NULL;
	mpBackBrush = NULL;
	mpTextLayout = NULL;
	mpTextFormat = NULL;

	mbDirty = true;
	mlBlinking = -1;
	mbLbPressed = false;

	muLines = 0;
	mdTextEdge.left = mdTextEdge.top = mdTextEdge.right = mdTextEdge.bottom = 0.0f;
	mdTextPos.x = mdTextPos.y = 0.0f;

}

CEvLabelImp::~CEvLabelImp()
{
	moText.Clear();
	ReleaseDeviceResource();

	CMM_SAFE_RELEASE(mpTextFormat);
}

ULONG CEvLabelImp::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	LONG llValue;

	do 
	{
		if(npTemplete == NULL)
			return ERESULT_UNSUCCESSFUL;

		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_LAZY_UPDATE);

		EnableInput(mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_HYPER_LINK,0));

		if(mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_UNDER_LINE,0)!=0)
		{
			CXuiElement::SetFlags(ELAB_FLAG_UNDERLINE,true);
		}
		else
		{
			CXuiElement::SetFlags(ELAB_FLAG_UNDERLINE,false);
		}

		llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_EDGE_LEFT,0);
		if(llValue > 0 )
			mdTextEdge.left = (FLOAT)llValue;

		llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_EDGE_TOP,0);
		if(llValue > 0 )
			mdTextEdge.top = (FLOAT)llValue;

		llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_EDGE_RIGHT,0);
		if(llValue > 0 )
			mdTextEdge.right = (FLOAT)llValue;

		llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_EDGE_BOTTOM,0);
		if(llValue > 0 )
			mdTextEdge.bottom = (FLOAT)llValue;

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

// 允许输入
void CEvLabelImp::EnableInput(LONG nlSet)
{
	if(nlSet)
	{
		CXuiElement::SetFlags(ELAB_FLAG_HYPERLINK,true);
		mpIterator->ModifyStyles(EITR_STYLE_MOUSE|EITR_STYLE_KEYBOARD|EITR_STYLE_DRAG);
		CXuiElement::mhInnerCursor = LoadCursor(NULL,IDC_HAND);
	}
	else
	{
		CXuiElement::SetFlags(ELAB_FLAG_HYPERLINK,false);
		mpIterator->ModifyStyles(0,EITR_STYLE_MOUSE|EITR_STYLE_KEYBOARD|EITR_STYLE_DRAG);
		CXuiElement::mhInnerCursor = NULL;
	}
}


//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvLabelImp::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpValueKey = NULL;

	do
	{
		luResult = CXuiElement::OnElementCreate(npIterator);
		if(luResult != ERESULT_SUCCESS)
			break;

		lpValueKey = mpTemplete->GetSubKey(TF_ID_LABEL_VALUE);
		if(lpValueKey != NULL && lpValueKey->GetValueLength()>0)
		{
			wchar_t lswValue[MAX_PATH];

			int liSize = lpValueKey->GetValue(lswValue,sizeof(wchar_t)*MAX_PATH)/sizeof(wchar_t);
			for(int i=0;i<liSize;i++)
				moText.Insert(-1,lswValue[i]);

			for(int i=liSize-1;i>=0;i--)
			{
				if(moText[i]== UNICODE_NULL)
					moText.RemoveByIndex(i);
			}

			//mlSelBegin = 2;
			//mlSelEnd = 4;
		}

		// 首先获得边界
		ULONG llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_MAX_WIDTH, 0);
		if (llValue <= 0)
			mfTextMaxWidth = 800.0f;
		else
			mfTextMaxWidth = (FLOAT)llValue;
		llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_MAX_HEIGHT, 0);
		if (llValue <= 0)
			mfTextMaxHeight = 600.0f;
		else
			mfTextMaxHeight = (FLOAT)llValue;

		CreateFont();

		GenerateTextLayout();

		luResult = ERESULT_SUCCESS;

	}while(false);

	CMM_SAFE_RELEASE(lpValueKey);
	return luResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvLabelImp::OnElementDestroy()
{
	ReleaseDeviceResource();
	CMM_SAFE_RELEASE(mpTextFormat);

	CXuiElement::OnElementDestroy();

	return ERESULT_SUCCESS;
}

// 鼠标落点检测
ERESULT CEvLabelImp::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = CXuiElement::OnMouseOwnerTest(rPoint);

	if(ERESULT_FAILED(luResult) || luResult == ERESULT_MOUSE_OWNERSHIP)
		return luResult;

	if(!(rPoint.x < mdTextEdge.left || rPoint.x >= mdTextEdge.left+mdTextSize.width || rPoint.y < mdTextEdge.top || rPoint.y >= mdTextEdge.top+mdTextSize.height))
		return ERESULT_MOUSE_OWNERSHIP;

	return ERESULT_SUCCESS;
}


// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvLabelImp::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_LABEL_SET_TEXT:
		{
			const wchar_t* lswInputString = (const wchar_t*)npMsg->GetInputData();

			luResult = OnSetText(lswInputString);
		}
		break;
	case EACT_LABEL_GET_TEXT:
		{
			wchar_t* lpTextBuf = (wchar_t*)npMsg->GetOutputBuffer();
			LONG llCharCount = npMsg->GetOutputBufferSize()/sizeof(wchar_t);
			luResult = OnGetText(lpTextBuf,llCharCount);
		}
		break;
	case EACT_LABEL_GET_TEXT_LENGTH:
		{
			if(npMsg->GetOutputBuffer()==NULL || npMsg->GetOutputBufferSize()<sizeof(ULONG))
				luResult = ERESULT_WRONG_PARAMETERS;
			else
			{
				*(ULONG*)npMsg->GetOutputBuffer() = moText.Size();
				npMsg->SetOutputDataSize(sizeof(ULONG));
				luResult = ERESULT_SUCCESS;
			}
		}
		break;
	case EACT_LABEL_SET_UNDERLINE:
		{
			if(npMsg->GetInputDataSize() != sizeof(LONG))
				luResult = ERESULT_WRONG_PARAMETERS;
			else
			{
				if(*(LONG*)npMsg->GetInputData() != 0)
					CXuiElement::SetFlags(ELAB_FLAG_UNDERLINE,true);
				else
					CXuiElement::SetFlags(ELAB_FLAG_UNDERLINE,false);

				UpdateView();
				luResult = ERESULT_SUCCESS;
			}
		}
		break;
	case EACT_LABEL_SET_HYPERLINK:
		{
			if(npMsg->GetInputDataSize() != sizeof(LONG))
				luResult = ERESULT_WRONG_PARAMETERS;
			else
			{
				EnableInput(*(LONG*)npMsg->GetInputData());

				UpdateView();
				luResult = ERESULT_SUCCESS;
			}
		}
		break;
	case EACT_LABEL_SET_FORE_COLOR:
		luResult = SetBrushColor(npMsg,true);
		break;
	case EACT_LABEL_SET_BACK_COLOR:
		luResult = SetBrushColor(npMsg,false);
		break;
	case EACT_LABEL_GET_FORE_COLOR:
		luResult = GetBrushColor(npMsg,true);
		break;
	case EACT_LABEL_GET_BACK_COLOR:
		luResult = GetBrushColor(npMsg,false);
		break;
	case EACT_LABEL_GET_LAYOUT:
		luResult = GetLayout(npMsg);
	case EMSG_DISCARD_DEVICE_RESOURCE:
		ReleaseDeviceResource();
		luResult = ERESULT_SUCCESS;
		break;
	case EACT_LABEL_SET_MAX_WIDTH:
	{
		if (npMsg->GetInputDataSize() != sizeof(LONG))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		// 获取输入数据
		LONG* lpValue = (LONG*)npMsg->GetInputData();
		mfTextMaxWidth = (float)*lpValue;
		GenerateTextLayout();
		UpdateView();
		
		luResult = ERESULT_SUCCESS;
		break;
	}
	case EACT_LABEL_SET_MAX_HEIGHT:
	{
		if (npMsg->GetInputDataSize() != sizeof(LONG))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		// 获取输入数据
		LONG* lpValue = (LONG*)npMsg->GetInputData();
		mfTextMaxHeight = (float)*lpValue;
		GenerateTextLayout();
		UpdateView();

		luResult = ERESULT_SUCCESS;
		break;
	}
	default:
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

ERESULT CEvLabelImp::GetLayout(IEinkuiMessage* npMsg)
{
	if(npMsg->GetOutputBuffer()==NULL || npMsg->GetOutputBufferSize()<sizeof(STCTL_LABEL_LAYOUT))
		return ERESULT_WRONG_PARAMETERS;

	PSTCTL_LABEL_LAYOUT lpLayout;
	LONG llValue;

	lpLayout = (PSTCTL_LABEL_LAYOUT)npMsg->GetOutputBuffer();

	llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_MAX_WIDTH,0);
	lpLayout->MaxSize.width = (FLOAT)llValue;
	llValue = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_MAX_HEIGHT,0);
	lpLayout->MaxSize.height = (FLOAT)llValue;

	lpLayout->Background = mdBgRect;
	lpLayout->TextPos.x = mdTextEdge.left+mdTextPos.x;
	lpLayout->TextPos.y = mdTextEdge.top+mdTextPos.y;
	lpLayout->TextSize = mdTextSize;
	lpLayout->Lines = (LONG)muLines;
	if(mlVisibleCount >= moText.Size())
		lpLayout->VisibleChars = mlVisibleCount;
	else
	{
		if(mlVisibleCount > 3)
			lpLayout->VisibleChars = mlVisibleCount - 3;
		else
		if(mlVisibleCount >= 1)
			lpLayout->VisibleChars = 1;
		else
			lpLayout->VisibleChars = 0;
	}

	npMsg->SetOutputDataSize(sizeof(STCTL_LABEL_LAYOUT));

	return ERESULT_SUCCESS;
}

ERESULT CEvLabelImp::SetBrushColor(IEinkuiMessage* npMsg,bool nbFore)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpKey = NULL;

	do 
	{
		if(npMsg->GetInputDataSize() != sizeof(ULONG))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		ULONG luColor = *(ULONG*)npMsg->GetInputData();

		if(nbFore != false)
		{
			muForeColor = luColor;
			CXuiElement::SetFlags(ELAB_FLAG_FORE_COLOR,true);
			CMM_SAFE_RELEASE(mpForeBrush);
		}
		else
		{
			muBackColor = luColor;
			CXuiElement::SetFlags(ELAB_FLAG_BACK_COLOR,true);
			CMM_SAFE_RELEASE(mpBackBrush);
		}

		luResult = ERESULT_SUCCESS;
	} while (false);

	CMM_SAFE_RELEASE(lpKey);

	return luResult;
}

ERESULT CEvLabelImp::GetBrushColor(IEinkuiMessage* npMsg,bool nbFore)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(npMsg->GetOutputBufferSize() != sizeof(ULONG))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		ULONG luColor;

		if(nbFore != false)
		{
			if(CXuiElement::TestFlag(ELAB_FLAG_FORE_COLOR)==false)
				luColor = (ULONG)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_COLOR_FORE,0xFFFFFF);
			else
				luColor = muForeColor;
		}
		else
		{
			if(CXuiElement::TestFlag(ELAB_FLAG_BACK_COLOR)==false)
				luColor = (ULONG)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_COLOR_BACK,0xFFFFFF);
			else
				luColor = muBackColor;
		}
		if(luColor == 0xFFFFFF)
			break;

		*(ULONG*)npMsg->GetOutputBuffer() = luColor;
		npMsg->SetOutputDataSize(sizeof(luColor));

		luResult = ERESULT_SUCCESS;
	} while (false);

	return luResult;
}

//Set Text
ERESULT CEvLabelImp::OnSetText(const wchar_t* nswText)
{
	moText.Clear();

	if(nswText != NULL)
	{
		for (int i=0;nswText[i]!=UNICODE_NULL;i++)
		{
			moText.Insert(-1,nswText[i]);
		}
	}

	UpdateView();

	GenerateTextLayout();

	return ERESULT_SUCCESS;
}

//Get Text
ERESULT CEvLabelImp::OnGetText(wchar_t* nswTextBuf,LONG nlCharCount)
{
	if(nlCharCount < moText.Size()+1 || nswTextBuf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	for (int i=0;i<moText.Size();i++)
	{
		nswTextBuf[i] = moText[i];
	}

	nswTextBuf[moText.Size()] = UNICODE_NULL;

	return ERESULT_SUCCESS;
}

// 慢刷新
void  CEvLabelImp::OnLazyUpdate(
	PSTEMG_LAZY_UPDATE npLazyUpdate
	)
{
	if(mlBlinking >= 0 && npLazyUpdate->Updated%2 == 0)
	{
		mlBlinking = (mlBlinking==0?1:0);

		EinkuiGetSystem()->UpdateView();
	}
}

//键盘焦点获得或者失去
void CEvLabelImp::OnKeyBoardFocus(PSTEMS_STATE_CHANGE npState)
{
	if(CXuiElement::TestFlag(ELAB_FLAG_HYPERLINK)==false)
		return;

	if(npState->State != 0)
		mlBlinking = 0;
	else
		mlBlinking = -1000;

	PostMessageToParent(EEVT_LABEL_KEYBOARD_FOCUS,npState->State);

	EinkuiGetSystem()->UpdateView();
}

//鼠标进入或离开
void CEvLabelImp::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	PostMessageToParent(EEVT_LABEL_MOUSE_FOCUS,npState->State);

	if(npState->State == 0 && mbLbPressed !=false)
		mbLbPressed = false;

	CXuiElement::OnMouseFocus(npState);
}


//鼠标按下
ERESULT CEvLabelImp::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	if(npInfo->ActKey == VK_LBUTTON)
	{
		if(npInfo->Presssed != false)
		{
			mbLbPressed = true;
		}
		else
		if(mbLbPressed != false)
		{
			// Open instruction
			PostMessageToParent(EEVT_LABEL_OPEN,CExMessage::DataInvalid);
			//mpIterator->ReleaseKeyBoardFocus();
			mbLbPressed = false;
		}
	}

	//UpdateView();

	return ERESULT_SUCCESS;
}


//绘制消息
ERESULT CEvLabelImp::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	 
	ERESULT luResult;

	luResult = PrepareBrush(npPaintBoard);
	if(luResult != ERESULT_SUCCESS)
		return luResult;

	luResult = GenerateTextLayout();
	if(luResult != ERESULT_SUCCESS)
		return luResult;

	// 绘制背景
	if(mpBgBitmap != NULL)
		npPaintBoard->DrawBitmap(mdBgRect,
		mpBgBitmap,
		ESPB_DRAWBMP_EXTEND);

	// 文字背景，仅当获得键盘焦点时，闪烁
	if(mlBlinking >= 0 && mlBlinking != 1 && mpBackBrush!= NULL)
	{
		npPaintBoard->GetD2dRenderTarget()->FillRectangle(D2D1::RectF(mdTextEdge.left+mdTextPos.x,mdTextEdge.top+mdTextPos.y,
			mdTextEdge.left+mdTextPos.x+mdTextSize.width,mdTextEdge.top+mdTextPos.y+mdTextSize.height),mpBackBrush);
	}

	// 绘制文字
	if(mpTextLayout != NULL)
		npPaintBoard->GetD2dRenderTarget()->DrawTextLayout(D2D1::Point2F(mdTextEdge.left+mdTextPos.x,mdTextEdge.top+mdTextPos.y),mpTextLayout,mpForeBrush);

	return ERESULT_SUCCESS;
}

// 删除设别相关资源
void CEvLabelImp::ReleaseDeviceResource()
{
	CMM_SAFE_RELEASE(mpForeBrush);
	CMM_SAFE_RELEASE(mpBackBrush);
	CMM_SAFE_RELEASE(mpTextLayout);
	mbDirty = true;
}

ERESULT CEvLabelImp::CreateFont(void)
{
	ICfKey* lpKey = NULL;
	HRESULT hr;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		// 建立字体
		if(mpTextFormat==NULL)
		{
			wchar_t lswFontName[100] = { L"Tahoma" };
			LONG llFontSize;

			//获取字体名称
			lpKey = mpTemplete->GetSubKey(TF_ID_LABEL_FONT);
			if (lpKey != NULL)
				lpKey->GetValue(lswFontName, sizeof(wchar_t) * 100);			

			llFontSize = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_FONT_SIZE,20);

			hr = EinkuiGetSystem()->GetDWriteFactory()->CreateTextFormat(
				lswFontName,
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				(FLOAT)llFontSize,
				L"",
				&mpTextFormat
				);
			if(FAILED(hr))
				break;

			CMM_SAFE_RELEASE(lpKey);
		}

		luResult = ERESULT_SUCCESS;
	} while (false);

	CMM_SAFE_RELEASE(lpKey);

	return luResult;
}

//准备画笔
ERESULT CEvLabelImp::PrepareBrush(IEinkuiPaintBoard* npPaintBoard)
{
	HRESULT hr;
	ULONG luColor;
	FLOAT lfAlpha;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		// 下面建立画刷
		if(mpForeBrush ==NULL)
		{
			if(CXuiElement::TestFlag(ELAB_FLAG_FORE_COLOR)==false)
			{
				luColor = (ULONG)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_COLOR_FORE,0xFFFFFF);
				if(luColor == 0xFFFFFF)
				{
					luColor = 0xFF000000;
				}
			}
			else
			{
				luColor = muForeColor;
			}

			lfAlpha = ((FLOAT)(luColor>>24)/255.0f);
			if(lfAlpha > 0.99)
				lfAlpha = 1.0f;
			luColor &= 0xFFFFFF;

			hr = npPaintBoard->GetD2dRenderTarget()->CreateSolidColorBrush(
				D2D1::ColorF(luColor,lfAlpha),
				&mpForeBrush
				);
			if(FAILED(hr))
				break;

		}

		// 建立back color brush
		if(mpBackBrush == NULL)
		{
			if(CXuiElement::TestFlag(ELAB_FLAG_FORE_COLOR)==false)
			{
				luColor = (ULONG)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_COLOR_BACK,0xFFFFFF);
			}
			else
			{
				luColor = muBackColor;
			}

			if(luColor != 0xFFFFFF)
			{
				lfAlpha = ((FLOAT)(luColor>>24)/255.0f);
				if(lfAlpha > 0.99)
					lfAlpha = 1.0f;
				luColor &= 0xFFFFFF;
				hr = npPaintBoard->GetD2dRenderTarget()->CreateSolidColorBrush(
					D2D1::ColorF(luColor,lfAlpha),
					&mpBackBrush
					);
				if(FAILED(hr))
					break;	
			}
		}

		luResult = ERESULT_SUCCESS;
	} while (false);

	return luResult;
}


//键盘消息
ERESULT CEvLabelImp::OnKeyPressed(const STEMS_KEY_PRESSED* npInfo)
{
	ERESULT luResult;

	luResult = SendMessageToParent(EEVT_LABEL_KEY_STIKE,*npInfo,NULL,0);
	if(luResult == ERESULT_DISCARD)
		return ERESULT_DISCARD;

	luResult = ERESULT_KEY_UNEXPECTED;

	if(npInfo->VirtualKeyCode == VK_SPACE && npInfo->IsPressDown == false)
	{
		PostMessageToParent(EEVT_LABEL_OPEN,CExMessage::DataInvalid);
		//mpIterator->ReleaseKeyBoardFocus();

		luResult = ERESULT_SUCCESS;
	}
		
	return luResult;
}

void CEvLabelImp::UpdateView(void)
{
	mbDirty = true;
	EinkuiGetSystem()->UpdateView();
}

// 重新生成格式化文字
ERESULT CEvLabelImp::GenerateTextLayout(void)
{
	DWRITE_TEXT_RANGE ldRange;

	if(mbDirty == false)
		return ERESULT_SUCCESS;

	mbDirty = false;

	CMM_SAFE_RELEASE(mpTextLayout);

	CalculateTextRect();

	mdBgRect.left= mdTextPos.x;
	mdBgRect.top = mdTextPos.y;
	mdBgRect.right = mdTextPos.x + mdTextEdge.left + mdTextEdge.right + mdTextSize.width;
	mdBgRect.bottom = mdTextPos.y + mdTextEdge.top + mdTextEdge.bottom + mdTextSize.height;
	mpIterator->SetSize(mdBgRect.right-mdBgRect.left,mdBgRect.bottom-mdBgRect.top);

	if(moText.Size()>0  && CXuiElement::TestFlag(ELAB_FLAG_UNDERLINE)!=false)
	{
		ldRange.startPosition = 0;
		ldRange.length = mlVisibleCount;

		mpTextLayout->SetUnderline(TRUE,ldRange);
	}

	return ERESULT_SUCCESS;
}


// 计算文字的排布区域
void CEvLabelImp::CalculateTextRect(void)
{
	IDWriteTextLayout* lpTextLayout = NULL;
	//UINT32 luLines;
	HRESULT hr;
	DWRITE_TEXT_METRICS ldMetrics;

	int i;
	int alt;
	cmmVector<wchar_t> loAlter;
	wchar_t* lpText;
	bool lbReduce = false;
	LONG llAlign;


	mlVisibleCount = moText.Size();
	if(mlVisibleCount < 0)
		mlVisibleCount = 0;

	// 而后检查减少字符个数是否可以符合最大宽度要求
	while(mlVisibleCount > 0)
	{
		CMM_SAFE_RELEASE(lpTextLayout);
		loAlter.Clear();

		if(lbReduce != false)
		{
			if(mlVisibleCount <= 3)
				alt = mlVisibleCount-1;
			else
				alt = 3;

			for(i=0;i<mlVisibleCount-alt;i++)
				loAlter.Insert(-1,moText[i]);

			for(;i<mlVisibleCount;i++)
				loAlter.Insert(-1,L'.');

			lpText = loAlter.GetBuffer();
		}
		else
			lpText = moText.GetBuffer();

		hr = EinkuiGetSystem()->GetDWriteFactory()->CreateTextLayout(lpText,mlVisibleCount,mpTextFormat, mfTextMaxWidth, mfTextMaxHeight,&lpTextLayout);
		if(SUCCEEDED(hr))
		{
			hr = lpTextLayout->GetMetrics(&ldMetrics);
			if(SUCCEEDED(hr))
			{
				if(ldMetrics.width <= mfTextMaxWidth && ldMetrics.height <= mfTextMaxHeight)
				{
					lpTextLayout->GetLineMetrics(NULL,0,&muLines);
					break;
				}
			}
		}

		if(lbReduce != false)
			mlVisibleCount--;
		else
			lbReduce = true;
	}

	if(lpTextLayout == NULL)
	{
		// 取一个字符的情况'Q'
		hr = EinkuiGetSystem()->GetDWriteFactory()->CreateTextLayout(L"Q",1,mpTextFormat, mfTextMaxWidth, mfTextMaxHeight,&lpTextLayout);
		if(SUCCEEDED(hr))
		{
			hr = lpTextLayout->GetMetrics(&ldMetrics);
			lpTextLayout->Release();
		}

		if(FAILED(hr))
		{
			mdTextSize.width = 0.0f;
			mdTextSize.height = 0.0f;
		}
		else
		{
			mdTextSize.width = ldMetrics.width;
			mdTextSize.height = ldMetrics.height;
		}
	}
	else
	{
		mpTextLayout = lpTextLayout;
		mdTextSize.width = ldMetrics.width;
		mdTextSize.height = ldMetrics.height;
	}

	// 调整排布
	llAlign = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_ALIGN_VERTICAL,1); 	// center 
	// 垂直方向
	switch(llAlign)
	{
	case 1:	// center
		mdTextPos.y = CExFloat::Round(mdTextEdge.top + (mfTextMaxHeight - mdTextSize.height)/2.0f);
		break;
	case 2:	// bottom
		mdTextPos.y = mdTextEdge.top + (mfTextMaxHeight - mdTextSize.height);
		break;
	case 0:	// top
	default:
		mdTextPos.y = 0.0f;
	}

	llAlign = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LABEL_ALIGN_HORIZONTAL,0);// left align
	// 水平方向
	switch(llAlign)
	{
	case 1:	// center
		mdTextPos.x =CExFloat::Round( mdTextEdge.left + (mfTextMaxWidth - mdTextSize.width)/2.0f);
		break;
	case 2:	// right
		mdTextPos.x = mdTextEdge.left + (mfTextMaxWidth - mdTextSize.width);
		break;
	case 0:	// left
	default:
		mdTextPos.x = 0.0f;
	}

}


