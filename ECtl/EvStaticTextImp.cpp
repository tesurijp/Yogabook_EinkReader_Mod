/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvStaticTextImp.h"
#include "XCtl.h"
//

DEFINE_BUILTIN_NAME(StaticText)

CEvStaticText::CEvStaticText()
{
	mpswText = NULL;
	mdwColor = 0;
	mdwFontSize = 0;
	mpswFontName = NULL;
	mdDrawRect.left = mdDrawRect.top = mdDrawRect.right = mdDrawRect.bottom = 0.0f;
}

CEvStaticText::~CEvStaticText()
{
	CMM_SAFE_DELETE(mpswFontName);
	CMM_SAFE_DELETE(mpswText);
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvStaticText::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CXuiElement::OnElementDestroy();	//调用基类

		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CEvStaticText::InitOnCreate(
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

		//设置自己的类型
		mpIterator->ModifyStyles(/*EITR_STYLE_CONTROL*/NULL,EITR_STYLE_MOUSE);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}


//装载配置资源
ERESULT CEvStaticText::LoadResource()
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;
	LONG llLen = 0;

	do 
	{
		lpValue = mpTemplete->GetSubKey(TF_ID_ST_TEXT); //显示文字
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			mpswText = new wchar_t[llLen];
			BREAK_ON_NULL(mpswText);

			lpValue->GetValue(mpswText,lpValue->GetValueLength());
			CMM_SAFE_RELEASE(lpValue);
		}

		//文字颜色
		mdwColor = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_COLOR,0xFFFFFFFF);
		mLimit = (STETXT_BMP_INIT::eSIZELIMIT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_SIZE_LIMIT,0);
		mTalign = (STETXT_BMP_INIT::eTEXTALIGN)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_TALIGN,0);
		mPalign = (STETXT_BMP_INIT::ePARAALIGN)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_PALIGN,0);
		mFontWidget = (STETXT_BMP_INIT::eFONTWEIGHT)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_FONTWIDGET,0);

		//字体
		lpValue = mpTemplete->GetSubKey(TF_ID_ST_FONT);
		if (lpValue != NULL)
		{
			llLen = lpValue->GetValueLength();
			if(llLen <= 0)
				break;

			mpswFontName = new wchar_t[llLen];
			BREAK_ON_NULL(mpswFontName);

			lpValue->GetValue(mpswFontName,lpValue->GetValueLength());
		}
		CMM_SAFE_RELEASE(lpValue);

		if(mpswFontName !=NULL && mpswFontName[0] == UNICODE_NULL)
			CMM_SAFE_DELETE(mpswFontName);	//如果没有读到，就清掉

		if(mpswFontName == NULL)
		{
			llLen = wcslen(L"Tahoma") + 1;
			mpswFontName = new wchar_t[llLen];
			BREAK_ON_NULL(mpswFontName);
			wcscpy_s(mpswFontName,llLen,L"Tahoma");	//默认字体
		}

		//字号
		mdwFontSize = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ST_FONT_SIZE,15);

		//重新生成图片
		ReCreateBmp();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvStaticText::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EACT_STATICTEXT_SET_TEXT:
		{
			//更换显示文字
			// 获取输入数据
			wchar_t* lpswText = (wchar_t*)npMsg->GetInputData();
			SetText(lpswText);

			break;
		}
	case EACT_STATICTEXT_GET_TEXT:
		{
			//获取显示文字
			if(npMsg->GetOutputBufferSize() != sizeof(wchar_t*))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			// 设置输出数据
			wchar_t** lpOut = (wchar_t**)npMsg->GetOutputBuffer();
			*lpOut = mpswText;

			break;
		}
	case EACT_STATICTEXT_SET_TEXT_COLOR:
		{
			//设置文字颜色
			if(npMsg->GetOutputBufferSize() != sizeof(LONG))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}
			LONG * lpValue = (LONG*)npMsg->GetInputData();
			mdwColor = *lpValue;

			ReCreateBmp();	//重新生成图片

			luResult = ERESULT_SUCCESS;

		}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvStaticText::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//装载一些必要的配置资源
		LoadResource();

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//更换显示文字
bool CEvStaticText::SetText(wchar_t* npswText)
{
	bool lbRet = false;

	do 
	{
		BREAK_ON_NULL(npswText);

		//固定大小，就算改变了字符串，该元素的参考大小也不会变化

		CMM_SAFE_DELETE(mpswText);	//清除原来的字符缓冲区
		int liLen = wcslen(npswText)+1;
		mpswText = new wchar_t[liLen];
		BREAK_ON_NULL(mpswText);
		wcscpy_s(mpswText,liLen,npswText);	//Copy新内容

		//重新生成图片
		ReCreateBmp();

		lbRet = true;

	} while (false);

	return lbRet;
}

//绘制
ERESULT CEvStaticText::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);			

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(mdDrawRect,mdDrawRect,mpBgBitmap,ESPB_DRAWBMP_NEAREST);

		//if(mpBgBitmap != NULL)
		//	npPaintBoard->DrawBitmap(D2D1::Point2F(0.0f,0.0f),mpBgBitmap/*,ESPB_DRAWBMP_NEAREST*/);
		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//重新生成图片
bool CEvStaticText::ReCreateBmp()
{
	bool lbRet = false;
	mdDrawRect.left = mdDrawRect.top = mdDrawRect.right = mdDrawRect.bottom = 0.0f;

	do 
	{ 
		//构建结构体
		STETXT_BMP_INIT ldInit;
		ZeroMemory(&ldInit,sizeof(STETXT_BMP_INIT));
		ldInit.Text = mpswText;
		ldInit.FontName = mpswFontName;
		ldInit.FontSize = mdwFontSize;
		ldInit.TextColor = mdwColor;
		ldInit.Width = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_WIDTH,0);
		ldInit.Height = (DWORD)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_HEIGHT,0);
		ldInit.Limit = mLimit;
		ldInit.Talign = mTalign;
		ldInit.Palign = mPalign;
		ldInit.FontWeight = mFontWidget;

		CMM_SAFE_RELEASE(mpBgBitmap);	//去掉原来的图片
		mpBgBitmap = EinkuiGetSystem()->GetAllocator()->CreateImageByText(ldInit);

		BREAK_ON_NULL(mpBgBitmap);

		if(mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_WIDTH,0)==0)
			mpIterator->SetSize((FLOAT)mpBgBitmap->GetWidth(),mpIterator->GetSizeY());	//没有设置固定大小，那就以生成的文字图片大小做为自己的大小

		if(mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_HEIGHT,0)==0)
			mpIterator->SetSize(mpIterator->GetSizeX(),(FLOAT)mpBgBitmap->GetHeight());	//没有设置固定大小，那就以生成的文字图片大小做为自己的大小

		//为了保证文字不被拉伸，这里哪个值小就用哪个
		mdDrawRect.right = mpIterator->GetSizeX()<(FLOAT)mpBgBitmap->GetWidth()?mpIterator->GetSizeX():(FLOAT)mpBgBitmap->GetWidth();
		mdDrawRect.bottom = mpIterator->GetSizeY()<(FLOAT)mpBgBitmap->GetHeight()?mpIterator->GetSizeY():(FLOAT)mpBgBitmap->GetHeight();

		lbRet = true;

	} while (false);

	return lbRet;
}