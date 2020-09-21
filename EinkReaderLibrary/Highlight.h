/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	高亮选择控制器
*/

#include "PdfPicture.h"
#include "HighlightToolbar.h"

DECLARE_BUILTIN_NAME(Highlight)

class CHighlight:
	public CXuiElement<CHighlight,GET_BUILTIN_NAME(Highlight)>
{
	friend CXuiElement<CHighlight,GET_BUILTIN_NAME(Highlight)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置文档对象指针
	void SetPdfPicture(CPdfPicture* npPdfPicture);
	//设置上任务栏高度和下任务栏高度
	void SetToolbarHeight(int niTopHeight, int niBottomHeight);
	//隐藏选中
	void HideSelect(void);
	//重新定位
	void Relocation(void);
	//输入事件
	void TouchDown(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);
	void TouchMove(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);
	void TouchUp(PEI_TOUCHINPUT_POINT npPoint, ULONG nulPenMode, bool nbIsHand);
	void PenLeave(ULONG nulPenMode);
protected:
	CHighlight(void);
	~CHighlight(void);

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
	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);
	////鼠标移动
	//virtual ERESULT OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo);
	//处理移动完成事件
	void DragEndPro(D2D1_POINT_2F ndTop, D2D1_POINT_2F ndBottom);
private:


	D2D1_POINT_2F mdPressPos;
	DWORD mdwClickTicount; //用于判断长按 超过500ms算长按
	CPdfPicture* mpPdfPicture;
	CHighlightToolbar* mpHighlightToolbar;
	IEinkuiIterator* mpIterBtTop;
	IEinkuiIterator* mpIterBtBottom;

	bool mbIsPressed;
	bool mbIsTouchDown;
	SELECT_HIGHLIGHT mdHighlightStruct;
	bool mbModifyStatus; //为真表示当前是修改状态，

	int miTopHeight;
	int miBottomHeight;

	EI_TOUCHINPUT_POINT mdPrePoint; //上一个点的信息


};


