/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "FileListItem.h"
#include "XCtl.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"

DEFINE_BUILTIN_NAME(FileListItem)

CFileListItem::CFileListItem(void)
{
	mpszFilePath = new wchar_t[MAX_PATH];
	mdwClickTicount = 0;
}


CFileListItem::~CFileListItem(void)
{
	delete mpszFilePath;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CFileListItem::OnElementCreate(IEinkuiIterator* npIterator)
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

ULONG CFileListItem::InitOnCreate(
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

		do
		{
			//获取多语言字符串
			//为了翻译方便，字符串存放在root/string
			lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
			ICfKey* lpCfKey = NULL;
			if (lpProfile != NULL)
			{
				lpCfKey = lpProfile->OpenKey(L"String2/Title3");
				if (lpCfKey != NULL)
					lpCfKey->GetValue(mpszItem, MAX_PATH * sizeof(wchar_t));

			}

		} while (false);

		

		//获取对像句柄
		mpIterFolderIcon = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterFolderIcon);

		mpIterPdfIcon = mpIterator->GetSubElementByID(5);
		BREAK_ON_NULL(mpIterPdfIcon);

		mpIterName = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterName);

		mpIterAttrib = mpIterator->GetSubElementByID(3);
		BREAK_ON_NULL(mpIterAttrib);

		mpIterBt = mpIterator->GetSubElementByID(FLI_BT);
		BREAK_ON_NULL(mpIterBt);
		
		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);
	CMM_SAFE_RELEASE(lpProfile);

	// 向系统注册需要收到的消息
	return leResult;
}

//设置路径
void CFileListItem::SetPath(wchar_t* npszPath, wchar_t* npszDisplayName)
{
	do 
	{
		BREAK_ON_NULL(npszPath);

		//首先尝试获取文件属性
		if ((GetFileAttributes(npszPath)&FILE_ATTRIBUTE_DIRECTORY) == false)
		{
			//是文件
			mpIterFolderIcon->SetVisible(false);
			mpIterPdfIcon->SetVisible(true);

			ProcFile(npszPath);
		}
		else
		{
			//是文件夹
			mpIterFolderIcon->SetVisible(true);
			mpIterPdfIcon->SetVisible(false);

			ProcFolder(npszPath, npszDisplayName);
		}

		wcscpy_s(mpszFilePath, MAX_PATH, npszPath);

	} while (false);
}

//是文件的处理
void CFileListItem::ProcFile(wchar_t* npszPath)
{
	do 
	{
		BREAK_ON_NULL(npszPath);
		WIN32_FILE_ATTRIBUTE_DATA ldData;
		GetFileAttributesEx(npszPath, GetFileExInfoStandard, &ldData);
		SYSTEMTIME ldSystime;
		FileTimeToSystemTime(&ldData.ftLastWriteTime,&ldSystime);

		wchar_t lszString[MAX_PATH] = { 0 };
		swprintf_s(lszString, MAX_PATH, L"%04d/%02d/%02d  %02d:%02d",
			ldSystime.wYear, ldSystime.wMonth, ldSystime.wDay,
			ldSystime.wHour, ldSystime.wMinute);
		CExMessage::SendMessageWithText(mpIterAttrib, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

		wchar_t* lpszFileName = wcsrchr(npszPath, L'\\')+1;
		CExMessage::SendMessageWithText(mpIterName, mpIterator, EACT_LABEL_SET_TEXT, lpszFileName, NULL, 0);

	} while (false);
}

//获取目录下指定文件及目录个数
DWORD CFileListItem::GetFolderCount(wchar_t* npszPath, wchar_t* npszName)
{
	DWORD ldwCount = 0;

	do
	{
		BREAK_ON_NULL(npszPath);

		wchar_t lszFindPath[MAX_PATH] = { 0 };
		wcscpy_s(lszFindPath, MAX_PATH, npszPath);
		if (npszName == NULL)
			wcscat_s(lszFindPath, MAX_PATH, L"\\*.*");
		else
			wcscat_s(lszFindPath, MAX_PATH, npszName);

		WIN32_FIND_DATA FindFileData;
		ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));
		HANDLE hFindFile = FindFirstFile(lszFindPath, &FindFileData);

		if (hFindFile == INVALID_HANDLE_VALUE)
			break;

		BOOL bContinue = true;

		while (bContinue != false)
		{
			//bIsDots为真表示是.或..
			bool lbIsDots = (wcscmp(FindFileData.cFileName, L".") == 0 || wcscmp(FindFileData.cFileName, L"..") == 0);

			if (lbIsDots == false)
			{
				if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					//是目录
					wchar_t* lpPath = new wchar_t[MAX_PATH];
					wcscpy_s(lpPath, MAX_PATH, npszPath);
					wcscat_s(lpPath, MAX_PATH, L"\\");
					wcscat_s(lpPath, MAX_PATH, FindFileData.cFileName);
					//判断一下，如果是隐藏或系统文件夹，就不展示了
					if ((GetFileAttributes(lpPath)&FILE_ATTRIBUTE_HIDDEN) == 0 && (GetFileAttributes(lpPath)&FILE_ATTRIBUTE_SYSTEM) == 0)
						if (npszName == NULL)
							ldwCount++;
				}
				else
				{
					//是文件,npszName=null表明只想要目录的数量
					if (npszName != NULL)
						ldwCount++;

				}
			}

			//寻找下一文件
			bContinue = FindNextFile(hFindFile, &FindFileData);

		}

		FindClose(hFindFile);

	} while (false);

	return ldwCount;
}

//是文件夹的处理
void CFileListItem::ProcFolder(wchar_t* npszPath, wchar_t* npszDisplayName)
{
	do
	{
		BREAK_ON_NULL(npszPath);

		//获取时间
		WIN32_FILE_ATTRIBUTE_DATA ldData;
		GetFileAttributesEx(npszPath, GetFileExInfoStandard, &ldData);
		SYSTEMTIME ldSystime;
		FileTimeToSystemTime(&ldData.ftLastWriteTime, &ldSystime);

		//获取文件数目，只计算目录及pdf文件
		DWORD ldwCount = GetFolderCount(npszPath, NULL);
		ldwCount += GetFolderCount(npszPath, L"\\*.pdf");
		ldwCount += GetFolderCount(npszPath, L"\\*.epub");
		ldwCount += GetFolderCount(npszPath, L"\\*.mobi");
		ldwCount += GetFolderCount(npszPath, L"\\*.txt");

		wchar_t lszString[MAX_PATH] = { 0 };
		swprintf_s(lszString, MAX_PATH, L"%d %s | %04d/%02d/%02d  %02d:%02d", ldwCount, mpszItem,
			ldSystime.wYear, ldSystime.wMonth, ldSystime.wDay,
			ldSystime.wHour, ldSystime.wMinute);
		CExMessage::SendMessageWithText(mpIterAttrib, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

		wchar_t* lpszFileName = wcsrchr(npszPath, L'\\');
		if (lpszFileName == NULL)
			lpszFileName = npszPath;
		else
			lpszFileName = lpszFileName + 1;

		//如果指定了显示名称就使用指定的
		if (npszDisplayName != NULL)
			lpszFileName = npszDisplayName;
		CExMessage::SendMessageWithText(mpIterName, mpIterator, EACT_LABEL_SET_TEXT, lpszFileName, NULL, 0);


	} while (false);
}

//按钮单击事件
ERESULT CFileListItem::OnCtlButtonClick(IEinkuiIterator* npSender)
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
				CExMessage::PostMessageWithText(mpIterator->GetParent(), mpIterator, EEVT_ER_LIST_CLICK, mpszFilePath);
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
//ERESULT CFileListItem::ParseMessage(IEinkuiMessage* npMsg)
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
void CFileListItem::OnTimer(
	PSTEMS_TIMER npStatus
	)
{

}

//元素参考尺寸发生变化
ERESULT CFileListItem::OnElementResized(D2D1_SIZE_F nNewSize)
{
	//CExMessage::SendMessage(mpIterBtFull, mpIterator, EACT_BUTTON_SET_ACTION_RECT, nNewSize);
	////mpIterLineOne->SetSize(nNewSize.width, mpIterLineOne->GetSize().height);

	//mpIterBtOk->SetPosition(nNewSize.width - 85, mpIterBtOk->GetPositionY());

	return ERESULT_SUCCESS;
}


//通知元素【显示/隐藏】发生改变
ERESULT CFileListItem::OnElementShow(bool nbIsShow)
{
	//EiSetHomebarStatus(nbIsShow == false ? GI_HOMEBAR_SHOW : GI_HOMEBAR_HIDE);

	return ERESULT_SUCCESS;
}