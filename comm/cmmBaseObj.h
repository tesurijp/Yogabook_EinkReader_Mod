/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _CMMBASEOBJ_H_
#define _CMMBASEOBJ_H_
/*
	FileName: cmmBaseObject.h
	Author: Ax
	Purpose: 定义和实现相对完善的C++对象约束和管理，特别是对象的生命期控制；
	More words; 本文件中涉及较多的模板运用例子和C语言宏的特殊用法，并带有翔实注释，便于作者本人将来运用和复习，也是其他初学者很好的参考材料。
	Revision History:
		Ax Dec.01,2010 Created
		Ax Mar.16,2011 Corrected the allocation and freeing of objects Derived from 'cmmBaseObject'
*/

// 基础接口
__interface IBaseObject{
public:
	// 增加引用，返回增加后的引用数
	virtual int __stdcall AddRefer(void)=NULL;
	// 释放对象，返回释放后的引用数
	virtual int __stdcall Release(void)=NULL;
	// 获得对象名称
	virtual const wchar_t* __stdcall GetObjectName(void)=NULL;
	// 动态类型确认，nszBuiltInName不能直接传入字符串，必须使用GET_BUILTIN_NAME(XXX)宏书写，其中的XXX为最终实例化的类型名
	virtual bool IsKindOf(const wchar_t* nszBuiltInName)=NULL;
};


// 基础接口实现辅助模板；注意这个模板中的参数CInterfaceDerivedFrom应该是一个派生之IBaseObject的虚基类或者其他的不由cmmBaseObject模板派生的类；
// 如果需要派生一个本身又cmmBaseObject派生的类，不能重复使用本模板，考虑到可能需要支持类型识别，请使用下面的宏DEFINE_DERIVED_TYPECAST，搜索这个宏下文有详细的介绍
// 更多信息，参考示例工程
template<class CClassDeriveTo,class CInterfaceDerivedFrom,const wchar_t* Name>
class cmmBaseObject:public CInterfaceDerivedFrom{
protected:
	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(void){
		return 0;	// 本处是最底层的基类，无实际操作
	}

public:
	// 构造函数仅用于给内部变量赋予初值，对于更多的进一步操作，如：资源申请，启动过程等都应该通过重载InitOnCreate函数实现
	// 不要使用其他的带参数的构造函数
	cmmBaseObject(){
		miReferenceCount=1;
		muFlags = 0;
	}
	virtual ~cmmBaseObject(){}

	void Reuse(){
		miReferenceCount = 1;
		muFlags = (muFlags&1);
	}


	/*/ 这段注释的代码，实现了分解普通new 创建对象的过程，即分配内存和显示调用构造函数，还涉及了placement new的调用；
	// 经过测试是正确的，注释掉的原因是由更简单的写法，即下面没有注释的调用
	//// 创建对象，具体用法参考本文件头部的说明；也可以实现需要参数的创建工具函数，使用下面的宏就可以做到
	static CClassDeriveTo* CreateInstance(void){
		// 下面的语句是调用C++的默认的operator new，仅仅分配内存，注意，C++默认的operator new与new operator的区别，可以参考C++ primer 以及More effective C++的相关介绍
		// 简单的说就是 operator new 仅仅分配内存而不调用构造函数，而new operator（也是我们普通写法 new ClassA(xx);)内部会去调用前面的‘operator new’而后调用对应的构造函数；
		// 除了上面的两种new外，C++还有一种叫做placement new，这个函数不是分配函数，它默认是完成对一个给定内存的初始化，也就是调用对应类的构造函数，当然，也可以改写它而实现分配
		// 在本模板类中就重载了这个plancement new见函数void* __cdecl operator new(size_t nSize,void* npObj)，但我们直接返回了出入的npObj对象，这是符合C++规则的实现
		CClassDeriveTo* lpObj = (CClassDeriveTo*)operator new(sizeof(CClassDeriveTo),false);
		if(lpObj != NULL)
		{
			// 调用placement new从而触发构造函数，在Release building的情况下，并不会去调用我们实现的内联的void* __cdecl operator new(size_t nSize,void* npObj)，而直接调用构造函数
			// 或者执行内联的构造函数代码
			new (lpObj)CClassDeriveTo();
			// 设置标志表示这个对象的内存需要被释放
			lpObj->SetDeleteRequired();
		}
		// 返回构造完毕的对象
		return lpObj;
	}
	// 仅为了配合placement new而提供的默认构造函数，Release Building中无任何存在
	void* __cdecl operator new(size_t nSize,void* npObj){
		return npObj;
	}
	// 防止编译器出现警告，实现与上一个函数配对的析构函数，并不会调用它
	void __cdecl operator delete(void* nObj,void* npObj){
	}*/

	// 创建对象，具体用法参考本文件头部的说明；也可以实现需要参数的建造工具函数，使用下面的宏DEFINE_CUMSTOMIZE_CREATE就可以做到
	// 这儿实现的代码与使用DEFINE_CUMSTOMIZE_CREATE(CClassDeriveTo,(),())是一样的，如果修改了这个地方的代码!!!请务必!!!对DEFINE_CUMSTOMIZE_CREATE做等价修改
	static CClassDeriveTo* CreateInstance(){
		// 调用C++默认的new operator而不是operator new操作，它首先导致我们重载的void* __cdecl operator new(size_t nSize,bool nbDummy)被调用，而后会去调用默认的构造函数
		CClassDeriveTo* lpObj = new (false)CClassDeriveTo;
		if(lpObj != NULL)
		{
			// 设置标志表示这个对象的内存需要被释放
			lpObj->SetDeleteRequired();
			// 调用初始化函数，检查返回值，如果失败，释放掉该对象
			if((lpObj->InitOnCreate()&0x80000000)!=0)
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
	// 本模板的第一次派生的类可以不调用本模板实例化的CompleteCreation函数，因为它没有意义
	void CompleteCreation(void){
	}

	// 下面重载new和delete的目的是禁用默认的new和delete方法，使用本模板类实现的派生类，在动态分配时，必须使用成员方法CreateInstance;
	// 本模板类的客户代码!!!绝对不要!!!使用C++的 new来创建对象，虽然可以通过‘new (false)ClassA’创建对象，但这样会导致Release方法遗漏内存释放；
	// 如果某个派生类需要自定义内存分配的方法，请重载下面的两个函数
	void* __cdecl operator new(size_t nSize,bool nbDummy){
		return ::operator new(nSize);
	}
	// 实现与上一个函数配对的删除函数，但不调用它
	void __cdecl operator delete(void* npObj,bool nbDummy){
		// 直接调用C++默认析够函数
		::operator delete(npObj);
	}
	// 提供默认删除函数
	void __cdecl operator delete(void* npObj){
		// 直接调用C++默认析够函数
		::operator delete(npObj);
	}

	// 增加引用；如果不是动态创建的对象（即嵌入别的对象中的子对象，或者程序栈上的局部变量对象），不能狗通过AddRef改变生命期，不推荐将它传递到作用域之外的区域去，因为它的生命期仅由宿主的生命期决定；
	virtual int __stdcall AddRefer(void){
		return InterlockedIncrement(&miReferenceCount);
	}

	// 释放对象
	virtual int __stdcall Release(void){
		int liReval = InterlockedDecrement(&miReferenceCount);

		if(liReval == 0 && (muFlags&1)!=0)
		{
			delete this;
			// 下面的代码可以分两步执行对象释放
			////// 调用析构函数
			//((CClassDeriveTo*)this)->~CClassDeriveTo();
			////// 系统默认内存释放函数，将不再调用析够函数了
			//operator delete(this,false);
		}
		return liReval;
	}

	// 获得对象名称
	virtual const wchar_t* __stdcall GetObjectName(void){
		return Name;
	}

	// 动态类型确认，nszBuiltInName不能直接传入字符串，必须使用GET_BUILTIN_NAME(XXX)宏书写，其中的XXX为最终实例化的类型名
	virtual bool IsKindOf(const wchar_t* nszBuiltInName){
		return nszBuiltInName==Name;
	}

	// 设置为需要释放的内存类型
	void SetDeleteRequired(void){
		muFlags |= 1;
	}

	// 设置位标志，这个函数是提供给派生类或者外部调用的；派生类也可以重载这个函数以提供给自己的派生类和用户使用
 	bool SetFlags(
 		int niIndex,		// 标志的序号，从0开始；如果派生类重载这个函数，并且该派生类有2个不希望被后续类和用户修改的标志，那么它的函数调用时的niIndex=0表示的是它的基类的2
 		bool nbSet=true		// 设置或者清除标志
 		) {
		niIndex+=1;

		if(niIndex > 31)
			return false;

		if(nbSet != false)
			muFlags |= (1<<niIndex);
		else
			muFlags &= (~(1<<niIndex));

		return true;
		// 下面供派生类重载参考，假设该派生类需要2个位标志
		// return ::SetFlags(niIndex+2,nbSet);
	}

	// 获取标志
	bool TestFlag(int niIndex){
		niIndex+=1;
		if(niIndex > 31)
			return false;

		return (muFlags & (1<<niIndex))!=0;
		// 下面供派生类重载参考，假设该派生类需要2个位标志
		// return ::TestFlag(niIndex+2,nbSet);
	}

protected:
	LONG miReferenceCount;
private:
	ULONG muFlags;	// 最低位表示本对象是否为new分配，次低2位表示调用构造函数的时候，构造函数执行的执行结果

};

#define DECLARE_BUILTIN_NAME(_X) extern const wchar_t glszBN##_X[];
#define DEFINE_BUILTIN_NAME(_X) extern const wchar_t glszBN##_X[]=L#_X;
#define GET_BUILTIN_NAME(_X) glszBN##_X

// 下面的宏‘DEFINE_CUMSTOMIZE_CREATE’用于实现自定义的创建工具函数，用法如：DEFINE_CUMSTOMIZE_CREATE(CClassX,(int niA,void* npB),(niA,npB))；请参考相关例程
#define DEFINE_CUMSTOMIZE_CREATE(_X,_Y,_Z)	static _X* CreateInstance##_Y{_X* p = new  (false)##_X();if(p!=NULL)\
{p->SetDeleteRequired();if((p->InitOnCreate##_Z&0x80000000)!=0){p->Release();p=NULL;}else p->CompleteCreation();}return p;}
	// 接上，上面的宏用于实现一个static函数，C++的规则是派生类只要定义了与基类同名的非虚拟函数，基类的所有同名的虚拟函数都不会暴露给外部，与这儿的意义就是
	// 如果你用上面的宏实现了一个自己特定的CreateInstance(任意参数)，那么如果你派生类也希望允许用户使用原始的CreateInstance(void)，那，请在代码中加入DEFINE_CUMSTOMIZE_CREATE(派生类名,(bool nbTest),(nbTest))

	// 在我写上面的宏的前一天，我犯了个小错误，我错误地试图使用##_X为了将输入_X展开为原文，其实直接写_X就是展开原文，使用##的目的是同其他字符相互衔接时用来区分和展开的；
	// 因为使用##_X所以我遇到了问题，‘##_X’不能跟在空格后面，为了在空格后添加_X字符串，只能利用宏第二次展开的办法
	//#define __CMM_CC1(_X)
	//#define __CMM_CC2(_X) __CMM_CC1(_X) 
	//#define DEFINE_CUMSTOMIZE_CREATE(_X,_Y,_Z)	static __CMM_CC2(_X)##_X* CreateInstance##_Y{##_X* p = new  (false)##_X##_Z;if(p!=NULL)\
	//{p->SetDeleteRequired();if(p->GetConstructorResult()==1){p->Release();p=NULL;}}return p;}
	// 这是一个利用宏的逐次展开编写预处理代码的好思路，虽然是错误，但值得参考

// 重载类型识别宏，_X填写派生类名，_Y填写基类名，这个函数实现的原理是，当输入的名称不是我们自身时，我们需要询问基类是否是它的名字
// 更多帮助见下文
#define DEFINE_DERIVED_TYPECAST(_X,_Y) virtual bool IsKindOf(const wchar_t* s){if(s!=GET_BUILTIN_NAME(_X))return _Y::IsKindOf(s);return true;}virtual const wchar_t* __stdcall GetObjectName(void){return GET_BUILTIN_NAME(_X);}
// 展开的样子等价于
#if 0	// 下面这段是演示，不参与编译
virtual bool IsKindOf(const wchar_t* nszBuiltInName){
	if(nszBuiltInName!=Name)
		return CClassDerivedFrom::IsKindOf(nszBuiltInName);
	return true;
}

// 从基础接口实现类派生新类用方法
DECLARE_BUILTIN_NAME(CNewDerivedClass)
class CNewDerivedClass:public CExistBaseClass		// CExistBaseClass必须是一个用cmmBaseObject派生的基础类
{
public:
	//////////////////////////////////////////////////////////////////////////
	// 可实现构造函数
	//////////////////////////////////////////////////////////////////////////
	CNewDerivedClass(){}

	//////////////////////////////////////////////////////////////////////////
	// 可实现重载函数
	//////////////////////////////////////////////////////////////////////////
	~CNewDerivedClass(){}

	//////////////////////////////////////////////////////////////////////////
	// 可重载初始化函数
	//////////////////////////////////////////////////////////////////////////
	// 定义自己的初始化函数
	ULONG InitOnCreate(){
		// 要调用基类的初始化函数
		CExistBaseClass::InitOnCreate();
		// 在调用基类初始化后，再调用自己的其他初始化代码
	}
	// 也可以带有参数 
	ULONG InitOnCreate(int niParam){
		// 也可以在调用基类初始化前，先调用自己的某些初始化代码
		...
		// 要调用基类的初始化函数
		CExistBaseClass::InitOnCreate(niParam);
		// 基类初始化之后，再调用自己的剩下的初始化代码
		...
	}
	//////////////////////////////////////////////////////////////////////////
	// 一定要重载创建工具函数
	//////////////////////////////////////////////////////////////////////////
	// 可以是不带有调用参数的
	DEFINE_CUMSTOMIZE_CREATE(CNewDerivedClass,(),())
	// 也可以是带有调用参数的
	DEFINE_CUMSTOMIZE_CREATE(CNewDerivedClass,(int niParam),(niParam))

	//////////////////////////////////////////////////////////////////////////
	// 一定要加入这一行，这是重载类型识别
	DEFINE_DERIVED_TYPECAST(CNewDerivedClass,CExistBaseClass)

};

#endif//演示结束



// 简单互斥访问对象，用来控制对多线程共享数据的访问，内核态使用SpinLock，用户态使用CriticalSection
class CExclusiveAccess{
public:
	void Enter(){
		InterlockedIncrement(&count);
#if defined(KERNEL_CODE)
		KeAcquireSpinLock((PKSPIN_LOCK)&AccessLock,(PKIRQL)&SavedIrql);
#elif defined(NATIVE_CODE)
		ZwWaitForSingleObject(mhEvent,FALSE,NULL);
#else
		EnterCriticalSection((LPCRITICAL_SECTION)&CriticalSection);
		//if(mhMutex == NULL)
		//	RaiseException(0x888888,0,0,0); // 绝对不能继续执行，让程序崩溃

		//WaitForSingleObject(mhMutex,INFINITE);
#endif//KERNEL_CODE
	}
	void Leave(){
		if (InterlockedDecrement(&count) >= 0)
		{
#if defined(KERNEL_CODE)
		KeReleaseSpinLock((PKSPIN_LOCK)&AccessLock,SavedIrql);
#elif defined(NATIVE_CODE)
		ZwSetEvent(mhEvent,NULL);
#else
		LeaveCriticalSection((LPCRITICAL_SECTION)&CriticalSection);
		//if(mhMutex != NULL)
		//	ReleaseMutex(mhMutex);
#endif//KERNEL_CODE
		}
	}

	CExclusiveAccess(){
		count = 0;
#if defined(KERNEL_CODE)
		KeInitializeSpinLock(&AccessLock);
		SavedIrql = PASSIVE_LEVEL;
#elif defined(NATIVE_CODE)
		if(ZwCreateEvent(&mhEvent,SYNCHRONIZE|DELETE|EVENT_MODIFY_STATE,NULL,SynchronizationEvent,TRUE)!=STATUS_SUCCESS)
			mhEvent = NULL; // ???尚未测试

		#ifdef _DEBUG
		if(mhEvent == NULL)
			DbgBreakPoint();			
		#endif//_DEBUG
#else
		InitializeCriticalSection(&CriticalSection);
		//mhMutex = CreateMutex(NULL,FALSE,NULL);
		//Test1 = Test2 = 0x8866;
#endif//KERNEL_CODE
	}
	~CExclusiveAccess(){
#if defined(KERNEL_CODE)
#elif defined(NATIVE_CODE)
		if(mhEvent != NULL)
			ZwClose(mhEvent);
#else
		DeleteCriticalSection(&CriticalSection);
		//if(mhMutex != NULL)
		//	CloseHandle(mhMutex);
#endif
	}
protected:
	volatile LONG count;
#if defined(KERNEL_CODE)
	KSPIN_LOCK AccessLock;
	KIRQL SavedIrql;
#elif defined(NATIVE_CODE)
	HANDLE mhEvent;
#else
	CRITICAL_SECTION CriticalSection;
	//HANDLE mhMutex;
#endif//KERNEL_CODE
};


#ifdef FLOW_CONTROL_ASSISTANT
#error Micro redifinition
#else//FLOW_CONTROL_ASSISTANT
#define FLOW_CONTROL_ASSISTANT

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS STATUS_SUCCESS
#endif

#define BREAK_ON_NULL(_X) {if(_X==NULL) break;}
#define LEAVE_ON_NULL(_X) {if(_X==NULL) __leave;}
#define THROW_ON_NULL(_X) {if(_X==NULL) throw 1;}
#define THROW_ON_FALSE(_X) {if((_X)==false) throw 2;}
#define THROW_ON_FAILED(_X)	  {if((_X&0x80000000)!=0) throw 3;}
#define THROW_ON_INVALID(_X) {if(_X==INVALID_HANDLE_VALUE) throw 4;}
#define BREAK_NOT_SUCCESS(_X) {if((_X)!=ERROR_SUCCESS) break;}
#define LEAVE_NOT_SUCCESS(_X) {if((_X)!=ERROR_SUCCESS) __leave;}
#define BREAK_ON_FAILED(_X)	  {if((_X&0x80000000)!=0) break;}
#define LEAVE_ON_FAILED(_X)   {if((_X&0x80000000)!=0) __leave;}
#define BREAK_ON_FALSE(_X) {if((_X)==false) break;}
#define CMM_SAFE_RELEASE(_X) {if((_X)!=NULL){(_X)->Release();(_X)=NULL;}}
#define CMM_SAFE_DELETE(_X) {if((_X)!=NULL){delete (_X);(_X)=NULL;}}
#define CMM_SAFE_CLOSE_HANDLE(_X) {if((_X)!=NULL){CloseHandle((_X));(_X)=NULL;}}

#define RETURN_ON_FAILED(_X)  {if((_X&0x80000000)!=0) return (_X);}
#define RETURN_NOT_SUCCESS(_X) {if((_X)!=ERROR_SUCCESS) return (_X);}
#define RETURN_ON_NULL(_X,_Y) {if((_X)==NULL) return (_Y);}

#define THROW_NULL	(throw 1)
#define THROW_FALSE (throw 2)
#define THROW_OVERFLOW (throw 3)
#define THROW_INVALID (throw 4)
#define THROW_EMPTY	(throw 5)
#define THROW_PARAMTER (throw 6)
#define THROW_USER	(throw 7)
#define THROW_FAILED (throw 8)
#define THROW_WRONG_FORMAT (throw 9)
#define THROW_UNKNOWN (throw 100)
#endif//FLOW_CONTROL_ASSISTANT

#ifdef CMM_RELEASE
#error Micro redifinition
#else
#define CMM_RELEASE( x )  \
	if (NULL != x)      \
{                   \
	x->Release();   \
	x = NULL;       \
}
#endif//CMM_RELEASE

#ifndef SAFE_CLOSE_HANDLE
#undef SAFE_CLOSE_HANDLE
#define SAFE_CLOSE_HANDLE( x )  \
	if (NULL != x)      \
{                   \
	CloseHandle(x);   \
	x = NULL;       \
}
#endif

#ifndef CMMASSERT
#if defined(DBG)||defined(_DEBUG)	// >>>>>>>>>>>>>>>>>>>

#if defined(KERNEL_CODE)||defined(NATIVE_CODE)
#define CMMASSERT(_X) {if(!(_X))DbgBreakPoint();}
#else
#define CMMASSERT(_X) {if(!(_X))RaiseException(0x888888,0,0,0);}
#endif
#else
#define CMMASSERT

#endif		//  define(DBG) || define(_DEBUG) <<<<<<<<<<<<<<
#endif//CMMASSERT

class CmmTrace
{
public:
	static void Trace(TCHAR *format, ...) {
	#ifdef _DEBUG

		wchar_t buffer[1000];

		va_list argptr;

		va_start(argptr, format);

		wvsprintf(buffer, format, argptr);

		va_end(argptr);

		OutputDebugString(buffer);
	}
	#else
	}
	#endif
};



#endif//_CMMBASEOBJ_H_