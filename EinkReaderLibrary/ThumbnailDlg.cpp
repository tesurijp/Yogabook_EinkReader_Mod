/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ThumbnailDlg.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include "PreNextButton.h"
#include "..\ECtl\EvButtonImp.h"

DEFINE_BUILTIN_NAME(ThumbnailDlg)

CThumbnailDlg::CThumbnailDlg(void)
{
	mpMenuThumbnail = NULL;
	mpIterBtPageNumber = NULL;
	mpIteratorPre = NULL;
	mpIteratorNext = NULL;
	mpIteratorSelect = NULL;

	miDocType = DOC_TYPE_PDF;
	mpPageProgress = NULL;

	mpDocument = NULL;

	mulCurrentPage = 0;
	mulDocMaxPage = 0;
	mulMaxPage = 0;
	mulAnnotPage = 0;
	mulCurrentDoc = 1;

	mszPageAll[0] = UNICODE_NULL;
	mszPageAnnot[0] = UNICODE_NULL;

	mulSelectType = THUMBNAIL_SELECT_ALL;
}


CThumbnailDlg::~CThumbnailDlg(void)
{
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CThumbnailDlg::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		

		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


ULONG CThumbnailDlg::InitOnCreate(
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
		leResult = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
		if (leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄
		mpIterBtPageNumber = mpIterator->GetSubElementByID(TD_BT_NUMBER);
		BREAK_ON_NULL(mpIterBtPageNumber);

		mpIteratorPre = mpIterator->GetSubElementByID(TD_BT_PRE);
		BREAK_ON_NULL(mpIteratorPre);

		mpIteratorNext = mpIterator->GetSubElementByID(TD_BT_NEXT);
		BREAK_ON_NULL(mpIteratorNext);

		mpIteratorSelect = mpIterator->GetSubElementByID(TD_BT_SELECT);
		BREAK_ON_NULL(mpIteratorSelect);

		//页面跳转
		lpSubKey = mpTemplete->OpenKey(L"PageProgress");
		mpPageProgress = CPageProgress::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPageProgress);
		mpPageProgress->GetIterator()->SetVisible(false);


		//创建list对象
		lpSubKey = mpTemplete->OpenKey(L"ListItem");
		for (int i = 0; i < TD_ITEM_MAX; i++)
		{
			CThumbnailListItem* lpListItem = CThumbnailListItem::CreateInstance(mpIterator, lpSubKey);
			//lpListItem->GetIterator()->SetVisible(false);

			mdList.Insert(i, lpListItem);
		}
		CMM_SAFE_RELEASE(lpSubKey);


		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}

//显示或隐藏元素
void CThumbnailDlg::ShowItem(bool nbIsShow)
{
	mpIterator->SetVisible(nbIsShow);
		
}

//按钮单击事件
ERESULT CThumbnailDlg::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		PostMessageToParent(EEVT_RESET_HIDE_TIME, true);

		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case TD_BT_PRE:
		{
			//上一页
			NextPage(false);

			//mpIterator->SetVisible(false);
			break;
		}
		case TD_BT_NEXT:
		{
			//下一页
			NextPage(true);

			//mpIterator->SetVisible(false);
			break;
		}
		case TD_BT_NUMBER:
		{
			//调整页码
			//PostMessageToParent(EEVT_ER_TWO_SCREEN, true);
			if (mpPageProgress != NULL)
			{
				mpPageProgress->SetData((FLOAT)mulCurrentPage, (FLOAT)mulMaxPage);
				mpPageProgress->GetIterator()->SetVisible(!mpPageProgress->GetIterator()->IsVisible());
			}
				

			break;
		}
		case TD_BT_SELECT:
		{
			if (mpMenuThumbnail != NULL)
			{
				mpMenuThumbnail->ExitModal();
				mpMenuThumbnail = NULL;
			}

			ICfKey* lpSubKey = mpTemplete->OpenKey(L"Menu");
			mpMenuThumbnail = CMenuThumbnail::CreateInstance(mpIterator, lpSubKey);
			EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			mpMenuThumbnail->GetIterator()->SetSize(float(ldPaintSize.w), float(ldPaintSize.h));
			mpMenuThumbnail->SetData(mulSelectType, mszPageAll, mszPageAnnot);
			mpMenuThumbnail->DoModal();

			CMM_SAFE_RELEASE(lpSubKey);
			mpMenuThumbnail = NULL;

			EinkuiGetSystem()->UpdateView(true);

			break;

		}
		case TD_BT_CLOSE:
		{
			ExitModal();
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


//绘制消息
ERESULT CThumbnailDlg::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0, 0, mpIterator->GetSizeX(), mpIterator->GetSizeY()),
				mpBgBitmap,
				ESPB_DRAWBMP_EXTEND);

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}

//消息处理函数
ERESULT CThumbnailDlg::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_MODAL_ENTER:
	{
		//// 创建要弹出的对话框
		//mpIterator->SetVisible(true);
		luResult = ERESULT_SUCCESS;
		break;
	}
	case EEVT_ER_PAGE_JUMP:
	{
		//页码跳转
		int liPage = 1;
		luResult = CExMessage::GetInputData(npMsg, liPage);
		if (luResult != ERESULT_SUCCESS)
			break;

		EnterPage(liPage);

		mpIteratorPre->SetEnable(mulCurrentPage <= 1 ? false : true);
		mpIteratorNext->SetEnable(mulCurrentPage < mulMaxPage ? true : false);

		break;
	}
	case EEVT_THUMBNAIL_CLICK:
	{
		//缩略图被点击
		int liPage = 1;
		luResult = CExMessage::GetInputData(npMsg, liPage);
		if (luResult != ERESULT_SUCCESS)
			break;

		SendMessageToParent(EEVT_ER_PAGE_JUMP, liPage,NULL,0);
		ExitModal();

		break;
	} 
	case EEVT_THUMBNAIL_SELECT:
	{
		luResult = CExMessage::GetInputData(npMsg, mulSelectType);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (mpPageProgress != NULL)
			mpPageProgress->GetIterator()->SetVisible(false);

		SelectAll(mulSelectType == THUMBNAIL_SELECT_ALL ? true : false);


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


//定时器
void CThumbnailDlg::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//设置当前打开的文件类型
void CThumbnailDlg::SetDoctype(int niType)
{
	//if (miDocType != niType)
	{

		miDocType = niType;

		//关闭跳页滚动条
		mpPageProgress->GetIterator()->SetVisible(false);
		
		if (miDocType == DOC_TYPE_PDF)
		{
			mpIteratorSelect->SetVisible(true);
		}
		else
		{
			mpIteratorSelect->SetVisible(false);
		}
	}
	
}

void CThumbnailDlg::DoModal()
{
	do
	{
		EiSetHomebarStatus(GI_HOMEBAR_HIDE);
		EiSetBatteryStatus(GI_BATTERY_HIDE);
		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->EnablePaintboard(false);
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);

		EiSetHomebarStatus(GI_HOMEBAR_SHOW);
		EiSetBatteryStatus(GI_BATTERY_SHOW);
		mpIterator->Close();
		//mpIterator->Release();

	} while (false);
}

void CThumbnailDlg::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator, 0);

}

//进入全部或筛选
void CThumbnailDlg::SelectAll(bool nbIsAll)
{
	do 
	{
		//计算最大页码
		ULONG lulThumbnailPage = 0;
		if (nbIsAll != false)
		{
			
			lulThumbnailPage = mulDocMaxPage;
			CExMessage::SendMessageWithText(mpIteratorSelect, mpIterator, EACT_BUTTON_SETTEXT, mszPageAll);
		}
		else
		{
			lulThumbnailPage = mulAnnotPage;

			CExMessage::SendMessageWithText(mpIteratorSelect, mpIterator, EACT_BUTTON_SETTEXT, mszPageAnnot);

			if (mdAnnotPageNumber.Size() <= 0 && lulThumbnailPage > 0)
			{
				for (ULONG i=0;i<mulDocMaxPage;i++)
				{
					wchar_t lpszFolderPath[MAX_PATH] = { 0 };
					bool lbIsAnnot = false;
					mpDocument->GetThumbnailPathName(i, lpszFolderPath, &lbIsAnnot);
					if (lbIsAnnot != false)
					{
						mdAnnotPageNumber.Insert(-1, i);
						if(ULONG(mdAnnotPageNumber.Size()) >= lulThumbnailPage)
							break; //找够了，不需要继续找了
					}
				}
			}
		}
			
		mulMaxPage = lulThumbnailPage / TD_ITEM_MAX;
		if (mulMaxPage*TD_ITEM_MAX < lulThumbnailPage)
			mulMaxPage++; //页码计算使用进1法

		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);
		if (mulMaxPage > 1)
		{
			mpIteratorNext->SetEnable(true);
		}

		EnterPage(1);

	} while (false);
}

//设置文档对象指针
void CThumbnailDlg::SetPdfPicture(CPdfPicture* npPdfPicture)
{
	do 
	{
		BREAK_ON_NULL(npPdfPicture);

		mpDocument = npPdfPicture->GetDoc();
		BREAK_ON_NULL(mpDocument);

		mulAnnotPage = mpDocument->GetAnnotPageCount();
		mulDocMaxPage = (ULONG)mpDocument->GetPageCount();

		ULONG lulS = 0;
		mulCurrentDoc = npPdfPicture->GetCurrentPageNumber(lulS);
		ULONG lulIndex = int(mulCurrentDoc / TD_ITEM_MAX) + 1;

		GetTranslateString(mulDocMaxPage, mulAnnotPage);

		SelectAll(true);

		//进入当前页面所在缩略图
		if (lulIndex > 1)
		{
			EnterPage(lulIndex);
			mpIteratorPre->SetEnable(mulCurrentPage <= 1 ? false : true);
			mpIteratorNext->SetEnable(mulCurrentPage < mulMaxPage ? true : false);
		}
			

	} while (false);

	
}//获取翻译字符串
void CThumbnailDlg::GetTranslateString(ULONG nulPageCount, ULONG nulPageAnnot)
{
	IConfigFile* lpProfile = NULL;

	do
	{
		//查找
		//修改文字
		wchar_t lszText[MAX_PATH] = { 0 };
		lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		ICfKey* lpCfKey = NULL;
		if (lpProfile != NULL)
		{
			lpCfKey = lpProfile->OpenKey(L"String2/AllPage");
			if (lpCfKey != NULL)
			{
				lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));
				CMM_SAFE_RELEASE(lpCfKey);

				swprintf_s(mszPageAll, lszText, nulPageCount);
			}

			lpCfKey = lpProfile->OpenKey(L"String2/InkPage");
			if (lpCfKey != NULL)
			{
				lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));
				CMM_SAFE_RELEASE(lpCfKey);

				swprintf_s(mszPageAnnot, lszText, nulPageAnnot);
			}
		}
		CMM_SAFE_RELEASE(lpCfKey);

	} while (false);

	CMM_SAFE_RELEASE(lpProfile);
}

//重新定位元素
void CThumbnailDlg::RelocationItem(void)
{
	if (mpIteratorNext != NULL)
	{
		EI_SIZE ldPaintSize;
		//D2D1_SIZE_F ldActionSize;

		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
		float lfWidth = 0.0f;
		D2D1_POINT_2F ldPos;
		ldPos.y = 10.0f;

		if (mpMenuThumbnail != NULL)
		{
			mpMenuThumbnail->GetIterator()->SetSize(float(ldPaintSize.w), float(ldPaintSize.h));
		}

		float lfBackHeight = 80.0f;

		if (ldPaintSize.w > ldPaintSize.h)
		{
			//横屏
			D2D1_POINT_2F ldPos = { 0.0f,80.0f };
			int li = 0;
			for (li = 0; li < TD_ITEM_MAX; li++)
			{
				if (mdList.Size() <= li)
					break;

				mdList.GetEntry(li)->GetIterator()->SetPosition(ldPos);
				mdList.GetEntry(li)->GetIterator()->SetSize(480.0f,251.0f);
				ldPos.x += 480.0f;

				if (((li + 1) % 4) == 0)
				{
					ldPos.x = 0.0f;
					ldPos.y += 251.0f;
				}
			}

		}
		else
		{
			//坚屏
			D2D1_POINT_2F ldPos = { 0.0f,80.0f };
			int li = 0;
			for (li = 0; li < TD_ITEM_MAX; li++)
			{
				if (mdList.Size() <= li)
					break;

				mdList.GetEntry(li)->GetIterator()->SetPosition(ldPos);
				mdList.GetEntry(li)->GetIterator()->SetSize(270.0f, 461.0f);
				ldPos.x += 270.0f;

				if (((li + 1) % 4) == 0)
				{
					ldPos.x = 0.0f;
					ldPos.y += 461.0f;
				}
			}
		}

		lfWidth = mpIteratorPre->GetSizeX() + mpIteratorNext->GetSizeX() + mpIterBtPageNumber->GetSizeX();

		ldPos.x = (ldPaintSize.w - lfWidth) - 20.0f;

		mpIteratorPre->SetPosition(ldPos);
		ldPos.x += mpIteratorPre->GetSizeX();
		mpIterBtPageNumber->SetPosition(ldPos);
		ldPos.x += mpIterBtPageNumber->GetSizeX();
		mpIteratorNext->SetPosition(ldPos);

		ldPos.x = mpIterBtPageNumber->GetPositionX() - (mpPageProgress->GetIterator()->GetSizeX() - mpIterBtPageNumber->GetSizeX()) / 2.0f;
		ldPos.y = 80.0f;
		if (ldPos.x + mpPageProgress->GetIterator()->GetSizeX() > ldPaintSize.w)
			ldPos.x = ldPaintSize.w - mpPageProgress->GetIterator()->GetSizeX()-20.0f; //如果超出屏幕就往回来点

		mpPageProgress->GetIterator()->SetPosition(ldPos);
		ldPos.x = mpIterBtPageNumber->GetPositionX() - ldPos.x + mpIterBtPageNumber->GetSizeX()/2.0f;
		ldPos.y = -12.0f;
		mpPageProgress->SetArrowPos(ldPos);


	}
}

//元素参考尺寸发生变化
ERESULT CThumbnailDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{

	RelocationItem();

	return ERESULT_SUCCESS;
}

//通知元素【显示/隐藏】发生改变
ERESULT CThumbnailDlg::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);
	if (nbIsShow != false)
	{
		mpIterator->BringToTop();
	}
	else
	{
		//自己隐藏时把翻页滚动条也隐藏了，下次显示时不显示，需要用户再次点击页码区域才会显示
		mpPageProgress->GetIterator()->SetVisible(false);
	}

	return ERESULT_SUCCESS;
}


//根据页码设置显示
void CThumbnailDlg::EnterPage(ULONG nulPage)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	int i = 0, k = 0;

	do
	{
		BREAK_ON_NULL(mpDocument);

		

		if (nulPage < 1)
			nulPage = 1;

		if (nulPage > mulMaxPage)
			nulPage = mulMaxPage;

				   //记录当前页码
		if (mulMaxPage <= 0)
			mulCurrentPage = 0; //说明是个空文件夹
		else
			mulCurrentPage = nulPage;

		//显示页码
		swprintf_s(lszString, MAX_PATH, L"%d/%d", mulCurrentPage, mulMaxPage);
		CExMessage::SendMessageWithText(mpIterBtPageNumber, mpIterator, EACT_BUTTON_SETTEXT, lszString, NULL, 0);

		//设置list对象
		int liBegin = (mulCurrentPage - 1) * TD_ITEM_MAX;

		if (mulSelectType == THUMBNAIL_SELECT_ALL)
		{
			//全部页面
			for (i = liBegin, k = 0; i < (int)mulDocMaxPage && k < TD_ITEM_MAX; i++, k++)
			{
				CThumbnailListItem* lpItem = mdList.GetEntry(k);

				wchar_t lpszFolderPath[MAX_PATH] = { 0 };
				bool lbIsAnnot = false;
				mpDocument->GetThumbnailPathName(i, lpszFolderPath, &lbIsAnnot);

				lpItem->SetData(lpszFolderPath, i + 1, (i + 1) == mulCurrentDoc ? true : false);

				lpItem->GetIterator()->SetVisible(true);
			}
		}
		else
		{
			//已标注页面
			for (i = liBegin, k = 0; i < mdAnnotPageNumber.Size() && k < TD_ITEM_MAX; i++, k++)
			{
				CThumbnailListItem* lpItem = mdList.GetEntry(k);

				wchar_t lpszFolderPath[MAX_PATH] = { 0 };
				bool lbIsAnnot = false;
				int liIndex = mdAnnotPageNumber.GetEntry(i);
				mpDocument->GetThumbnailPathName(liIndex, lpszFolderPath, &lbIsAnnot);

				lpItem->SetData(lpszFolderPath, liIndex+1, (liIndex+1) == mulCurrentDoc ? true : false);

				lpItem->GetIterator()->SetVisible(true);
			}
		}
		

	} while (false);

	for (int j = k; j < TD_ITEM_MAX; j++)
	{
		//没有那么多项了，把后面的隐藏
		mdList.GetEntry(j)->GetIterator()->SetVisible(false);
	}

}

//上一页或下一页
void CThumbnailDlg::NextPage(bool nbIsNext)
{
	do
	{
		if (nbIsNext == false)
		{
			//上一页
			mpIteratorNext->SetEnable(true);
			if (mulCurrentPage > 1)
				mulCurrentPage--;

			if (mulCurrentPage <= 1)
			{
				mpIteratorPre->SetEnable(false);

				//已经是第一页了
			}
		}
		else
		{
			//增加
			mpIteratorPre->SetEnable(true);
			if (mulCurrentPage < mulMaxPage)
				mulCurrentPage++;

			if (mulCurrentPage >= mulMaxPage)
			{
				mpIteratorNext->SetEnable(false);
				//已经是后一页了
			}
		}

		EnterPage(mulCurrentPage);

		mpPageProgress->SetData((FLOAT)mulCurrentPage, (FLOAT)mulMaxPage);

	} while (false);
}
