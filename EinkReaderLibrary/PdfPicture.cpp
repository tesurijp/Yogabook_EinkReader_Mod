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

// 轻度颜色加深
const unsigned char pixelGradeImprovement1[256] = \
	{	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x08, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x12,
		0x14, 0x15, 0x16, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1f, 0x20, 0x21, 0x23, 0x24, 0x26, 0x27, 0x28,
		0x2a, 0x2b, 0x2d, 0x2e, 0x2f, 0x31, 0x32, 0x34, 0x35, 0x36, 0x38, 0x39, 0x3b, 0x3c, 0x3d, 0x3f,
		0x40, 0x41, 0x43, 0x44, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4d, 0x4e, 0x4f, 0x51, 0x52, 0x54, 0x55,
		0x56, 0x58, 0x59, 0x5b, 0x5c, 0x5d, 0x5f, 0x60, 0x62, 0x63, 0x64, 0x66, 0x67, 0x69, 0x6a, 0x6b,
		0x6d, 0x6e, 0x6f, 0x71, 0x72, 0x74, 0x75, 0x76, 0x78, 0x79, 0x7b, 0x7c, 0x7d, 0x7f, 0x80, 0x82,
		0x83, 0x84, 0x86, 0x87, 0x89, 0x8a, 0x8b, 0x8d, 0x8e, 0x90, 0x91, 0x92, 0x94, 0x95, 0x96, 0x98,
		0x99, 0x9b, 0x9c, 0x9d, 0x9f, 0xa0, 0xa2, 0xa3, 0xa4, 0xa6, 0xa7, 0xa9, 0xaa, 0xab, 0xad, 0xae,
		0xb0, 0xb1, 0xb2, 0xb4, 0xb5, 0xb7, 0xb8, 0xb9, 0xbb, 0xbc, 0xbe, 0xbf, 0xc0, 0xc2, 0xc3, 0xc4,
		0xc6, 0xc7, 0xc9, 0xca, 0xcb, 0xcd, 0xce, 0xd0, 0xd1, 0xd2, 0xd4, 0xd5, 0xd7, 0xd8, 0xd9, 0xdb,
		0xdc, 0xde, 0xdf, 0xe0, 0xe2, 0xe3, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xed, 0xee, 0xf0, 0xf1,
		0xf2, 0xf4, 0xf5, 0xf7, 0xf8, 0xf9, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

// 重度颜色加深
const unsigned char pixelGradeImprovement2[256] = \
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1d, 0x1e, 0x20, 0x21, 0x22, 0x24, 0x25, 0x27, 0x29, 0x2a, 0x2c, 0x2d,
	0x2f, 0x31, 0x33, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 0x44, 0x45, 0x47, 0x49, 0x4b,
	0x4d, 0x4f, 0x52, 0x54, 0x56, 0x58, 0x5b, 0x5d, 0x5f, 0x62, 0x64, 0x67, 0x69, 0x6c, 0x6e, 0x71,
	0x74, 0x76, 0x79, 0x7c, 0x7e, 0x81, 0x83, 0x85, 0x88, 0x8b, 0x8e, 0x91, 0x94, 0x97, 0x9a, 0x9d,
	0xa0, 0xa3, 0xa6, 0xa9, 0xac, 0xb0, 0xb3, 0xb6, 0xb9, 0xbd, 0xc0, 0xc3, 0xc7, 0xca, 0xce, 0xd1,
	0xd3, 0xd6, 0xda, 0xde, 0xe1, 0xe5, 0xe8, 0xec, 0xf0, 0xf4, 0xf7, 0xfb, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

CPdfPicture::CPdfPicture(void)
{
	mulPageCount = 0;

	pdfDoc = NULL;
	pdfCrtPage = NULL;
	pdfCrtPage2 = NULL;
	pdfImage = NULL;
	pdfImage2 = NULL;
	mpElBitmap = NULL;
	pageCanvas = NULL;
	pageCanvasSize = 0;
	landScope = true;
	duopageMode = false;
	pageGap = 0;// 20;
	pageNo = 0;
	mIsTxtDoc = false;
	mFontSize = 12;
	mTitleBarHeight = 0.0f;
	mLoading = 0;
	mpLoadingView = NULL;

	wchar_t* const moduleName[] = {
		//L"EdPdf.dll",
		L"EdSmt.dll",
		L"EdTxt.dll"
	};	// 不要大于pdfModuleArr数组，目前是10

	for (auto &i : pdfModuleArr)
		i = NULL;

	for (auto j : moduleName)
	{
		auto libModule = LoadLibrary(j);
		if (libModule != NULL)
		{
			auto proc = (GetModule_Proc)GetProcAddress(libModule, "EdGetModule");
			for (auto &i : pdfModuleArr)
			{
				if (i == NULL)
				{
					i = proc();
					break;
				}
			}
		}
	}

}


CPdfPicture::~CPdfPicture(void)
{
	if (pageCanvas != NULL)
		HeapFree(GetProcessHeap(), 0, pageCanvas);

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
		/*mpIterBtShowTip = mpIterator->GetSubElementByID(PP_BT_SHOW_TIP);
		BREAK_ON_NULL(mpIterBtShowTip);*/
		

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

void __stdcall PageLoadedCallBack(uint32Eink loadingStep, uint32Eink pageCount, CPdfPicture* viewObject)
{
	viewObject->OnPageLoaded(loadingStep, pageCount);
}

void CPdfPicture::OnPageLoaded(int32 loadingStep, int32 pagecount)
{
	static int count = 0;

	InterlockedExchange(&mLoading, loadingStep + 1);

	if (pdfCrtPage != NULL)
		pageNo = pdfCrtPage->GetPageIndex();

	mulPageCount = pagecount;

	switch (loadingStep)
	{
	case 0:
		// 这儿表示排版开始
		PostMessageToParent(EEVT_TXT_ARRANGED_START, CExMessage::DataInvalid);
		break;
	case MAXULONG32:
		{
			// 这儿表示排版完毕
			PostMessageToParent(EEVT_ARRANGED_COMPLETE, pagecount);

			if (mpLoadingView != NULL)
			{
				mpLoadingView->ExitModal();
				mpLoadingView = NULL;
			}

			// 给上CPdfPicture::OpenFile(wchar_t* npszPath, PPAGE_PDF_CONTEXT npPageContext)中阻塞处建立的模态对话框发送消息，让这个对话框退出
			// ???niu???
			break;
		}
	default:
		
		PostMessageToParent(EEVT_TXT_ARRANGED_DOING, pagecount);
		break;
	}
}

void CPdfPicture::SetLoadingPageIndex(LONG nlPageIndex)
{
	if (mpLoadingView != NULL)
	{
		mpLoadingView->SetPage(nlPageIndex);
	}
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
	if (pdfDoc != NULL && pdfCrtPage !=NULL)
	{
		pdfDoc -> SetViewPort(CExFloat::ToLong(viewPort.x-176.0f), CExFloat::ToLong(viewPort.y - 176.0f - mTitleBarHeight));	// 图像高低缩小一定像素

		//if (IsTxtDocument() != false)
		//{
			auto newPage = pdfDoc->Rearrange(pdfCrtPage);

			mulPageCount = pdfDoc->GetPageCount();

			GoToPage(newPage);
		//}

	}

	if(mpLoadingView != NULL)
		mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());

	return ERESULT_SUCCESS;
}

//设置要预览的图片,和接收消息的对象
ULONG CPdfPicture::OpenFile(wchar_t* npszPath, PPAGE_PDF_CONTEXT npPageContext)
{
	ULONG lulRet = EDERR_UNSUCCESSFUL;
	bool updateLayout = true;
	PAGE_PDF_CONTEXT defaultContext = { 0,0,0 };

	if (npPageContext != NULL)
	{
		defaultContext.pageIndex = PAGEID_LIB(npPageContext->pageIndex);
		defaultContext.pageContext = npPageContext->pageContext;
		defaultContext.pageContext2 = npPageContext->pageContext2;
	}

	CloseFile(false);

	//CMMASSERT(pdfDoc == NULL);

	do 
	{
		BREAK_ON_NULL(npszPath);

		for (auto i : pdfModuleArr)
		{
			if(i == NULL)
				break;

			lulRet = i->OpenDocument(npszPath, &pdfDoc);
			if (ERR_SUCCESS(lulRet) && pdfDoc != NULL)
			{
				InterlockedExchange(&mLoading, 1);

				if (pdfDoc->LoadAllPage((PEDDOC_CALLBACK)PageLoadedCallBack, (void*)this) != false)
				{
					if (_wcsicmp(i->GetTypeName(pdfDoc->GetDocType()), L"TXT") == 0)
					{
						mIsTxtDoc = true;
					}
					else
					{
						mIsTxtDoc = false;

						if (mLoading != 0)
						{
							// ???niu???在此显示一个自己ui库的模态对话框，阻挡住程序下行，在模态对话框中显示进度
							ICfKey* lpSubKey = mpTemplete->OpenKey(L"Loading");
							mpLoadingView = CLoadingView::CreateInstance(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), lpSubKey);
							wchar_t* lpszFileName = wcsrchr(npszPath, L'\\');
							if (lpszFileName != NULL)
								lpszFileName = lpszFileName + 1;
							mpLoadingView->SetData(lpszFileName);
							mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());
							mpLoadingView->DoModal(&mLoading,NULL);
							mpLoadingView = NULL;
							CMM_SAFE_RELEASE(lpSubKey);
							
						}

						// 在下面的PageLoadedCallBack函数中，给本模态对话框发送消息，让这个对话框退出
					}
					break;
				}
				else
					CMM_SAFE_RELEASE(pdfDoc);
			}
		}

		BREAK_ON_NULL(pdfDoc);


		if (mIsTxtDoc != false)
			mTitleBarHeight = 0.0f;
		else
			mTitleBarHeight = 0.0f;

		if (mViewCtl.GetViewPort().x > 0 && mViewCtl.GetViewPort().y > 0)
		{
			pdfDoc->SetViewPort(CExFloat::ToLong(mViewCtl.GetViewPort().x - 176.0f), CExFloat::ToLong(mViewCtl.GetViewPort().y - 176.0f - mTitleBarHeight));	// 图像高低缩小一定像素
		}
		else
			updateLayout = false;

		if (mFontSize > 0)
			pdfDoc->SetFont(L"微软雅黑", mFontSize);
		else
			updateLayout = false;

		if (updateLayout != false)
		{
			pdfCrtPage = pdfDoc->Rearrange(&defaultContext);
			if (pdfCrtPage == NULL)
			{
				//打开文件失败
				CloseFile();
				lulRet = EDERR_UNSUCCESSFUL;
				break;
			}

			//文件总页数
			mulPageCount = pdfDoc->GetPageCount();
		}
		else
			mulPageCount = 0;

		duopageMode = false;

		lulRet = EDERR_SUCCESS;

	} while (false);

	return lulRet;
}

void CPdfPicture::CloseFile(bool nbUpdateView)
{
	__try
	{
		CMM_SAFE_RELEASE(pdfImage);
		CMM_SAFE_RELEASE(pdfImage2);
		CMM_SAFE_RELEASE(pdfCrtPage);
		CMM_SAFE_RELEASE(pdfCrtPage2);
		CMM_SAFE_RELEASE(mpElBitmap);

		CMM_SAFE_RELEASE(pdfDoc);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		pdfDoc = NULL;
	}
	mulPageCount = 0;

	if (nbUpdateView != false)
	{
		EinkuiGetSystem()->ClearEinkBuffer();
		EinkuiGetSystem()->UpdateView();
	}
}

bool CPdfPicture::IsTxtDocument(void)
{
	return mIsTxtDoc;
}

//页面跳转
bool CPdfPicture::GoToPage(ULONG nulPageNumber)
{
	if (nulPageNumber > mulPageCount)
		return false; //无效页码

	IEdPage_ptr pageGoto;

	pageGoto = pdfDoc->GetPage(PAGEID_LIB(nulPageNumber));
	if (pageGoto == NULL)
		return false;

	return GoToPage(pageGoto);
}

bool CPdfPicture::GoToPage(IEdPage_ptr pageGoto)
{
	bool lbRet = false;
	IEdPage_ptr pageDuo = NULL;
	ED_SIZE sizePageGoto, sizePageDuo;

	if (pageGoto == NULL)
	{
		if (pdfCrtPage == NULL)
			return false;

		pageGoto = pdfCrtPage;
		pageGoto->AddRefer();	// 增加引用，防止下面的释放pdfCrtPage，将不同名的同一对象删除
	}

	// 如果是双页显示模式，则从奇数页对其，打开两页
	if (landScope != false && duopageMode != false)
	{
		if ((pageGoto->GetPageIndex() % 2) != 0)		// 这个地方不能这样改，现在的行号是不能作为准确判断，因为装入过程在变化，你需要告诉我，修改的目的，我帮你改成想要的效果。 ???niu???
		{
			//双页显示要从奇数页开始
			pageDuo = pageGoto;
			pageGoto = pdfDoc->GetPage(pageGoto, -1);
		}
		else
		{
			pageDuo = pdfDoc->GetPage(pageGoto, 1);	// 取下一页
		}
	}

	GetPageSize(pageGoto,sizePageGoto);

	//pageGoto->AddRefer();
	CMM_SAFE_RELEASE(pdfCrtPage);
	pdfCrtPage = pageGoto;

	if (pageDuo != NULL)
	{
		GetPageSize(pageDuo, sizePageDuo);

		sizePageGoto.x += sizePageDuo.x;
		mViewCtl.SetCenterGap((float)pageGap);

		CMM_SAFE_RELEASE(pdfCrtPage2);
		pdfCrtPage2 = pageDuo;
	}
	else
	{
		CMM_SAFE_RELEASE(pdfCrtPage2);
		mViewCtl.SetCenterGap(0.0f);
	}

	mViewCtl.SetImageInit(sizePageGoto);

	lbRet = RenderPages();
	if (lbRet != false)
	{
		EinkuiGetSystem()->ClearEinkBuffer();
		EinkuiGetSystem()->UpdateView();
	}

	return lbRet;
}

bool CPdfPicture::PageFoward(bool forward)
{
#ifdef _AX_TEST_MEMLEAK
	if (forward == false)
	{
		EinkuiGetSystem()->ExitXui();
		return false;
	}
#endif//_AX_TEST_MEMLEAK
	CMMASSERT(pdfCrtPage != NULL);

	IEdPage_ptr pageJump = NULL;

	if (duopageMode != false && landScope != false)
	{
		//只有在双页并且横屏下
		if (forward != false)
			pageJump = pdfDoc->GetPage(pdfCrtPage, 2);
		else
			pageJump = pdfDoc->GetPage(pdfCrtPage, -2);
	}

	if (pageJump == NULL)
	{
		if (forward != false)
			pageJump = pdfDoc->GetPage(pdfCrtPage, 1);
		else
			pageJump = pdfDoc->GetPage(pdfCrtPage, -1);
	}


	if(pageJump != NULL)
		return GoToPage(pageJump);

	return false;
}

ULONG CPdfPicture::GetCurrentPageNumber(ULONG& secondPage)
{
	if (pdfCrtPage == NULL)
		return 0;

	if (pdfCrtPage2 != NULL)
		secondPage = PAGEID_USER(pdfCrtPage2->GetPageIndex());
	else
		secondPage = 0;

	return PAGEID_USER(pdfCrtPage->GetPageIndex());
}

//获得当前第一页的上下文，当前页的页码在结构体的pageIndex返回
bool CPdfPicture::GetCrtPageContext(PPAGE_PDF_CONTEXT npPageContext)
{
	if (pdfCrtPage == NULL)
		return false;

	pdfCrtPage->GetPageContext(npPageContext);

	npPageContext->pageIndex = PAGEID_USER(npPageContext->pageIndex);

	return true;
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

bool CPdfPicture::SetFontSize(int fontSize)
{
	mFontSize = fontSize;
	
	if (pdfDoc != NULL)
	{
		pdfDoc->SetFont(L"微软雅黑", mFontSize);

		PAGE_PDF_CONTEXT pageConText = { 0,0,0 };

		if (pdfCrtPage != NULL)
			pdfCrtPage->GetPageContext(&pageConText);

		auto newPage = pdfDoc->Rearrange(pdfCrtPage);

		mulPageCount = pdfDoc->GetPageCount();

		GoToPage(newPage);
	}

	return true;
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

		//if (pdfCrtPage != NULL)
		//{
		//	pdfCrtPage->AddRefer();
		//	GoToPage(pdfCrtPage);
		//}

	} while (false);

}

void CPdfPicture::EnableDuopageView(bool nbEnable)
{
	bool32 changed = (duopageMode == false && nbEnable != false || duopageMode != false && nbEnable == false);

	duopageMode = nbEnable;

	if (changed == false)
		return;

	if (pdfCrtPage != NULL)
	{
		pdfCrtPage->AddRefer();
		GoToPage(pdfCrtPage);
	}
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

		//不能超出原图大小
		if (mpElBitmap->GetWidth() < (intersectRect.right- intersectRect.left))
			intersectRect.right = mpElBitmap->GetWidth() + intersectRect.left;
		if (mpElBitmap->GetHeight() < (intersectRect.bottom- intersectRect.top))
			intersectRect.bottom = mpElBitmap->GetHeight() + intersectRect.top;

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

bool32 CPdfPicture::GetPageSize(IEdPage_ptr pagePtr, ED_SIZE& sizeInit)
{
	IEdPage_ptr pageGot = NULL;
	ED_RECTF box = { 0,0,1,1 };

	if (pagePtr->GetMediaBox(&box) == false)
		return false;

	sizeInit.x = CExFloat::ToLong(ED_RECT_WIDTH(box));
	sizeInit.y = CExFloat::ToLong(ED_RECT_HEIGHT(box));

	return true;
}

bin_ptr CPdfPicture::PrepareCanvas(int32 totalBytes)
{
	if (totalBytes > pageCanvasSize)
	{
		if (pageCanvas != NULL)
		{
			HeapFree(GetProcessHeap(), 0, pageCanvas);
			pageCanvas = NULL;
			pageCanvasSize = 0;
		}

		pageCanvas = (bin_ptr)HeapAlloc(GetProcessHeap(), 0, totalBytes);
		if (pageCanvas != NULL)
			pageCanvasSize = totalBytes;
	}

	return pageCanvas;
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

			imageBuffer = PrepareCanvas((imageSize.x * 2 + pageGap) * 4 * imageSize.y);
			if(imageBuffer == NULL)
				break;

			bin_ptr src1, src2;
			register bin_ptr dst, dstEnd;
			int32 stridemin = imageSize.x * 4;
			int32 stride1 = pdfImage->GetWidth() * 4;
			int32 stride2 = pdfImage2->GetWidth() * 4;
			dst = imageBuffer;
			src1 = pdfImage->GetBuffer();
			src2 = pdfImage2->GetBuffer();

			if (mIsTxtDoc != false)
			{
				for (int32 i = 0; i < imageSize.y; i++)
				{
					RtlCopyMemory(dst, src1, stridemin);
					dst += stridemin;
					if (pageGap >= 1.0f)
					{
						//				RtlZeroMemory(dst, pageGap * 4);
						memset(dst, 0xFF, pageGap * 4);
						dst += pageGap * 4;
					}
					RtlCopyMemory(dst, src2, stridemin);
					dst += stridemin;
					src1 += stride1;
					src2 += stride2;
				}
			}
			else
			{
				for (int32 i = 0; i < imageSize.y; i++)
				{
					register unsigned char* src = src1;
					dstEnd = dst + stridemin;
					while (dst < dstEnd)
					{
						*dst++ = pixelGradeImprovement2[*src++];
					}

					if (pageGap >= 1.0f)
					{
		//				RtlZeroMemory(dst, pageGap * 4);
						memset(dst, 0xFF, pageGap * 4);
						dst += pageGap * 4;
					}

					src = src2;
					dstEnd = dst + stridemin;
					while (dst < dstEnd)
					{
						*dst++ = pixelGradeImprovement2[*src++];
					}

					src1 += stride1;
					src2 += stride2;
				}
			}

			imageSize.x = imageSize.x*2 + pageGap;
		}
		else
		{
			auto src = pdfImage->GetBuffer();
			imageSize.x = pdfImage->GetWidth();
			imageSize.y = pdfImage->GetHeight();

			if (mIsTxtDoc != false)
			{
				imageBuffer = src;
			}
			else
			{
				imageBuffer = PrepareCanvas(imageSize.x*imageSize.y * 4);

				LONG totalBytes = imageSize.x * imageSize.y * 4;
				register unsigned char* lpDst = imageBuffer;
				register unsigned char* lpSrc = src;
				register unsigned char* lpSrcEnd = src +totalBytes;

				while(lpSrc < lpSrcEnd)
				{
					*lpDst++ = pixelGradeImprovement2[*lpSrc++];
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// 图像增强算法，对于显示图片内容十分有效
			//LONG stride = imageSize.x * 4;

			//RtlCopyMemory(imageBuffer, src, stride);
			//RtlCopyMemory(imageBuffer + stride*(imageSize.y - 1), src + stride*(imageSize.y - 1), stride);

			//for (LONG i = 1; i < imageSize.y - 1; i++)
			//{
			//	register unsigned char* lpDst = imageBuffer + stride*i;
			//	register unsigned char* lpSrc = src + stride*i;
			//	register short EdgeVaule;
			//	LONG j;

			//	*(ULONG*)lpDst = *(ULONG*)lpSrc;
			//	lpDst += 4;
			//	lpSrc += 4;

			//	for (j = 0; j < imageSize.x - 2; j++)
			//	{
			//		EdgeVaule = (*lpSrc) * 5 - ((*(lpSrc - stride)) + (*(lpSrc - 4)) + (*(lpSrc + 4)) + (*(lpSrc + stride)));
			//		if (EdgeVaule > 255)
			//			*lpDst = 255;
			//		else if (EdgeVaule < 0)
			//			*lpDst = 0;
			//		else
			//			*lpDst = (unsigned char)EdgeVaule;

			//		lpSrc++;
			//		lpDst++;

			//		EdgeVaule = (*lpSrc) * 5 - ((*(lpSrc - stride)) + (*(lpSrc - 4)) + (*(lpSrc + 4)) + (*(lpSrc + stride)));
			//		if (EdgeVaule > 255)
			//			*lpDst = 255;
			//		else if (EdgeVaule < 0)
			//			*lpDst = 0;
			//		else
			//			*lpDst = (unsigned char)EdgeVaule;

			//		lpSrc++;
			//		lpDst++;

			//		EdgeVaule = (*lpSrc) * 5 - ((*(lpSrc - stride)) + (*(lpSrc - 4)) + (*(lpSrc + 4)) + (*(lpSrc + stride)));
			//		if (EdgeVaule > 255)
			//			*lpDst = 255;
			//		else if (EdgeVaule < 0)
			//			*lpDst = 0;
			//		else
			//			*lpDst = (unsigned char)EdgeVaule;

			//		lpSrc++;
			//		lpDst++;

			//		EdgeVaule = (*lpSrc) * 5 - ((*(lpSrc - stride)) + (*(lpSrc - 4)) + (*(lpSrc + 4)) + (*(lpSrc + stride)));
			//		if (EdgeVaule > 255)
			//			*lpDst = 255;
			//		else if (EdgeVaule < 0)
			//			*lpDst = 0;
			//		else
			//			*lpDst = (unsigned char)EdgeVaule;

			//		lpSrc++;
			//		lpDst++;
			//	}

			//	*(ULONG*)lpDst = *(ULONG*)lpSrc;
			//}
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
			mRecentDst.top = (float32)dstRect.top + mTitleBarHeight/2.0f;// 下移一半
			mRecentDst.bottom = (float32)dstRect.bottom + mTitleBarHeight / 2.0f;// 下移一半
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

