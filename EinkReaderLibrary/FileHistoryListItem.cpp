/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "FileHistoryListItem.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <time.h>

DEFINE_BUILTIN_NAME(FileHistoryListItem)

CFileHistoryListItem::CFileHistoryListItem(void)
{
	mdwClickTicount = 0;
	mpIterFolderIcon = NULL;
	mpIterModifyIcon = NULL;
	mpIterName = NULL;
	mpIterAttrib = NULL;
}


CFileHistoryListItem::~CFileHistoryListItem(void)
{

}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CFileHistoryListItem::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		
		//mpIterPicture->SetRotation(90.0f);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CFileHistoryListItem::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;
	IConfigFile* lpProfile = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;


		//获取对像句柄
		mpIterFolderIcon = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterFolderIcon);

		mpIterName = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterName);

		mpIterAttrib = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIterAttrib);

		mpIterModifyIcon = mpIterator->GetSubElementByID(4);
		BREAK_ON_NULL(mpIterModifyIcon);

		mpIterBt = mpIterator->GetSubElementByID(FLI_BT);
		BREAK_ON_NULL(mpIterBt);

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);
	CMM_SAFE_RELEASE(lpProfile);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置文件属性
void CFileHistoryListItem::SetData(HISTORY_FILE_ATTRIB* npFileAttrib)
{
	do 
	{
		BREAK_ON_NULL(npFileAttrib);
		mpFileAttrib = npFileAttrib;
		wchar_t lpszPicPath[MAX_PATH] = { 0 };
		wchar_t* lpszFilePath = npFileAttrib->FilePath;
		int liLastChar = int(wcslen(lpszFilePath) - 1);
		if (lpszFilePath[liLastChar] == 't')
			wcscpy_s(lpszPicPath, MAX_PATH, L".\\Pic\\txt.png");
		else if (lpszFilePath[liLastChar] == 'b')
			wcscpy_s(lpszPicPath, MAX_PATH, L".\\Pic\\epub.png");
		else if (lpszFilePath[liLastChar] == 'i')
			wcscpy_s(lpszPicPath, MAX_PATH, L".\\Pic\\mobi.png");
		else if (lpszFilePath[liLastChar] == 'f')
			wcscpy_s(lpszPicPath, MAX_PATH, L".\\Pic\\pdf.png");

		ProcFile(npFileAttrib);

		CExMessage::SendMessageWithText(mpIterFolderIcon, mpIterator, EACT_PICTUREFRAME_CHANGE_PIC, lpszPicPath);


	} while (false);
}

//是否启用点击
void CFileHistoryListItem::SetEnable(bool nbIsEnable)
{
	mpIterBt->SetEnable(nbIsEnable);
}

//是文件的处理
void CFileHistoryListItem::ProcFile(HISTORY_FILE_ATTRIB* npFileAttrib)
{
	do 
	{
		BREAK_ON_NULL(npFileAttrib);

		SYSTEMTIME ldSystime;
		GetSystemTime(&ldSystime);
		//FileTimeToSystemTime(&ldData.ftLastWriteTime,&ldSystime);
		struct tm ldTime;
		time_t ldReadTime = (time_t)npFileAttrib->TimeStamp;
		localtime_s(&ldTime, &ldReadTime);
		ldTime.tm_year = ldTime.tm_year + 1900;
		wchar_t lszString[MAX_PATH] = { 0 };
		/*swprintf_s(lszString, MAX_PATH, L"%d%s | %04d/%02d/%02d  %02d:%02d",
			npFileAttrib->ReadProgress,L"%",
			ldTime.tm_year + 1900, ldTime.tm_mon+1, ldTime.tm_mday,
			ldTime.tm_hour, ldTime.tm_min);*/

		

		if (ldSystime.wYear == ldTime.tm_year)
		{
			swprintf_s(lszString, MAX_PATH, L"%d%s | %02d-%02d",
				npFileAttrib->ReadProgress, L"%", ldTime.tm_mon + 1, ldTime.tm_mday);
		}
		else
		{
			swprintf_s(lszString, MAX_PATH, L"%d%s | %04d-%02d-%02d",
				npFileAttrib->ReadProgress, L"%",
				ldTime.tm_year, ldTime.tm_mon + 1, ldTime.tm_mday);
		}
		
		if (npFileAttrib->IsModify == 0)
		{
			//不显示修改图标 
			mpIterModifyIcon->SetVisible(false);
			mpIterAttrib->SetPosition(mpIterModifyIcon->GetPosition());
		}
		else
		{
			//显示修改图标
			mpIterModifyIcon->SetVisible(true);
			mpIterAttrib->SetPosition(mpIterModifyIcon->GetPositionX() + 40.0f, mpIterModifyIcon->GetPositionY());

			wchar_t lszTemp[MAX_PATH] = { 0 };
			wcscpy_s(lszTemp, MAX_PATH, lszString);
			swprintf_s(lszString, MAX_PATH, L" | %s", lszTemp);
		}

		CExMessage::SendMessageWithText(mpIterAttrib, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

		wchar_t* lpszFileName = wcsrchr(npFileAttrib->FilePath, L'\\')+1;
		CExMessage::SendMessageWithText(mpIterName, mpIterator, EACT_LABEL_SET_TEXT, lpszFileName, NULL, 0);



	} while (false);
}

//按钮单击事件
ERESULT CFileHistoryListItem::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case FLI_BT:
		{
			//自己被点击
			if (GetTickCount() - mdwClickTicount > 400)
			{
				//为了防止双击
				mdwClickTicount = GetTickCount();
				CExMessage::PostMessage(mpIterator->GetParent(), mpIterator, EEVT_ER_LIST_CLICK, mpFileAttrib);
				//CExMessage::PostMessageWithText(mpIterator->GetParent(), mpIterator, EEVT_ER_LIST_CLICK, mpFileAttrib->FilePath);
			}
			
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


//消息处理函数
//ERESULT CFileHistoryListItem::ParseMessage(IEinkuiMessage* npMsg)
//{
//	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
//
//	switch (npMsg->GetMessageID())
//	{
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
void CFileHistoryListItem::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CFileHistoryListItem::OnElementResized(D2D1_SIZE_F nNewSize)
{
	//CExMessage::SendMessage(mpIterBtFull, mpIterator, EACT_BUTTON_SET_ACTION_RECT, nNewSize);
	////mpIterLineOne->SetSize(nNewSize.width, mpIterLineOne->GetSize().height);

	//mpIterBtOk->SetPosition(nNewSize.width - 85, mpIterBtOk->GetPositionY());

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CFileHistoryListItem::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}