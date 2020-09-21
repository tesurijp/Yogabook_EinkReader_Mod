/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include <vector>
#include <string>
#include "FileListItem.h"
#include "cmmstruct.h"

//打开文件对话框

DECLARE_BUILTIN_NAME(FileOpenDlg)

using std::vector;
using std::wstring;


class COfficeConvertDlg;
class CFileOpenDlg:
	public CXuiElement<CFileOpenDlg,GET_BUILTIN_NAME(FileOpenDlg)>
{
public:
	enum eFileType
	{
		e_File_UnKnown = 0,
		e_File_Pdf,
		e_File_Epub,
		e_File_Mobi,
		e_File_Txt,
		e_File_Word,
		e_File_Excel,
		e_File_PowerPoint,
		e_File_Visio
	};
public:
	// 如果将构造函数设定为protected，就需要加这句话; 否则，不需要下面这句
	friend CXuiElement<CFileOpenDlg,GET_BUILTIN_NAME(FileOpenDlg)>;

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用EUI系统自动分配
		);

	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);

public:
	CFileOpenDlg();
	~CFileOpenDlg(void);

	// 模态显示该对话框
	wstring DoModal(bool* npbIsSuccess);
	//设置历史记录
	void SetHistoryList(wchar_t* npHistroyPath);

	void ExitModal();

	const wstring& SelectedFile();

protected:
	

	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	

private:
	IEinkuiIterator* mpIteratorClose;
	IEinkuiIterator* mpIteratorPre;
	IEinkuiIterator* mpIteratorNext;
	IEinkuiIterator* mpIteratorPage;
	IEinkuiIterator* mpIteratorPath; //显示当前路径
	IEinkuiIterator* mpIteratorImBack; //返回上级的图标
	IEinkuiIterator* mpIteratorBtBack; //返回上一级

	cmmVector<CFileListItem*> mdList; //存放list对象
	ULONG mulCurrentPage; //当前显示页码
	ULONG mulMaxPage;	//最大页码
	wchar_t mszCurrentPath[MAX_PATH]; //当前路径
	wchar_t mszDisplayPath[MAX_PATH]; //用于在上方显示路径

	vector<wstring> m_FolderPathList; //存放某个文件夹里面所有子文件夹

	//cmmVector<wchar_t*>* mpdHistroyPath; //历史文件
	cmmVector<ULONG> mdFolderLevel;//用于存放上次进入目录时的页码

	bool* mpbIsSucess;
	COfficeConvertDlg *m_pDemoDlg = nullptr;

private:
	//初始化list,默认显示几个常用文件夹及盘符
	void InitList(void);
	//list项被点击
	void ListClick(wchar_t* npszFilePath);
	//进入文件夹
	void EnterFolder(wchar_t* npszPath,bool nbIsBack = false);
	//获取目录下指定文件及目录
	DWORD FillSubDirAndRelatedFiles(wchar_t* npszPath, const vector<const wchar_t*>& relatedExtList);

	//跳转到指定页面
	void SetPage(ULONG nulPage);
	//上一页或下一页
	void NextPage(bool nbIsNext);
	//返回上一级目录
	void BackFolder(void);
	//清理原来的路径数据
	void ClearFilePath(void);
	//获取特殊目录的多语言字符串
	bool GetDisplayName(GUID niCSIDL, OUT wchar_t* npszName, IN int niLen);

	eFileType GetFileType(wstring strFilePath);
	bool m_bCanConvertToPDF = true; // true打开pdf转换逻辑，false关闭pdf转换逻辑

	wstring m_selectedFile;
};

#define FP_LIST_MAX 8	//LIST一页最多几个对象

#define FP_ID_BT_OPEN 1
#define FP_ID_BT_CLOSE 7
#define FP_ID_BT_PRE 3
#define FP_ID_BT_NEXT 4
#define FP_ID_BT_Back 11


#define FP_TIMER_ID_SHOW 1

