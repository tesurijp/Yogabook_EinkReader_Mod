#include "stdafx.h"
#include "ProcessStarter.h"

#include <userenv.h>
#pragma comment(lib, "Userenv.lib")

#include <wtsapi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#include <Sddl.h>

CProcessStarter::CProcessStarter()
{

}

BOOL CProcessStarter::FindActiveSessionId(OUT DWORD& dwSessionId, BOOL bNeedLog)
{
	BOOL bFindActiveSession = FALSE;
	DWORD dwIndex = 0;

	PWTS_SESSION_INFO pWtsSessionInfo = NULL;
	DWORD dwCntWtsSessionInfo = 0;


	do
	{
		dwSessionId = (DWORD)(-1);

		if ((!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWtsSessionInfo, &dwCntWtsSessionInfo))
			|| (NULL == pWtsSessionInfo))
		{
			break;
		}

		for (dwIndex = 0; dwIndex < dwCntWtsSessionInfo; dwIndex++)
		{
			if (WTSActive == pWtsSessionInfo[dwIndex].State)
			{
				dwSessionId = pWtsSessionInfo[dwIndex].SessionId;
				bFindActiveSession = TRUE;
				break;
			}
		}

		WTSFreeMemory(pWtsSessionInfo);

		if (!bFindActiveSession)
		{

			break;
		}
	} while (0);

	return bFindActiveSession;
}

DWORD CProcessStarter::GetSessionID(void)
{
	DWORD ldwSessionId = 0;
	do
	{
		ldwSessionId = WTSGetActiveConsoleSessionId();

	} while (false);

	return ldwSessionId;
}

HANDLE CProcessStarter::GetCurrentUserToken()
{
	DWORD ldwSessionId = 0;
	HANDLE lhCurrentToken = NULL;
	HANDLE lhPrimaryToken = NULL;

	do
	{
		ldwSessionId = GetSessionID();
		if (ldwSessionId <= 0)
			break;

		//获取当前用户的权限
		if (!WTSQueryUserToken(ldwSessionId, &lhCurrentToken)
			|| (NULL == lhCurrentToken))
			break;

		if (!DuplicateTokenEx(lhCurrentToken, TOKEN_ASSIGN_PRIMARY | TOKEN_ALL_ACCESS, 0, SecurityImpersonation, TokenPrimary, &lhPrimaryToken))
			break;

		//设置为管理员权限
		SID ldSID;
		int liaaa = 0;
		ZeroMemory(&ldSID, sizeof(SID));
		ldSID.Revision = SID_REVISION;
		ldSID.SubAuthorityCount = 1;
		ldSID.IdentifierAuthority.Value[5] = 16;
		ldSID.SubAuthority[0] = SECURITY_MANDATORY_HIGH_RID;
		//ldSID.SubAuthority[1] = 0;

		TOKEN_MANDATORY_LABEL ltokenIntegrityLevel = { 0 };
		ltokenIntegrityLevel.Label.Attributes = SE_GROUP_INTEGRITY;
		ltokenIntegrityLevel.Label.Sid = &ldSID;

		if (::SetTokenInformation(lhPrimaryToken, TokenIntegrityLevel, &ltokenIntegrityLevel, sizeof(TOKEN_MANDATORY_LABEL) + ::GetLengthSid(&ldSID)) == FALSE)
			break;

	} while (0);

	SAFE_CLOSE_HANDLE(lhCurrentToken);

	return lhPrimaryToken;
}

//获取当前用户SDK,返回的字符串在不用的时候调用LocalFree释放
wchar_t* CProcessStarter::GetUserSID(int niSessionID)
{
	wchar_t* lpszSID = NULL;
	DWORD ldwSessionId = 0;
	HANDLE lhCurrentToken = NULL;
	PTOKEN_USER lpUserInfo = NULL;

	do
	{
		if (niSessionID == -1)
			ldwSessionId = GetSessionID();
		else
			ldwSessionId = niSessionID;

		if (ldwSessionId <= 0)
			break;

		//获取当前用户的权限
		if (!WTSQueryUserToken(ldwSessionId, &lhCurrentToken)
			|| (NULL == lhCurrentToken))
			break;

		DWORD ldwSize = 0;
		if (!GetTokenInformation(lhCurrentToken, TokenUser, NULL, ldwSize, &ldwSize))
		{
		}

		lpUserInfo = (PTOKEN_USER)GlobalAlloc(GPTR, ldwSize);

		if (!GetTokenInformation(lhCurrentToken, TokenUser, lpUserInfo,
			ldwSize, &ldwSize))
		{
			break;
		}

		ConvertSidToStringSid(lpUserInfo->User.Sid, &lpszSID);

	} while (false);

	if (lpUserInfo != NULL)
		GlobalFree(lpUserInfo);
	SAFE_CLOSE_HANDLE(lhCurrentToken);

	return lpszSID;
}
#include <stdio.h>
//如果nbIsSystem=true则直接使用system用户启动
//npszCommndLine参数最前面一定要是一个空格，否则无法成功传入
DWORD CProcessStarter::Run(const wchar_t* pcProcessPathName, HANDLE& rProHandle, int niSessionID, wchar_t* npszCommndLine)
{
	DWORD ldwProID = 0;
	BOOL bTmp = FALSE;
	HANDLE lhPrimaryToken = NULL;
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION processInfo = { 0 };
	LPVOID lpEnvironment = NULL;

	do
	{
		if (NULL == pcProcessPathName)
			break;

		if (niSessionID != 0)
		{
			//需要以当前用户权限启动
			lhPrimaryToken = GetCurrentUserToken();
			if (NULL == lhPrimaryToken)
				break;
			if (!CreateEnvironmentBlock(&lpEnvironment, lhPrimaryToken, FALSE))
			{

			}
		}


		StartupInfo.cb = sizeof(STARTUPINFO);

		bTmp = CreateProcessAsUser(
			lhPrimaryToken,
			pcProcessPathName,
			npszCommndLine,
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
			lpEnvironment, // __in_opt    LPVOID lpEnvironment,
			0,
			&StartupInfo,
			&processInfo);

		if (NULL != lpEnvironment)
			DestroyEnvironmentBlock(lpEnvironment);


		if (!bTmp)
			break;

		ldwProID = processInfo.dwProcessId;
		rProHandle = processInfo.hProcess;

	} while (0);

	SAFE_CLOSE_HANDLE(lhPrimaryToken);

	return ldwProID;
}
