/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#include "stdafx.h"
#include "InstallDriver.h"

CInstallDriver::CInstallDriver(void)
{
}

CInstallDriver::~CInstallDriver(void)
{
}

BOOLEAN CInstallDriver::InstallDriver(
	IN SC_HANDLE  SchSCManager,
	IN LPCTSTR    DriverName,
	IN LPCTSTR    ServiceExe,
	IN DWORD      dwDesiredAccess,
	IN DWORD      dwServiceType,
	IN DWORD		dwStartType
)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	SC_HANDLE   schService;
	DWORD       err;

	//
	// NOTE: This creates an entry for a standalone driver. If this
	//       is modified for use with a driver that requires a Tag,
	//       Group, and/or Dependencies, it may be necessary to
	//       query the registry for existing driver information
	//       (in order to determine a unique Tag, etc.).
	//

	//
	// Create a new a service object.
	//

	schService = CreateService(SchSCManager,           // handle of service control manager database
		DriverName,             // address of name of service to start
		DriverName,             // address of display name
		dwDesiredAccess,        // type of access to service
		dwServiceType,			// type of service
		dwStartType,            // when to start service
		SERVICE_ERROR_SEVERE,   // severity if service fails to start
		ServiceExe,             // address of name of binary file
		NULL,                   // service does not belong to a group
		NULL,                   // no tag requested
		NULL,                   // no dependency names
		NULL,                   // use LocalSystem account
		NULL                    // no password for service account
	);

	if (schService == NULL) {

		err = GetLastError();


		if (err == ERROR_SERVICE_EXISTS) {

			//
			// Ignore this error.
			//

			return TRUE;

		}
		else {

			//printf("CreateService failed!  Error = %d \n", err );

			//wchar_t wcError[MAX_PATH] = {0};
			//wcscpy_s(wcError,MAX_PATH,_T("CreateService failed!  Error = "));
			//wchar_t wcBuffer[MAX_PATH] = {0};
			//_itow(err,wcBuffer,10);
		//	wcscat_s(wcError,MAX_PATH,wcBuffer);
			//OutputDebugString(wcError);
			//
			// Indicate an error.
			//

			return  FALSE;
		}
	}

	//
	// Close the service object.
	//

	if (schService) {
		//if(dwServiceType != SERVICE_KERNEL_DRIVER)
		//{
		//	// 服务错误时，自动重新启动服务
		//	SERVICE_FAILURE_ACTIONS_FLAG sfaf;
		//	sfaf.fFailureActionsOnNonCrashFailures = TRUE;
		//	ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &sfaf);

		//	SERVICE_FAILURE_ACTIONS sfa;
		//	SC_ACTION sa;
		//	sfa.dwResetPeriod = INFINITE;
		//	sfa.lpRebootMsg = NULL;
		//	sfa.lpCommand = NULL;
		//	sfa.cActions = 1;
		//	sa.Delay = 1000;
		//	sa.Type = SC_ACTION_RESTART;
		//	sfa.lpsaActions = &sa;
		//	ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa);
		//}

		//设置
		SC_ACTION ldAction;
		ldAction.Delay = 1;
		ldAction.Type = SC_ACTION_RESTART;
		SERVICE_FAILURE_ACTIONS ldFailureActions;
		ldFailureActions.dwResetPeriod = 1;
		ldFailureActions.lpRebootMsg = L"";
		ldFailureActions.lpCommand = NULL;
		ldFailureActions.cActions = 1;
		ldFailureActions.lpsaActions = &ldAction;
		ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &ldFailureActions);

		CloseServiceHandle(schService);
	}

	//
	// Indicate success.
	//

	return TRUE;

}   // InstallDriver

BOOLEAN CInstallDriver::ManageDriver(
	IN LPCTSTR  DriverName,
	IN LPCTSTR  ServiceName,
	IN DWORD    dwDesiredAccess,
	IN DWORD    dwServiceType,
	IN DWORD	 dwStartType,
	IN USHORT   Function,
	OUT LPSERVICE_STATUS npServiceStatus
)
{

	SC_HANDLE   schSCManager;

	BOOLEAN rCode = TRUE;

	//
	// Insure (somewhat) that the driver and service names are valid.
	//

	if (!DriverName || !ServiceName) {

		//printf("Invalid Driver or Service provided to ManageDriver() \n");

		return FALSE;
	}

	//
	// Connect to the Service Control Manager and open the Services database.
	//

	schSCManager = OpenSCManager(NULL,                   // local machine
		NULL,                   // local database
		SC_MANAGER_ALL_ACCESS   // access required
	);

	if (!schSCManager) {

		//DWORD dwErr = GetLastError();
		//wchar_t wcError[MAX_PATH] = {0};
		//wcscpy_s(wcError,MAX_PATH,_T("Open SC Manager failed! Error = "));
		//wchar_t wcBuffer[MAX_PATH] = {0};
		//_itow(dwErr,wcBuffer,10);
		//wcscat_s(wcError,MAX_PATH,wcBuffer);
		//OutputDebugString(wcError);
		//printf("Open SC Manager failed! Error = %d \n", GetLastError());

		return FALSE;
	}

	//
	// Do the requested function.
	//

	switch (Function) {

	case DRIVER_FUNC_INSTALL:

		//
		// Install the driver service.
		//

		if (InstallDriver(schSCManager,
			DriverName,
			ServiceName,
			dwDesiredAccess,
			dwServiceType,
			dwStartType
		)) {

			//
			// Start the driver service (i.e. start the driver).
			//
			Sleep(500);
			rCode = StartDriver(schSCManager,
				DriverName
			);
			rCode = TRUE;

		}
		else {

			//
			// Indicate an error.
			//

			rCode = FALSE;
		}

		break;

	case DRIVER_FUNC_REMOVE:

		//
		// Stop the driver.
		//

		StopDriver(schSCManager,
			DriverName
		);

		//
		// Remove the driver service.
		//

		RemoveDriver(schSCManager,
			DriverName
		);

		//
		// Ignore all errors.
		//

		rCode = TRUE;

		break;

	case DRIVER_FUNC_GET_STATES:
	{
		//获取服务状态
		rCode = GetServiceStates(schSCManager, DriverName, npServiceStatus);
	}
	break;
	case DRIVER_FUNC_START:
		rCode = StartDriver(schSCManager,
			DriverName
		);
		rCode = TRUE;
		break;
	case DRIVER_FUNC_STOP:
		rCode = StopDriver(schSCManager, DriverName);
		rCode = TRUE;
		break;
	default:

		//printf("Unknown ManageDriver() function. \n");

		rCode = FALSE;

		break;
	}

	//
	// Close handle to service control manager.
	//

	if (schSCManager) {

		CloseServiceHandle(schSCManager);
	}

	return rCode;

}   // ManageDriver


BOOLEAN CInstallDriver::RemoveDriver(
	IN SC_HANDLE    SchSCManager,
	IN LPCTSTR      DriverName
)
{
	SC_HANDLE   schService;
	BOOLEAN     rCode;

	//
	// Open the handle to the existing service.
	//

	schService = OpenService(SchSCManager,
		DriverName,
		SERVICE_ALL_ACCESS
	);

	if (schService == NULL) {

		//printf("OpenService failed!  Error = %d \n", GetLastError());

		//
		// Indicate error.
		//

		return FALSE;
	}

	//
	// Mark the service for deletion from the service control manager database.
	//

	if (DeleteService(schService)) {

		//
		// Indicate success.
		//

		rCode = TRUE;

	}
	else {

		//printf("DeleteService failed!  Error = %d \n", GetLastError());

		//
		// Indicate failure.  Fall through to properly close the service handle.
		//

		rCode = FALSE;
	}

	//
	// Close the service object.
	//

	if (schService) {

		CloseServiceHandle(schService);
	}

	return rCode;

}   // RemoveDriver



BOOLEAN CInstallDriver::StartDriver(
	IN SC_HANDLE    SchSCManager,
	IN LPCTSTR      DriverName
)
{
	SC_HANDLE   schService = NULL;
	DWORD       err;
	BOOLEAN lbRet = FALSE;
	//
	// Open the handle to the existing service.
	//

	schService = OpenService(SchSCManager,
		DriverName,
		SERVICE_ALL_ACCESS
	);

	if (schService == NULL) {

		//DWORD dwErr = GetLastError();
		//printf("OpenService failed!  Error = %d \n",dwErr);

		//wchar_t wcError[MAX_PATH] = {0};
		//wcscpy_s(wcError,MAX_PATH,_T("OpenService failed!  Error = "));
		//wchar_t wcBuffer[MAX_PATH] = {0};
		//_itow(dwErr,wcBuffer,10);
		//wcscat_s(wcError,MAX_PATH,wcBuffer);
		//OutputDebugString(wcError);
		//
		// Indicate failure.
		//

		return FALSE;
	}

	//
	// Start the execution of the service (i.e. start the driver).
	//

	if (!StartService(schService,     // service identifier
		0,              // number of arguments
		NULL            // pointer to arguments
	)) {

		err = GetLastError();

		if (err == ERROR_SERVICE_ALREADY_RUNNING) {

			//
			// Ignore this error.
			//

			return TRUE;

		}
		else {

			//printf("StartService failure! Error = %d \n", err );

			//wchar_t wcError[MAX_PATH] = {0};
			//wcscpy_s(wcError,MAX_PATH,_T("StartService failure! Error = "));
			//wchar_t wcBuffer[MAX_PATH] = {0};
			//_itow(err,wcBuffer,10);
			//wcscat_s(wcError,MAX_PATH,wcBuffer);
			//OutputDebugString(wcError);

			//
			// Indicate failure.  Fall through to properly close the service handle.
			//

				lbRet = FALSE;
		}

	}

	//
	// Close the service object.
	//

	if (schService != NULL) {

		CloseServiceHandle(schService);
	}

	return lbRet;

}   // StartDriver



BOOLEAN CInstallDriver::StopDriver(
	IN SC_HANDLE    SchSCManager,
	IN LPCTSTR      DriverName
)
{
	BOOLEAN         rCode = TRUE;
	SC_HANDLE       schService;
	SERVICE_STATUS  serviceStatus;

	//
	// Open the handle to the existing service.
	//

	schService = OpenService(SchSCManager,
		DriverName,
		SERVICE_ALL_ACCESS
	);

	if (schService == NULL) {

		//printf("OpenService failed!  Error = %d \n", GetLastError());

		return FALSE;
	}

	//
	// Request that the service stop.
	//

	if (ControlService(schService,
		SERVICE_CONTROL_STOP,
		&serviceStatus
	)) {

		//
		// Indicate success.
		//

		rCode = TRUE;

	}
	else {

		//printf("ControlService failed!  Error = %d \n", GetLastError() );

		//
		// Indicate failure.  Fall through to properly close the service handle.
		//

		rCode = FALSE;
	}

	//
	// Close the service object.
	//

	if (schService) {

		CloseServiceHandle(schService);
	}

	return rCode;

}   //  StopDriver

BOOLEAN CInstallDriver::SetupDriverName(
	PUCHAR DriverLocation,
	PUCHAR DriverName
)
{
	HANDLE fileHandle;

	DWORD driverLocLen = 0;

	//
	// Get the current directory.
	//
	driverLocLen = GetCurrentDirectory(MAX_PATH, (LPWSTR)DriverLocation
	);

	if (!driverLocLen) {

		//printf("GetCurrentDirectory failed!  Error = %d \n", GetLastError());

		return FALSE;
	}

	//
	// Setup path name to driver file.
	//

	//strcat((char *)DriverLocation, "\\");
	//strcat((char *)DriverLocation, (char *)DriverName);
	//strcat((char *)DriverLocation, ".sys");

	//
	// Insure driver file is in the specified directory.
	//

	if ((fileHandle = CreateFile((LPWSTR)DriverLocation,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	)) == INVALID_HANDLE_VALUE) {


		//printf("Driver: %s.SYS is not in the %s directory. \n", DriverName, DriverLocation );

		//
		// Indicate failure.
		//

		return FALSE;
	}

	//
	// Close open file handle.
	//

	if (fileHandle) {

		CloseHandle(fileHandle);
	}

	//
	// Indicate success.
	//

	return TRUE;


}   // SetupDriverName


//获取服务状态
BOOLEAN CInstallDriver::GetServiceStates(
	IN SC_HANDLE  SchSCManager,
	IN LPCTSTR    DriverName,
	OUT LPSERVICE_STATUS npServiceStatus
)
{
	//打开服务
	SC_HANDLE schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);

	BOOLEAN lbRet = FALSE;

	do 
	{
		if (schService == NULL)
			break;

		lbRet = QueryServiceStatus(schService, npServiceStatus);

		CloseServiceHandle(schService);

	} while (FALSE);


	return lbRet;
}