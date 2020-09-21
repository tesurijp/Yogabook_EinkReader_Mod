/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#pragma once


#define DRIVER_FUNC_INSTALL 0x01 //install

#define DRIVER_FUNC_REMOVE 0x02 //remove

#define DRIVER_FUNC_GET_STATES 0x03 //Get States

#define DRIVER_FUNC_START 0x04 //start

#define DRIVER_FUNC_STOP 0x05 //stop

#include <Windows.h>
#include <tchar.h>
#include <winsvc.h>

class CInstallDriver
{
public:
	CInstallDriver(void);
	~CInstallDriver(void);

	BOOLEAN	InstallDriver(
		IN SC_HANDLE  SchSCManager,
		IN LPCTSTR    DriverName,
		IN LPCTSTR    ServiceExe,
		IN DWORD      dwDesiredAccess,
		IN DWORD      dwServiceType,
		IN DWORD	  dwStartType
	);


	BOOLEAN	RemoveDriver(
		IN SC_HANDLE  SchSCManager,
		IN LPCTSTR    DriverName
	);

	BOOLEAN	StartDriver(
		IN SC_HANDLE  SchSCManager,
		IN LPCTSTR    DriverName
	);

	BOOLEAN	StopDriver(
		IN SC_HANDLE  SchSCManager,
		IN LPCTSTR    DriverName
	);

	BOOLEAN ManageDriver(
		IN LPCTSTR  DriverName,
		IN LPCTSTR  ServiceName,
		IN DWORD    dwDesiredAccess,
		IN DWORD    dwServiceType,
		IN DWORD	dwStartType,
		IN USHORT   Function,
		OUT LPSERVICE_STATUS npServiceStatus = NULL
	);

	BOOLEAN SetupDriverName(
		PUCHAR DriverLocation,
		PUCHAR DriverName
	);

	//获取服务状态
	BOOLEAN GetServiceStates(
		IN SC_HANDLE  SchSCManager,
		IN LPCTSTR    DriverName,
		OUT LPSERVICE_STATUS npServiceStatus = NULL
	);

};
