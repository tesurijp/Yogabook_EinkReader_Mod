#include "stdafx.h"
#include "BCoverCtrl.h"
#include <Shellapi.h>
#include "EinkIteAPI.h"
#include "EinkInternal.h"

HWND ghWndMsg = NULL;
DWORD gdwEventTicketCount = 0;

BOOL EnablePrivilege(LPTSTR PrivilegeName)
{
	HANDLE hProc = NULL, hToken = NULL;
	TOKEN_PRIVILEGES TP;
	hProc = GetCurrentProcess(); //打开当前进程的一个伪句柄

								 //打开进程访问令牌，hToken表示新打开的访问令牌标识
	if (!OpenProcessToken(hProc, TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
#ifdef DEBUGMSG
		printf("OpenProcessToken() GetLastError reports %d\n", erron);
#endif
		goto Close;
	}

	//提升权限
	if (!LookupPrivilegeValue(NULL, PrivilegeName, &TP.Privileges[0].Luid))
	{
#ifdef DEBUGMSG
		printf("LookupPrivilegeValue() GetLastError reports %d\n", erron);
#endif
		goto Close;
	}

	TP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	TP.PrivilegeCount = 1;

	//允许权限，主要根据TP这个结构
	if (!AdjustTokenPrivileges(hToken, FALSE, &TP, sizeof(TP), 0, 0))
	{
#ifdef DEBUGMSG
		printf("AdjustTokenPrivileges() GetLastError reports %d\n", erron);
#endif
		goto Close;
	}

Close:

	if (hProc != NULL)
		CloseHandle(hProc);

	if (hToken != NULL)
		CloseHandle(hToken);

	return FALSE;

	if (hProc != NULL)
		CloseHandle(hProc);

	if (hToken != NULL)
		CloseHandle(hToken);

	return TRUE;
}

EventSink::EventSink(HANDLE PenBtEvt) :m_PenBtEvt(PenBtEvt) {
	m_lRef = 0;
}

EventSink::~EventSink() {
	bDone = true;
}

ULONG EventSink::AddRef() {
	return InterlockedIncrement(&m_lRef);
}

ULONG EventSink::Release() {
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}

HRESULT EventSink::Indicate(LONG lObjectCount,
	IWbemClassObject **apObjArray) {
	HRESULT hres = S_OK;

	for (int i = 0; i < lObjectCount; i++) {
		TCHAR tp[200] = { '\0' };
		wsprintf(tp, TEXT("[SSR] ==>%d  PenButton Down Event\n"), __LINE__);
		OutputDebugString(tp);

		//关机
		SHELLEXECUTEINFO shellexecuteinfo;
		ZeroMemory(&shellexecuteinfo, sizeof(shellexecuteinfo));
		shellexecuteinfo.cbSize = sizeof(shellexecuteinfo);
		shellexecuteinfo.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		shellexecuteinfo.lpVerb = _T("open");
		shellexecuteinfo.lpFile = L"shutdown.exe";
		shellexecuteinfo.nShow = SW_SHOW;
		shellexecuteinfo.lpParameters = L" -s -t 60";

		BOOL bContinue = ShellExecuteEx(&shellexecuteinfo);
		//ExitWindowsEx(EWX_SHUTDOWN, EWX_FORCEIFHUNG);
		//MessageBox(NULL, TEXT("Hello, Windows!"), TEXT("hello"), MB_OK);

	}

	return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
) {
	if (lFlags == WBEM_STATUS_COMPLETE) {
		//printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		//printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}


EventResetTpArea::EventResetTpArea(HANDLE PenBtEvt) :m_PenResettpEvt(PenBtEvt) {
	m_lRef = 0;
}

EventResetTpArea::~EventResetTpArea() {
	bDone = true;
}

ULONG EventResetTpArea::AddRef() {
	return InterlockedIncrement(&m_lRef);
}

ULONG EventResetTpArea::Release() {
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventResetTpArea::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}

HRESULT EventResetTpArea::Indicate(LONG lObjectCount,
	IWbemClassObject **apObjArray) {
	HRESULT hres = S_OK;

	for (int i = 0; i < lObjectCount; i++) {
		TCHAR tp[200] = { '\0' };
		wsprintf(tp, TEXT("[SSR] ==>%d  EiResetTpArea Event\n"), __LINE__);
		OutputDebugString(tp);
		EiResetTpArea();
		//SendMessage(ghWndMsg, WM_BIOS_EVENT_RESET_TP_AREA, 0, 0);
		break;
	}

	return WBEM_S_NO_ERROR;
}

HRESULT EventResetTpArea::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
) {
	if (lFlags == WBEM_STATUS_COMPLETE) {
		//printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		//printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}



Event8951Wakeup::Event8951Wakeup(HANDLE PenBtEvt) :m_PenResettpEvt(PenBtEvt) {
	m_lRef = 0;
}

Event8951Wakeup::~Event8951Wakeup() {
	bDone = true;
}

ULONG Event8951Wakeup::AddRef() {
	return InterlockedIncrement(&m_lRef);
}

ULONG Event8951Wakeup::Release() {
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT Event8951Wakeup::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}

HRESULT Event8951Wakeup::Indicate(LONG lObjectCount,
	IWbemClassObject **apObjArray) {
	HRESULT hres = S_OK;

	for (int i = 0; i < lObjectCount; i++) {
		gdwEventTicketCount = GetTickCount();
		TCHAR tp[200] = { '\0' };
		wsprintf(tp, TEXT("[SSR] ==>%d  Event8951Wakeup Event\n"), __LINE__);
		OutputDebugString(tp);
		//EiResetTpArea();

		SendMessage(ghWndMsg, WM_BIOS_EVENT_8951_STATUS_CHANGE, 1, 0);
		break;
	}

	return WBEM_S_NO_ERROR;
}

HRESULT Event8951Wakeup::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
) {
	if (lFlags == WBEM_STATUS_COMPLETE) {
		//printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		//printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}


Event8951Sleep::Event8951Sleep(HANDLE PenBtEvt) :m_PenResettpEvt(PenBtEvt) {
	m_lRef = 0;
}

Event8951Sleep::~Event8951Sleep() {
	bDone = true;
}

ULONG Event8951Sleep::AddRef() {
	return InterlockedIncrement(&m_lRef);
}

ULONG Event8951Sleep::Release() {
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT Event8951Sleep::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}

HRESULT Event8951Sleep::Indicate(LONG lObjectCount,
	IWbemClassObject **apObjArray) {
	HRESULT hres = S_OK;

	for (int i = 0; i < lObjectCount; i++) {

		//EiResetTpArea();
		//if (GetTickCount() - gdwEventTicketCount < 50)
		//{
			//bool lbIs8951Sleep = false;
			//Ei8951IsSleep(lbIs8951Sleep);
			//if(lbIs8951Sleep != false) //这种情况只有8951真的处于sleep状态了，才发,否则不处理
			//	SendMessage(ghWndMsg, WM_BIOS_EVENT_8951_STATUS_CHANGE, 0, 0);

		//}
		//else
		//{
		//	SendMessage(ghWndMsg, WM_BIOS_EVENT_8951_STATUS_CHANGE, 0, 0);
		//}

		//bool lbIs8951Sleep = false;
		//Ei8951IsSleep(lbIs8951Sleep);
		//if (lbIs8951Sleep != false) //这种情况只有8951真的处于sleep状态了，才发,否则不处理
		SendMessage(ghWndMsg, WM_BIOS_EVENT_8951_STATUS_CHANGE, 0, 0);

		TCHAR tp[200] = { '\0' };
		wsprintf(tp, TEXT("[SSR] ==>%d  Event8951Sleep Event\n"), __LINE__);
		OutputDebugString(tp);


		break;
	}

	return WBEM_S_NO_ERROR;
}

HRESULT Event8951Sleep::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
) {
	if (lFlags == WBEM_STATUS_COMPLETE) {
		//printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		//printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}

BCoverCtrl::BCoverCtrl() {
	HRESULT hres = NOERROR;
	TCHAR tp[200] = { '\0' };
	::CoInitializeEx(NULL, COINIT_MULTITHREADED); // Initializes COM
	hres = CoInitializeSecurity(
		NULL, -1, NULL, NULL,
		RPC_C_AUTHN_LEVEL_CONNECT,
		RPC_C_IMP_LEVEL_IDENTIFY,
		NULL, EOAC_NONE, 0
	);
	if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  CoInitializeSecurity() Hres = %d"), __LINE__, hres);
		OutputDebugString(tp);
	}

	hres = m_AcWmi.ConnectServer(_T("root\\wmi"), L"", L"");
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  ConnectServer() Hres = %d"), __LINE__, hres);
		OutputDebugString(tp);

	}

	//initHighTempEvt();
	//InitBCoverMethod();

}

BCoverCtrl::~BCoverCtrl() {
	UninitBCoverMethod();
	m_AcWmi.ReleaseWMI();
	::CoUninitialize(); // Uninitialize COM

}

HRESULT BCoverCtrl::BackLightControl(BOOL DoEnable) {
	CComPtr<IWbemClassObject> pOutParams;
	HRESULT rt = WBEM_S_NO_ERROR;

	TCHAR tp[200] = { '\0' };


	if (DoEnable) {
		OutputDebugStringA("SetBackLightEnable");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("SetBackLightEnable"), mszName, m_strPath, vector<CString>(), NULL, 0);
	}
	else {
		OutputDebugStringA("SetBackLightDisable");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("SetBackLightDisable"), mszName, m_strPath, vector<CString>(), NULL, 0);
	}

	if (rt != WBEM_S_NO_ERROR) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%04d ExecMethodWMI err = 0x%x Path = %s\n"), __LINE__, rt, m_strPath.GetString());
		OutputDebugString(tp);

	}

	return rt;
}


//设置震动级别
HRESULT BCoverCtrl::SetHapticLevel(int niLevel)
{
	CComPtr<IWbemClassObject> pOutParams;
	HRESULT rt = WBEM_S_NO_ERROR;

	TCHAR tp[200] = { '\0' };
	vector<CString> ldParam;
	ldParam.push_back(L"parameter");
	VARIANT ldArray[1];
	VariantInit(&ldArray[0]);
	V_VT(&ldArray[0]) = VT_I4;
	V_I4(&ldArray[0]) = (unsigned long)niLevel;
	rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("HapticByteWrite"), _T("YB2_HapticControlWrite"), m_strPath, ldParam, ldArray, 1);

	VariantClear(&ldArray[0]);

	return rt;
}

HRESULT BCoverCtrl::TouchControl(BOOL DoEnable) {
	CComPtr<IWbemClassObject> pOutParams;
	HRESULT rt = WBEM_S_NO_ERROR;
	return rt; //暂时没有需要禁用TP的需求了
	TCHAR tp[200] = { '\0' };


	if (DoEnable) {
		OutputDebugStringA("SetTouchEnable");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("SetTouchEnable"), mszName, m_strPath, vector<CString>(), NULL, 0);
	}
	else {
		OutputDebugStringA("SetTouchDisable");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("SetTouchDisable"), mszName, m_strPath, vector<CString>(), NULL, 0);

	}

	if (rt != WBEM_S_NO_ERROR) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%04d ExecMethodWMI err = 0x%x Path = %s\n"), __LINE__, rt, m_strPath.GetString());
		OutputDebugString(tp);

	}

	return rt;
}

//设置A面knock knock功能
HRESULT BCoverCtrl::KnockKncokControl(BOOL DoEnable) {
	CComPtr<IWbemClassObject> pOutParams;
	HRESULT rt = WBEM_S_NO_ERROR;
	TCHAR tp[200] = { '\0' };


	if (DoEnable) {
		OutputDebugStringA("enableknock");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("enableknock"), mszName, m_strPath, vector<CString>(), NULL, 0);
	}
	else {
		OutputDebugStringA("disableknock");
		rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("disableknock"), mszName, m_strPath, vector<CString>(), NULL, 0);

	}

	if (rt != WBEM_S_NO_ERROR) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%04d ExecMethodWMI err = 0x%x Path = %s\n"), __LINE__, rt, m_strPath.GetString());
		OutputDebugString(tp);

	}

	return rt;
}

HRESULT BCoverCtrl::Reset8951() {
	CComPtr<IWbemClassObject> pOutParams;
	HRESULT rt = WBEM_S_NO_ERROR;

	TCHAR tp[200] = { '\0' };

	rt = m_AcWmi.ExecMethodWMI(&pOutParams, _T("ResetTcon"), mszName, m_strPath, vector<CString>(), NULL, 0);

	if (rt != WBEM_S_NO_ERROR) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%04d ExecMethodWMI err = 0x%x Path = %s\n"), __LINE__, rt, m_strPath.GetString());
		OutputDebugString(tp);

	}

	return rt;
}

//初始化reset tp area
HRESULT BCoverCtrl::initResetTpAreaEvt(HWND nhWnd)
{
	EnablePrivilege(SE_BACKUP_NAME);
	EnablePrivilege(SE_RESTORE_NAME);
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SHUTDOWN_NAME);

	HRESULT hres;
	TCHAR tp[200] = { '\0' };

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&m_pUnsecApp);

	m_EventResetTpArea = new EventResetTpArea(m_PenResettpEvt);
	m_EventResetTpArea->AddRef();

	hres = m_pUnsecApp->CreateObjectStub(m_EventResetTpArea, &m_pStubUnk);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()1 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	hres = m_pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&m_pStubSink);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()2 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs

	//	pSvc->ExecNotificationQueryAsync(
	hres = m_AcWmi.m_pWmiServ->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * FROM YB2NotifiyEvent"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		m_pStubSink);

	// Check for errors.
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()3 Hres = %x"), __LINE__, hres);
		OutputDebugString(tp);

		m_pUnsecApp->Release();
		m_pStubUnk->Release();
		m_pStubSink->Release();
		m_EventResetTpArea->Release();
	}

	ghWndMsg = nhWnd;

	return hres;
}

//初始化8951 sleep事件
HRESULT BCoverCtrl::init8951SleepEvt(HWND nhWnd)
{
	EnablePrivilege(SE_BACKUP_NAME);
	EnablePrivilege(SE_RESTORE_NAME);
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SHUTDOWN_NAME);

	HRESULT hres;
	TCHAR tp[200] = { '\0' };

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&m_pUnsecApp);

	m_Event8951Sleep = new Event8951Sleep(m_Pen8951SleepEvt);
	m_Event8951Sleep->AddRef();

	hres = m_pUnsecApp->CreateObjectStub(m_Event8951Sleep, &m_pStubUnk);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()1 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	hres = m_pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&m_pStubSink);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()2 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs

	//	pSvc->ExecNotificationQueryAsync(
	hres = m_AcWmi.m_pWmiServ->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * FROM YB2NotifiyEvent_8951Sleep"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		m_pStubSink);

	// Check for errors.
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()3 Hres = %x"), __LINE__, hres);
		OutputDebugString(tp);

		m_pUnsecApp->Release();
		m_pStubUnk->Release();
		m_pStubSink->Release();
		m_Event8951Sleep->Release();
	}

	ghWndMsg = nhWnd;

	return hres;
}

//初始化8951 wake up事件
HRESULT BCoverCtrl::init8951WakeupEvt(HWND nhWnd)
{
	EnablePrivilege(SE_BACKUP_NAME);
	EnablePrivilege(SE_RESTORE_NAME);
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SHUTDOWN_NAME);

	HRESULT hres;
	TCHAR tp[200] = { '\0' };

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&m_pUnsecApp);

	m_Event8951Wakeup = new Event8951Wakeup(m_PenResetCCoverEvt);
	m_Event8951Wakeup->AddRef();

	hres = m_pUnsecApp->CreateObjectStub(m_Event8951Wakeup, &m_pStubUnk);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()1 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	hres = m_pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&m_pStubSink);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()2 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs

	//	pSvc->ExecNotificationQueryAsync(
	hres = m_AcWmi.m_pWmiServ->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * FROM YB2NotifiyEvent_8951Wakeup"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		m_pStubSink);

	// Check for errors.
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()3 Hres = %x"), __LINE__, hres);
		OutputDebugString(tp);

		m_pUnsecApp->Release();
		m_pStubUnk->Release();
		m_pStubSink->Release();
		m_Event8951Wakeup->Release();
	}

	ghWndMsg = nhWnd;

	return hres;
}

HRESULT BCoverCtrl::initHighTempEvt(HWND nhWnd) {

	EnablePrivilege(SE_BACKUP_NAME);
	EnablePrivilege(SE_RESTORE_NAME);
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SHUTDOWN_NAME);

	HRESULT hres;
	TCHAR tp[200] = { '\0' };

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&m_pUnsecApp);

	m_EvtSink = new EventSink(m_PenBtEvt);
	m_EvtSink->AddRef();

	hres = m_pUnsecApp->CreateObjectStub(m_EvtSink, &m_pStubUnk);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()1 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	hres = m_pStubUnk->QueryInterface(IID_IWbemObjectSink,
		(void **)&m_pStubSink);
	ZeroMemory(tp, sizeof(tp));
	wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()2 Hres =0x %x"), __LINE__, hres);
	OutputDebugString(tp);

	// The ExecNotificationQueryAsync method will call
	// The EventQuery::Indicate method when an event occurs

	//	pSvc->ExecNotificationQueryAsync(
	hres = m_AcWmi.m_pWmiServ->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * FROM HighTempEvent"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		m_pStubSink);

	// Check for errors.
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d  InitPenBtEvt()3 Hres = %x"), __LINE__, hres);
		OutputDebugString(tp);

		m_pUnsecApp->Release();
		m_pStubUnk->Release();
		m_pStubSink->Release();
		m_EvtSink->Release();
	}

	ghWndMsg = nhWnd;

	return hres;
}

HRESULT BCoverCtrl::InitBCoverMethod() {
	HRESULT hres = NOERROR;
	CString strlog = L"";
	TCHAR tp[200] = { '\0' };

	CComPtr<IEnumWbemClassObject> pEnumClass;
	CComPtr<IWbemClassObject> pClass;
	CComPtr<IWbemClassObject> pOutParams;
	DWORD nReturned;
	VARIANT varPath;

	wcscpy_s(mszName, MAX_PATH, L"Lenovo_SmartpadCtrl");

	do
	{
		hres = m_AcWmi.EnumInstances(mszName, &pEnumClass);
		if (FAILED(hres)) {
			//@TODO report bug
			strlog.Format(L"%03d   m_AcWmi.EnumInstances", __LINE__);
			goto ErrClean;

		}

		hres = pEnumClass->Next(WBEM_INFINITE, 1, &pClass, &nReturned);
		if (hres != WBEM_S_NO_ERROR) {
			//@TODO report bug
			if (_wcsicmp(mszName, L"Lenovo_SmartpadCtrl_YB2") == 0)
				goto ErrClean; //新的也找不到

			wcscpy_s(mszName, MAX_PATH, L"Lenovo_SmartpadCtrl_YB2");
			hres = m_AcWmi.EnumInstances(mszName, &pEnumClass);
			continue;
		}

		hres = pClass->Get(CComBSTR(L"__PATH"), 0, &varPath, NULL, NULL);
		if (FAILED(hres)) {
			//@TODO report bug
			strlog.Format(L"%03d   pClass->Get(__PATH)", __LINE__);
			goto ErrClean;
		}

		break;

	} while (true);


	m_strPath = varPath.bstrVal;

ErrClean:
	if (FAILED(hres)) {
		ZeroMemory(tp, sizeof(tp));
		wsprintf(tp, TEXT("[SSR] ==>%d ERROR %s \n"), __LINE__, strlog.GetString());
		OutputDebugString(tp);

	}
	return hres;
}

HRESULT BCoverCtrl::UninitBCoverMethod() {
	m_AcWmi.ReleaseWMI();
	return WBEM_S_NO_ERROR;
}
