/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	缩略图界面
*/

#include "PageProgress.h"
#include "ThumbnailListItem.h"
#include "cmmstruct.h"
#include "PdfPicture.h"
#include "MenuThumbnail.h"

DECLARE_BUILTIN_NAME(ThumbnailDlg)

#define TD_ITEM_MAX 16 //最大数量

class CThumbnailDlg:
	public CXuiElement<CThumbnailDlg,GET_BUILTIN_NAME(ThumbnailDlg)>
{
	friend CXuiElement<CThumbnailDlg,GET_BUILTIN_NAME(ThumbnailDlg)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//显示或隐藏元素
	void ShowItem(bool nbIsShow);
	//设置当前打开的文件类型
	void SetDoctype(int niType);
	//设置文档对象指针
	void SetPdfPicture(CPdfPicture* npPdfPicture);
	// 模态显示该对话框
	void DoModal();
	void ExitModal();


protected:
	CThumbnailDlg(void);
	~CThumbnailDlg(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//重新定位元素
	void RelocationItem(void);

	//跳转到指定页面
	void EnterPage(ULONG nulPage);
	//上一页或下一页
	void NextPage(bool nbIsNext);
	//获取翻译字符串
	void GetTranslateString(ULONG nulPageCount,ULONG nulPageAnnot);
	//进入全部或筛选
	void SelectAll(bool nbIsAll);
private:

	IEinkuiIterator* mpIterBtPageNumber;	//页码
	IEinkuiIterator* mpIteratorPre;	//上一页
	IEinkuiIterator* mpIteratorNext;	//下一页
	IEinkuiIterator* mpIteratorSelect;	//筛选项
	CPageProgress* mpPageProgress; //页面跳转

	int miDocType;
	IEdDocument_ptr mpDocument;
	CMenuThumbnail* mpMenuThumbnail;
	wchar_t mszPageAll[MAX_PATH];
	wchar_t mszPageAnnot[MAX_PATH];
	
	ULONG mulCurrentPage; //当前显示页码
	ULONG mulMaxPage;	//缩略图最大页码
	ULONG mulDocMaxPage; //文档最大页码
	ULONG mulAnnotPage; //有标注的页面数量
	ULONG mulSelectType; 
	ULONG mulCurrentDoc; //当前用户正在看的书是哪页
	cmmVector<ULONG> mdAnnotPageNumber; //已标注页面页码

	cmmVector<CThumbnailListItem*> mdList; //存放list对象
	
};

#define TD_BT_CLOSE 1
#define TD_BT_PRE 2
#define TD_BT_NEXT 3
#define TD_BT_NUMBER 5
#define TD_BT_SELECT 6

