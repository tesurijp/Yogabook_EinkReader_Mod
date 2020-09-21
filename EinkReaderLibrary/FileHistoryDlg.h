/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include <tuple>
#include "FileHistoryListItem.h"
#include "cmmstruct.h"
#include "FileOpenDlg.h"
#include "DeleteHistory.h"

//历史文件对话框

DECLARE_BUILTIN_NAME(FileHistoryDlg)

class CFileHistoryDlg:
	public CXuiElement<CFileHistoryDlg,GET_BUILTIN_NAME(FileHistoryDlg)>
{
public:
	// 如果将构造函数设定为protected，就需要加这句话; 否则，不需要下面这句
	friend CXuiElement<CFileHistoryDlg,GET_BUILTIN_NAME(FileHistoryDlg)>;

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
	CFileHistoryDlg();
	~CFileHistoryDlg(void);

	// 模态显示该对话框
	std::tuple<HISTORY_FILE_ATTRIB*, wstring> DoModal(bool nbIsEnableCancel = true);
	//设置历史记录
	void SetHistoryList(cmmVector<HISTORY_FILE_ATTRIB*>* npdHistroyPath);

	void ExitModal();

protected:
	

	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

private:
	IEinkuiIterator* mpIteratorClose;
	IEinkuiIterator* mpIteratorOpen;
	IEinkuiIterator* mpIteratorMore;
	IEinkuiIterator* mpIteratorSelected;

	cmmVector<CFileHistoryListItem*> mdList; //存放list对象

	cmmVector<HISTORY_FILE_ATTRIB*>* mpdHistroyPath; //历史文件
	CFileOpenDlg* mpFileOpenDlg;
	CDeleteHistory* mpDeleteHistory;

	HISTORY_FILE_ATTRIB* m_resultHistoryFile = nullptr;
	wstring m_resultFileName;


	//初始化list,默认显示几个常用文件夹及盘符
	void InitList(void);
	//list项被点击
	void ListClick(HISTORY_FILE_ATTRIB* npFileAttrib);
	//设置打开文件窗口的位置
	void SetOpenFilePos(void);
	//重新定位元素
	void Relocation(void);
};

#define FHD_LIST_MAX 12	//最多显示多少历史记录

#define FHD_BT_CLOSE 3
#define FHD_BT_OPEN 4
#define FHD_BT_MORE 6


#define FHD_TIMER_ID_SHOW 1

