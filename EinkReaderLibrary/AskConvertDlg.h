#pragma once

#include "StdAfx.h"
#include <string>
#include "ElementImp.h"
#include "cmmstruct.h"

using std::wstring;

DECLARE_BUILTIN_NAME(AskConvertDlg)

enum class PromptDialogAskResult : int
{
	Yes = 0,
	No = 1,
};

class CAskConvertDlg: 
	public CXuiElement<CAskConvertDlg, GET_BUILTIN_NAME(AskConvertDlg)>
{
public:
	CAskConvertDlg() = default;
	~CAskConvertDlg() = default;

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator) override;
	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用EUI系统自动分配
		);

	// 模态显示该对话框
	PromptDialogAskResult DoModal();
	void ExitModal(PromptDialogAskResult result);

protected:
	//消息处理函数
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg) override;
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender) override;
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize) override;

private:

	void NotAskConvert(bool nbIsShow);
	IEinkuiIterator* mpIteratoraskswitch = nullptr;	
	PromptDialogAskResult m_result;
	bool mbIsAsk = false;
};

#define PDF_COV_CONTINUE  1
#define PDF_COV_CANCEL    2
#define PDF_COV_ASKSWITCH 6

