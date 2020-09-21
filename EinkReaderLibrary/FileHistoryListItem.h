/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	文件打开历史列表项
*/


// 文件属性结构体
typedef struct _HISTORY_FILE_ATTRIB {
	wchar_t FilePath[MAX_PATH]; //文件路径
	wchar_t ThumbanilFilePath[MAX_PATH]; //缩略图文件路径
	DWORD ReadProgress;			//阅读进度
	DWORD IsModify;				//是否修改过
	ULONG TimeStamp;			//最后打开时间
	ULONG PageContext;			//页面打开标记
	ULONG PageContext2;			//页面打开标记
	ULONG PageNumber;			//页码

// [zhuhl5@20200116:ZoomRecovery]
	int	autoZoom{ 0 };
	float	scaling{ 1.0f };
	int		scalingLevel{ 0 };
	float	scalingPosX{ 0.0f };
	float	scalingPosY{ 0.0f };
}HISTORY_FILE_ATTRIB, *PHISTORY_FILE_ATTRIB;

DECLARE_BUILTIN_NAME(FileHistoryListItem)

class CFileHistoryListItem:
	public CXuiElement<CFileHistoryListItem,GET_BUILTIN_NAME(FileHistoryListItem)>
{
	friend CXuiElement<CFileHistoryListItem,GET_BUILTIN_NAME(FileHistoryListItem)>;

public:
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID = MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	//设置文件属性
	void SetData(HISTORY_FILE_ATTRIB* npFileAttrib);
	//是否启用点击
	void SetEnable(bool nbIsEnable);
protected:
	CFileHistoryListItem(void);
	~CFileHistoryListItem(void);

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


	//是文件的处理
	void ProcFile(HISTORY_FILE_ATTRIB* npFileAttrib);


private:
	HISTORY_FILE_ATTRIB* mpFileAttrib;

	IEinkuiIterator* mpIterFolderIcon;	//文件类型图标
	IEinkuiIterator* mpIterModifyIcon;	//修改标志
	IEinkuiIterator* mpIterName;		//文件名称
	IEinkuiIterator* mpIterAttrib;		//其它属性
	IEinkuiIterator* mpIterBt;			//点击按钮

	DWORD mdwClickTicount;

	wchar_t mpszItem[100];
};

#define FLI_BT 10 //点击