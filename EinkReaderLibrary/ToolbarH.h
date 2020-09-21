/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	横屏时的工具栏
*/

#include "MsgDefine.h"
#include "MenuTxt.h"
#include "EditToolbar.h"
#include "MenuPdf.h"
#include "MenuEpubMobi.h"

DECLARE_BUILTIN_NAME(ToolbarH)

class CToolbarH:
	public CXuiElement<CToolbarH,GET_BUILTIN_NAME(ToolbarH)>
{
	friend CXuiElement<CToolbarH,GET_BUILTIN_NAME(ToolbarH)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置笔状态按钮选中
	void SetPenStatusSelect(ULONG nulID);
	//设置双页显示按钮状态
	void SetDuopageButton(bool nbSingle);
	//设置屏幕方向按钮状态
	void SetScreenOriButton(ULONG nulScreenOri);
	//获取当前双页显示状态
	bool GetDuopageStatus(void);
	//设置当前打开的文件类型
	void SetDoctype(int niType);
	//设置txt字号
	void SetTxtFontSizeIndex(DWORD ndwIndex);
	// 设置笔迹初始化数据
	void SetPenData(DWORD ndwPenWidthIndex, DWORD ndwPenColorIndex);
	//设置redo,undo按钮状态
	void SetUndoRedoStatus(bool nbUndoEnable, bool nbRedoEnable);
	//更新页面状态
	void UpdatePageStatus(PAGE_STATUS ndStatus);
	//设置当前页面标注数量
	void SetCurrentPageInkCount(int niCount);
	//设置文件属性
	void SetFileAttrib(DWORD ndwAttrib);
	//获取文件属性
	DWORD GetFileAttrib(void);
	//文档加载完成
	void DocmentLoadComplete(void);
	//缩略图加载完成
	void ThumbnailsLoadComplete(void);

	//bool GetBCoverState();//读取B面状态是否关闭， true 点亮；false 关闭
	//void SetBCoverState();//根据注册表的值设置B面状态是否关闭， true 点亮；false 关闭
	//void UpdateBCoverState();//同步注册表中的值与变量值一致， true 点亮；false 关闭
	//隐藏更多菜单
	void HideMoreMenu();

protected:
	CToolbarH(void);
	~CToolbarH(void);

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
	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);
private:
	IEinkuiIterator* mpIterBtFileOpen;	//打开文件对话框
	IEinkuiIterator* mpIterBtMore; //更多按钮
	IEinkuiIterator* mpIterBackground;	//背景图
	IEinkuiIterator* mpIterBtThumbnail; //缩略图

	CMenuTxt* mpMenuTxt; //txt文件时的更多菜单
	CMenuPdf* mpMenuPdf; //PDF文件时的更多菜单
	CEditToolbar* mpEditToolbar; //编辑工具栏
	CMenuEpubMobi* mpMenuEpubMobi; //epub/mobi更多菜单

	int miCurrentPageInkCount; //当前页面标注数量
	bool mbIsTwoScreen;//true表示双屏
	//bool mbIsLCDClose;//true表示亮屏；false表示关屏
	int miDocType;
	DWORD mdwTxtFontSizeIndex;
	bool mbIsHScreen; //true表示是横屏
	DWORD mdwAttrib;
	ULONG mulScreenOritent; //屏幕方向

	//txt的更多菜单
	void ShowTxtMoreMenu();
	//pdf的更多菜单
	void ShowPDFMoreMenu();
	//epub/mobi的更多菜单
	void ShowEpubMobiMoreMenu();
	//重新定位元素位置
	void Relocation(void);
};

#define TBH_BT_OPEN_FILE 8
#define TBH_BT_THUMBNAIL 9
#define TBH_BT_MORE 2
