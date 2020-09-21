/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include <vector>
#include <algorithm>
#include "ElementImp.h"
#include "FileOpenDlg.h"
#include "MsgDefine.h"
#include <stdio.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <Shellapi.h>

#include "util.h"

using std::vector;

namespace util = EInkReaderUtil;

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
		D2D1_POINT_2F ldPos = {20.0f,90.0f};
		for (int i=0;i<FP_LIST_MAX;i++)
		{
			CFileListItem* lpListItem = CFileListItem::CreateInstance(mpIterator, lpSubKey);
			lpListItem->GetIterator()->SetPosition(ldPos);
			lpListItem->GetIterator()->SetVisible(false);
			ldPos.y += 160.0f;
			mdList.Insert(i, lpListItem);

			if (i==3)
			{
				ldPos.x = 500.0f;
				ldPos.y = 90.0f;
			}
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
	m_FolderPathList.clear();
}

//初始化list,默认显示几个常用文件夹及盘符
void CFileOpenDlg::InitList(void)
{
	wchar_t* lpszPath = nullptr;
	CFileListItem* lpListItem = NULL;

	do 
	{
		ClearFilePath();
		while (mdFolderLevel.Size() > 0)
		{
			mdFolderLevel.RemoveByIndex(0);
		}

		//桌面
		wchar_t* lpszPath = new wchar_t[MAX_PATH];
		SHGetSpecialFolderPath(NULL, lpszPath, CSIDL_DESKTOP, FALSE);
		m_FolderPathList.push_back(lpszPath);

		//我的文档
		wchar_t* lpszDocPath = new wchar_t[MAX_PATH];
		SHGetSpecialFolderPath(NULL, lpszDocPath, CSIDL_MYDOCUMENTS, FALSE);
		m_FolderPathList.push_back(lpszDocPath);

		//下载
		wchar_t* lpszDownPath = new wchar_t[MAX_PATH];

		wcscpy_s(lpszDownPath, MAX_PATH, lpszDocPath);
		*(wcsrchr(lpszDownPath, L'\\') + 1) = UNICODE_NULL;
		wcscat_s(lpszDownPath, MAX_PATH, L"Downloads");
		m_FolderPathList.push_back(lpszDownPath);

		//mpIteratorImBack->SetVisible(false);
		LONG llIndex = 1;
		CExMessage::SendMessage(mpIteratorImBack, mpIterator, EACT_PICTUREFRAME_SET_INDEX, llIndex, NULL, 0);

		mpIteratorPath->SetVisible(false);
		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);

		CExMessage::SendMessageWithText(mpIteratorPage, mpIterator, EACT_LABEL_SET_TEXT, L"1/1", NULL, 0);

		//获取所有盘符
		wchar_t lszDrive[MAX_PATH] = { 0 };
		int i = 0;
		GetLogicalDriveStrings(MAX_PATH, lszDrive);
		while (i < MAX_PATH && lszDrive[i] != UNICODE_NULL)
		{
			wchar_t* lpszPatht = new wchar_t[MAX_PATH];
			lpszPatht[0] = lszDrive[i];
			lpszPatht[1] = L':';
			lpszPatht[2] = UNICODE_NULL;
			m_FolderPathList.push_back(lpszPatht);

			i += 4;

			delete[] lpszPatht;
			lpszPatht = nullptr;
		}

		//计算最大页码
		mulMaxPage = m_FolderPathList.size() / FP_LIST_MAX;
		if (mulMaxPage*FP_LIST_MAX < (ULONG)m_FolderPathList.size())
			mulMaxPage++; //页码计算使用进1法

		mpIteratorPre->SetEnable(false);
		mpIteratorNext->SetEnable(false);
		if (mulMaxPage > 1)
		{
			mpIteratorNext->SetEnable(true);
		}

		//打开第一页
		SetPage(1);

		delete[] lpszPath;
		lpszPath = nullptr;

		delete[] lpszDocPath;
		lpszDocPath = nullptr;

		delete[] lpszDownPath;
		lpszDownPath = nullptr;

	} while (false);
}

wstring CFileOpenDlg::DoModal(bool* npbIsSuccess)
{
	mpbIsSucess = npbIsSuccess;

	mpIterator->SetActive();
	mpIterator->BringToTop();
	EinkuiGetSystem()->EnablePaintboard(false);
	EinkuiGetSystem()->UpdateView(true);
	EinkuiGetSystem()->DoModal(mpIterator);

	wstring result = m_selectedFile;

	mpIterator->Close();

	return result;
}

void CFileOpenDlg::ExitModal()
{
	EinkuiGetSystem()->ExitModal(mpIterator,0);
}

const wstring & CFileOpenDlg::SelectedFile()
{
	return m_selectedFile;
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

		if (!util::IsPathDirectory(npszFilePath)) // 选中了文件，记录文件名并退出。
		{
			m_selectedFile = npszFilePath;
			*mpbIsSucess = true;
			ExitModal();
		}
		else
		{
			//是文件夹，进入该 文件夹
			EnterFolder(npszFilePath);
		}

	} while (false);
}

//获取目录下指定文件及目录
DWORD CFileOpenDlg::FillSubDirAndRelatedFiles(wchar_t* npszPath, const vector<const wchar_t*>& relatedExtList)
{
	if (!npszPath) return 0;

	wchar_t lszFindPath[MAX_PATH] = { 0 };
	wcscpy_s(lszFindPath, MAX_PATH, npszPath);
	//wsprintfW(lszFindPath, L"%s\\*", npszPath);
	swprintf_s(lszFindPath, MAX_PATH, L"%s\\*", npszPath);
	WIN32_FIND_DATA FindFileData;
	ZeroMemory(&FindFileData, sizeof(WIN32_FIND_DATA));
	HANDLE hFindFile = FindFirstFile(lszFindPath, &FindFileData);

	if (hFindFile == INVALID_HANDLE_VALUE)
		return 0;

	BOOL haveNextFile = true;

	vector<wstring> dirList;
	vector<wstring> fileList;

	while (haveNextFile)
	{
		//是.或者..
		if (wcscmp(FindFileData.cFileName, L".") == 0 || wcscmp(FindFileData.cFileName, L"..") == 0)
		{
			haveNextFile = FindNextFile(hFindFile, &FindFileData);
			continue;
		}

		if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)  // 加入目录
		{
			wstring result = npszPath;
			result += L"\\";
			result += FindFileData.cFileName;
			// 忽略隐藏目录和系统目录
			if ((GetFileAttributes(result.c_str())&FILE_ATTRIBUTE_HIDDEN) == 0 && (GetFileAttributes(result.c_str())&FILE_ATTRIBUTE_SYSTEM) == 0)
			{
				dirList.push_back(result);
			}
		}
		else
		{
			wstring currentExtName = util::GetFileExtention(FindFileData.cFileName, util::NameCaseOption::ToLower);
			if (std::any_of(relatedExtList.begin(), relatedExtList.end(), [currentExtName](const wchar_t* ext) {
				return currentExtName == ext;
			}))
			{
				wstring result = npszPath;
				result += L"\\";
				result += FindFileData.cFileName;

				if ((GetFileAttributes(result.c_str())&FILE_ATTRIBUTE_HIDDEN) == 0 && (GetFileAttributes(result.c_str())&FILE_ATTRIBUTE_SYSTEM) == 0)
				{
					fileList.push_back(result);
				}
			}

		}

		haveNextFile = FindNextFile(hFindFile, &FindFileData);
	}

	FindClose(hFindFile);
	for (const wstring& dirName : dirList)
	{
		m_FolderPathList.push_back(dirName);
	}

	for (const wstring& fileName: fileList)
	{
		m_FolderPathList.push_back(fileName);
	}
	return dirList.size() + fileList.size();
}

//进入文件夹
void CFileOpenDlg::EnterFolder(wchar_t* npszPath, bool nbIsBack)
{
	const vector<const wchar_t*> kNotableExtentionList{
		L".pdf",
		L".epub",
		L".mobi",
		L".txt",
		L".doc",
		L".docx",
		L".xls",
		L".xlsx",
		L".ppt",
		L".pptx",
		L".vsd",
		L".vsdx",
	};

	do 
	{
		BREAK_ON_NULL(npszPath);

		wcscpy_s(mszCurrentPath, MAX_PATH, npszPath);

		ClearFilePath();
		FillSubDirAndRelatedFiles(npszPath, kNotableExtentionList);

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
		//mpIteratorImBack->SetVisible(true);
		LONG llIndex = 2;
		CExMessage::SendMessage(mpIteratorImBack, mpIterator, EACT_PICTUREFRAME_SET_INDEX, llIndex, NULL, 0);
		

		//计算最大页码
		mulMaxPage = m_FolderPathList.size() / FP_LIST_MAX;
		if (mulMaxPage*FP_LIST_MAX < (ULONG)m_FolderPathList.size())
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
void CFileOpenDlg::SetHistoryList(wchar_t* npHistroyPath)
{
	if (npHistroyPath != NULL)
	{
		//如果之前已经打开过文件，就打开那个文件所在的目录
		wchar_t lszPath[MAX_PATH] = { 0 };
		wcscpy_s(lszPath, MAX_PATH, npHistroyPath);

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

//************************************
// Method:    GetFileType
// FullName:  CFileOpenDlg::GetFileType
// Access:    private 
// Qualifier: 获取当前文件的类型(office文件转pdf文件使用)
// Author:    xiezm2
// Returns:   CFileOpenDlg::eFileType
// Parameter: wstring strFilePath
//************************************
CFileOpenDlg::eFileType CFileOpenDlg::GetFileType(wstring strFilePath) {
	eFileType eFile = e_File_UnKnown;
	std::wstring file_exten;
	size_t pos = strFilePath.rfind(L'.');
	if (pos == std::wstring::npos)
		return eFile;
	file_exten = strFilePath.substr(pos, std::wstring::npos);
	transform(file_exten.begin(), file_exten.end(), file_exten.begin(), tolower);
	if (0 == file_exten.compare(L".pdf")) {
		eFile = e_File_Pdf;
	}
	else if (0 == file_exten.compare(L".epub")) {
		eFile = e_File_Epub;
	}
	else if (0 == file_exten.compare(L".mobi")) {
		eFile = e_File_Mobi;
	}
	else if (0 == file_exten.compare(L".txt")) {
		eFile = e_File_Txt;
	}
	else if (0 == file_exten.compare(L".docx")) {
		eFile = e_File_Word;
	}
	else if (0 == file_exten.compare(L".doc")) {
		eFile = e_File_Word;
	}
	else if (0 == file_exten.compare(L".rtf")) {
		eFile = e_File_Word;
	}
	else if (0 == file_exten.compare(L".xlsx")) {
		eFile = e_File_Excel;
	}
	else if (0 == file_exten.compare(L".xls")) {
		eFile = e_File_Excel;
	}
	else if (0 == file_exten.compare(L".csv")) {
		eFile = e_File_Excel;
	}
	else if (0 == file_exten.compare(L".pptx")) {
		eFile = e_File_PowerPoint;
	}
	else if (0 == file_exten.compare(L".ppt")) {
		eFile = e_File_PowerPoint;
	}
	else if (0 == file_exten.compare(L".vsdx")) {
		eFile = e_File_Visio;
	}
	else if (0 == file_exten.compare(L".vsd")) {
		eFile = e_File_Visio;
	}

	return eFile;
}

static wstring GetFullPath(const wstring& path)
{
	const int bufferSize = 2048;
	std::unique_ptr<wchar_t[]> buffer = std::make_unique<wchar_t[]>(bufferSize);

	GetFullPathNameW(path.c_str(), bufferSize, buffer.get(), nullptr);

	return buffer.get();
}

long GetFileSize(const wstring& path)
{
	FILE* f = nullptr;
	_wfopen_s(&f, path.c_str(), L"rb");
	if (f == nullptr) return 0;
	fseek(f, 0, SEEK_END);
	long result = ftell(f);
	fclose(f);
	return result;
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
		
		for (i= liBegin,k=0;i<m_FolderPathList.size() && k<FP_LIST_MAX;i++,k++)
		{
			CFileListItem* lpItem = mdList.GetEntry(k);

			wchar_t lszDisplayName[MAX_PATH] = { 0 };
			const wchar_t* lpszFolderPath = m_FolderPathList[i].c_str();
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

			auto buffer = std::make_unique<wchar_t[]>(MAX_PATH);
			wcscpy_s(buffer.get(), MAX_PATH, m_FolderPathList[i].c_str());

			lpItem->SetPath(buffer.get(), lszDisplayName[0] == UNICODE_NULL ? NULL : lszDisplayName);
			m_FolderPathList[i] = buffer.get();
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
