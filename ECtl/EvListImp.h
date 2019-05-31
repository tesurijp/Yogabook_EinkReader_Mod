/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EVLISTIMP_H_
#define _EVLISTIMP_H_


#include "cmmstruct.h"
#include "EvScrollBarImp.h"
// Element的基础实现，开发其他Element时，请以此为例；不要尝试从此类字节派生新类，因为，新类往往用于实现派生的接口，直接派生下面的CEvButton将仅仅是提供IXsElement接口
// 如果实现的是相同接口的类别，就可以直接从某个实例化类派生新类。

//CEvList是一个简单控件容器控件，里面的控件可以自己指定
//目前List控件需求
//    1： 可以设置背景，切换背景,背景支持多帧图片
//    2： 可以设置显示模式，目前支持 Report和 SmallItem(对应MFC ClistCtrl SmallIcon模式)
//        Report 和 smallItem 都只支持纵向的滚动条
//    3： 支持Vert 和 Hor 的滚动条
//    4： 可以设置是否需要滚动条，目前滚动条默认放在list的右和下方

DECLARE_BUILTIN_NAME(List)
class CEvList :
	public CXuiElement<CEvList ,GET_BUILTIN_NAME(List)>
{
	friend CXuiElement<CEvList ,GET_BUILTIN_NAME(List)>;
public:
	
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent = NULL,	// 父对象指针
		IN ICfKey* npTemplete = NULL,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		) ;

protected:
	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CEvList();
	// 用于释放成员对象
	virtual ~CEvList();
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);
	//绘制
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);
	
	
	
	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
	
	//切换显示帧,第一帧为1
	virtual ERESULT OnChangeBackImageIndex(LONG nlIndex = 0);
	//更换显示图片
	virtual ERESULT OnChangeBackImagePic(wchar_t* npswPicPath = NULL);

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//重载Resize 重新设置显示区域
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	//根据位置，计算成员变量的位置
	bool CalcElementPosition();

	//鼠标滑轮
	virtual ERESULT OnMouseWheel(const STEMS_MOUSE_WHEEL* npInfo);

	//接受windows键盘命令
	ERESULT OnKeyPressed(const STEMS_KEY_PRESSED* npInfo);

	//字符输入消息
	//virtual ERESULT OnChar(const PSTEMS_CHAR_INPUT npChar);
public:
	//设置显示模式
	bool SetListViewStyle(LONG nlShowStyle);

	//设置是否显示横向和纵向的滚动条
	bool ShowScrollBar(bool nbShowVer,bool nbShowHor);

	//添加一个元素到List容器，list容器将负责释放该元素
	bool AddElement(IEinkuiIterator * npElement,int nIndex ) ;

	//清空List，将释放原有的元素
	bool ResetList();

	//根据索引删除元素
	bool DeleteElement(LONG nlIndex,bool nbClostElement=true);

	//根据元素指针删除元素
	bool DeleteElement(IEinkuiIterator * npElement);

	//设置ScrollBar的位置
	bool SetScrollBarPositionAndSize();

	bool CheckVScrollBarNeedToShow();

private:

	LONG mlCurrentIndex;			 //当前显示第几帧
	LONG mlMaxFrame;				 //最大帧数

	FLOAT mlDocumentHeight;         //list文档的高度，如果是report模式，相当于所有子控件的高度和
	FLOAT mlDocumentWidth;          //list文档的宽度

	bool mbShowVerScroll;          //强制是否显示纵向滚动条
	bool mbShowHorScroll;          //强制是否显示横向滚动条

	LONG mlShowStyle;              //显示模式

	cmmVector<IEinkuiIterator*>     mpElementVector;//元素Vector
	bool mbNeedMemoryManager;   //是否需要List来管理内存，负责释放子Item

	FLOAT mlDocToView0PositionY;    //Doc 位置，相对于View的0位置


	//纵向滚动条
	bool        mbSetScrollBarPositionAndSize;
	CEvScrollBar * mpVScrollBar;
	CEvScrollBar * mpHScrollBar; 
	IEinkuiIterator * mpIterInsertMark;     //插入标识,每个列表自己的key下面创建

	GUID    mGuidDrop;                  //接受Drop的GUID
	bool      mbAcceptDrop;             //是否接受DropItem
	IEinkuiIterator * mpIterMoveItem;       //需要系统Item的Iterator
	IEinkuiIterator * mpIterPicMoveForMouse;//跟随鼠标跑动的图像
   
	D2D1_RECT_F mRectMove;              //转换坐标系的
private:
	//装载配置资源
	ERESULT LoadResource();

	//检查鼠标落点，返回插入的索引 0.n 如果失败返回-1
	int CheckPos(D2D1_POINT_2F nPos);
	
	////把指定项移动到可显示区域
	void ShowByIndex(LONG nlIndex);

	//重新定位位置
	void Recaculate(void);
};


#define LIST_MIN_WHEEL_OFFSET  20
#define TF_ID_LIST_STYLE				 L"Style"			// 0表示SmallIcon风格，1表示Report风格，2表示AutoFitX风格，3表示AutoFitY风格
#define TF_ID_LIST_BACKIAMGE_FRAME_COUNT L"BackFrameCount"	//该图有几帧





#endif//_EVLISTIMP_H_
