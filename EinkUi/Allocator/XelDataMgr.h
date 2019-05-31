/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EOB_DATA_MGR_
#define _EOB_DATA_MGR_



// 工厂类DLL与对象类在内存中的pair结构体
class CLelFactoryNode{
public:
	HANDLE	mpDllHandle;
	wchar_t*	mswDllPath;
	IElementFactory*	pfnElementFact;
	ULONG muHash;
	__inline void operator=(class CLelFactoryNode& src) {
		mpDllHandle = src.mpDllHandle;
		mswDllPath = src.mswDllPath;
		src.mswDllPath = NULL;
		pfnElementFact = src.pfnElementFact;
		muHash = src.muHash;
	}
	CLelFactoryNode(){
		mpDllHandle = NULL;
		mswDllPath = NULL;
		pfnElementFact = NULL;
	}
	~CLelFactoryNode() {
		CMM_SAFE_DELETE(mswDllPath);
	}
	// 用字符串生成Hash码,返回的是名字的有效长度
	int GenerateHashCode(
		IN const wchar_t* nszName,
		IN int niNameLen	// -1 indicate that the name was terminated by '\0' or '\\' or '/'
		)
	{
		LONG liIndex = 0;
		muHash = 0;

		if(niNameLen < 0)
		{
			niNameLen = 255;
		}

		wchar_t lcCrt;
		while (nszName[liIndex] != 0 && liIndex < niNameLen)
		{
			lcCrt = nszName[liIndex++];
			if((lcCrt|0x20) >= L'a' && (lcCrt|0x20) <= L'z')
				lcCrt |= 0x20;
			muHash += lcCrt;
			muHash = (muHash<<1) | ((muHash&0x010000)>>16);
		}
		
		return liIndex;
	}
	bool SavePath(const wchar_t* nswDllPath){
		if(nswDllPath == NULL || nswDllPath[0]==UNICODE_NULL)
			return true;
		int liSize = GenerateHashCode(nswDllPath,-1)+1;
		mswDllPath = new wchar_t[liSize];
		if(mswDllPath == NULL)
			return false;

		RtlCopyMemory(mswDllPath,nswDllPath,liSize*sizeof(wchar_t));

		return true;
	}
	void FreePath(void)	// 当一个对象从容器对象中删除后，才能调用本方法释放掉额外的内存
	{
		CMM_SAFE_DELETE(mswDllPath);
	}
};

enum REGSTATUS{
	Invalid = 0,	// 没有被注册
	LOAD = 1,		// 已经被加载
	UNLOAD = 2		// 在配置中，返回DLL路径
};



// 管理运行态的数据，运行全局唯一实例化
// 配置中保存对象的注册信息

DECLARE_BUILTIN_NAME(CXelDataMgr)
class CXelDataMgr:public cmmBaseObject<CXelDataMgr, IBaseObject, GET_BUILTIN_NAME(CXelDataMgr)> 
{
public:
	CXelDataMgr();
	virtual ~CXelDataMgr();

	
public:
	// 获取唯一实例化的对象接口
	static CXelDataMgr* SingleInstance(const wchar_t* nswRegPath=NULL);
	static CXelDataMgr* sm_Inst;

	// 初始化
	ULONG InitOnCreate(const wchar_t* nswRegPath);
	DEFINE_CUMSTOMIZE_CREATE(CXelDataMgr,(const wchar_t* nswRegPath),(nswRegPath))
	
public:

	// 通过指定类名，查找对应的DLL
	// nswClsName：传入参数，对象类名
	// nswDllPath：传出参数，如果找到，则将DLL全路径传到给字符指针指向的缓冲区
	BOOL __stdcall FindFactoryDll(
		const wchar_t* nswClsName, 
		wchar_t* nswDllPath
		);
	
	// 修改加载的DLL句柄表
	// niMode：修改模式。niMode=1，表示添加；niMode=0,表示删除
	BOOL __stdcall ModLoadedList(
		const wchar_t* nswDllPath,
		HANDLE nhDllHandle,
		IElementFactory* pfnElementFact,
		BOOL niMode
		);

	//// 注销一个工厂类
	//void RemoveFactory(
	//	IElementFactory* npElementFact
	//);

	// 查看指定的DLL是否已经被加载
	IElementFactory* __stdcall HasFactoryBeenLoaded(
		const wchar_t* nswDllPath
		);


	// 查看是否已经被注册,根据给定的类型，查看是否存在一个对应的工厂类。调用者根据ENUM返回值分别取传出参数的值
	// 方法，先检查是否已经被注册，然后查询句柄表，看是否已经加载。如果都不存在，则证明没有被注册
	// nswClsName：类名
	// nswDllPath：传出参数，DLL的路径
	// npElementFact：传出参数，工厂类接口
	REGSTATUS __stdcall IsRegisted(
		const wchar_t* nswClsName, 
		wchar_t* nswDllPath,
		IElementFactory** npElementFact
		);

	// 获取对象类注册信息的主键KEY
	HKEY GetMainKey();	


	// 用于维护对象管理的数据列表
protected:
	// {工厂类，对象类}映射表
//	cmmVector<FACTCLASSPAIR> mFactClassPairVec;
	cmmVector<CLelFactoryNode> moFactories;
	HKEY	mpMainKey;

private:
	BOOL mb64Bit;

};



#endif