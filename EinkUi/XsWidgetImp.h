/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once



//////////////////////////////////////////////////////////////////////////
// Widget运行态
DECLARE_BUILTIN_NAME(CXsWidget)
class CXsWidget: public cmmBaseObject<CXsWidget,IXsWidget,GET_BUILTIN_NAME(CXsWidget)>
{
	friend class CEinkuiSystem;
	friend class CXelManager;
public:
	// 获取微件所在安装目录，不带有后缀的'\'
	virtual const wchar_t* __stdcall GetWidgetDefaultPath(void);

	// 该Widget的模块文件名，即实现此Widget的DLL名称，如"IdeaMain.dll"
	virtual const wchar_t* __stdcall GetModuleName(void);

	// 获得本微件实例名，系统启动时建立的第一个微件叫做‘system’
	virtual const wchar_t* __stdcall GetInstanceName(void);

	// 获得微件所在Module的工厂接口
	virtual IElementFactory* __stdcall GetDefaultFactory(void);

	// 获得微件的实例专属配置；一个微件Module可以在一个进程中同时运行多个实例，与此同时，一台电脑上，每一个Windows用户帐户下，都可以运行一个Idealife进程；
	//		所以需要为微件的每一个运行态实例提供一个专属配置，它可以将需要长期保存的设置类信息存放到这个专属配置中
	virtual ICfKey* __stdcall GetInstanceConfig(void);

	//获取微件用来存放临时文件的目录,返回的地址不包含\\结尾,同一个的微件的不同实例得到的是不同的文件夹路径
	//npswPath为输出缓冲区，nlBufferLen为缓冲区长度，BY TCHAR
	virtual bool __stdcall GetInstanceTempPath(OUT wchar_t* npswPath,IN LONG nlBufferLen);

	// 获得微件实例的Home Page
	virtual IEinkuiIterator* __stdcall GetHomePage(void);

	// 关闭Widget
	virtual void __stdcall Close(void);

private:
	IElementFactory* mpFactory;
	IEinkuiIterator* mpHomePage;
	ICfKey*	mpInstanceConfig;
	wchar_t* mswModulePath;	// 与下面的两个字符串mswName，mswInstance公用缓冲区
	wchar_t* mswModuleName;
	wchar_t* mswInstance;
	bool mbValid;

	DEFINE_CUMSTOMIZE_CREATE(CXsWidget,(const wchar_t* nswModulePathName,const wchar_t* nswInstanceName,ICfKey* npInstanceConfig),(nswModulePathName, nswInstanceName,npInstanceConfig))

	ERESULT InitOnCreate(
		IN const wchar_t* nswModulePathName,	// 该Widget的模块文件的路径名，即实现此Widget的DLL名称
		IN const wchar_t* nswInstanceName,	// 本次运行的实例名，实例名不能相同，如果存在相同的实例名，系统将会返回失败
		IN ICfKey* npInstanceConfig	// 本运行实例的专属配置
		);

	void SetHomePage(IEinkuiIterator* npHomePage);

	void SetFactory(IElementFactory* npFactory);

	bool IsValid(void){
		return mbValid;
	}

	void SetInvalid(void){
		mbValid = false;
	}


	CXsWidget();
	~CXsWidget();

};











