/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// ECtl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CfgIface.h"
#include "cmmstruct.h"
#include "cmmBaseObj.h"
#include "cmmPath.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "FactoryImp.h"
#include "SmCuveImp.h"

#include "ReaderBaseFrame.h"
#include "MenuBase.h"


CFactoryImp* CFactoryImp::gpFactoryInstance=NULL;
DEFINE_BUILTIN_NAME(CFactoryImp)

CFactoryImp::CFactoryImp()
{
	mpConfig = NULL;
}
CFactoryImp::~CFactoryImp()
{
	CMM_SAFE_RELEASE(mpConfig);
}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CFactoryImp::InitOnCreate(void)
{
	if(gpFactoryInstance != NULL)
		return ERESULT_OBJECT_EXISTED;

	gpFactoryInstance = this;

	return ERESULT_SUCCESS;
}

// 从配置文件中创建对象
IEinkuiIterator* __stdcall CFactoryImp::CreateElement(
	IN IEinkuiIterator* npParent,		// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	IXsElement* lpoElement = NULL;
	IEinkuiIterator* lpILwIterator = NULL;
	wchar_t lswClsName[MAX_PATH] ={0};

	do 
	{
		BREAK_ON_NULL(npTemplete);

		// 获取要创建的对象类名
		if(npTemplete->GetValue(lswClsName, MAX_PATH*sizeof(wchar_t)) <= 0)
			break;

		if (0 == _wcsicmp(lswClsName, L"CVfSlMainFrame"))
			lpoElement = CReaderBaseFrame::CreateInstance(npParent, npTemplete,nuEID);
		else if(0 == _wcsicmp(lswClsName, L"Cuve"))
			lpoElement = CSmCuveImp::CreateInstance(npParent, npTemplete,nuEID);
		else if (0 == _wcsicmp(lswClsName, L"MenuBase"))
			lpoElement = CMenuBase::CreateInstance(npParent, npTemplete, nuEID);

	} while (false);

	if(lpoElement != NULL)
		lpILwIterator = lpoElement->GetIterator();	//获取Iterator指针

	return lpILwIterator;
}

// 通过类名，创建对象
IEinkuiIterator* __stdcall CFactoryImp::CreateElement(
	IN IEinkuiIterator* npParent,		// 父对象指针
	IN const wchar_t*		nswClassName,	// 类名
	IN ULONG nuEID					// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	IXsElement* lpoElement = NULL;
	IEinkuiIterator* lpILwIterator = NULL;

	if(lpoElement != NULL)
		lpILwIterator = lpoElement->GetIterator();	//获取Iterator指针

	return lpILwIterator;
}

// 获得与此Module配套的Profile文件接口，返回的接口当不再使用时，需要Release
IConfigFile* __stdcall CFactoryImp::GetTempleteFile(void)
{
	const wchar_t* lpszWidgetPath = NULL;
	const wchar_t* lpszLanguage = NULL;
	wchar_t lpszConfigFileName[CONFIG_FILE_NAME_MAX_LEN] = {0};

	IConfigFile* lpIConfigFile = NULL;
	do 
	{
		if(mpConfig == NULL)
		{
			lpszWidgetPath = EinkuiGetSystem()->GetCurrentWidget()->GetWidgetDefaultPath();		//获取Widget的安装路径
			BREAK_ON_NULL(lpszWidgetPath);

			CFilePathName loConfigFilePath(lpszWidgetPath);
			loConfigFilePath.AssurePath();	//设置为目录，也就是在最后增加"\"

			BREAK_ON_FALSE(loConfigFilePath.Transform(L"Profile\\"));

			lpszLanguage = EinkuiGetSystem()->GetCurrentLanguage();		//获取当前系统语言对应的字符串,例如：中文简体对应：chn
			BREAK_ON_NULL(lpszLanguage);

			wcscpy_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,L"Widget");		//拼接文件名 示例：System_chn.set
			wcscat_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,L"_");
			wcscat_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,lpszLanguage);
			//wcscat_s(lpszConfigFileName, CONFIG_FILE_NAME_MAX_LEN, L"bg");
			wcscat_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,L".set");

			BREAK_ON_FALSE(loConfigFilePath.Transform(lpszConfigFileName));	//拼成全路径

			lpIConfigFile = EinkuiGetSystem()->OpenConfigFile(loConfigFilePath.GetPathName(),OPEN_EXISTING);	//打开该配置文件

			if (lpIConfigFile == NULL)
			{
				//如果打不开就使用英文配置文件
				wcscpy_s(lpszConfigFileName, CONFIG_FILE_NAME_MAX_LEN, L"Widget");		//拼接文件名 示例：System_chn.set
				wcscat_s(lpszConfigFileName, CONFIG_FILE_NAME_MAX_LEN, L"_enu.set");

				BREAK_ON_FALSE(loConfigFilePath.Transform(lpszConfigFileName));	//拼成全路径

				lpIConfigFile = EinkuiGetSystem()->OpenConfigFile(loConfigFilePath.GetPathName(), OPEN_EXISTING);	//打开该配置文件

			}

			BREAK_ON_NULL(lpIConfigFile);

			mpConfig = lpIConfigFile;
		}
		else
			lpIConfigFile = mpConfig;

		lpIConfigFile->AddRefer();	//增加引用记数

	} while (false);

	return lpIConfigFile;
}

// 获得本DLL唯一的工厂对象
CFactoryImp* CFactoryImp::GetUniqueObject(void)
{
	if(gpFactoryInstance ==NULL)
		CFactoryImp::CreateInstance();

	CMMASSERT(gpFactoryInstance !=NULL);

	return gpFactoryInstance;
}

