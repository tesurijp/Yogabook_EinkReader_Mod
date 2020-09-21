/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	缩略图列表项
*/


DECLARE_BUILTIN_NAME(ThumbnailListItem)

class CThumbnailListItem:
	public CXuiElement<CThumbnailListItem,GET_BUILTIN_NAME(ThumbnailListItem)>
{
	friend CXuiElement<CThumbnailListItem,GET_BUILTIN_NAME(ThumbnailListItem)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置属性
	void SetData(wchar_t* npszFilePath, ULONG nulPageNumber,bool nbIsCurrent);

protected:
	CThumbnailListItem(void);
	~CThumbnailListItem(void);

	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);
	//消息处理函数
	//virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);
	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);
	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);
	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);
	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
	//等比缩放，ndSizeDest原始图片大小  ndSizeSrc要绘制的区域
	//返回等比缩放的大小
	D2D1_RECT_F GetProportionalScaling(D2D1_SIZE_F ndSizeSrc, D2D1_SIZE_F ndSizeDest);

	//调整图片大小和位置
	void RelocationItem(void);
private:

	IEinkuiIterator* mpIterPicture;	//缩略国投新集
	IEinkuiIterator* mpIterPageNumber;		//文件名称
	IEinkuiIterator* mpIterBtClick;		

	DWORD mdwClickTicount;
	ULONG mulPageNumber;
	D2D1_SIZE_F mdPicSize;
	bool mbIsCurrent;

	IEinkuiBrush* mpLineBrush;	// 设备相关
};

#define TL_BT_CLICK 10