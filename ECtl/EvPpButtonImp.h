/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


DECLARE_BUILTIN_NAME(PingpongButton)

class CEvPingpongButton : public CEvButton
{

public:
	CEvPingpongButton();
	~CEvPingpongButton();

 	// 重载初始化函数
 	ULONG InitOnCreate(
 		IN IEinkuiIterator* npParent,		// 父对象指针
 		IN ICfKey* npTemplete,			// npTemplete的Key ID就是EID，值就是类型EType
 		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
 		);
	
	// 重载实例化接口
	DEFINE_CUMSTOMIZE_CREATE(CEvPingpongButton,(IEinkuiIterator* npParent,ICfKey* npTemplete, ULONG nuEID=MAXULONG32), (npParent, npTemplete, nuEID))

	//////////////////////////////////////////////////////////////////////////
	// 一定要加入这一行，这是重载类型识别
	DEFINE_DERIVED_TYPECAST(PingpongButton,CEvButton)

protected:
	
	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);
	
private:

	int	 miDirection;	// 按钮方向标识，00，标识无效，1标识水平，2标识垂直

};


