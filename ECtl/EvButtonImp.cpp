#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvButtonImp.h"


//using namespace D2D1;
DEFINE_BUILTIN_NAME(Button)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvButton::CEvButton()
{
	mpszButtonText = mpszNormolTip = mpszCheckedTip = NULL;
	mpTextBitmap = NULL;
	mbIsMouseFocus = false;
	mbIsKeyboardFocus = false;
	mbIsPressed = false;
	mbIsChecked = false;
	mbIsCheckEnable = false;
	mbIsOther = false;
	mlCurrentPage = mlPageCountMax = 0;
	mpswFontName = NULL;
	mdFrameSize.width = mdFrameSize.height = 0.0f;
	mbIsPlayTimer = false;
	mfTextTop = mfTextLeft = mfTextRight = mfTextBottom = 0.0f;
	ZeroMemory(mdArrayFrame, sizeof(mdArrayFrame));
}

// 用于释放成员对象
CEvButton::~CEvButton()
{
	CMM_SAFE_DELETE(mpszButtonText);
	CMM_SAFE_DELETE(mpszNormolTip);
	CMM_SAFE_DELETE(mpszCheckedTip);
	CMM_SAFE_RELEASE(mpTextBitmap);
	CMM_SAFE_DELETE(mpswFontName);
}

ULONG CEvButton::InitOnCreate(
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

		//设置自己的类型
		mpIterator->ModifyStyles(/*EITR_STYLE_CONTROL|*/EITR_STYLE_DRAG);

		//装载一些必要的配置资源
		LoadResource();


		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		StartPlayTimer();

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvButton::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CXuiElement::OnElementDestroy();	//调用基类

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//装载配置资源
ERESULT CEvButton::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;
	LONG llAttibute[16] = { 0 };	//按钮属性
	LONG llLen = -1;

	do
	{
		//文字颜色
		mdwColor = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_COLOR, 0xFFFFFFFF);

		mdwDisabledColor = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_DISABLED_COLOR, 0xFF7d7d7d);

		//字体
		lpValue = mpTemplete->GetSubKey(TF_ID_BT_FONT);
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			if (llLen <= 0)
				break;

			mpswFontName = new wchar_t[llLen];
			BREAK_ON_NULL(mpswFontName);

			lpValue->GetValue(mpswFontName, lpValue->GetValueLength());
		}
		CMM_SAFE_RELEASE(lpValue);

		if (mpswFontName != NULL && mpswFontName[0] == UNICODE_NULL)
			CMM_SAFE_DELETE(mpswFontName);	//如果没有读到，就清掉

		if (mpswFontName == NULL)
		{
			llLen = wcslen(L"Tahoma") + 1;
			mpswFontName = new wchar_t[llLen];
			BREAK_ON_NULL(mpswFontName);
			wcscpy_s(mpswFontName, llLen, L"Tahoma");	//默认字体
		}

		//字号
		mdwFontSize = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_FONT_SIZE, 15);

		lpValue = mpTemplete->GetSubKey(TF_ID_BT_TEXT); //按钮文字
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			mpszButtonText = new wchar_t[llLen];
			BREAK_ON_NULL(mpszButtonText);

			lpValue->GetValue(mpszButtonText, lpValue->GetValueLength());
			CMM_SAFE_RELEASE(lpValue);

			if (mpszButtonText[0] != UNICODE_NULL)
				ReCreateTextBmp();
		}

		lpValue = mpTemplete->GetSubKey(TF_ID_BT_NORMAL_TIP); //Tip1
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			mpszNormolTip = new wchar_t[llLen];
			BREAK_ON_NULL(mpszNormolTip);

			lpValue->GetValue(mpszNormolTip, lpValue->GetValueLength());
			CMM_SAFE_RELEASE(lpValue);
			mpIterator->SetToolTip(mpszNormolTip);
		}

		lpValue = mpTemplete->GetSubKey(TF_ID_BT_CHECKED_TIP); //Tip2
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			mpszCheckedTip = new wchar_t[llLen];
			BREAK_ON_NULL(mpszCheckedTip);

			lpValue->GetValue(mpszCheckedTip, lpValue->GetValueLength());
			CMM_SAFE_RELEASE(lpValue);
		}

		//文字对齐方式
		mlAlignType = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_ALIGN_TYPE, 2);

		//文字左边距
		mfTextLeft = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_LEFT, 1);

		//文字右边距
		mfTextRight = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_RIGHT, 1);

		//文字上边距
		mfTextTop = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_TOP, 1);

		//文字下边距
		mfTextBottom = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_BOTTOM, 1);

		//感应区宽
		mdAcionSize.width = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_ACTION_WIDTH, 0);

		//感应区高
		mdAcionSize.height = (FLOAT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_ACTION_HEIGHT, 0);


		//每个动画从第几帧开始
		lpValue = mpTemplete->GetSubKey(TF_ID_BT_FRAME); //四态按钮帧数据
		if (lpValue != NULL)
		{
			lpValue->GetValue(llAttibute, lpValue->GetValueLength());
			lpValue->Release();

			// [CheckMarx Fix by zhuhl5, [TRY]]
			for (int i = ARRAY_INDEX_DISABLE; i < ARRAY_INDEX_PRESSED + 1; i++)
			{
				mdArrayFrame[i].Index = mlPageCountMax;			//从第几帧开始
				mdArrayFrame[i].Count = llAttibute[i];		//共几帧

				mlPageCountMax += llAttibute[i];
			}
		}

		lpValue = mpTemplete->GetSubKey(TF_ID_BT_CHECKED_FRAME); //Checked态帧数据
		if (lpValue != NULL)
		{
			lpValue->GetValue(llAttibute, lpValue->GetValueLength());
			lpValue->Release();

			mbIsCheckEnable = true; //开启Checked态

			// [CheckMarx Fix by zhuhl5, [TRY]]
			for (int i = ARRAY_INDEX_CHECKED_DISABLE; i < ARRAY_INDEX_CHECKED_PRESSED + 1; i++)
			{
				mdArrayFrame[i].Index = mlPageCountMax;			//从第几帧开始
				mdArrayFrame[i].Count = llAttibute[i - ARRAY_INDEX_CHECKED_DISABLE];		//共几帧

				mlPageCountMax += mdArrayFrame[i].Count;
			}
		}


		lpValue = mpTemplete->GetSubKey(TF_ID_BT_OTHER_FRAME); //其它动画帧数据
		if (lpValue != NULL)
		{
			ICfKey* lpSubKey = lpValue->GetSubKey();

			int i = 0;
			for (i = ARRAY_INDEX_OTHER; i < BUTTON_FRAME_ARRAY_MAX, lpSubKey != NULL; i++)
			{
				lpSubKey->GetValue(&(mdArrayFrame[i].Count), lpSubKey->GetValueLength());
				mdArrayFrame[i].Index = mlPageCountMax;			//从第几帧开始

				mlPageCountMax += mdArrayFrame[i].Count;

				lpSubKey = lpSubKey->MoveToNextKey();	//查看是否还有下一个动画数据
			}

			//if(i > ARRAY_INDEX_OTHER) //????这里为什么要开启Check状态，先屏掉 Edit by Jaryee
			//	mbIsCheckEnable = true;	//开启Checked态

			CMM_SAFE_RELEASE(lpValue);
			CMM_SAFE_RELEASE(lpSubKey);
		}

		if (mpBgBitmap != NULL)
		{
			UINT luiWidth = mpBgBitmap->GetWidth();
			UINT luiHeight = mpBgBitmap->GetHeight();	//获取背景图宽高


			if (mlPageCountMax > 0)
			{
				mdFrameSize.width = (float)luiWidth / mlPageCountMax;
				mdFrameSize.height = (float)luiHeight;

				mpIterator->SetSize(mdFrameSize.width, mdFrameSize.height); //设置按钮显示大小
			}
		}

		RelocateText();

		leResult = ERESULT_SUCCESS;

	} while (false);


	return leResult;
}

//定位文字图片显示位置
void CEvButton::RelocateText(void)
{
	do
	{
		if (mpTextBitmap == NULL)
			break;

		if (mpBgBitmap != NULL)
		{
			//if (wcsicmp(mpszButtonText,L"华语")==0)
			//{
			//	//break;
			//}
			FLOAT lfWidth = mfTextLeft + mfTextRight + mpTextBitmap->GetWidth();
			FLOAT lfHeight = mfTextTop + mfTextBottom + mpTextBitmap->GetHeight();
			if (lfWidth > mpIterator->GetSizeX()) //如果背景图比文字小,就要扩大背景图
				mpIterator->SetSize(lfWidth, mpIterator->GetSizeY());
			if (lfHeight > mpIterator->GetSizeY())
				mpIterator->SetSize(mpIterator->GetSizeX(), lfHeight);


			if (mlAlignType == 1)
			{
				//左对齐
			}
			else if (mlAlignType == 2)
			{
				//居中对齐
			}
			else/* if(mlAlignType == 2)*/
			{
				//右对齐

			}


			if (mlAlignType == 1)
				mdTextDestRect.left = mfTextLeft;
			else if (mlAlignType == 2)
				mdTextDestRect.left = CExFloat::Round((mpIterator->GetSizeX() - mpTextBitmap->GetWidth()) / 2.0f);
			else
				mdTextDestRect.left = mpIterator->GetSizeX() - mfTextRight - mpTextBitmap->GetWidth();

			mdTextDestRect.right = mdTextDestRect.left + mpTextBitmap->GetWidth();
			if (mfTextTop - 1.0f < 0.5f)
				mdTextDestRect.top = CExFloat::Round((mpIterator->GetSizeY() - mpTextBitmap->GetHeight()) / 2.0f);
			else
				mdTextDestRect.top = mfTextTop;

			mdTextDestRect.bottom = mdTextDestRect.top + mpTextBitmap->GetHeight();
		}
		else
		{
			mdTextDestRect.left = mdTextDestRect.top = 0.0f;
			mdTextDestRect.right = (FLOAT)mpTextBitmap->GetWidth();
			mdTextDestRect.bottom = (FLOAT)mpTextBitmap->GetHeight();
		}

	} while (false);
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvButton::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_BUTTON_SET_CHECKED:	//设置Check状态
	{
		if (npMsg->GetInputDataSize() != sizeof(bool))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		// 获取输入数据
		bool* lpValue = (bool*)npMsg->GetInputData();
		luResult = SetChecked(*lpValue);// 返回值，IEinkuiMessage对象的返回值由本函数的调用者依据本函数的结果设置
		break;
	}
	case EACT_BUTTON_GET_CHECKED:	//获取是否处于Check状态
	{
		if (npMsg->GetOutputBufferSize() != sizeof(bool))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		// 设置输出数据
		bool* lpOut = (bool*)npMsg->GetOutputBuffer();

		*lpOut = IsChecked();

		npMsg->SetOutputDataSize(sizeof(bool));

		luResult = ERESULT_SUCCESS;
		break;
	}
	case EACT_BUTTON_PLAY_OTHER_ANIMATION:
	{
		//播放自定义动画消息
		if (npMsg->GetInputDataSize() != sizeof(LONG))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		// 获取输入数据
		LONG* lpValue = (LONG*)npMsg->GetInputData();
		mlOtherIndex = *lpValue;	//播放的是哪个动画
		mlOtherIndex -= 1;	//动画的顺序是从1开始，我们的数组是从0开始
		if (mlOtherIndex < 0 || mlOtherIndex >(BUTTON_FRAME_ARRAY_MAX - ARRAY_INDEX_OTHER + 1))
			break;

		if (mdArrayFrame[mlOtherIndex + ARRAY_INDEX_OTHER].Count != 0)
		{
			mbIsOther = true;	//设置播放动画
			StartPlayTimer();
		}

		break;
	}
	case EACT_BUTTON_SETTEXT:
	{
		//更换显示文字
		// 获取输入数据
		wchar_t* lpswText = (wchar_t*)npMsg->GetInputData();
		OnChangeText(lpswText);

		break;
	}
	case EACT_BUTTON_CHANGE_PIC:
	{
		//更换背景图片，相对路径
		// 获取输入数据
		wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

		luResult = OnChangeBackGround(lpValue, false);

		break;
	}
	case EACT_BUTTON_CHANGE_PIC_FULLPATH:
	{
		//更换背景图片，全路径
		// 获取输入数据
		wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

		luResult = OnChangeBackGround(lpValue, true);

		break;
	}
	case EACT_BUTTON_GETTEXT:
	{
		//获取显示文字
		if (npMsg->GetOutputBufferSize() != sizeof(wchar_t*))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		// 设置输出数据
		wchar_t** lpOut = (wchar_t**)npMsg->GetOutputBuffer();
		*lpOut = mpszButtonText;

		break;
	}
	case EACT_BUTTON_SET_ACTION_RECT:
	{
		//设置激活区域
		if (npMsg->GetInputDataSize() != sizeof(D2D1_SIZE_F))
		{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}
		D2D1_SIZE_F* lpSize = (D2D1_SIZE_F*)npMsg->GetInputData();
		BREAK_ON_NULL(lpSize);

		mdAcionSize = *lpSize;

		break;
	}
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

//绘制
ERESULT CEvButton::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
		{
			//绘制背景图
			float lfX = 0;
			ULONG lulMethod = ESPB_DRAWBMP_LINEAR;
			LONG llIndex = GetCurrentStatesArrayIndex();		//获取当前状态图片信息所在的数组下标

			if (llIndex != -1)
			{
				//????要把这步计算放到接收到自己参考尺寸改变的地方
				//if((mpBgBitmap->GetExtnedLineX() != 0 && mpBgBitmap->GetExtnedLineY() != 0) && ((mpIterator->GetSizeX() - mdFrameSize.width > 1) || mpIterator->GetSizeY() - mdFrameSize.height > 1))
				//	lulMethod = ESPB_DRAWBMP_EXTEND;	//如果设置了延展线并且参考尺寸大于实际尺寸，那就使用延展方式

				lfX = (mdArrayFrame[llIndex].Index + mlCurrentPage) * mdFrameSize.width; //从哪个位置开始显示

				npPaintBoard->DrawBitmap(D2D1::RectF(0.0f, 0.0f, mpIterator->GetSizeX(), mpIterator->GetSizeY()),
					D2D1::RectF(lfX, 0, lfX + mdFrameSize.width, mdFrameSize.height),
					mpBgBitmap,
					ESPB_DRAWBMP_EXTEND
				);
			}
		}

		if (mpTextBitmap != NULL)
		{
			//绘制文字
			FLOAT lfValue = mbIsPressed ? 1.0f : 0.0f;
			lfValue = 0.0f;//eink项目不需要向右下移动
			npPaintBoard->DrawBitmap(D2D1::RectF(mdTextDestRect.left + lfValue, mdTextDestRect.top + lfValue, mdTextDestRect.right + lfValue, mdTextDestRect.bottom + lfValue),
				mpTextBitmap,
				ESPB_DRAWBMP_NEAREST
			);
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//定时器
void CEvButton::OnTimer(PSTEMS_TIMER npStatus)
{
	do
	{
		if (npStatus->TimerID == BUTTON_TIMER_ID_PAGE)
		{
			//播放动画
			if (mpIterator->IsVisible() == false)	//如果进入隐藏状态，就关掉定时器
			{
				mpIterator->KillTimer(npStatus->TimerID);
				mbIsPlayTimer = false;
				break;
			}

			LONG llIndex = GetCurrentStatesArrayIndex();		//获取当前状态图片信息所在的数组下标

			if (llIndex != -1)
			{

				if (mlCurrentPage < mdArrayFrame[llIndex].Count - 1)
				{
					mlCurrentPage++;
				}
				else
				{
					mlCurrentPage = 0;	//超过最大页
					if (mbIsOther != false)
					{
						mbIsOther = false;

						StartPlayTimer(); //自定义动画只播放一次
						LONG llOtherIndex = mlOtherIndex + 1;
						PostMessageToParent(EEVT_BUTTON_PLAYED, llOtherIndex); //通知父窗口，动画播放完成
					}
				}

				EinkuiGetSystem()->UpdateView();
			}
		}

	} while (false);

}

//鼠标进入或离开
void CEvButton::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	CXuiElement::OnMouseFocus(npState);

	if (npState->State != 0)
	{
		//鼠标进入
		if (mbIsMouseFocus == false)
			mbIsMouseFocus = true;

		PostMessageToParent(EEVT_BUTTON_MOUSE_IN, mpIterator);
	}
	else
	{
		//鼠标移出
		if (mbIsMouseFocus != false)
			mbIsMouseFocus = false;

		if (mbIsPressed != false)  //如果鼠标移走了，就去掉按下状态
			mbIsPressed = false;

		PostMessageToParent(EEVT_BUTTON_MOUSE_OUT, mpIterator);
	}

	if (mbIsOther == false)
		StartPlayTimer(); //如果正在播放自定义动画，则不执行下面操作


	EinkuiGetSystem()->UpdateView();
}

//鼠标按下
ERESULT CEvButton::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npInfo);
		if (mpIterator->IsEnable() == false)
			break;	//如果是禁用状态，就不接收输入

		if (MOUSE_LB(npInfo->ActKey) == false)  //如果不是鼠标左键就不处理
			break;

		if (npInfo->Presssed == false)
		{
			//鼠标抬起
			if (mbIsPressed != false)
			{
				//到这里才算一次Click
				PostMessageToParent(EEVT_BUTTON_CLICK, CExMessage::DataInvalid);

				mbIsPressed = false;

				if (mbIsCheckEnable != false)
				{
					//允许Check
					if (mbIsChecked == false)
					{
						//Check状态
						mpIterator->SetToolTip(mpszCheckedTip);
						mbIsChecked = true;
						PostMessageToParent(EEVT_BUTTON_CHECKED, CExMessage::DataInvalid);
					}
					else
					{
						//UnCheck状态
						mpIterator->SetToolTip(mpszNormolTip);
						mbIsChecked = false;
						PostMessageToParent(EEVT_BUTTON_UNCHECK, CExMessage::DataInvalid);
					}
				}
			}
		}
		else
		{
			//鼠标按下
			if (mbIsPressed == false)
			{
				mbIsPressed = true;
			}
		}

		StartPlayTimer();
		EinkuiGetSystem()->UpdateView();

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//禁用或启用
ERESULT CEvButton::OnElementEnable(bool nbIsEnable)
{
	StartPlayTimer();

	// 改变文字颜色
	ReCreateTextBmp();

	return ERROR_SUCCESS;
}

//设置Check状态
ERESULT CEvButton::SetChecked(bool nbIsChecked)
{
	mbIsChecked = nbIsChecked;
	StartPlayTimer();
	return ERESULT_SUCCESS;
}

//判断是否处于Check状态
bool CEvButton::IsChecked()
{
	return mbIsChecked;
}

//获取当前状态所在的数组下标
LONG CEvButton::GetCurrentStatesArrayIndex()
{
	LONG llIndex = -1;

	//按钮动画绘制Timer
	if (mbIsOther != false)
	{
		//自定义动画
		llIndex = mlOtherIndex + ARRAY_INDEX_OTHER;
	}
	else if (mpIterator->IsEnable() == false)		// 禁用态
	{
		if (mbIsChecked == false)
		{
			//普通禁用态
			llIndex = ARRAY_INDEX_DISABLE;
		}
		else
		{
			//Checked禁用态
			llIndex = ARRAY_INDEX_CHECKED_DISABLE;
		}
	}
	else if (mbIsPressed != false)
	{
		if (mbIsChecked == false)
		{
			//普通按下态
			llIndex = ARRAY_INDEX_PRESSED;
		}
		else
		{
			//Checked按下态
			llIndex = ARRAY_INDEX_CHECKED_PRESSED;
		}
	}
	else if (mbIsMouseFocus != false)
	{
		if (mbIsChecked == false)
		{
			//普通焦点态
			llIndex = ARRAY_INDEX_FOCUS;
		}
		else
		{
			//Checked焦点态
			llIndex = ARRAY_INDEX_CHECKED_FOCUS;
		}
	}
	else
	{
		if (mbIsChecked == false)
		{
			//普通正常态
			llIndex = ARRAY_INDEX_NORMAL;
		}
		else
		{
			//Checked正常态
			llIndex = ARRAY_INDEX_CHECKED_NORMAL;
		}
	}

	return llIndex;
}

//开启或关闭动画定时器
void CEvButton::StartPlayTimer()
{
	do
	{
		LONG llIndex = GetCurrentStatesArrayIndex();
		mlCurrentPage = 0;
		if (mdArrayFrame[llIndex].Count <= 1 && mbIsPlayTimer != false)
		{
			mpIterator->KillTimer(BUTTON_TIMER_ID_PAGE);
			mbIsPlayTimer = false;
		}
		else if (mdArrayFrame[llIndex].Count > 1 && mbIsPlayTimer == false && mpIterator->IsVisible() != false)
		{
			mpIterator->SetTimer(BUTTON_TIMER_ID_PAGE, MAXULONG32, 100, NULL);
			mbIsPlayTimer = true;
		}

	} while (false);

}

//切换显示帧,第一帧为1
ERESULT CEvButton::OnPlayAnimation(LONG nlIndex)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (nlIndex <= 0 || nlIndex > mlPageCountMax)
			break;

		mlCurrentPage = nlIndex - 1;

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

// 鼠标落点检测
ERESULT CEvButton::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do
	{
		if (mpIterator->IsVisible() == false || mpIterator->IsEnable() == false)
			break;


		if (mdAcionSize.width > 1.0f && mdAcionSize.height > 1.0f)
		{
			//如果有感应区，就以感应区为主
			if (rPoint.x < 0.0f || rPoint.x >= mdAcionSize.width || rPoint.y < 0.0f || rPoint.y >= mdAcionSize.height)
				break;
		}
		else if (mpBgBitmap != NULL)
		{
			//有背景图的时候
			if (rPoint.x < 0.0f || (UINT)rPoint.x >= mpIterator->GetSizeX() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpIterator->GetSizeY())
				break;

			float lfX = 0;
			LONG llIndex = GetCurrentStatesArrayIndex();		//获取当前状态图片信息所在的数组下标
			if (llIndex < 0)
				break;
			lfX = (mdArrayFrame[llIndex].Index + mlCurrentPage) * mdFrameSize.width;  	//从哪个位置开始显示

			D2D1_POINT_2F ldPoint = CExPoint::BigToOldPoint(mdFrameSize, mpIterator->GetSize(), D2D1::Point2(rPoint.x, rPoint.y), ESPB_DRAWBMP_EXTEND, D2D1::Point2((FLOAT)mpBgBitmap->GetExtnedLineX(), (FLOAT)mpBgBitmap->GetExtnedLineY()));
			//通过像素Alpha值检测????
			DWORD luPixel;
			if (ERESULT_SUCCEEDED(mpBgBitmap->GetPixel(DWORD(lfX + ldPoint.x), (DWORD)ldPoint.y, luPixel)))
			{
				if (luPixel != 1)
					break;
			}
		}
		else if (mpTextBitmap != NULL)
		{
			//只有文字的时候
			if (rPoint.x < 0.0f || (UINT)rPoint.x >= mpTextBitmap->GetWidth() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpTextBitmap->GetHeight())
				break;
		}
		else
		{
			break;
		}

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}

//改变按钮文字
ERESULT CEvButton::OnChangeText(wchar_t* npswText)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do
	{
		BREAK_ON_NULL(npswText);

		CMM_SAFE_DELETE(mpszButtonText);	//清除原来的字符缓冲区
		int liLen = wcslen(npswText) + 1;
		mpszButtonText = new wchar_t[liLen];
		BREAK_ON_NULL(mpszButtonText);
		wcscpy_s(mpszButtonText, liLen, npswText);	//Copy新内容

		//生成新的文字图片
		ReCreateTextBmp();

		RelocateText(); //重新定位文字位置

		EinkuiGetSystem()->UpdateView();

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}


//更换显示图片
ERESULT CEvButton::OnChangeBackGround(wchar_t* npswPicPath, bool nbIsFullPath)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (npswPicPath == NULL || npswPicPath[0] == UNICODE_NULL)
			break;

		//????先做这么简单处理，这里还需要重新计算图片的相关信息
		CMM_SAFE_RELEASE(mpBgBitmap);	//去除原来的图片

		mpBgBitmap = EinkuiGetSystem()->GetAllocator()->LoadImageFile(npswPicPath, nbIsFullPath);
		BREAK_ON_NULL(mpBgBitmap);

		EinkuiGetSystem()->UpdateView();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//重新生成文字图片
bool CEvButton::ReCreateTextBmp()
{
	//构建结构体
	STETXT_BMP_INIT ldInit;
	ZeroMemory(&ldInit, sizeof(STETXT_BMP_INIT));
	ldInit.Text = mpszButtonText;
	ldInit.FontName = mpswFontName;
	ldInit.FontSize = mdwFontSize;
	ldInit.TextColor = false != mpIterator->IsEnable() ? mdwColor : mdwDisabledColor;

	LONG liTextMaxWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_MAXWIDTH, 0);
	LONG liTextMaxHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_BT_TEXT_MAXHEIGHT, 0);
	if (liTextMaxWidth > 0)
	{
		//为了增加...
		ldInit.Limit = STETXT_BMP_INIT::EL_FIXEDSIZE;
		ldInit.Width = liTextMaxWidth;
		ldInit.Height = liTextMaxHeight;
	}


	CMM_SAFE_RELEASE(mpTextBitmap);	//去掉原来的图片

	if (mdwFontSize == 0 || mpswFontName[0] == UNICODE_NULL)
		return false;

	mpTextBitmap = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldInit);

	if (NULL == mpBgBitmap && mpTextBitmap != NULL)			// 如果没有背景图，则以文字图的大小作为宽高
	{
		mpIterator->SetSize((FLOAT)mpTextBitmap->GetWidth(), (FLOAT)mpTextBitmap->GetHeight());
	}

	return mpTextBitmap == NULL ? false : true;
}
