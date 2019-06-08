/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "CommonHeader.h"

#include "XsWidgetImp.h"



DEFINE_BUILTIN_NAME(CXsWidget)

CXsWidget::CXsWidget()
{
	mpFactory = NULL;
	mswModulePath = NULL;
	mswInstance = NULL;
	mswModuleName = NULL;
	mpHomePage = NULL;
	mpInstanceConfig = NULL;
	mbValid = true;

	mpInstanceConfig = NULL;
}

CXsWidget::~CXsWidget()
{
	if (mpHomePage != NULL)
		mpHomePage->Close();
	CMM_SAFE_DELETE(mswModulePath);
	//CMM_SAFE_RELEASE(mpFactory); 留给GUI系统的管理器删除
	CMM_SAFE_RELEASE(mpInstanceConfig);
}


// 该Widget的模块文件名，即实现此Widget的DLL名称，如"IdeaMain.dll"
const wchar_t* __stdcall CXsWidget::GetModuleName(void)
{
	return mswModuleName;
}

// 获得本微件实例名，系统启动时建立的第一个微件叫做‘system’
const wchar_t* __stdcall CXsWidget::GetInstanceName(void)
{
	return mswInstance;
}

// 获取微件所在安装目录，不带有后缀的'\'
const wchar_t* __stdcall CXsWidget::GetWidgetDefaultPath(void)
{
	return mswModulePath;
}

// 获得微件所在Module的工厂接口
IElementFactory* __stdcall CXsWidget::GetDefaultFactory(void)
{
	return mpFactory;
}

void CXsWidget::SetFactory(IElementFactory* npFactory)
{
	mpFactory = npFactory;
	mpFactory->AddRefer();
}

void CXsWidget::SetHomePage(IEinkuiIterator* npHomePage)
{
	mpHomePage = npHomePage;
	mpHomePage->AddRefer();
}


ERESULT CXsWidget::InitOnCreate(
	IN const wchar_t* nswModulePathName,	// 该Widget的模块文件的路径名，即实现此Widget的DLL名称
	IN const wchar_t* nswInstanceName,	// 本次运行的实例名，实例名不能相同，如果存在相同的实例名，系统将会返回失败
	IN ICfKey* npInstanceConfig	// 本运行实例的专属配置
	)
{
	int liPathLen = (int)wcslen(nswModulePathName)+1;
	int liInsLen = (int)wcslen(nswInstanceName)+1;
	int liTotal = liPathLen+liInsLen+1;

	mswModulePath = new wchar_t[liTotal];
	if(mswModulePath == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	RtlCopyMemory(mswModulePath,nswModulePathName,liPathLen*sizeof(wchar_t));
	mswModuleName = wcsrchr(mswModulePath,L'\\');
	if(mswModuleName == NULL)
		return ERESULT_UNSUCCESSFUL;

	*mswModuleName++ =UNICODE_NULL;

	mswInstance = mswModulePath + liPathLen;
	RtlCopyMemory(mswInstance,nswInstanceName,liInsLen*sizeof(wchar_t));

	mpInstanceConfig = npInstanceConfig;
	if(mpInstanceConfig != NULL)
		mpInstanceConfig->AddRefer();

	return ERESULT_SUCCESS;
}


// 获得微件的实例专属配置；一个微件Module可以在一个进程中同时运行多个实例，与此同时，一台电脑上，每一个Windows用户帐户下，都可以运行一个Idealife进程；
//		所以需要为微件的每一个运行态实例提供一个专属配置，它可以将需要长期保存的设置类信息存放到这个专属配置中
ICfKey* __stdcall CXsWidget::GetInstanceConfig(void)
{
	return mpInstanceConfig;
}

//获取微件用来存放临时文件的目录,同一个的微件的不同实例得到的是不同的文件夹路径
bool __stdcall CXsWidget::GetInstanceTempPath(OUT wchar_t* npswPath,IN LONG nlBufferLen)
{
	bool lbRet = false;
	wchar_t lswTempPath[MAX_PATH] = {0};

	do 
	{
		BREAK_ON_NULL(npswPath);
		if(nlBufferLen <= 0)
			break;

		CFilePathName loDest;
		loDest.SetByUserAppData();
		loDest.AssurePath();
		loDest.Transform(L".\\MagWriter\\Temp\\");

		wcscpy_s(lswTempPath,MAX_PATH,loDest.GetPathName());
		wcscat_s(lswTempPath,MAX_PATH,mswInstance);

		wcscpy_s(npswPath,nlBufferLen,lswTempPath);

		//创建这个目录
		SHCreateDirectory(NULL,lswTempPath);

		lbRet = true;

	} while (false);

	return lbRet;
}
// 获得微件实例的Home Page
IEinkuiIterator* __stdcall CXsWidget::GetHomePage(void)
{
	return mpHomePage;
}

// 关闭Widget
void __stdcall CXsWidget::Close(void)
{
	CMMASSERT(mpHomePage);

	// 首先隐藏
	mpHomePage->SetVisible(false);

	// 将HomePage移到Root下，防止SystemWidget对它重复删除
	EinkuiGetSystem()->GetElementManager()->SetParentElement(EinkuiGetSystem()->GetElementManager()->GetRootElement(),mpHomePage);

	// 请求系统将本Widget关闭
	CExMessage::PostMessage(NULL,NULL,EMSG_CLOSE_WIDGET,this);
}
