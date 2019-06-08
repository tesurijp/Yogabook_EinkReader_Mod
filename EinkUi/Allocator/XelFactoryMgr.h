/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EOB_FACTORY_MGR_
#define _EOB_FACTORY_MGR_


// 工厂类管理器
// 用于加载DLL，管理DLL句柄；注册DLL，销毁DLL


DECLARE_BUILTIN_NAME(CXelFactoryMgr)
class CXelFactoryMgr : public cmmBaseObject<CXelFactoryMgr, IBaseObject, GET_BUILTIN_NAME(CXelFactoryMgr)>
{
public:
	CXelFactoryMgr();
	virtual ~CXelFactoryMgr();
	
public:
	// 加载工厂类DLL
	HMODULE	__stdcall LoadDll(const wchar_t* nswDllPath);
	// 卸载工厂类DLL
	BOOL __stdcall UnLoadDll(HMODULE nhDllHandle);


	// 实例化工厂类，获取其接口。其本质是，调用工厂类DLL的一个引出函数，引出函数实例化工厂类
	// nuDllHandle1：DLL模块句柄
	IElementFactory* __stdcall GetFactInstance(const wchar_t* nswDllPath);

private:

};





#endif