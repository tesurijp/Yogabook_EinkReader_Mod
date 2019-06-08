/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "PdfPicture.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(PdfPicture)

CPdfPicture::CPdfPicture(void)
{
	mulPageCount = 0;

	pdfDoc = NULL;
	pdfCrtPage = NULL;
	pdfCrtPage2 = NULL;
	pdfImage = NULL;
	pdfImage2 = NULL;
	mpElBitmap = NULL;
	duopageBuf = NULL;
	landScope = true;
	duopageMode = false;
	pageGap = 0;// 20;
	pageNo = 0;

	auto libModule = LoadLibrary(L"Edpdf.dll");
	if (libModule != NULL)
	{
		auto proc = (GetModule_Proc)GetProcAddress(libModule, "EdGetModule");
		pdfModule = proc();
	}

	miFontSize[0] = 10;
	miFontSize[1] = 14;
	miFontSize[2] = 18;
	miFontSize[3] = 22;
	miFontSize[4] = 26;
	miFontPage[0] = 10;
	miFontPage[1] = 14;
	miFontPage[2] = 18;
	miFontPage[3] = 22;
	miFontPage[4] = 26;

	miTxtFontSizeIndex = 2;
	mHandleTxtFile = NULL;
	mpwcharTxt = new wchar_t[TXT_MAX_LEN];
	mpcharTxt = new char[TXT_MAX_LEN];;
}


CPdfPicture::~CPdfPicture(void)
{
	CloseFile(false);
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CPdfPicture::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CPdfPicture::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄

		//用于显示txt
		mpIterTxt = mpIterator->GetSubElementByID(PP_LABEL_TXT);
		BREAK_ON_NULL(mpIterTxt);
		CExMessage::SendMessageWithText(mpIterTxt, mpIterator, EACT_LABEL_SET_TEXT, mpwcharTxt);

		// 创建画刷
		mpXuiBrush = EinkuiGetSystem()->CreateBrush(XuiSolidBrush, D2D1::ColorF(0.0f, 0.0f, 0.0f));
		BREAK_ON_NULL(mpXuiBrush);

		mpXuiBrush->SetStrokeWidth(2.0f);
		mpXuiBrush->SetStrokeType(
			D2D1::StrokeStyleProperties(
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_ROUND,
				D2D1_LINE_JOIN_MITER,
				10.0f,
				D2D1_DASH_STYLE_DOT,
				0.0f),
			0,
			NULL);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//按钮单击事件
ERESULT CPdfPicture::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case PP_BT_SHOW_TIP:
		{
			PostMessageToParent(EEVT_ER_SHOW_TIP, CExMessage::DataInvalid);
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


////消息处理函数
//ERESULT CPdfPicture::ParseMessage(IEinkuiMessage* npMsg)
//{
//	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
//
//	switch (npMsg->GetMessageID())
//	{
//	case EMSG_MODAL_ENTER:
//	{
//		//// 创建要弹出的对话框
//		//mpIterator->SetVisible(true);
//		luResult = ERESULT_SUCCESS;
//		break;
//	}
//	default:
//		luResult = ERESULT_NOT_SET;
//		break;
//	}
//
//	if (luResult == ERESULT_NOT_SET)
//	{
//		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
//	}
//
//	return luResult;
//}

//定时器
void CPdfPicture::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CPdfPicture::OnElementResized(D2D1_SIZE_F nNewSize)
{
	//CExMessage::SendMessage(mpIterBtFull, mpIterator, EACT_BUTTON_SET_ACTION_RECT, nNewSize);
	////mpIterLineOne->SetSize(nNewSize.width, mpIterLineOne->GetSize().height);

	//mpIterBtOk->SetPosition(nNewSize.width - 85, mpIterBtOk->GetPositionY());
	if (nNewSize.width == 0 || nNewSize.height == 0)
		return ERESULT_SUCCESS;

	ED_SIZE viewPort;
	viewPort.x = CExFloat::ToLong(nNewSize.width);
	viewPort.y = CExFloat::ToLong(nNewSize.height);

	mViewCtl.SetViewPort(viewPort);

	GoToPage(pageNo);

	return ERESULT_SUCCESS;
}

//设置要预览的图片,和接收消息的对象
bool CPdfPicture::OpenFile(wchar_t* npszPath)
{
	bool lbRet = false;

	//if (pdfDoc != NULL)
	//	EinkuiGetSystem()->ExitXui(); // 4debug

	CloseFile(false);

	do 
	{
		BREAK_ON_NULL(npszPath);

		BREAK_ON_NULL(pdfModule);

		//判断文件扩展名
		if (wcsicmp(wcsrchr(npszPath, '.'), L".txt") == 0)
		{
			//如果是txt文件
			mbIsTxt = true;
			mHandleTxtFile = CreateFile(npszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			DWORD ldwReadLen = 0;
			char lszTest[50] = { 0 };
			ReadFile(mHandleTxtFile, lszTest, 50, &ldwReadLen, NULL);
			SetFilePointer(mHandleTxtFile, 0, NULL, FILE_BEGIN);
			if (IsTextUnicode(lszTest,50,NULL) == FALSE)
			{
				//非unicode
				ReadFile(mHandleTxtFile, mpcharTxt, TXT_MAX_LEN, &ldwReadLen, NULL);
				MultiByteToWideChar(CP_ACP, NULL, mpcharTxt, -1, mpwcharTxt, TXT_MAX_LEN);
			}
			else
			{
				ReadFile(mHandleTxtFile, mpwcharTxt, TXT_MAX_LEN, &ldwReadLen, NULL);
			}
			CExMessage::SendMessageWithText(mpIterTxt, mpIterator, EACT_LABEL_SET_TEXT, L"");
			if (landScope == false)
			{
				//坚屏
				CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_WIDTH, 1060);
				CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_HEIGHT, 1810);

			}
			else
			{
				//横屏
				CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_WIDTH, 1860);
				CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_HEIGHT, 980);
			}

			//总页码
			mulPageCount = ldwReadLen / miFontPage[miTxtFontSizeIndex];
			
			mpwcharTxt[2000] = UNICODE_NULL;
			
			CExMessage::SendMessageWithText(mpIterTxt, mpIterator, EACT_LABEL_SET_TEXT, mpwcharTxt);
			mpIterTxt->SetVisible(true);
			CMM_SAFE_CLOSE_HANDLE(mHandleTxtFile);
			lbRet = true;
			break;
		}
			
		mbIsTxt = false;
		mpIterTxt->SetVisible(false);

		auto error = pdfModule->OpenDocument(npszPath, &pdfDoc);
		if (ERR_FAILED(error))
			break;

		//文件总页数
		mulPageCount = pdfDoc->GetPageCount();

		lbRet = true;

	} while (false);

	return lbRet;
}

void CPdfPicture::CloseFile(bool nbUpdateView)
{
	CMM_SAFE_RELEASE(pdfImage);
	CMM_SAFE_RELEASE(pdfImage2);
	CMM_SAFE_RELEASE(pdfCrtPage);
	CMM_SAFE_RELEASE(pdfCrtPage2);
	CMM_SAFE_RELEASE(pdfDoc);
	CMM_SAFE_RELEASE(mpElBitmap);

	CMM_SAFE_CLOSE_HANDLE(mHandleTxtFile);

	if (nbUpdateView != false)
	{
		EinkuiGetSystem()->ClearEinkBuffer();
		EinkuiGetSystem()->UpdateView();
	}
}

//页面跳转
bool CPdfPicture::GoToPage(ULONG nulPageNumber)
{
	bool lbRet = false;
	bool duopageOK = false;
	ULONG duopageStart;

	do
	{
		if (pdfDoc == NULL)
			break; // 文档未打开

		if(nulPageNumber <= 0 || nulPageNumber > mulPageCount)
			break; //页码无效

		pageNo = 0;
		CMM_SAFE_RELEASE(pdfCrtPage);//释放
		CMM_SAFE_RELEASE(pdfCrtPage2);

		duopageStart = ((nulPageNumber - 1) & 0xFFFFFFFE) + 1;
		// 如果是双页显示模式，则从奇数页对其，打开两页
		if (landScope != false && duopageMode != false && duopageStart+1 <= mulPageCount)	// 从最后一页开始，就没有两页内容供展开了
		{
			ED_SIZE sizeP1, sizeP2;
			IEdPage_ptr pageNo1, pageNo2;
			do 
			{
				pageNo1 = loadPage(duopageStart-1, sizeP1);
				if (pageNo1 == NULL)
					break;
				pageNo2 = loadPage(duopageStart, sizeP2);
				if(pageNo2 == NULL)
					break;

				//if (sizeP1.x + sizeP2.x + 40> mViewCtl.GetViewPort().x)
					//break; // 两页拼合宽度大于屏幕尺寸，当前情况不支持双页显示

				//if(sizeP1.x != sizeP2.x || sizeP1.y != sizeP2.y)
				//	break;// 两页的基础放大倍数，基本尺寸必须完全一样，稍有不同则不能拼合成双页

				duopageOK = true;
			} while (false);

			if (pageNo1 == NULL || pageNo2 == NULL)
			{
				CMM_SAFE_RELEASE(pageNo1);
				CMM_SAFE_RELEASE(pageNo2);
				break;// 存在错误
			}

			if (duopageOK != false)
			{
				sizeP1.x += sizeP2.x;
				pdfCrtPage = pageNo1;
				pdfCrtPage2 = pageNo2;
			}
			else
			{
				if (nulPageNumber == duopageStart)
				{
					pdfCrtPage = pageNo1;
					CMM_SAFE_RELEASE(pageNo2);
				}
				else
				{
					pdfCrtPage = pageNo2;
					CMM_SAFE_RELEASE(pageNo1);
				}

			}

			mViewCtl.SetImageInit(sizeP1);
			mViewCtl.SetCenterGap(pageGap);
		}
		else
		{
			ED_SIZE sizeInit;
			pdfCrtPage = loadPage(nulPageNumber - 1,sizeInit);
			if(pdfCrtPage == NULL)
				break;
			mViewCtl.SetImageInit(sizeInit);
			mViewCtl.SetCenterGap(0.0f);
		}

		pageNo = nulPageNumber;
		lbRet = RenderPages();
		if (lbRet != false)
		{
			EinkuiGetSystem()->ClearEinkBuffer();
			EinkuiGetSystem()->UpdateView();
		}
			

	} while (false);

	return lbRet;
}

bool CPdfPicture::PageFoward(bool foward)
{
#ifdef _AX_TEST_MEMLEAK
	if (foward == false)
	{
		EinkuiGetSystem()->ExitXui();
		return false;
	}
#endif//_AX_TEST_MEMLEAK

	if (pdfCrtPage2 == NULL)
	{
		if (foward == false)
			return GoToPage(pageNo - 1);
		else
			return GoToPage(pageNo + 1);
	}
	else
	{
		auto pageStart = ((pageNo - 1) & 0xFFFFFFFE) + 1;
		if (foward == false)
			return GoToPage(pageStart - 2);
		else
			return GoToPage(pageStart + 2);
	}
}

ULONG CPdfPicture::GetCurrentPageNumber(ULONG& secondPage)
{
	if (pdfCrtPage == NULL)
		return 0;

	if (pdfCrtPage2 != NULL)
		secondPage = pdfCrtPage2->GetPageIndex() + 1;
	else
		secondPage = 0;

	return pdfCrtPage->GetPageIndex() + 1;
}

//设置缩放
//rMove4个方向，0表示不能移动了，1表示还能移动
bool CPdfPicture::SetScaleRatio(float nfScale, OUT RECT& rMove)
{
	bool lbRet = false;

	do
	{
		mViewCtl.SetUserRatio(nfScale);

		if (pdfImage == NULL)
			break;

		CalcMovalbe(rMove);

		lbRet = RenderPages();
		if (lbRet != false)
		{
			EinkuiGetSystem()->ClearEinkBuffer();
			EinkuiGetSystem()->UpdateView();
		}
			

	} while (false);

	return lbRet;
}

//移动页面
//niXOffset,niYOffset正数表示向右下移动，负数表示向左上移动
//rMove4个方向，0表示不能移动了，1表示还能移动
bool CPdfPicture::MovePage(int niXOffset, int niYOffset, OUT RECT& rMove)
{
	bool lbRet = false;

	do
	{
		ED_POINT moveOffset;
		moveOffset.x = (float32)niXOffset;
		moveOffset.y = (float32)niYOffset;
		mViewCtl.Move(moveOffset);

		CalcMovalbe(rMove);

		EinkuiGetSystem()->ClearEinkBuffer();
		EinkuiGetSystem()->UpdateView(true);
		lbRet = true;

	} while (false);

	return lbRet;

}


//获取总页数
ULONG CPdfPicture::GetPageCount(void)
{
	return mulPageCount;
}

//屏幕发生旋转
void CPdfPicture::SetRotation(ULONG nulRotation)
{
	do 
	{
		//mpIterPicture->SetPosition(mdOldPos);
		//

		if (nulRotation == GIR_NONE || nulRotation == GIR_180)
		{
			landScope = true;
		}
		else if (nulRotation == GIR_90 || nulRotation == GIR_270)
		{
			landScope = false;
		}

		if (landScope == false)
		{
			//坚屏
			CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_WIDTH, 1020);
			CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_HEIGHT, 1810);

		}
		else
		{
			//横屏
			CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_WIDTH, 1860);
			CExMessage::SendMessage(mpIterTxt, mpIterator, EACT_LABEL_SET_MAX_HEIGHT, 980);
		}

		if (pdfCrtPage != NULL)
			GoToPage(pageNo);

	} while (false);

}

void CPdfPicture::EnableDuopageView(bool nbEnable)
{
	bool32 changed = (duopageMode == false && nbEnable != false || duopageMode != false && nbEnable == false);

	duopageMode = nbEnable;

	if (changed == false)
		return;

	GoToPage(pageNo);
	//else
	//	GoToPage(1);
}

bool CPdfPicture::CopyToClipboard(const D2D1_RECT_F& rRegion)
{
	bin_ptr bufToCopy = NULL;
	D2D1_RECT_F intersectRect;
	bool resultOK = false;
	HBITMAP bitmapHandle = NULL;

	do 
	{
		if (mpElBitmap == NULL || CExRect::Intersect(rRegion,mRecentDst,intersectRect) == false)
			break;

		int32 imageWidth = CExFloat::ToLong(ED_RECT_WIDTH(intersectRect));
		int32 imageHeight = CExFloat::ToLong(ED_RECT_HEIGHT(intersectRect));
		if (imageWidth == 0 || imageHeight == 0)
			break;

		bufToCopy = (bin_ptr)HeapAlloc(GetProcessHeap(),0,imageWidth*imageHeight*4);
		if (bufToCopy == NULL)
			break;

		intersectRect.left = CExFloat::Floor(intersectRect.left - mRecentDst.left + mRecentSrc.left);
		intersectRect.right = intersectRect.left + (float32)imageWidth;
		intersectRect.top = CExFloat::Floor(intersectRect.top - mRecentDst.top + mRecentSrc.top);
		intersectRect.bottom = intersectRect.top + (float32)imageHeight;

		if (mpElBitmap->GetBitmapBuffer(intersectRect,bufToCopy)==false)
			break;

		bitmapHandle = CreateCompatibleBitmap(GetDC(NULL), imageWidth,imageHeight);
		if(bitmapHandle == NULL)
			break;

		if(SetBitmapBits(bitmapHandle, imageWidth*imageHeight * 4,bufToCopy)<=0)
			break;

		if(OpenClipboard(NULL)==FALSE)
			break;
		EmptyClipboard();

		SetClipboardData(CF_BITMAP,bitmapHandle);

		CloseClipboard();

		resultOK = true;
	} while (false);

	if (bitmapHandle != NULL)
		DeleteObject(bitmapHandle);

	if(bufToCopy != NULL)
		HeapFree(GetProcessHeap(), 0, bufToCopy);


	return resultOK;
}

float CPdfPicture::GetFatRatio(void)
{
	return mViewCtl.GetFatRatio() / mViewCtl.GetBaseRatio();
}

//通知元素【显示/隐藏】发生改变
ERESULT CPdfPicture::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}


IEdPage_ptr CPdfPicture::loadPage(int32 pageNo, ED_SIZE& sizeInit)
{
	IEdPage_ptr pageGot = NULL;
	ED_RECTF box = { 0,0,1,1 };
	IEdPage_ptr pageReturn = NULL;

	do
	{
		pageGot = pdfDoc->GetPage(pageNo);
		if (pageGot == NULL)
			break;

		if(pageGot->GetMediaBox(&box)==false)
			break;

		sizeInit.x = CExFloat::ToLong(ED_RECT_WIDTH(box));
		sizeInit.y = CExFloat::ToLong(ED_RECT_HEIGHT(box));

		pageReturn = pageGot;

	} while (false);

	return pageReturn;
}

bool CPdfPicture::RenderPages(void)
{
	bool lbRet = false;
	bin_ptr imageBuffer = NULL;

	do
	{
		if (pdfCrtPage == NULL)
			break;

		CMM_SAFE_RELEASE(pdfImage);
		CMM_SAFE_RELEASE(pdfImage2);
		CMM_SAFE_RELEASE(mpElBitmap);
		if (duopageBuf != NULL)
		{
			HeapFree(GetProcessHeap(), 0, duopageBuf);
			duopageBuf = NULL;
		}


		pdfImage = pdfCrtPage->Render(mViewCtl.GetRealRatio(), true);
		if (pdfImage == NULL)
			break;

		if (pdfCrtPage2 != NULL)
		{
			pdfImage2 = pdfCrtPage2->Render(mViewCtl.GetRealRatio(), true);
			if (pdfImage2 == NULL)
				break;

			imageSize.x = (pdfImage->GetWidth() <= pdfImage2->GetWidth() ? pdfImage->GetWidth() : pdfImage2->GetWidth());
			imageSize.y = (pdfImage->GetHeight() <= pdfImage2->GetHeight() ? pdfImage->GetHeight() : pdfImage2->GetHeight());

			imageBuffer = (bin_ptr)HeapAlloc(GetProcessHeap(), 0, (imageSize.x * 2 + pageGap) * 4 * imageSize.y);
			if(imageBuffer == NULL)
				break;

			bin_ptr src1, src2, dst;
			int32 stride = imageSize.x * 4;
			dst = imageBuffer;
			src1 = pdfImage->GetBuffer();
			src2 = pdfImage2->GetBuffer();

			for (int32 i = 0; i < imageSize.y; i++)
			{
				RtlCopyMemory(dst, src1, stride);
				dst += stride;
				if (pageGap >= 1.0f)
				{
	//				RtlZeroMemory(dst, pageGap * 4);
					memset(dst, 0xFF, pageGap * 4);
					dst += pageGap * 4;
				}
				RtlCopyMemory(dst, src2, stride);
				dst += stride;
				src1 += stride;
				src2 += stride;
			}
			imageSize.x = imageSize.x*2 + pageGap;

			duopageBuf = imageBuffer;
		}
		else
		{
			imageBuffer = pdfImage->GetBuffer();

			imageSize.x = pdfImage->GetWidth();
			imageSize.y = pdfImage->GetHeight();
		}

		mViewCtl.AdjustImageRealSize(imageSize);


		mpElBitmap = EinkuiGetSystem()->GetAllocator()->CreateBitmapFromMemory(imageSize.x,imageSize.y, 4,imageSize.x * 4, imageBuffer);
		if (mpElBitmap == NULL)
			break;

		lbRet = true;

	} while (false);

	return lbRet;
}

void CPdfPicture::CalcMovalbe(OUT RECT& rMove)
{
	ED_RECT dstRect;
	ED_RECT srcRect;
	UCHAR edgeImpact;

	if (mViewCtl.GetViewMapArea(dstRect, srcRect,&edgeImpact) == false)
	{
		rMove.left = rMove.right = rMove.top = rMove.bottom = 0;
	}
	else
	{
		rMove.left = ((edgeImpact & 1) > 0);
		rMove.right = ((edgeImpact & 2) > 0);
		rMove.top = ((edgeImpact & 4) > 0);
		rMove.bottom = ((edgeImpact & 8) > 0);
	}
}

//绘制
ERESULT CPdfPicture::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ED_RECT dstRect;
	ED_RECT srcRect;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		leResult = CXuiElement::OnPaint(npPaintBoard);
		if (ERESULT_FAILED(leResult))
			break;

		if (mpElBitmap != NULL && mViewCtl.GetViewMapArea(dstRect, srcRect) != false)
		{
			mRecentDst.left = (float32)dstRect.left;
			mRecentDst.right = (float32)dstRect.right;
			mRecentDst.top = (float32)dstRect.top;
			mRecentDst.bottom = (float32)dstRect.bottom;
			mRecentSrc.left = (float32)srcRect.left;
			mRecentSrc.right = (float32)srcRect.right;
			mRecentSrc.top = (float32)srcRect.top;
			mRecentSrc.bottom = (float32)srcRect.bottom;

			npPaintBoard->DrawBitmap(mRecentDst,mRecentSrc,mpElBitmap, ESPB_DRAWBMP_LINEAR);
		}

		leResult = ERESULT_SUCCESS;
	} while (false);

	return leResult;
}

void CPdfPicture::GetRectOfViewportOnPage(D2D1_SIZE_F& nrImageSize, D2D1_RECT_F& nrViewPort)
{
	nrImageSize.width = (FLOAT)imageSize.x;
	nrImageSize.height = (FLOAT)imageSize.y;

	nrViewPort = mRecentSrc;
}

