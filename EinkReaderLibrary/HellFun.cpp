/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "HellFun.h"


CHellFun::CHellFun()
{
}


CHellFun::~CHellFun()
{
}

//设置注册表数据
bool CHellFun::SetRegData(wchar_t* npszKeyName, DWORD ndwValue, wchar_t* npszData)
{
	bool lbRet = false;
	DWORD ldwRet = -1;
	HKEY lhKey = NULL;

	do
	{
		//打开注册表
		ldwRet = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
		if (ldwRet != ERROR_SUCCESS)
		{
			//如果还没有就创建
			ldwRet = RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", &lhKey);
		}

		if (npszData == NULL)
			ldwRet = RegSetValueEx(lhKey, npszKeyName, NULL, REG_DWORD, (BYTE*)&ndwValue, sizeof(DWORD));
		else
			ldwRet = RegSetValueEx(lhKey, npszKeyName, NULL, REG_SZ, (BYTE*)npszData, (DWORD)(sizeof(wchar_t)*(wcslen(npszData) + 1)));

		lbRet = true;

	} while (false);

	if (lhKey != NULL)
		RegCloseKey(lhKey);

	return lbRet;
}