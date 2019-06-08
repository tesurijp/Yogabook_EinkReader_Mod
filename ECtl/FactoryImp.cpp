/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// ECtl.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "CfgIface.h"
#include "cmmstruct.h"
#include "cmmBaseObj.h"
#include "cmmPath.h"
#include "mapDefine.h"
#include "mapList.h"


#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"

#include "EvButtonImp.h"
#include "EvPictureFrameImp.h"
#include "EvStaticTextImp.h"
#include "EvEditImp.h"
#include "EvScrollBarImp.h"
#include "EvListImp.h"
#include "EvSliderBarImp.h"
#include "FactoryImp.h"
#include "EvSliderButtonImp.h"
#include "EvPpButtonImp.h"
#include "EvLabelImp.h"
#include "EvBiSliderBarImp.h"
#include "EvAnimatorImp.h"

#include "EvPopupMenuImp.h"
#include "EvMenuButtonImp.h"

#include "EvMenuBarImp.h"

//#include "EvToolPane.h"
//#include "EvToolBar.h"
//#include "EvImageButtonImp.h"
#include "EvComboBoxImp.h"
//#include "EvSpinButtonImp.h"
#include "EvWhirlAngle.h"
#include "xSelectPoint.h"
#include "xSelectFrame.h"
#include "EvCheckButtonImp.h"
#include "EvRadioButtonGroupImp.h"
//#include "EvTimeSpinButtonImp.h"

CFactoryImp* CFactoryImp::gpFactoryInstance=NULL;
DEFINE_BUILTIN_NAME(CFactoryImp)

CFactoryImp::CFactoryImp()
{

	mpConfig = NULL;
}
CFactoryImp::~CFactoryImp()
{

}

// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
ULONG CFactoryImp::InitOnCreate(void)
{
	if(gpFactoryInstance != NULL)
		return ERESULT_OBJECT_EXISTED;

	gpFactoryInstance = this;

	// 在这里加入映射表元素
	bool lbStatus = false;	
	do 
	{
		lbStatus = moMapList.AddList(L"Button", CMapCallFP<CEvButton>::Custom2Normal(&CEvButton::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
 		lbStatus = moMapList.AddList(L"CheckButton", CMapCallFP<CEvCheckButton>::Custom2Normal(&CEvCheckButton::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"RadioButtonGroup", CMapCallFP<CEvRadioButtonGroup>::Custom2Normal(&CEvRadioButtonGroup::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"PictureFrame", CMapCallFP<CEvPictureFrame>::Custom2Normal(&CEvPictureFrame::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"StaticText", CMapCallFP<CEvStaticText>::Custom2Normal(&CEvStaticText::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"Edit", CMapCallFP<CEvEditImp>::Custom2Normal(&CEvEditImp::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"ScrollBar", CMapCallFP<CEvScrollBar>::Custom2Normal(&CEvScrollBar::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"List", CMapCallFP<CEvList>::Custom2Normal(&CEvList::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"SliderBar", CMapCallFP<CEvSliderBar>::Custom2Normal(&CEvSliderBar::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"BiSliderBar", CMapCallFP<CEvBiSliderBar>::Custom2Normal(&CEvBiSliderBar::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"SliderButton", CMapCallFP<CEvSliderButton>::Custom2Normal(&CEvSliderButton::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"PingpongButton", CMapCallFP<CEvPingpongButton>::Custom2Normal(&CEvPingpongButton::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"Label", CMapCallFP<CEvLabelImp>::Custom2Normal(&CEvLabelImp::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"Animator", CMapCallFP<CEvAnimatorImp>::Custom2Normal(&CEvAnimatorImp::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		//lbStatus = moMapList.AddList(L"MenuButton", CMapCallFP<CEvMenuButton>::Custom2Normal(&CEvMenuButton::CreateInstance));
		//BREAK_ON_FALSE(lbStatus);
		//lbStatus = moMapList.AddList(L"MenuBar", CMapCallFP<CEvMenuBar>::Custom2Normal(&CEvMenuBar::CreateInstance));
		//BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"PopupMenu", CMapCallFP<CEvPopupMenu>::Custom2Normal(&CEvPopupMenu::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
// 		lbStatus = moMapList.AddList(L"ToolPane", CMapCallFP<CEvToolPane>::Custom2Normal(&CEvToolPane::CreateInstance));
// 		BREAK_ON_FALSE(lbStatus);
		//lbStatus = moMapList.AddList(L"ToolBar", CMapCallFP<CEvToolBar>::Custom2Normal(&CEvToolBar::CreateInstance));
		//BREAK_ON_FALSE(lbStatus);
// 		lbStatus = moMapList.AddList(L"ImageButton", CMapCallFP<CEvImageButton>::Custom2Normal(&CEvImageButton::CreateInstance));
// 		BREAK_ON_FALSE(lbStatus);
 		lbStatus = moMapList.AddList(L"ComboBox", CMapCallFP<CEvComboBox>::Custom2Normal(&CEvComboBox::CreateInstance));
 		BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"WhirlAngle", CMapCallFP<CEvWhirlAngleImp>::Custom2Normal(&CEvWhirlAngleImp::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		//lbStatus = moMapList.AddList(L"SpinButton", CMapCallFP<CEvSpinButton>::Custom2Normal(&CEvSpinButton::CreateInstance));
		//BREAK_ON_FALSE(lbStatus);
		lbStatus = moMapList.AddList(L"SelectFrame", CMapCallFP<CSelectFrame>::Custom2Normal(&CSelectFrame::CreateInstance));
		BREAK_ON_FALSE(lbStatus);
		//lbStatus = moMapList.AddList(L"TimeSpinButton", CMapCallFP<CEvTimeSpinButton>::Custom2Normal(&CEvTimeSpinButton::CreateInstance));
		//BREAK_ON_FALSE(lbStatus);
	} while (false);

	if (lbStatus == false)
		return ERESULT_UNSUCCESSFUL;

	return ERESULT_SUCCESS;
}

// 从配置文件中创建对象
IEinkuiIterator* __stdcall CFactoryImp::CreateElement(
	IN IEinkuiIterator* npParent,		// 父对象指针
	IN ICfKey* npTemplete,			// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID					// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	IXsElement* lpoElement = NULL;
	IEinkuiIterator* lpXIterator = NULL;
	wchar_t lswClsName[MAX_PATH] ={0};

	do 
	{
		BREAK_ON_NULL(npTemplete);

		// 获取要创建的对象类名
		if(npTemplete->GetValue(lswClsName, MAX_PATH*sizeof(wchar_t)) <= 0)
			break;

		// 在MAPLIST表中查找对应的创建函数
		AFX_MAPCALL lpfnCreateFunction = moMapList.GetUserData(lswClsName,NULL);
		BREAK_ON_NULL(lpfnCreateFunction);

		// 用返回的函数指针创建对象
		lpoElement = lpfnCreateFunction(npParent, npTemplete, nuEID);

	} while (false);

	if(lpoElement != NULL)
		lpXIterator = lpoElement->GetIterator();	//获取Iterator指针

	return lpXIterator;
}


// 通过类名，创建对象
IEinkuiIterator* __stdcall CFactoryImp::CreateElement(
	IN IEinkuiIterator* npParent,		// 父对象指针
	IN const wchar_t*		nswClassName,	// 类名
	IN ULONG nuEID					// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	IXsElement* lpoElement = NULL;
	IEinkuiIterator* lpXIterator = NULL;

	do 
	{
		BREAK_ON_NULL(nswClassName);

		// 在MAPLIST表中查找对应的创建函数
		AFX_MAPCALL lpfnCreateFunction = moMapList.GetUserData(nswClassName,NULL);
		BREAK_ON_NULL(lpfnCreateFunction);

		// 用返回的函数指针创建对象
		lpoElement = lpfnCreateFunction(npParent, NULL, nuEID);

	} while (false);

	if(lpoElement != NULL)
		lpXIterator = lpoElement->GetIterator();	//获取Iterator指针

	return lpXIterator;
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

			wcscpy_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,L"Widget_");		//拼接文件名 示例：System_chn.set
			wcscat_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,lpszLanguage);
			wcscat_s(lpszConfigFileName,CONFIG_FILE_NAME_MAX_LEN,L".set");

			BREAK_ON_FALSE(loConfigFilePath.Transform(lpszConfigFileName));	//拼成全路径

			lpIConfigFile = EinkuiGetSystem()->OpenConfigFile(loConfigFilePath.GetPathName(),OPEN_EXISTING);	//打开该配置文件

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

