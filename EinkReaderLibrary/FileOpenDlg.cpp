/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "ElementImp.h"
#include "FileOpenDlg.h"
#include "MsgDefine.h"
#include <stdio.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <Shellapi.h>

DEFINE_BUILTIN_NAME(FileOpenDlg)

CFileOpenDlg::CFileOpenDlg()
{
	mulCurrentPage = 1;
	mszCurrentPath[0] = UNICODE_NULL;
	mszDisplayPath[0] = UNICODE_NULL;
}

CFileOpenDlg::~CFileOpenDlg(void)
{
	//清空原来的列表
	ClearFilePath();
	
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CFileOpenDlg::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		mpIterator->ModifyStyles(EITR_STYLE_POPUP);

		

		/*mpIteratorPre = mpIterator->GetSubElementByID(TS_ID_BT_PRE);
		BREAK_ON_NULL(mpIteratorPre);

		mpIteratorNext = mpIterator->GetSubElementByID(TS_ID_BT_NEXT);
		BREAK_ON_NULL(mpIteratorNext);*/


	
		mpIterator->SetTimer(100, 1, 10, NULL);

		//mpIterator->SetPosition((),mpIterator->GetPositionY());
		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

ULONG CFileOpenDlg::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用EUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用父类的方法
		leResult = 	CXuiElement::InitOnCreate(npParent, npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		//创建list对象
		lpSubKey = mpTemplete->OpenKey(L"ListItem");
		D2D1_POINT_2F ldPos = {0.0f,189};
		for (int i=0;i<FP_LIST_MAX;i++)
		{
			CFileListItem* lpListItem = CFileListItem::CreateInstance(mpIterator, lpSubKey);
			lpListItem->GetIterator()->SetPosition(ldPos);
			lpListItem->GetIterator()->SetVisible(false);
			ldPos.y += 140.0f;
			mdList.Insert(i, lpListItem);
		}
		CMM_SAFE_RELEASE(lpSubKey);
		

		mpIteratorClose = mpIterator->GetSubElementByID(FP_ID_BT_CLOSE);
		BREAK_ON_NULL(mpIteratorClose);

		mpIteratorPre = mpIterator->GetSubElementByID(FP_ID_BT_PRE);
		BREAK_ON_NULL(mpIteratorPre);

		mpIteratorNext = mpIterator->GetSubElementByID(FP_ID_BT_NEXT);
		BREAK_ON_NULL(mpIteratorNext);

		mpIteratorPage = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIteratorPage);

		mpIteratorPath = mpIterator->GetSubElementByID(5);
		BREAK_ON_NULL(mpIteratorPath);
		CExMessage::SendMessage(mpIteratorPath, mpIterator, EACT_LABEL_SET_MAX_HEIGHT, 44);

		mpIteratorBtBack = mpIterator->GetSubElementByID(FP_ID_BT_Back);
		BREAK_ON_NULL(mpIteratorBtBack);

		mpIteratorImBack = mpIterator->GetSubElementByID(6);
		BREAK_ON_NULL(mpIteratorImBack);
		

		leResult = ERESULT_SUCCESS;

	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	return leResult;
}

//清理原来的路径数据
void CFileOpenDlg::ClearFilePath(void)
{
	while (mdFolderPathList.Size() > 0)
	{
		delete mdFolderPathList.GetEntry(0);
		mdFolderPathList.RemoveByIndex(0);
	}
}

//初始化list,默认显示几个常用文件夹及盘符
void CFileOpenDlg::InitList(void)
{
	wchar_t* lpszPath = NULL;
	CFileListItem* lpListItem = NULL;

	do 
	{
		ClearFilePath();
		while (mdFolderLevel.Size() > 0)
		{
			mdFolderLevel.RemoveByIndex(0);
		}

		//桌面
		lpszPath = new wchar_t[MAX_PATH];
		SHGetSpecialFolderPath(NULL, lpszPath, CSIDL_DESKTOP, FALSE);
		mdFolderPathList.Insert(mdFolderPathList.Size(), lpszPath);

		//我的文档
		lpszPath = new wchar_t[MAX_PATH];
		SHGetSpecialFolderPath(NULL, lpszPath, CSIDL_MYDOCUMENTS, FALSE);
		mdFolderPathList.Insert(mdFolderPathList.Size(), lpszPath);

		//下载
		wchar_t* lpszDownPath = new wchar_t[MAX_PATH];
		wcscpy_s(lpszDownPath, MAX_PATH, lpszPath);
		*(wcsrchr(lpszDownPath, L'\\') + 1) = UNICODE_NULL;
		wcscat_s(lpszDownPath, MAX_PATH, L"Downloads");
		mdFolderPathList.Insert(mdFolderPathList.Size(), lpszDownPath);

		mpIteratorImBack->SetVisible(false);
		mpIteratorPath->SetVisible(false);
		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);

		CExMessage::SendMessageWithText(mpIteratorPage, mpIterator, EACT_LABEL_SET_TEXT, L"1/1", NULL, 0);

		//获取所有盘符
		wchar_t lszDrive[MAX_PATH] = { 0 };
		int i = 0;
		GetLogicalDriveStrings(MAX_PATH, lszDrive);
		while (lszDrive[i] != UNICODE_NULL)
		{
			lpszPath = new wchar_t[MAX_PATH];
			lpszPath[0] = lszDrive[i];
			lpszPath[1] = L':';
			lpszPath[2] = UNICODE_NULL;
			mdFolderPathList.Insert(mdFolderPathList.Size(), lpszPath);

			i += 4;
		}

		//计算最大页码
		mulMaxPage = mdFolderPathList.Size() / FP_LIST_MAX;
		if (mulMaxPage*FP_LIST_MAX < (ULONG)mdFolderPathList.Size())
			mulMaxPage++; //页码计算使用进1法

		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);
		if (mulMaxPage > 1)
		{
			mpIteratorNext->SetEnable(true);
		}

		//打开第一页
		SetPage(1);

	} while (false);
}

void CFileOpenDlg::DoModal(bool nbIsEnableCancel)
{
	do 
	{
		mpIteratorClose->SetEnable(nbIsEnableCancel);

		mpIterator->SetActive();
		mpIterator->BringToTop();
		EinkuiGetSystem()->UpdateView(true);
		EinkuiGetSystem()->DoModal(mpIterator);
		

		mpIterator->Close();
		//mpIterator->Release();

	} while (false);
}

void CFileOpenDlg::ExitModal()
{
	
	EinkuiGetSystem()->ExitModal(mpIterator,0);
	
}

//消息处理函数
ERESULT CFileOpenDlg::ParseMessage(IEinkuiMessage* npMsg)
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
	case EEVT_ER_LIST_CLICK:
	{
		//list某项被点击
		wchar_t* lpszPath = (wchar_t*)npMsg->GetInputData();
		ListClick(lpszPath);
		break;
	}
	case EEVT_RBG_SELECTED_CHANGED:
	{
		//选择了不同的页面
		ULONG lulID = 0;
		luResult = CExMessage::GetInputData(npMsg, lulID);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulID == FP_ID_GROUP_FILE)
		{
			mpIteratorPage->SetVisible(true);
			mpIteratorPre->SetVisible(true);
			mpIteratorNext->SetVisible(true);

			SetPage(mulCurrentPage);

			if (mdFolderLevel.Size() >= 1 && mszCurrentPath[0] != UNICODE_NULL)
			{
				//只有在非最顶层才显示
				mpIteratorPath->SetVisible(true);
				mpIteratorImBack->SetVisible(true);
			}
		}
		else if (lulID == FP_ID_GROUP_HISTORY)
		{
			wchar_t lszString[MAX_PATH] = { 0 };
			mpIteratorPath->SetVisible(false);
			mpIteratorImBack->SetVisible(false);

			do
			{
				//显示页码
				swprintf_s(lszString, MAX_PATH, L"%d/%d", 1, 1);
				CExMessage::SendMessageWithText(mpIteratorPage, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);
				mpIteratorPage->SetVisible(false);
				mpIteratorPre->SetVisible(false);
				mpIteratorNext->SetVisible(false); //历史记录只显示一页，不允许翻页

				//设置list对象
				int liBegin = (mulCurrentPage - 1) * FP_LIST_MAX;
				int i = 0, k = 0;
				for (i = 0, k = 0; i < mpdHistroyPath->Size() && k < FP_LIST_MAX; i++, k++)
				{
					if (GetFileAttributes(mpdHistroyPath->GetEntry(i)) == INVALID_FILE_ATTRIBUTES)
					{
						//如果文件不存在了，就不显示这一项了
						k--;
						continue;
					}

					CFileListItem* lpItem = mdList.GetEntry(k);
					lpItem->SetPath(mpdHistroyPath->GetEntry(i),NULL);
					lpItem->GetIterator()->SetVisible(true);
				}

				for (int j = k; j < FP_LIST_MAX; j++)
				{
					//没有那么多项了，把后面的隐藏
					mdList.GetEntry(j)->GetIterator()->SetVisible(false);
				}

			} while (false);
		}

		break;
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

//list项被点击
void CFileOpenDlg::ListClick(wchar_t* npszFilePath)
{
	do 
	{
		BREAK_ON_NULL(npszFilePath);

		//首先尝试获取文件属性
		if ((GetFileAttributes(npszFilePath)&FILE_ATTRIBUTE_DIRECTORY) == false)
		{
			//是文件，通知父窗口并隐藏自己
			bool lbRet = false;
			mpIterator->SetVisible(false);
			CExMessage::SendMessageWithText(mpIterator->GetParent(), mpIterator, EEVT_ER_OPEN_FILE_PATH, npszFilePath,&lbRet,sizeof(lbRet));
			if (lbRet != false)
				ExitModal(); //说明文件打开成功了，那就退出自己
			else
				mpIterator->SetTimer(FP_TIMER_ID_SHOW, 1, 3000, NULL);
				
				//mpIteratorClose->SetEnable(false); //打开文件失败,禁用取消按钮
		}
		else
		{
			//是文件夹，进入该 文件夹
			EnterFolder(npszFilePath);
		}

	} while (false);
}

//获取目录下指定文件及目录
DWORD CFileOpenDlg::GetFolderPath(wchar_t* npszPath, wchar_t* npszName)
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

			while (lbIsDots == false)
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
						mdFolderPathList.Insert(mdFolderPathList.Size(), lpPath);
				}
				else
				{
					//是文件,npszName=null表明只想要目录的数量
					if (npszName != NULL)
					{
						wchar_t* lpPath = new wchar_t[MAX_PATH];
						wcscpy_s(lpPath, MAX_PATH, npszPath);
						wcscat_s(lpPath, MAX_PATH, L"\\");
						wcscat_s(lpPath, MAX_PATH, FindFileData.cFileName);
						mdFolderPathList.Insert(mdFolderPathList.Size(), lpPath);
					}

				}

				break;
			}

			//寻找下一文件
			bContinue = FindNextFile(hFindFile, &FindFileData);

		}

		FindClose(hFindFile);

	} while (false);

	return ldwCount;
}

//进入文件夹
void CFileOpenDlg::EnterFolder(wchar_t* npszPath, bool nbIsBack)
{
	do 
	{
		BREAK_ON_NULL(npszPath);

		wcscpy_s(mszCurrentPath, MAX_PATH, npszPath);
		mpIteratorImBack->SetEnable(true);

		//清空原来的列表
		ClearFilePath();

		//遍历所有子文件夹e
		GetFolderPath(npszPath, NULL);
		GetFolderPath(npszPath, L"\\*.pdf");
		GetFolderPath(npszPath, L"\\*.epub");
		GetFolderPath(npszPath, L"\\*.mobi");
		GetFolderPath(npszPath, L"\\*.txt");

		wchar_t lszDisplayName[MAX_PATH] = { 0 };
		wchar_t* lpszFolderPath = npszPath;
		if (mdFolderLevel.Size() <= 0 && wcslen(lpszFolderPath) > 3)
		{
			//获取显示名称
			if (StrStrI(lpszFolderPath, L"Desktop") != NULL)
				GetDisplayName(FOLDERID_Desktop, lszDisplayName, MAX_PATH);
			else if (StrStrI(lpszFolderPath, L"Documents") != NULL)
				GetDisplayName(FOLDERID_Documents, lszDisplayName, MAX_PATH);
			else if (StrStrI(lpszFolderPath, L"Pictures") != NULL)
				GetDisplayName(FOLDERID_Pictures, lszDisplayName, MAX_PATH);
			else if (StrStrI(lpszFolderPath, L"Downloads") != NULL)
				GetDisplayName(FOLDERID_Downloads, lszDisplayName, MAX_PATH);

			
		}
		if (nbIsBack == false)
		{
			//只有不是返回时才增加路径
			if (lszDisplayName[0] != UNICODE_NULL)
			{
				//使用显示名称
				wcscpy_s(mszDisplayPath, MAX_PATH, lszDisplayName);
			}
			else
			{
				wchar_t* lpszFileName = wcsrchr(npszPath, L'\\');
				if (lpszFileName == NULL)
					wcscpy_s(mszDisplayPath, MAX_PATH, npszPath);
				else
				{
					if (mszDisplayPath[0] != UNICODE_NULL)
						wcscat_s(mszDisplayPath, MAX_PATH, lpszFileName);
					else
						wcscpy_s(mszDisplayPath, MAX_PATH, npszPath);
				}

			}
		}

		CExMessage::SendMessageWithText(mpIteratorPath, mpIterator, EACT_LABEL_SET_TEXT, mszDisplayPath, NULL, 0);

		mpIteratorPath->SetVisible(true);
		mpIteratorImBack->SetVisible(true);

		//计算最大页码
		mulMaxPage = mdFolderPathList.Size() / FP_LIST_MAX;
		if (mulMaxPage*FP_LIST_MAX < (ULONG)mdFolderPathList.Size())
			mulMaxPage++; //页码计算使用进1法

		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);
		mpIteratorPage->SetVisible(true);

		if (nbIsBack == false)
		{
			//如果不是返回上一级，就进入第一页
			mdFolderLevel.Insert(0, 1);
			SetPage(1);
		}
		else
		{
			//是返回上一级，进入之前离开时的页码
			if(mdFolderLevel.Size()>0)
				mdFolderLevel.RemoveByIndex(0);

			SetPage(mdFolderLevel.Size() > 0 ? mdFolderLevel.GetEntry(0) : 1);
		}
		
		if (mulMaxPage <= 0)
		{
			//说明是空文件夹
			//mpIteratorPage->SetVisible(false);
			//显示页码
			wchar_t lszString[MAX_PATH] = { 0 };
			swprintf_s(lszString, MAX_PATH, L"%d/%d", 1, 1);
			CExMessage::SendMessageWithText(mpIteratorPage, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

			break;
		}

		if (mulCurrentPage < mulMaxPage)
		{
			mpIteratorNext->SetEnable(true);
		}
		if(mulCurrentPage > 1)
		{
			mpIteratorPre->SetEnable(true);
		}

	} while (false);
}

//定时器
void CFileOpenDlg::OnTimer(PSTEMS_TIMER npStatus)
{
	if (npStatus->TimerID == FP_TIMER_ID_SHOW)
	{
		mpIterator->SetVisible(true);
	}
	else
	{
		mpIterator->SetActive();
		mpIterator->BringToTop();
	}

}


//按钮单击事件
ERESULT CFileOpenDlg::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		ULONG llBtnID = npSender->GetID();
		switch (llBtnID)
		{
		case FP_ID_BT_CLOSE:		// 关闭	
		{
			ExitModal();

			break;
		}
		case FP_ID_BT_PRE:
		{
			//上一页
			NextPage(false);
			
			break;
		}
		case FP_ID_BT_NEXT:
		{
			//下一页
			NextPage(true);

			break;
		}
		case FP_ID_BT_Back:
		{
			//返回上级目录
			BackFolder();
			break;
		}
		default:
			break;
		}


		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}

//元素参考尺寸发生变化
ERESULT CFileOpenDlg::OnElementResized(D2D1_SIZE_F nNewSize)
{
	/*EI_SIZE ldPaintSize;
	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

	D2D1_POINT_2F ldParentPos = mpIterator->GetParent()->GetPosition();
	FLOAT lfWidth = mpIterator->GetSizeX();
	FLOAT lfX = (ldPaintSize.w - lfWidth) / 2;
	lfX = lfX - ldParentPos.x;
	mpIterator->SetPosition(lfX, mpIterator->GetPositionY());*/

	return ERESULT_SUCCESS;
}

//设置历史记录
void CFileOpenDlg::SetHistoryList(cmmVector<wchar_t*>* npdHistroyPath)
{
	mpdHistroyPath = npdHistroyPath;

	if (mpdHistroyPath != NULL && mpdHistroyPath->Size() > 0)
	{
		//如果之前已经打开过文件，就打开那个文件所在的目录
		wchar_t lszPath[MAX_PATH] = { 0 };
		wcscpy_s(lszPath, MAX_PATH, mpdHistroyPath->GetEntry(0));

		*(wcsrchr(lszPath, L'\\')) = UNICODE_NULL;
		if (GetFileAttributes(lszPath) != INVALID_FILE_ATTRIBUTES)
		{
			while (mdFolderLevel.Size() > 0)
			{
				mdFolderLevel.RemoveByIndex(0);
			}

			//为了这种情况能一级一级返回，这里增加记录
			wchar_t* lpszTemp = wcsstr(lszPath, L"\\");
			do 
			{
				mdFolderLevel.Insert(0, 1);
				if(lpszTemp == NULL)
					break; //????
				lpszTemp++;
				lpszTemp = wcsstr(lpszTemp, L"\\");

			} while (lpszTemp != NULL);

			EnterFolder(lszPath);
		}
		else
		{
			//目录不存在了
			InitList();
		}
	}
	else
	{
		InitList();
	}
}


//获取特殊目录的多语言字符串
bool CFileOpenDlg::GetDisplayName(GUID niCSIDL, OUT wchar_t* npszName, IN int niLen)
{
	bool lbRet = false;

	do
	{
		CoInitialize(NULL);
		IShellItem* lpDocItem;
		auto lhrRet = SHGetKnownFolderItem(niCSIDL, KF_FLAG_DEFAULT, NULL, IID_IShellItem, (void**)&lpDocItem); // SHGetKnownFolderPath 用于获取目录
		if (SUCCEEDED(lhrRet))
		{
			LPWSTR lpDisplayName = NULL;
			lhrRet = lpDocItem->GetDisplayName(SIGDN_NORMALDISPLAY, &lpDisplayName);

			wcscpy_s(npszName, niLen, lpDisplayName);
			if (lpDisplayName != NULL)
				CoTaskMemFree(lpDisplayName);

			lpDocItem->Release();
		}

		lbRet = true;

	} while (false);

	return lbRet;
}

//根据页码设置显示
void CFileOpenDlg::SetPage(ULONG nulPage)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	int i = 0, k = 0;

	do 
	{
		if(nulPage < 0 || nulPage > mulMaxPage)
			break; //超过有效范围

		//记录当前页码
		if (mulMaxPage <= 0)
			mulCurrentPage = 0; //说明是个空文件夹
		else
			mulCurrentPage = nulPage;
		
		//显示页码
		swprintf_s(lszString, MAX_PATH, L"%d/%d", mulCurrentPage, mulMaxPage);
		CExMessage::SendMessageWithText(mpIteratorPage, mpIterator, EACT_LABEL_SET_TEXT, lszString, NULL, 0);

		//设置list对象
		int liBegin = (mulCurrentPage - 1) * FP_LIST_MAX;
		
		for (i= liBegin,k=0;i<mdFolderPathList.Size() && k<FP_LIST_MAX;i++,k++)
		{
			CFileListItem* lpItem = mdList.GetEntry(k);

			wchar_t lszDisplayName[MAX_PATH] = { 0 };
			wchar_t* lpszFolderPath = mdFolderPathList.GetEntry(i);
			if (mdFolderLevel.Size() <= 0 && wcslen(lpszFolderPath) > 3)
			{
				//获取显示名称
				if (StrStrI(lpszFolderPath, L"Desktop") != NULL)
					GetDisplayName(FOLDERID_Desktop, lszDisplayName, MAX_PATH);
				else if (StrStrI(lpszFolderPath, L"Documents") != NULL)
					GetDisplayName(FOLDERID_Documents, lszDisplayName, MAX_PATH);
				else if (StrStrI(lpszFolderPath, L"Pictures") != NULL)
					GetDisplayName(FOLDERID_Pictures, lszDisplayName, MAX_PATH);
				else if (StrStrI(lpszFolderPath, L"Downloads") != NULL)
					GetDisplayName(FOLDERID_Downloads, lszDisplayName, MAX_PATH);
			}

			lpItem->SetPath(mdFolderPathList.GetEntry(i), lszDisplayName[0] == UNICODE_NULL ? NULL : lszDisplayName);
			//lpItem->SetPath(mdFolderPathList.GetEntry(i));
			lpItem->GetIterator()->SetVisible(true);
		}

		//保存页码
		if(mdFolderLevel.Size() > 0)
			mdFolderLevel.GetEntry(0) = nulPage;

	} while (false);

	for (int j = k; j < FP_LIST_MAX; j++)
	{
		//没有那么多项了，把后面的隐藏
		mdList.GetEntry(j)->GetIterator()->SetVisible(false);
	}
}

//返回上一级目录
void CFileOpenDlg::BackFolder(void)
{
	do 
	{
		if (mdFolderLevel.Size() <= 1 || mszCurrentPath[0] == UNICODE_NULL)
		{
			mszDisplayPath[0] = UNICODE_NULL;
			InitList();
			break; //没有路径了，返回到最新的列表
		}
		
		wchar_t* lpszFind = wcsrchr(mszCurrentPath, L'\\');
		if (lpszFind != NULL)
		{
			//说明还有路径
			*(wcsrchr(mszCurrentPath, L'\\')) = UNICODE_NULL;
			*(wcsrchr(mszDisplayPath, L'\\')) = UNICODE_NULL;

			EnterFolder(mszCurrentPath,true);
		}
		else
		{
			//说明到最项层了
			mszCurrentPath[0] = UNICODE_NULL;
			mszDisplayPath[0] = UNICODE_NULL;

			InitList();
		}

	} while (false);
}

//上一页或下一页
void CFileOpenDlg::NextPage(bool nbIsNext)
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

		SetPage(mulCurrentPage);

	} while (false);
}
