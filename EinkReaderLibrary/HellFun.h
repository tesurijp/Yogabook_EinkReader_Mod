/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
class CHellFun
{
public:
	CHellFun();
	~CHellFun();

	//设置注册表数据
	static bool SetRegData(wchar_t* npszKeyName, DWORD ndwValue, wchar_t* npszData = NULL);
};

