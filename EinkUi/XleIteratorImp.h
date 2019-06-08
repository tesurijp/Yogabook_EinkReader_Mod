/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _ELEITERATORIMP_H_
#define _ELEITERATORIMP_H_
#include <d2d1.h>
/*
	本文定义元素迭代器的实现类

*/

class CXuiIterator;

class CEoSubItrNode{
public:
	ULONG muID;
	CXuiIterator* mpIterator;
	void operator=(const class CEoSubItrNode& src){
		muID = src.muID;
		mpIterator = src.mpIterator;
	}
};

class CEoSubItrNodeCriterion	// 默认的判断准则
{
public:
	bool operator () (const CEoSubItrNode& Obj1,const CEoSubItrNode& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.muID < Obj2.muID);
	}
};

// 按照ID排序的迭代器队列
typedef cmmSequence<CEoSubItrNode,CEoSubItrNodeCriterion> TSubItrSequence;


// 用于按照TabOrder或ZOrder排列子元素迭代器的数据结构
typedef cmmVector<CXuiIterator*>  TElIteratorVector;


class CXuiHotkeyEntry{
public:
	enum ExKey{
		eControl=1,
		eShit=2,
		eAlt=4
	};
	ULONG muHotKeyID;		//注册时提供的ID
	USHORT msuVkCode;	// 快捷键的普通按键
	UCHAR mcuExKey;
	IEinkuiIterator* mpApplicant;

	void operator=(const class CXuiHotkeyEntry& src){
		muHotKeyID = src.muHotKeyID;
		msuVkCode = src.msuVkCode;
		mcuExKey = src.mcuExKey;
		mpApplicant = src.mpApplicant;
	}
};

class CXuiHotkeyEntryCriterion	// 默认的判断准则
{
public:
	bool operator () (const CXuiHotkeyEntry& Obj1,const CXuiHotkeyEntry& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.msuVkCode < Obj2.msuVkCode || (Obj1.msuVkCode == Obj2.msuVkCode && Obj1.mcuExKey < Obj2.mcuExKey));
	}
};


typedef cmmSequence<CXuiHotkeyEntry,CXuiHotkeyEntryCriterion,0,8> THotKeyTable;



// 迭代器扩展包
class CXuiIteratorExtension{
public:
	TSubItrSequence moIDOrder;	// 按照ID排序的线性表
	TElIteratorVector moZOrder;	// 按照元素的层叠关系排列的链表
	TElIteratorVector moTabOder;	// 按照Tab按键的焦点选择次序来排序的链表
	// Direct2D的Layer
	// Direct32的相关参数

};

// 下面的标志是用来判断Iterator的内容的标志位
#define EITR_FLAG_INIT		1
#define EITR_FLAG_DIRTY		2	//此元素的位置和其他显示属性发生过改变，并且尚未重新刷新
#define EITR_FLAG_VISIBLE	3	//是否可视
#define EITR_FLAG_DISABLE	4	//是否使能
#define EITR_FLAG_ALPHA		5	//是否设置了透明度
#define EITR_FLAG_ROTATION	6	//是否设置了平面旋转	'rtal' 'rtcn'
#define EITR_FLAG_WIDGET_HOME	7	//是不是一个Widget的Home Page
#define EITR_FLAG_ENHANCER	8	//是否有增效器	'ehcr'
#define EITR_FLAG_HOOK		9	//是否被Hook		'hook'
#define EITR_FLAG_VREGION	10	//是否设置了可视区 'vrgn'
#define EITR_FLAG_TIP		11	//是否设置了Tip	'tip'
#define EITR_FLAG_HOTKEY	12  //是否注册了快捷键 'htky'
#define EITR_FLAG_DELETED	13	//已经被删除
#define EITR_FLAG_PAINTLEVEL_HOST 14 //内建类绘制层，层数通过'plvc'获得
#define EITR_FLAG_CRT_PAINTLEVEL  15 //设定了绘制层次，当前层通过'plvn'获得

#define ITR_CHECK() //CMMASSERT(gpElementManager->VerifyIterator(this)==ERESULT_SUCCESS)



DECLARE_BUILTIN_NAME(CXuiIterator)
class CXuiIterator: public cmmBaseObject<CXuiIterator,IEinkuiIterator,GET_BUILTIN_NAME(CXuiIterator)>
{
	friend class CXelManager;
	friend class CEinkuiSystem;
public:
	DEFINE_CUMSTOMIZE_CREATE(CXuiIterator,(),())

	// 启动Iterator，准备接受消息，调用这个方法后，Element首先会收到EMSG_CREATE消息，这个方法通常在Element的实例化函数退出前调用
	virtual void __stdcall Start(void);

	// 关闭一个元素
	virtual void __stdcall Close(void);

	// 重载增加引用接口，禁用它
	virtual int __stdcall AddRefer(void){
		return 1;
	}
	// 重载释放接口，禁用它
	virtual int __stdcall Release(void){
		return 1;
	}

	// 获得本Element的EID
	virtual ULONG __stdcall  GetID(void);

	// 获得本Element所属的Etype
	virtual const wchar_t* __stdcall  GetType(void);

	// 获得父对象
	virtual IEinkuiIterator* __stdcall  GetParent(void);

	// 获得本迭代器对应的Element对象
	virtual IXsElement* __stdcall GetElementObject(void);

	// 获得下一层的子对象的总数
	virtual int __stdcall GetSubElementCount(void);

	// 询问某个Iterator在层次结构上是不是当前Iterator的祖先
	virtual bool __stdcall FindAncestor(const IEinkuiIterator* npIsAncestor);

	// 通过ZOder的排列次序获得子节点，返回的接口需要释放
	virtual IEinkuiIterator* __stdcall  GetSubElementByZOder(
		int niPos	// zero base index value to indicate the position in z-order array
		);

	// 通过ID获得子节点，返回的接口需要释放
	virtual IEinkuiIterator* __stdcall  GetSubElementByID(ULONG nuEid);

	// 给此元素发送一条消息，发送模式是Send
	// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimplePostMessage方法
	virtual ERESULT __stdcall SendMessage(IEinkuiMessage* npMsg);

	// 给此元素发送一条消息，发送模式是Post
	// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimplePostMessage方法
	virtual ERESULT __stdcall PostMessage(IEinkuiMessage* npMsg);

	// 给此元素的父元素发送一条消息，发送的模式是FastPost
	// 消息发送后，发送者仍然需要释放
	// !!!注意!!! 仅用于发送者是本迭代器对应的元素之情况
	virtual ERESULT __stdcall PostMessageToParent(IEinkuiMessage* npMsg);

	// 申请键盘焦点，如果该元素具有popup属性，也将被调整到合适的上层
	virtual ERESULT __stdcall SetKeyBoardFocus(void);

	// 释放键盘焦点，这将导致Tab Order的下一个键盘接收者获得焦点
	virtual void __stdcall ReleaseKeyBoardFocus(bool nbShiftFocus);

	// 设置元素为活跃，如果本元素不具有EITR_STYLE_POPUP或EITR_STYLE_ACTIVE风格，那么最低的一个具有EITR_STYLE_POPUP或EITR_STYLE_ACTIVE风格的上层元素将被激活
	virtual void __stdcall SetActive(void);

	// 设置Style
	virtual void __stdcall SetStyles(ULONG nuStyles);

	// 修改Style，前一个参数是希望增加的Style，后一个参数是希望移除的Style，当前后两个参数中包括相同Style时，该Style不会被移除 
	virtual void __stdcall ModifyStyles(ULONG nuSet,ULONG nuClear=0);

	// 读取Style
	virtual ULONG __stdcall GetStyles(void);

	// 申请定时器，对于永久触发的定时器，需要注销
	virtual ERESULT __stdcall SetTimer(
		IN ULONG nuID,	  // 定时器ID
		IN ULONG nuRepeat,// 需要重复触发的次数，MAXULONG32表示永远重复
		IN ULONG nuDuration,	// 触发周期
		IN void* npContext//上下文，将随着定时器消息发送给申请者
		);

	// 销毁定时器
	virtual ERESULT __stdcall KillTimer(
		IN ULONG nuID	  // 定时器ID
		);

	// Hook目标，当前仅支持单层次的Hook，即，一个元素在同一时刻仅被一个元素Hook；试图Hook一个已经被Hook的元素时，将会返回失败ERESULT_ACCESS_CONFLICT
	virtual ERESULT __stdcall SetHook(
		IN IEinkuiIterator* npHooker,	// Hook请求者，一旦设置了Hook，本对象的所有消息，都会先发送给Hooker处理，Hooker可以修改任意的消息，也可以阻止消息发送给本对象
		IN bool nbSet		// true to set ,false to remove
		);

	// 获得Hooker，获取本元素被谁Hook
	virtual IEinkuiIterator* __stdcall GetHooker(void);

	// 设置渲染增效器，增效器用于给某个元素和它的子元素提供特定的渲染，增效器可以选择Direct2D，Direct3D技术完善XUI系统的渲染
	// 同一个元素在同一时刻只能有一个增效器在工作；并且，通常增效器都是对其父元素发生作用
	// 返回ERESULT_ACCESS_CONFLICT表示多个增效器发生冲突；增效器设置，请在接收到EMSG_PREPARE_PAINT时处理，其他地方做设置，有可能导致严重错误
	virtual ERESULT __stdcall SetEnhancer(
		IN IEinkuiIterator* npEnhancer,
		IN bool nbEnable		// true 启用，false 取消
		);

	// 获得增效器
	virtual IEinkuiIterator* __stdcall GetEnhancer(void);

	//////////////////////////////////////////////////////////////////////////
	// 下面是所有与显示和位置相关的方法

	// 设置显示/隐藏
	virtual void __stdcall SetVisible(bool nbVisible);

	// 读取显示/隐藏状态
	virtual bool __stdcall IsVisible(void);

	// 设定整体透明度
	virtual void __stdcall SetAlpha(FLOAT nfAlpha);

	// 读取整体透明度
	virtual FLOAT __stdcall GetAlpha(void);

	// 设置平面坐标
	virtual void __stdcall SetPosition(FLOAT nfX,FLOAT nfY);
	virtual void __stdcall SetPosition(const D2D1_POINT_2F& rPosition);

	// 读取平面坐标
	virtual FLOAT __stdcall GetPositionX(void);
	virtual FLOAT __stdcall GetPositionY(void);
	virtual D2D1_POINT_2F __stdcall GetPosition(void);
	virtual void __stdcall GetRect(D2D1_RECT_F& rRect);

	// 设置可视区域
	virtual void __stdcall SetVisibleRegion(
		IN const D2D1_RECT_F& rRegion		// 基于相对坐标的可视区域，此区域之外不会显示本元素及子元素的内容；如果rRegion.left > region.right 表示取消可视区设置
		);

	// 获取可视区域，返回false表示没有设置可是区域
	virtual bool __stdcall GetVisibleRegion(
		OUT D2D1_RECT_F& rRegion	// 返回可视区域，如果没有设置可视区域，则不会修改这个对象
		);

	// 设置平面转角
	virtual void __stdcall SetRotation(
		FLOAT nfAngle,			// 角度单位 -359 -> +359度
		D2D1_POINT_2F ndCenter
		);
	// 设置平面转角，以元素中心为旋转中心
	virtual void __stdcall SetRotation(
		FLOAT nfAngle			// 角度单位 -359 -> +359度
		);

	// 读取平面转角
	virtual FLOAT __stdcall GetRotationAngle(void);
	virtual D2D1_POINT_2F __stdcall GetRotationCenter(void);
	virtual FLOAT __stdcall GetRotation(D2D1_POINT_2F& rCenter);

	// 设置参考尺寸
	virtual void __stdcall SetSize(FLOAT nfCx,FLOAT nfCy);
	virtual void __stdcall SetSize(const D2D1_SIZE_F& rSize);

	// 读取参考尺寸
	virtual FLOAT __stdcall GetSizeX(void);
	virtual FLOAT __stdcall GetSizeY(void);
	virtual D2D1_SIZE_F __stdcall GetSize(void);

	// 设置是否有效
	virtual void __stdcall SetEnable(bool nbSet);

	// 读取是否有效
	virtual bool __stdcall IsEnable(void);

	// 将本元素调整到同级窗口最高层
	virtual void __stdcall BringToTop(void);

	// 将本元素在父元素的Z Order序列中，向下移动一位
	virtual bool __stdcall MoveDown(void);

	// 将本元素在父元素的Z Order序列中，向上移动一位
	virtual bool __stdcall MoveUp(void);

	// 整理所有子元素的叠放次序为原始设置（即配置文件制定的次序）
	virtual bool __stdcall ResetChildrenByDefualtZOrder(void);

	// 重新设置本元素的Z Order的值，如果两个兄弟Element具有相同的Z Order将无法确定它们的先后循序，但系统本身运行不会出错
	virtual void __stdcall SetDefaultZOrder(const LONG nlDefaultZOrder);

	// 获得默认的ZOrder值
	virtual LONG __stdcall GetDefaultZOrder(void);

	//设置ToolTip，鼠标在本对象上悬停，将会自动显示的单行简短文字提示，鼠标一旦移开显示的ToolTip，它自动消失
	virtual void __stdcall SetToolTip(const wchar_t* nswTip);

	//设置IME输入窗口位置，ndPosition是本元素局部坐标中的位置; 只有当原元素具有EITR_STYLE_IME属性时，才有效
	virtual void __stdcall SetIMECompositionWindow(D2D1_POINT_2F ndPosition);

	// 从局部地址到世界地址
	virtual bool __stdcall LocalToWorld(const D2D1_POINT_2F& crLocalPoint,D2D1_POINT_2F& rWorldPoint);

	// 从世界地址转换为局部地址
	virtual bool __stdcall WorldToLocal(const D2D1_POINT_2F& crWorldPoint,D2D1_POINT_2F& rLocalPoint);

	// 对子元素建立绘制层，绘制层是一个改变子元素绘制时叠放次序的方法，通常的子元素按照从属关系排列为树形结构，绘制时也是按照树形结构先根遍历执行；
	//		引入绘制层技术后，子元素将在不同层上逐次绘制，同一个层的子元素，仍然按照从属关系的树形结构先根遍历；
	//		可以设定任意的绘制层，但不能大于max-ulong；鼠标落点的判定也收到绘制层的影响，层次高的元素首先被判定获得鼠标
	//		如果在一个已经设定了绘制层次的子树中的某个元素再次建立绘制层次，那么它的子树中的所有子元素将不受到上一个绘制层次的影响，按照新的层次工作；
	//		为了避免错误，请尽可能避免使用嵌套的绘制层次;
	virtual ERESULT __stdcall CreatePaintLevel(LONG nlLevelCount);


	// 获得子元素绘制层数量
	virtual LONG __stdcall GetPaintLevelCount(void);

	// 删除绘制层次设定
	virtual ERESULT __stdcall DeletePaintLevel(void
		//bool nbClearAll=true		// 清除掉所有子元素的绘制层次设定
		);

	// 设定自身的绘制层次
	virtual ERESULT __stdcall SetPaintLevel(LONG nlLevel);

	// 获得自身的绘制层次
	virtual LONG __stdcall GetPaintLevel(void);


	void SaveWorldMatrix(const D2D1::Matrix3x2F& rWorld){

		if(	mdWorldMatrix._11 == rWorld._11 &&
			mdWorldMatrix._12 == rWorld._12 &&
			mdWorldMatrix._21 == rWorld._21 &&
			mdWorldMatrix._22 == rWorld._22 &&
			mdWorldMatrix._31 == rWorld._31 &&
			mdWorldMatrix._32 == rWorld._32 )
			return;

		mdWorldMatrix = rWorld;
		mbInverted = false;
	}

	// 获取最后一次绘制时用的World Matrix
	const D2D1::Matrix3x2F& GetWorldMatrix(void){
		return mdWorldMatrix;
	}

	// 供LaunchWidget方法调用，用于设定这个页是不是Widget的主页，其他目的不要使用
	void SetWidgetHomeFlag(bool nbSet){
		SetFlags(EITR_FLAG_WIDGET_HOME,nbSet);
	}
	bool IsWidgetHome(void){
		return TestFlag(EITR_FLAG_WIDGET_HOME);
	}

	// 检查初始化标志
	bool HasInitialized(void){
		return TestFlag(EITR_FLAG_INIT);
	}

	// 检查Style，执行‘与’运算和相等比较，检查是否设置了对应的Style组合
	bool CheckStyle(ULONG nuStyle){
		return ((muStyle&nuStyle)==nuStyle);
	}

	// 设置为已被删除
	void SetAsDeleted(void){
		SetFlags(EITR_FLAG_DELETED,true);
	}

	// 是否已经被删除
	bool IsDeleted(void){
		return TestFlag(EITR_FLAG_DELETED);
	}

	// 获得下一绘制层，返回值小于等于nuCrtLevel，表示没有下一绘制层
	LONG GetNextPaintLevel(LONG nlCrtLevel);

private:
	static CXelManager* gpElementManager;

	// 内部成员
	cmmAttributes<16> moAtts;
	static CExclusiveAccess goAttributesLock;	// 所有对象访问Attribute使用同一个互斥区
	CXuiIteratorExtension* mpExtension;
	ULONG muEID;
	LONG mlTabOrder;
	LONG mlZOrder;
	ULONG muStyle;
	D2D1_POINT_2F mdPosition;
//	D2D1_POINT_2F mdCenter;
	D2D1_SIZE_F mdSize;
//	FLOAT mfRotation;
	FLOAT mfAlpha;
	D2D1::Matrix3x2F mdWorldMatrix;	// 最后一次绘制时用的World Matrix
	D2D1::Matrix3x2F mdWMInverted;	// 上面的矩阵mdWorldMatrix的逆阵
	bool mbInverted;	// 是否已经切换成了逆阵
	IXsElement* mpElement;
	CXuiIterator* mpParent;
	IXsWidget* mpWidget;
	//IEinkuiIterator* mpEnhancer;	// 挂接的增效器，同一个元素在同一时刻只能有一个增效器在工作；并且，通常增效器都是对其父元素发生作用

	
	CXuiIterator();
	~CXuiIterator();

	// 实际增加引用接口
	virtual int __stdcall KAddRefer(void){
		return cmmBaseObject::AddRefer();
	}
	// 实际释放接口
	virtual int __stdcall KRelease(void){
		return cmmBaseObject::Release();
	}
	

	// 设置标志，共有31位可用；设为私有，派生类不能调用
	bool SetFlags(
		int niIndex,		// 标志的序号，从0开始；如果派生类重载这个函数，并且该派生类有2个不希望被后续类和用户修改的标志，那么它的函数调用时的niIndex=0表示的是它的基类的2
		bool nbSet=true		// 设置或者清除标志
		){
			return false;
	}

	// 获取标志，设为私有，派生类不能调用
	bool TestFlag(int niIndex){
		return false;
	}

	// 添加一个子节点
	ERESULT AddSubElement(
		CXuiIterator* npSubElement
		);

	// 删除一个子节点
	void RemoveSubElement(
		CXuiIterator* npSubElement
		);

	// 将某个子元素调整到ZOder最高层
	void BringSubElementToTop(
		CXuiIterator* npSubElement
		);

	// 将元素插入到ZOrder中
	void InsertToZOder(
		CXuiIterator* npSubElement
		);

	// 在子节点中查找携带目标的迭代器
	CXuiIterator* SeekIteratorInChild(IXsElement* npElement);

	// 供元素管理器用，增加引用接口
	int ProtectedAddRefer(void){
		return cmmBaseObject<CXuiIterator,IEinkuiIterator,GET_BUILTIN_NAME(CXuiIterator)>::AddRefer();
	}
	// 供元素管理器用，释放接口
	int ProtectedRelease(void){
		return cmmBaseObject<CXuiIterator,IEinkuiIterator,GET_BUILTIN_NAME(CXuiIterator)>::Release();
	}

	// 调整TabOrder和ZOrder，初始化之际有Element管理器调用
	void UpdateOrder(void);

	// 调整子元素在ZOrder的顺序
	bool MoveOnZOrder(bool nbUp,CXuiIterator* npChild);

	// 获得下一个EITR_STYLE_KEYBOARD的元素，不会进入Iterator临界区
	CXuiIterator* GetNextKeyBoardAccepter(CXuiIterator* npCurrent);

	void ClearTip();

	// 获得Tip
	const wchar_t* GetToolTip(void);

	// 注册快捷键，当快捷键被触发，注册快捷键的Element将会受到；
	// 如果普通按键组合（不包含Alt键)按下的当时，存在键盘焦点，按键消息会首先发送给键盘焦点，如果焦点返回ERESULT_KEY_UNEXPECTED才会判断是否存在快捷键行为
	bool RegisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册的元素，将有它收到注册是快捷键消息
		IN ULONG nuHotKeyID,	// 事先定义好的常数，用来区分Hotkey；不能出现相同的ID，试图注册已有的Hotkey将会失败
		IN ULONG nuVkNumber,	// 虚拟键码
		IN bool nbControlKey,	// 是否需要Control组合
		IN bool nbShiftKey,		// 是否需要Shift组合
		IN bool nbAltKey		// 是否需要Alt组合
		);

	// 注销快捷键
	bool UnregisterHotKey(
		IN IEinkuiIterator* npApplicant,	// 注册者
		IN ULONG UnuKeyNumber
		);

	// 检查是否具有符合的HotKey
	bool DetectHotKey(
		CXuiHotkeyEntry& rToDetect
		);

	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();
};















#endif//_ELEITERATORIMP_H_
