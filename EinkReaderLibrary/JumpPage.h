/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

#include "cmmstruct.h"

//页码跳转对话框

DECLARE_BUILTIN_NAME(JumpPage)

class CJumpPage:
	public CXuiElement<CJumpPage,GET_BUILTIN_NAME(JumpPage)>
{
public:
	// 如果将构造函数设定为protected，就需要加这句话; 否则，不需要下面这句
	friend CXuiElement<CJumpPage,GET_BUILTIN_NAME(JumpPage)>;

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
	CJumpPage();
	~CJumpPage(void);

	// 模态显示该对话框
	void DoModal();

	void ExitModal();
	//设置当前页码
	void SetCurrentPage(int niPage, int niMaxPage);

protected:
	

	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	

private:
	IEinkuiIterator* mpIteratorClose;
	IEinkuiIterator* mpIteratorOk;
	IEinkuiIterator* mpIteratorBackspace; //删除按钮
	IEinkuiIterator* mpIteratorCurrentPage;//当前位置
	IEinkuiIterator* mpIteratorPageMax; //输入页码
	IEinkuiIterator* mpIteratorPageMaxText; //最大页码范围
	IEinkuiIterator* mpIteratorInput; //输入的页码
	IEinkuiIterator* mpIteratorCur; //输入时闪烁的光标
	wchar_t mszInputNumber[MAX_PATH];	//输入的数字
	int miMaxPage;
	int miInputPage; //输入的页码

	//处理输入的数字
	void InputNumber(ULONG nulNumber);
	//删除一位数据
	void DeleteNumber();
	//清空当前输入
	void ClearNumber();
};

#define JP_ID_BT_OK 8
#define JP_ID_BT_CANCAL 7
#define JP_ID_BT_BACKSPACE 6

#define JP_ID_BT_ZERO 10
#define JP_ID_BT_ONE 11
#define JP_ID_BT_TWO 12
#define JP_ID_BT_THREE 13
#define JP_ID_BT_FOR 14
#define JP_ID_BT_FIVE 15
#define JP_ID_BT_SIX 16
#define JP_ID_BT_SEVEN 17
#define JP_ID_BT_EIGHT 18
#define JP_ID_BT_NINE 19




