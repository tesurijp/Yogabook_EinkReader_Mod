/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _ELEMENTIMP_H_
#define _ELEMENTIMP_H_

#include "XCtl.h"
#include "Xuix.h"

#if 0
//////////////////////////////////////////////////////////////////////////
// Element模板的使用方法

// 头文件中
DECLARE_BUILTIN_NAME(Button)	// 此处的Button就是EType字符串，注意，不要使用引号
class CEvButton: public CXuiElement<CEvButton,GET_BUILTIN_NAME(Button)>
{
	// 如果将构造函数设定为protected，就需要加这句话; 否则，不需要下面这句
	friend CXuiElement<CEvButton,GET_BUILTIN_NAME(Button)>;
public:

	// 只需要重载自己关心的方法
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

protected:
	CEvButton();		// 可以将构造函数设定为保护类型，确保实例对象是通过标准的CreateInstance建立，而不是直接用C++定义的方法建立
	~CEvButton();

};


// 源文件中
DEFINE_BUILTIN_NAME(Button)
ERESULT CEvButton::ParseMessage(IEinkuiMessage* npMsg)
{
	return CXuiElement<CEvButton,GET_BUILTIN_NAME(Button)>::ParseMessage(npMsg);
}

/////////////////////////////////////////////////////////////////////////*/
#endif



//////////////////////////////////////////////////////////////////////////
// 实现Element的辅助模板，开发其他Element时，可以从此模板实例化类派生新类。
template<class CClassDeriveTo,const wchar_t* Name>
class CXuiElement: public cmmBaseObject<CClassDeriveTo,IXsElement,Name>
{
public:
	// 获得本元素的元素类型
	virtual const wchar_t* __stdcall  GetType(void);

	// 在全局范围内验证此对象是否是nswType指定的类型
	virtual bool __stdcall  GlobleVerification(const wchar_t* nswType);

	// 默认消息入口函数，用于接收输入消息
	virtual ERESULT __stdcall MessageReceiver(IEinkuiMessage* npMsg = NULL);

	// 获得本元素的迭代器接口
	virtual IEinkuiIterator* __stdcall GetIterator(void);

	// 获得Tab Order, -1 未设置
	virtual LONG __stdcall GetTabOrder(void);

	// 获得Z Order，-1 未设置
	virtual LONG __stdcall GetZOrder(void);

	// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
	// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
	ULONG InitOnCreate(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	// 通过配置数据实例化对象
	static CClassDeriveTo* CreateInstance(IEinkuiIterator* npParent,ICfKey* npTemplete,ULONG nuEID=0) 
	{		// 调用C++默认的new operator而不是operator new操作，它首先导致我们重载的void* __cdecl operator new(size_t nSize,bool nbDummy)被调用，而后会去调用默认的构造函数
			CClassDeriveTo* lpObj = new (false)CClassDeriveTo;
			if(lpObj != NULL)
			{
				// 设置标志表示这个对象的内存需要被释放
				lpObj->SetDeleteRequired();
				// 调用初始化函数，检查返回值，如果失败，释放掉该对象
				if((lpObj->InitOnCreate(npParent,npTemplete,nuEID)&0x80000000)!=0)
				{
					lpObj->Release();
					lpObj = NULL;
				}
				else
					lpObj->CompleteCreation();
			}
			// 返回构造完毕的对象
			return lpObj;
	}


	// 建立完成函数，提供一个在派生类建立完成之际调用基类方法的机会，如果从某个派生类继续派生新类，并且希望重载这个方法，请!!!注意!!!，一定要在退出之前的最后时刻调用基类的相同方法
	void CompleteCreation(void){
		mpIterator->Start();
	}

private:
	// 屏蔽基类提供的默认实例化函数
	static CClassDeriveTo* CreateInstance(){
		return NULL;
	} 

public:

	//////////////////////////////////////////////////////////////////////////
	// 下面三个函数是消息处理的三个阶段，他们是由本类的CXuiElement::MessageReceiver调用的，一旦修改了默认的消息接受函数，请自行处理消息处理过程
	//////////////////////////////////////////////////////////////////////////

	// 分解消息前步骤，提供一个在消息分解前获得消息的机会，仅用于修改消息内容的目的；这个函数的规则是，一定要首先调用基类的BeforeParseMessage，从而确保基类保留对消息的优先权；
	// 通常情况下，不许要重载本函数；本函数的设定目的是为了确保某种消息的处理行为能够延续到派生类中，而不被派生类修改；
	// 返回ERESULT_UNEXPECTED_MESSAGE将继续消息分解过程，其他值都会使得消息分解过程中止
	virtual ERESULT BeforeParseMessage(IEinkuiMessage* npMsg);

	// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
	// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
	virtual ERESULT ParseMessage(IEinkuiMessage* npMsg);

	// 分解消息后步骤，提供一个在消息处理完毕后执行的机会，通常用于监控消息的结果或则产生后续行为；这个函数的调用规则是，一定要最后调用基类的实现，从而确保基类最后执行
	// 通常情况下，不许要重载本函数；本函数的设定目的是为了确保某种消息的处理行为能够延续到派生类中，在派生类之后执行；
	// 如无特定目的，不应该修改函数的消息处理的返回值，该返回值存放在npMsg指向的消息对象中;本函数不设返回值
	virtual void AfterParseMessage(IEinkuiMessage* npMsg);
	
	//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
	virtual ERESULT OnElementCreate(IEinkuiIterator* npIterator);

	// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
	// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
	virtual ERESULT OnElementDestroy();

	// 鼠标落点检测
	virtual ERESULT OnMouseOwnerTest(const D2D1_POINT_2F& rPoint);

	//按钮单击事件
	virtual ERESULT OnCtlButtonClick(IEinkuiIterator* npSender);

	//元素位置被改变,已经改变完成
	virtual ERESULT OnElementMoved(D2D1_POINT_2F nNewPoint);

	//元素参考尺寸发生变化
	virtual ERESULT OnElementResized(D2D1_SIZE_F nNewSize);

	//通知元素【显示/隐藏】发生改变
	virtual ERESULT OnElementShow(bool nbIsShow);

	//定时器
	virtual void OnTimer(
		PSTEMS_TIMER npStatus
		);

	// 慢刷新
	virtual void OnLazyUpdate(
		PSTEMG_LAZY_UPDATE npLazyUpdate
		);

	//用于转发被Hook的消息
	virtual ERESULT OnHookMessage(IEinkuiIterator* npOriginalElement,	// 原始的消息接受元素
								  IEinkuiMessage* npOriginalMessage		// 被捕获的消息
								 );

	//反馈消息
	virtual ERESULT OnResponseMessage(IEinkuiIterator* npOriginalElement,	// 原始消息的接受元素
									  IEinkuiMessage* npOriginalMessage,		// 被反馈的消息
									  void* npContext					// 设置消息反馈时，指定的上下文
									 );

	//鼠标按下
	virtual ERESULT OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo);

	//鼠标双击
	virtual ERESULT OnMouseDbClick(const STEMS_MOUSE_BUTTON* npInfo);

	//鼠标移动
	virtual ERESULT OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo);

	//鼠标进入或离开
	virtual void OnMouseFocus(PSTEMS_STATE_CHANGE npState);

	//鼠标悬停
	virtual ERESULT OnMouseHover();

	//鼠标滑轮
	virtual ERESULT OnMouseWheel(const STEMS_MOUSE_WHEEL* npInfo);

	//键盘焦点获得或者失去
	virtual void OnKeyBoardFocus(PSTEMS_STATE_CHANGE npState);

	//键盘消息
	virtual ERESULT OnKeyPressed(const STEMS_KEY_PRESSED* npInfo);

	//快捷键消息
	virtual ERESULT OnHotKey(const STEMS_HOTKEY* npHotKey);

	//绘制消息
	virtual ERESULT OnPaint(IEinkuiPaintBoard* npPaintBoard);

	//禁用或启用
	virtual ERESULT OnElementEnable(bool nbIsEnable);

	//元素拖拽
	virtual ERESULT OnDragging(const STMS_DRAGGING_ELE* npInfo);

	//拖拽开始
	virtual ERESULT OnDragBegin(const STMS_DRAGGING_ELE* npInfo);

	//拖拽结束
	virtual ERESULT OnDragEnd(const STMS_DRAGGING_ELE* npInfo);

	//命令
	virtual ERESULT OnCommand(const nes_command::ESCOMMAND neCmd);

	////给自己父窗口发送简单消息
	//ERESULT __stdcall PostMessageToParent(
	//	IN ULONG nuMsgID,		// 消息编码
	//	IN void* npInputBuffer,	// 输入数据的缓冲区
	//	IN int niInputSize		// 输入数据的大小
	//	);

	// Post模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Post发送消息需要注意这个问题，防止地址到消息接收者出无法访问。
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;
	template<typename DataType>
	ERESULT PostMessageToParent(ULONG nuMsgID,const DataType& rDataToPost);

	// Send模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Send模式发送消息时，这不会导致特殊问题
	// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
	template<typename DataType>
	ERESULT SendMessageToParent(ULONG nuMsgID,const DataType& rDateForSend,void* npBufferForReceive,int niBytesOfBuffer);

	//创建自己所有的子元素
	virtual ERESULT LoadSubElement();

protected:
	ICfKey* mpTemplete;			     //配置文件指针，用来读取配置数据
	IEinkuiIterator* mpIterator;
	IEinkuiBitmap* mpBgBitmap;			 //默认背景
	HCURSOR mhPreviousCursor;		//在鼠标进入自己前的状态
	HCURSOR mhInnerCursor;			//本元素内用的Cursor

	// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
	CXuiElement()
	{
		mpTemplete = NULL;
		mpIterator = NULL;
		mpBgBitmap = NULL;
		mhPreviousCursor = NULL;
		mhInnerCursor = NULL;
	}

	// 用于释放成员对象
	~CXuiElement()
	{
		//CMMASSERT(mpTemplete!=NULL);
		//CMMASSERT(mpBgBitmap!=NULL);
	}

	
	// 设置位标志；可以 0 - 28
	bool SetFlags(
		int niIndex,		// 标志的序号，从0开始；如果派生类重载这个函数，并且该派生类有2个不希望被后续类和用户修改的标志，那么它的函数调用时的niIndex=0表示的是它的基类的2
		bool nbSet		// 设置或者清除标志
		) {
		return cmmBaseObject::SetFlags(niIndex+2,nbSet);
	}

	// 获取标志
	bool TestFlag(int niIndex){
		return cmmBaseObject::TestFlag(niIndex+2);
	}

};

#define TF_ID_ELEMENT_BACKGROUND L"BackGround"			//默认背景图
#define TF_ID_ELEMENT_X L"X"							//X坐标
#define TF_ID_ELEMENT_Y L"Y"							//Y坐标
#define TF_ID_ELEMENT_WIDTH L"Width"					//参考宽
#define TF_ID_ELEMENT_HEIGHT L"Height"					//参考高
#define TF_ID_ELEMENT_EXTEND_X L"BackGround/ExtendX"				//横向延展线
#define TF_ID_ELEMENT_EXTEND_Y L"BackGround/ExtendY"				//纵向延展线



//////////////////////////////////////////////////////////////////////////
// 将迭代器转换为元素类，!!!注意!!!只能转换为本模块内实现的元素类(本模块即同一个DLL)，绝对不能去试图直接访问其他模块实现的元素类的方法，对其他元素类的访问一律使用消息模型。
template <typename Type>
__inline 
void EsIterator2Element(IEinkuiIterator* npIterator, const wchar_t* nswEType,Type& rElementPointer){

	IXsElement* lpElement;
	CMMASSERT(nswEType != NULL && nswEType[0]!=UNICODE_NULL);
	if(npIterator != NULL && (lpElement = npIterator->GetElementObject())!=NULL && lpElement->IsKindOf(nswEType)!=false)
		rElementPointer = dynamic_cast<Type>(lpElement);
	else
		rElementPointer = NULL;
}



//////////////////////////////////////////////////////////////////////////
// 下面是Eelement模板的函数实现

template<class CClassDeriveTo,const wchar_t* Name>
const wchar_t* CXuiElement<CClassDeriveTo,Name>::GetType(void)
{
	return GetObjectName();
}

// 在全局范围内验证此对象是否是nswType指定的类型
template<class CClassDeriveTo,const wchar_t* Name>
bool CXuiElement<CClassDeriveTo,Name>::GlobleVerification(const wchar_t* nswType)
{
	return (_wcsicmp(nswType,GetObjectName())==0);
}

// 默认消息入口函数，用于接收输入消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT __stdcall CXuiElement<CClassDeriveTo,Name>::MessageReceiver(IEinkuiMessage* npMsg)
{
	ERESULT luResult=ERESULT_UNSUCCESSFUL;

	try
	{
		// 调用消息预处理
		luResult = BeforeParseMessage(npMsg);
		if(luResult != ERESULT_UNEXPECTED_MESSAGE)
			THROW_FALSE;	// 中止消息分解过程

		luResult = ParseMessage(npMsg);

		// 设置消息返回值
		npMsg->SetResult(luResult);

		// 调用后处理过程
		AfterParseMessage(npMsg);
	}
	catch (...)
	{
		luResult = ERESULT_UNSUCCESSFUL;
		// 设置消息返回值
		npMsg->SetResult(luResult);
	}

	return luResult;
}

// 派生本类及派生本函数时，请特别注意!!! 一定要首先调用基类的方法
// 本函数仅用于建立子元素对象、初始化自身数据（位置、大小、背景图等）
template<class CClassDeriveTo,const wchar_t* Name>
ULONG CXuiElement<CClassDeriveTo,Name>::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(nuEID == MAXULONG32 && npTemplete != NULL)
			nuEID = npTemplete->GetID();

		mpTemplete = npTemplete;

		// !!!必须!!! 首先向系统注册自己
		mpIterator = EinkuiGetSystem()->GetElementManager()->RegisterElement(npParent,this,nuEID);	//ID为0表示请求自动分配
		if(mpIterator == NULL)
			break;

		//设置自己的风格
		mpIterator->SetStyles(EITR_STYLE_MOUSE);

		ICfKey* lpBgBitmapKey = NULL;
		LONG llWidth = 0;
		LONG llHeight = 0;
		do 
		{
			BREAK_ON_NULL(npTemplete);

			mpTemplete->AddRefer();

			//这里面的设置，就算不成功也不影响元素的创建
			//设置元素坐标
			LONG llPosX = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_X,MAXUINT32);
			LONG llPosY = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_Y,MAXUINT32);
			if(llPosX != MAXUINT32 && llPosY != MAXUINT32)
				mpIterator->SetPosition((FLOAT)llPosX,(FLOAT)llPosY);

			//读取参考尺寸设置
			llWidth = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_WIDTH,MAXUINT32);
			llHeight = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_HEIGHT,MAXUINT32);
			if (llWidth != MAXUINT32 && llHeight != MAXUINT32 )
				mpIterator->SetSize((FLOAT)llWidth,(FLOAT)llHeight);
			
			//装入默认背景
			wchar_t lszPath[MAX_PATH] = {0};
			lpBgBitmapKey = mpTemplete->GetSubKey(TF_ID_ELEMENT_BACKGROUND);
			BREAK_ON_NULL(lpBgBitmapKey);
			if(lpBgBitmapKey->GetValue(lszPath,MAX_PATH*sizeof(wchar_t)) <= 0)	//读取图片名称
				break;

			mpBgBitmap = EinkuiGetSystem()->GetAllocator()->LoadImageFile(lszPath,lszPath[0] == L'.'?false:true);
			BREAK_ON_NULL(mpBgBitmap);

			//设置延展线,0表示未设置，-1表示使用中线作为延展线
			mpBgBitmap->SetExtendLine(mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_EXTEND_X,0),mpTemplete->QuerySubKeyValueAsLONG(TF_ID_ELEMENT_EXTEND_Y,0));

			// 如果模板中没有指定参考尺寸，那么从背景图中提取参考尺寸
			if (llWidth == MAXUINT32 || llHeight == MAXUINT32 )
			{
				//如果没有设置参考尺寸，就以图片大小为参考尺寸
				llWidth = (LONG)mpBgBitmap->GetWidth();
				llHeight = (LONG)mpBgBitmap->GetHeight();

				//设置参考尺寸
				if(llWidth != 0 && llHeight != 0)
					mpIterator->SetSize((FLOAT)llWidth,(FLOAT)llHeight);
			}
		} while (false);

		CMM_SAFE_RELEASE(lpBgBitmapKey);

		lResult = LoadSubElement();	//创建其它子元素

	}while(false);


	//创建所有子元素
	return lResult;
}

// 获得本元素的迭代器接口
template<class CClassDeriveTo,const wchar_t* Name>
IEinkuiIterator* __stdcall CXuiElement<CClassDeriveTo,Name>::GetIterator(void)
{
	return mpIterator;
}

// 获得Tab Order, -1 未设置
template<class CClassDeriveTo,const wchar_t* Name>
LONG __stdcall CXuiElement<CClassDeriveTo,Name>::GetTabOrder(void)
{
	LONG llOrder;
	if(mpTemplete != NULL)
		llOrder = mpTemplete->QuerySubKeyValueAsLONG(L"TOrder",-1);
	else
		llOrder = -1;

	return llOrder;
}

// 获得Z Order，-1 未设置
template<class CClassDeriveTo,const wchar_t* Name>
LONG __stdcall CXuiElement<CClassDeriveTo,Name>::GetZOrder(void)
{
	LONG llOrder;
	if(mpTemplete != NULL)
		llOrder = mpTemplete->QuerySubKeyValueAsLONG(L"ZOrder",-1);
	else
		llOrder = -1;
	return llOrder;
}



//////////////////////////////////////////////////////////////////////////
// 下面三个函数是消息处理的三个阶段，他们是由本类的CXuiElement::MessageReceiver调用的，一旦修改了默认的消息接受函数，请自行处理消息处理过程
//////////////////////////////////////////////////////////////////////////

// 分解消息前步骤，提供一个在消息分解前获得消息的机会，仅用于修改消息内容的目的；这个函数的规则是，一定要首先调用基类的BeforeParseMessage，从而确保基类保留对消息的优先权；
// 通常情况下，不许要重载本函数；本函数的设定目的是为了确保某种消息的处理行为能够延续到派生类中，而不被派生类修改；
// 返回ERESULT_UNEXPECTED_MESSAGE将继续消息分解过程，其他值都会使得消息分解过程中止
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::BeforeParseMessage(IEinkuiMessage* npMsg)
{
	// 下面的是示例，如果要使用请复制，并去掉前后的失效宏语句

#if 0

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	// 首先调用基类的实现；注意：一定要调用自身直接基类
	luResult = CXuiElement::BeforeParseMessage(npMsg);
	if(luResult != ERESULT_UNEXPECTED_MESSAGE)
		return luResult;

	switch (npMsg->GetMessageID())
	{
	case EMSG_TEST:	// 感兴趣的消息
		//可以修改消息参数
		//可以产生衍生消息
		//luResult = ERESULT_SUCCESS;	有意中止消息分解
	default:
		luResult = ERESULT_UNEXPECTED_MESSAGE;
	}

	// 如果希望中止消息分解过程，则返回除ERESULT_UNEXPECTED_MESSAGE以外的值
	return luResult;
#else
	return ERESULT_UNEXPECTED_MESSAGE;
#endif
}

// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
//ERESULT CXuiElement::ParseMessage(IEinkuiMessage* npMsg)
//{
//	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类
//
//	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;
//
//	switch (npMsg->GetMessageID())
//	{
//	case EMSG_TEST:	// 感兴趣的消息
//		{
//			if(npMsg->GetInputDataSize() != sizeof(bool) || npMsg->GetOutputBufferSize() != sizeof(bool))
//			{
//				luResult = ERESULT_WRONG_PARAMETERS;
//				break;
//			}
//			// 获取输入数据
//			bool* lpValue = (bool*)npMsg->GetInputData();
//
//			// 设置输出数据
//			bool* lpOut = (bool*)npMsg->GetOutputBuffer();
//
//			*lpOut = !(*lpValue);
//
//			npMsg->SetOutputDataSize(sizeof(bool));
//
//			luResult = ERESULT_SUCCESS;// 返回值，IEinkuiMessage对象的返回值由本函数的调用者依据本函数的结果设置
//		}
//	default:
//		luResult = ERESULT_NOT_SET;
//	}
//
//	if(luResult == ERESULT_NOT_SET)
//	{
//		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
//	}
//
//	return luResult;
//}


// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_TEST:	// 感兴趣的消息
		{
			if(npMsg->GetInputDataSize() != sizeof(bool) || npMsg->GetOutputBufferSize() != sizeof(bool))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}
			// 获取输入数据
			bool* lpValue = (bool*)npMsg->GetInputData();

			// 设置输出数据
			bool* lpOut = (bool*)npMsg->GetOutputBuffer();

			*lpOut = !(*lpValue);

			npMsg->SetOutputDataSize(sizeof(bool));

			luResult = ERESULT_SUCCESS;// 返回值，IEinkuiMessage对象的返回值由本函数的调用者依据本函数的结果设置

			break;
		}
	case EMSG_CREATE: 
		{
			//初始建立
			luResult = OnElementCreate(*(IEinkuiIterator**)npMsg->GetInputData());
			break;
		}
	case EMSG_DESTROY: 
		{
			//元素销毁
			luResult = OnElementDestroy();
			break;
		}
	case EMSG_MOUSE_OWNER_TEST:
		{
			D2D1_POINT_2F ldPoint;
			if(npMsg->GetInputDataSize() != sizeof(ldPoint))
				return ERESULT_WRONG_PARAMETERS;

			ldPoint = *(D2D1_POINT_2F*)npMsg->GetInputData();
			luResult = OnMouseOwnerTest(ldPoint);
		}
		break;
	case EEVT_BUTTON_CLICK: 
		{
			//按钮单击
			luResult = OnCtlButtonClick(npMsg->GetMessageSender());

			break;
		}
	case EMSG_ELEMENT_MOVED: 
		{
			//元素位置被改变,已经改变完成
			if(npMsg->GetInputDataSize() != sizeof(D2D1_POINT_2F))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			D2D1_POINT_2F* lpValue = (D2D1_POINT_2F*)npMsg->GetInputData();

			luResult = OnElementMoved(*lpValue);

			break;
		}
	case EMSG_ELEMENT_RESIZED:
		{
			//元素参考尺寸发生变化
			if(npMsg->GetInputDataSize() != sizeof(D2D1_SIZE_F))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			D2D1_SIZE_F* lpValue = (D2D1_SIZE_F*)npMsg->GetInputData();

			luResult = OnElementResized(*lpValue);

			break;
		}
	case EMSG_SHOW_HIDE: 
		{
			//显示或隐藏
			if(npMsg->GetInputDataSize() != sizeof(bool))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			bool* lpValue = (bool*)npMsg->GetInputData();

			luResult = OnElementShow(*lpValue);

			break;
		}
	case EMSG_ENALBE_DISABLE: 
		{
			//启用或禁用
			if(npMsg->GetInputDataSize() != sizeof(bool))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			bool* lpValue = (bool*)npMsg->GetInputData();

			luResult = OnElementEnable(*lpValue);

			break;
		}
	case EMSG_TIMER: 
		{
			//定时器
			if(npMsg->GetInputDataSize() != sizeof(STEMS_TIMER))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			OnTimer((STEMS_TIMER*)npMsg->GetInputData());

			luResult = ERESULT_SUCCESS;
			break;
		}
	case EMSG_LAZY_UPATE:
		{
			if(npMsg->GetInputDataSize() != sizeof(STEMG_LAZY_UPDATE))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			OnLazyUpdate((STEMG_LAZY_UPDATE*)npMsg->GetInputData());

			luResult = ERESULT_SUCCESS;
			break;
		}
	case EMSG_HOOKED_MESSAGE: 
		{
			//用于转发被Hook的消息
			if(npMsg->GetInputDataSize() != sizeof(STEMS_HOOKED_MSG))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			STEMS_HOOKED_MSG* lpValue = (STEMS_HOOKED_MSG*)npMsg->GetInputData();

			luResult = OnHookMessage(lpValue->OriginalElement,lpValue->OriginalMessage);
			break;
		}
	case EMSG_RESPONSE_MESSAGE: 
		{
			//反馈消息
			if(npMsg->GetInputDataSize() != sizeof(STEMS_RESPONSE_MSG))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			STEMS_RESPONSE_MSG* lpValue = (STEMS_RESPONSE_MSG*)npMsg->GetInputData();

			luResult = OnResponseMessage(lpValue->OriginalElement,lpValue->OriginalMessage,lpValue->Context);
			break;
		}
	case EMSG_MOUSE_BUTTON: 
		{
			//鼠标按下
			if(npMsg->GetInputDataSize() != sizeof(STEMS_MOUSE_BUTTON))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const STEMS_MOUSE_BUTTON* lpValue = (const STEMS_MOUSE_BUTTON*)npMsg->GetInputData();

			luResult = OnMousePressed(lpValue);

			break;
		}
	case EMSG_MOUSE_DBCLICK: 
		{
			//鼠标按下
			if(npMsg->GetInputDataSize() != sizeof(STEMS_MOUSE_BUTTON))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const STEMS_MOUSE_BUTTON* lpValue = (const STEMS_MOUSE_BUTTON*)npMsg->GetInputData();

			luResult = OnMouseDbClick(lpValue);

			break;
		}
	case EMSG_MOUSE_MOVING: 
		{
			//鼠标移动
			if(npMsg->GetInputDataSize() != sizeof(STEMS_MOUSE_MOVING))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const STEMS_MOUSE_MOVING* lpValue = (const STEMS_MOUSE_MOVING*)npMsg->GetInputData();

			luResult = OnMouseMoving(lpValue);
			break;
		}
	case EMSG_MOUSE_FOCUS:
		{
			//鼠标进入或离开
			if(npMsg->GetInputDataSize()!=sizeof(STEMS_STATE_CHANGE))
				break;
			OnMouseFocus((STEMS_STATE_CHANGE*)npMsg->GetInputData());
			luResult = ERESULT_SUCCESS;
			break;
		}
	case EMSG_MOUSE_HOVER: 
		{
			//鼠标悬停
			luResult = OnMouseHover();
			break;
		}
	case EMSG_MOUSE_WHEEL: 
		{
			//鼠标滑轮
			if(npMsg->GetInputDataSize() != sizeof(STEMS_MOUSE_WHEEL))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const STEMS_MOUSE_WHEEL* lpValue = (const STEMS_MOUSE_WHEEL*)npMsg->GetInputData();

			luResult = OnMouseWheel(lpValue);

			break;
		}
	case EMSG_KEYBOARD_FOCUS:
		{
			//键盘焦点获得或者失去
			if(npMsg->GetInputDataSize()!=sizeof(STEMS_STATE_CHANGE))
				break;
			OnKeyBoardFocus((STEMS_STATE_CHANGE*)npMsg->GetInputData());

			luResult = ERESULT_SUCCESS;
			break;
		}
	case EMSG_KEY_PRESSED: 
		{
			//键盘消息
			if(npMsg->GetInputDataSize() != sizeof(STEMS_KEY_PRESSED))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			const STEMS_KEY_PRESSED* lpValue = (const STEMS_KEY_PRESSED*)npMsg->GetInputData();

			luResult = OnKeyPressed(lpValue);
			break;
		}
	case EMSG_HOTKEY_PRESSED:
		{
			//快捷键消息
			const STEMS_HOTKEY* lpHotKey;
			luResult = CExMessage::GetInputDataBuffer(npMsg,lpHotKey);
			if(luResult != ERESULT_SUCCESS)
				break;
			luResult = OnHotKey(lpHotKey);
			break;
		}
	case EMSG_PAINT:	
		{
			//绘制消息
			if(npMsg->GetInputDataSize() != sizeof(IEinkuiPaintBoard*))
			{
				luResult = ERESULT_WRONG_PARAMETERS;
				break;
			}

			IEinkuiPaintBoard* lpValue = *(IEinkuiPaintBoard**)npMsg->GetInputData();

			luResult = OnPaint(lpValue);
			break;
		}
	case EMSG_DRAGGING_ELEMENT:
		if(npMsg->GetInputDataSize() != sizeof(STMS_DRAGGING_ELE))	{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		luResult = OnDragging((const STMS_DRAGGING_ELE*)npMsg->GetInputData());
		break;
	case EMSG_DRAG_BEGIN: 
		if(npMsg->GetInputDataSize() != sizeof(STMS_DRAGGING_ELE))	{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		luResult = OnDragBegin((const STMS_DRAGGING_ELE*)npMsg->GetInputData());
		break;
	case EMSG_DRAG_END: 
		if(npMsg->GetInputDataSize() != sizeof(STMS_DRAGGING_ELE))	{
			luResult = ERESULT_WRONG_PARAMETERS;
			break;
		}

		luResult = OnDragEnd((const STMS_DRAGGING_ELE*)npMsg->GetInputData());
		break;
	case EMSG_COMMAND:
		{
			nes_command::ESCOMMAND leCmd;
			luResult = CExMessage::GetInputData(npMsg,leCmd);
			if(luResult != ERESULT_SUCCESS)
				break;

			luResult = OnCommand(leCmd);
		}
		break;
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if(luResult == ERESULT_NOT_SET)
	{
		// luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		luResult = ERESULT_UNEXPECTED_MESSAGE;	// 这儿没有基类，派生本类时，删除本句；
	}

	return luResult;
}

// 分解消息后步骤，提供一个在消息处理完毕后执行的机会，通常用于监控消息的结果或则产生后续行为；这个函数的调用规则是，一定要最后调用基类的实现，从而确保基类最后执行
// 通常情况下，不许要重载本函数；本函数的设定目的是为了确保某种消息的处理行为能够延续到派生类中，在派生类之后执行；
// 如无特定目的，不应该修改函数的消息处理的返回值，该返回值存放在npMsg指向的消息对象中;本函数不设返回值
template<class CClassDeriveTo,const wchar_t* Name>
void CXuiElement<CClassDeriveTo,Name>::AfterParseMessage(IEinkuiMessage* npMsg)
{
	// 执行某些操作

	// 最后，调用基类的同名函数
	// CXuiElement::AfterParseMessage(npMsg);	// 注意：一定要调用自身直接基类
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_SUCCESS;
	wchar_t lswTip[128];
	ICfKey* lpTipKey;

	if(mpTemplete != NULL && (lpTipKey = mpTemplete->GetSubKey(L"ToolTip"))!=NULL)
	{
		if(lpTipKey->GetValueLength()>0)
		{

			lpTipKey->GetValue(lswTip,sizeof(wchar_t)*128);

			mpIterator->SetToolTip(lswTip);
		}

		lpTipKey->Release();
	}

	return lResult;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CMM_SAFE_RELEASE(mpTemplete);
		//CMM_SAFE_RELEASE(mpIterator);
		CMM_SAFE_RELEASE(mpBgBitmap);

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//按钮单击事件
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//元素位置被改变,已经改变完成
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementMoved(D2D1_POINT_2F nNewPoint)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//元素参考尺寸发生变化
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementResized(D2D1_SIZE_F nNewSize)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//元素显示或隐藏
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementShow(bool nbIsShow)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//启用或禁用
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnElementEnable(bool nbIsShow)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//定时器
template<class CClassDeriveTo,const wchar_t* Name>
void CXuiElement<CClassDeriveTo,Name>::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
}

// 慢刷新
template<class CClassDeriveTo,const wchar_t* Name>
void  CXuiElement<CClassDeriveTo,Name>::OnLazyUpdate(
	PSTEMG_LAZY_UPDATE npLazyUpdate
	)
{
}


//用于转发被Hook的消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnHookMessage(IEinkuiIterator* npOriginalElement,	// 原始的消息接受元素
	IEinkuiMessage* npOriginalMessage		// 被捕获的消息
	)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{

		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//反馈消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnResponseMessage(IEinkuiIterator* npOriginalElement,	// 原始消息的接受元素
	IEinkuiMessage* npOriginalMessage,	// 被反馈的消息
	void* npContext					// 设置消息反馈时，指定的上下文
	)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//鼠标按下
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMousePressed(const STEMS_MOUSE_BUTTON* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//鼠标双击
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMouseDbClick(const STEMS_MOUSE_BUTTON* npInfo)
{
	return ERESULT_SUCCESS;
}

//鼠标移动
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMouseMoving(const STEMS_MOUSE_MOVING* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//鼠标进入/离开
template<class CClassDeriveTo,const wchar_t* Name>
void CXuiElement<CClassDeriveTo,Name>::OnMouseFocus(PSTEMS_STATE_CHANGE npState)
{
	if(mhInnerCursor == NULL)
		return;

	if (npState->State != 0)
	{
		//鼠标进入
		mhPreviousCursor = CExWinPromptBox::SetCursor(mhInnerCursor);
	}
	else
	if(mhPreviousCursor != NULL && mhPreviousCursor != LoadCursor(NULL,IDC_WAIT)) //不允许出现忙碌标志
	{
		//鼠标移出
		CExWinPromptBox::SetCursor(mhPreviousCursor);
		mhPreviousCursor = NULL;
	}
}

//键盘焦点获得或者失去
template<class CClassDeriveTo,const wchar_t* Name>
void CXuiElement<CClassDeriveTo,Name>::OnKeyBoardFocus(PSTEMS_STATE_CHANGE npState)
{
}

//鼠标悬停
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMouseHover()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//鼠标滑轮
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMouseWheel(const STEMS_MOUSE_WHEEL* npInfo)
{
	return ERESULT_UNEXPECTED_MESSAGE;
}

//键盘消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnKeyPressed(const STEMS_KEY_PRESSED* npInfo)
{
	return ERESULT_KEY_UNEXPECTED;
}

//快捷键消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnHotKey(const STEMS_HOTKEY* npHotKey)
{
	return ERESULT_KEY_UNEXPECTED;
}


//绘制消息
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if(mpBgBitmap != NULL)
			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,(FLOAT)mpBgBitmap->GetWidth(),(FLOAT)mpBgBitmap->GetHeight()),
			mpBgBitmap,
			ESPB_DRAWBMP_LINEAR);

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//元素拖拽
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnDragging(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	return lResult;
}

//拖拽开始,nulActKey哪个鼠标按钮按下进行拖拽
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnDragBegin(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	return lResult;
}

//拖拽结束,nulActKey哪个鼠标按钮按下进行拖拽
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnDragEnd(const STMS_DRAGGING_ELE* npInfo)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	return lResult;
}

//命令
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnCommand(const nes_command::ESCOMMAND neCmd)
{
	return ERESULT_UNSUCCESSFUL;
}

//创建自己所有的子元素
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::LoadSubElement()
{
	ERESULT leResult = ERESULT_SUCCESS;	

	ICfKey* lpICfKey = NULL;

	do 
	{
		BREAK_ON_NULL(mpTemplete);

		lpICfKey = mpTemplete->OpenKey(L"Children"); //打开子元素键
		BREAK_ON_NULL(lpICfKey);

		lpICfKey = lpICfKey->MoveToSubKey();	
		while (lpICfKey != NULL)	//循环创建所有子元素
		{
			EinkuiGetSystem()->GetAllocator()->CreateElement(mpIterator,lpICfKey);
			lpICfKey = lpICfKey->MoveToNextKey();	//打开下一个子元素键
		}

	} while (false);

	return leResult;
}

// 鼠标落点检测
template<class CClassDeriveTo,const wchar_t* Name>
ERESULT CXuiElement<CClassDeriveTo,Name>::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	if(mpBgBitmap != NULL && !(rPoint.x < 0.0f || (ULONG)rPoint.x >= mpBgBitmap->GetWidth() || rPoint.y < 0.0f || (ULONG)rPoint.y >= mpBgBitmap->GetHeight()))
	{
		DWORD luPixel;
		if(ERESULT_SUCCEEDED(mpBgBitmap->GetPixel((ULONG)CExFloat::Round(rPoint.x),(ULONG)CExFloat::Round(rPoint.y),luPixel)))
		{
			if(luPixel == 1)
				luResult = ERESULT_MOUSE_OWNERSHIP;
		}
	}

	return luResult;
}

////给自己父窗口发送简单消息
//template<class CClassDeriveTo,const wchar_t* Name>
//ERESULT __stdcall CXuiElement<CClassDeriveTo,Name>::PostMessageToParent(
//	IN ULONG nuMsgID,		// 消息编码
//	IN void* npInputBuffer,	// 输入数据的缓冲区
//	IN int niInputSize		// 输入数据的大小
//	)
//{
//	ERESULT luResult = ERESULT_UNSUCCESSFUL;
//	IEinkuiMessage* lpMsgIntf = NULL;
//
//	do 
//	{
//		//构建消息结构体
//		lpMsgIntf = EinkuiGetSystem()->GetElementManager()->AllocateMessage(nuMsgID,npInputBuffer,niInputSize,NULL,0);
//		BREAK_ON_NULL(lpMsgIntf);
//
//		mpIterator->PostMessageToParent(lpMsgIntf);
//
//	} while (false);
//
//	CMM_SAFE_RELEASE(lpMsgIntf);
//
//	return luResult;
//}

// Post模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Post发送消息需要注意这个问题，防止地址到消息接收者出无法访问。
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;
template<class CClassDeriveTo,const wchar_t* Name>
template<typename DataType>
ERESULT CXuiElement<CClassDeriveTo,Name>::PostMessageToParent(ULONG nuMsgID,const DataType& rDataToPost)
{
	return CExMessage::PostMessage(mpIterator->GetParent(),mpIterator,nuMsgID,rDataToPost,EMSG_POSTTYPE_FAST);
}

// Send模式发送消息，并且直接发送携带的参数，注意，如果参数是一个指针，则只会复制传递指针（一个地址）本身，而不会复制传递指针指向的内容，Send模式发送消息时，这不会导致特殊问题
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
template<class CClassDeriveTo,const wchar_t* Name>
template<typename DataType>
ERESULT CXuiElement<CClassDeriveTo,Name>::SendMessageToParent(ULONG nuMsgID,const DataType& rDateForSend,void* npBufferForReceive,int niBytesOfBuffer)
{
	return CExMessage::SendMessage(mpIterator->GetParent(),mpIterator,nuMsgID,rDateForSend,npBufferForReceive,niBytesOfBuffer);
}































#endif//_ELEMENTIMP_H_
