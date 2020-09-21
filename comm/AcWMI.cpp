#include "stdafx.h"
#include "AcWMI.h"

CAccessWmi::CAccessWmi()
{
}

CAccessWmi::~CAccessWmi()
{
	ReleaseWMI();
}

HRESULT	CAccessWmi::ConnectServer(
	const CString&	csNamespace,
	const CString&	csUsername,
	const CString&	csPassword)
{
	ReleaseWMI();

	HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		(LPVOID *)&m_pWmiLoc);

	if (!SUCCEEDED(hres)) {
		return hres;
	}

	m_strNameSpace = csNamespace;
	CComBSTR bstrNamespace(csNamespace);
	// connect server
	if (csPassword.IsEmpty() || csUsername.IsEmpty()) {
		hres = m_pWmiLoc->ConnectServer(bstrNamespace,
			NULL, NULL,
			0, NULL, 0, 0,
			&m_pWmiServ);
	}
	else {
		// using password and username
		CComBSTR bstrUsername(csUsername), bstrPassword(csPassword);

		hres = m_pWmiLoc->ConnectServer(bstrNamespace,
			bstrUsername, bstrPassword,
			0, NULL, 0, 0,
			&m_pWmiServ);
	}

	if (SUCCEEDED(hres)) {
		hres = CoSetProxyBlanket(m_pWmiServ,
			RPC_C_AUTHN_WINNT,
			RPC_C_AUTHZ_NONE,
			NULL,
			RPC_C_AUTHN_LEVEL_CALL,
			RPC_C_IMP_LEVEL_IMPERSONATE,
			NULL,
			EOAC_NONE);
	}

	if (!SUCCEEDED(hres)) {
		ReleaseWMI();
	}
	return hres;
}

HRESULT	CAccessWmi::EnumInstances(
	const CString& csInstName,
	OUT IEnumWbemClassObject**	ppiWmiEnum)
{
	//ASSERT(m_pWmiServ);
	if (NULL == ppiWmiEnum)
		return E_INVALIDARG;

	CComBSTR	bstrObjectName(csInstName);
	return m_pWmiServ->CreateInstanceEnum(bstrObjectName,
		WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
		NULL,
		ppiWmiEnum);
}

HRESULT	CAccessWmi::QueryInstances(
	const CString& csQuery,
	OUT IEnumWbemClassObject** ppiWmiEnum)
{
	//ASSERT(m_pWmiServ);
	if (NULL == ppiWmiEnum)
		return E_INVALIDARG;

	CComBSTR	bstrLanguage(L"WQL");
	CComBSTR	bstrQuery(csQuery);
	return m_pWmiServ->ExecQuery(bstrLanguage, bstrQuery,
		WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
		NULL,
		ppiWmiEnum);
}

HRESULT	CAccessWmi::ExecMethodWMI(
	OUT IWbemClassObject** ppiWmiOut,
	const CString& csMethodName,
	const CString& csClassName,
	const CString& csObjectPath,
	const vector<CString>& csarrPropNames,
	IN VARIANT *arrVarInArg,
	const int size)
{
	//OutputDebugStringA("111111111111111111111");
	/*if (NULL == ppiWmiOut || size && (csarrPropNames.size() != size))
		return E_INVALIDARG;*/
		//OutputDebugStringA("22222222222222");
		//ASSERT(m_pWmiServ);
		//ASSERT(*ppiWmiOut==NULL);

	CComPtr<IWbemClassObject>	pClassObj;
	CComPtr<IWbemClassObject>	pInClass;
	CComPtr<IWbemClassObject>	pInParam;

	CComBSTR	bstrMethodName(csMethodName);
	CComBSTR	bstrClassName(csClassName);
	CComBSTR	bstrObjectPath(csObjectPath);

	HRESULT	hres;
	hres = m_pWmiServ->GetObject(bstrClassName, 0, NULL, &pClassObj, NULL);
	TCHAR tp[200] = { '\0' };

	if (WBEM_S_NO_ERROR == hres)
	{
		//	get the input-argument class object and create an instance.
		//	pInClass == NULL indicates that no input parameters needed.
		//OutputDebugStringA("3333333333333333");
		hres = pClassObj->GetMethod(bstrMethodName, 0, &pInClass, NULL);

		if (WBEM_S_NO_ERROR != hres)
		{
			ZeroMemory(tp, sizeof(tp));
			wsprintfW(tp, L"[SSR] ==>%04d:  pClassObj->GetMethod() hres = 0x%x", __LINE__, hres);
			OutputDebugString(tp);
		}
		//OutputDebugStringA("4444444444444444");
		if (WBEM_S_NO_ERROR == hres)
		{
			if (pInClass != NULL)
			{
				//OutputDebugStringA("555555555555555");
				//	create instance copy
				if (WBEM_S_NO_ERROR == (hres = pInClass->SpawnInstance(0, &pInParam)))
				{
					// set each property
					for (long i = 0; i < size; ++i)
					{
						CComBSTR bstrPropName(csarrPropNames.at(i));
						hres = pInParam->Put(bstrPropName, 0, &arrVarInArg[i], 0);

						//	Put failed, check the properties and their types
						//if (WBEM_S_NO_ERROR != hres)
						{
							ZeroMemory(tp, sizeof(tp));
							//wsprintfW(tp,L"[SSR] ==>%04d:  pInParam->Put() %ws 0x%x hres = 0x%x", __LINE__, bstrPropName.m_str,arrVarInArg[i].cVal, hres);
							//OutputDebugString(tp);

							break;
						}
					}
				}
				else {
					ZeroMemory(tp, sizeof(tp));
					//wsprintfW(tp, L"[SSR] ==>%04d:  pInClass->SpawnInstance() hres = 0x%x", __LINE__, hres);
					//OutputDebugString(tp);
				}


			}
			//	finally call the method
			if (WBEM_S_NO_ERROR == hres)
			{
				//OutputDebugStringA("666666666666666");
				hres = m_pWmiServ->ExecMethod(bstrObjectPath, bstrMethodName, 0, NULL, NULL, NULL, NULL);
				//hres = m_pWmiServ->ExecMethod(bstrClassName, bstrMethodName, 0, NULL, pInParam, ppiWmiOut, NULL);
				if (WBEM_NO_ERROR != hres)
				{
					//OutputDebugStringA("777777777777777");
					ZeroMemory(tp, sizeof(tp));
					//wsprintfW(tp, L"[SSR] ==>%04d:  m_pWmiServ->ExecMethod() %s  %s   hres = 0x%x", __LINE__, csObjectPath.GetString(), csMethodName.GetString(), hres);
					//OutputDebugString(tp);
				}
				else
				{
					//OutputDebugStringA("888888888888");
				}
			}
		}
	}

#ifdef DEBUG
	if (!SUCCEEDED(hres))
	{
		CString		cStrErr;

		GetLastErrorWMI(hres, cStrErr);
		ZeroMemory(tp, sizeof(tp));
		//wsprintf(tp, _T("[SSR] ==>%04d:  ExecMethodWMI__GetErrorInfo :%s"), __LINE__, cStrErr);
		//OutputDebugString(tp);
//		AfxMessageBox(cStrErr, MB_OK);
	}
	else if (*ppiWmiOut != NULL)
	{
		CComBSTR	bstrResult;
		hres = (*ppiWmiOut)->GetObjectText(0, &(bstrResult.m_str));

		USES_CONVERSION;
		ZeroMemory(tp, sizeof(tp));
		//wsprintf(tp, _T("[SSR] ==>%04d:  ExecMethodWMI__GetErrorInfo :%s"), __LINE__, OLE2T(bstrResult));
		//OutputDebugString(tp);
		//AfxMessageBox(OLE2T(bstrResult), MB_OK);
	}
#endif

	return hres;
}

void CAccessWmi::ReleaseWMI()
{
	//	release IWeb interfaces
	m_pWmiLoc = NULL;
	m_pWmiServ = NULL;

	m_strNameSpace = "";
}

void CAccessWmi::GetLastErrorWMI(HRESULT hresErr, OUT CString& cStrErr)
{
	CComPtr<IWbemStatusCodeText> pStatus;

	if (SUCCEEDED(CoCreateInstance(CLSID_WbemStatusCodeText, 0, CLSCTX_INPROC_SERVER, IID_IWbemStatusCodeText, (LPVOID *)&pStatus))) {
		CComBSTR bstrError;
		pStatus->GetErrorCodeText(hresErr, 0, 0, &bstrError);

		USES_CONVERSION;
		cStrErr = OLE2T(bstrError);
	}
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
HRESULT	EnumInstPropNameWMI(IN IWbemClassObject* piappObj,
	OUT LPSAFEARRAY* ppsarProp)
{
	if (NULL == ppsarProp || NULL == piappObj)
		return E_INVALIDARG;

	//	GetNames methods will create SAFEARRAY, 
	//	but on entry this parameter must point to NULL
	if (NULL != *ppsarProp)
	{
		SafeArrayDestroy(*ppsarProp);
		//delete *ppsarProp;     //change by xingej1
		*ppsarProp = NULL;

		if (NULL == ppsarProp)
			return E_INVALIDARG;
	}

	HRESULT hres;
	hres = piappObj->GetNames(NULL,
		WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY,
		NULL,
		ppsarProp);
	return hres;
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
HRESULT	EnumInstPropNameWMI(IN IWbemClassObject* piappObj,
	OUT vector<CString>& psarPropRef)
{
	HRESULT		hres;
	SAFEARRAY*	pSafeArrProp = NULL;

	psarPropRef.clear();

	hres = EnumInstPropNameWMI(piappObj, &pSafeArrProp);

	if (WBEM_S_NO_ERROR != hres)
		return hres;

	long	lLower, lUpper;
	SafeArrayGetLBound(pSafeArrProp, 1, &lLower);
	SafeArrayGetUBound(pSafeArrProp, 1, &lUpper);

	for (long i = lLower; i <= lUpper; ++i)
	{
		CComBSTR	bstrPropName;

		if (S_OK != (hres = SafeArrayGetElement(pSafeArrProp, &i, &bstrPropName)))
		{
			if (NULL != pSafeArrProp)
				SafeArrayDestroy(pSafeArrProp);
			return hres;
		}

		USES_CONVERSION;
		psarPropRef.push_back(OLE2T(bstrPropName));
	}

	if (NULL != pSafeArrProp)
		SafeArrayDestroy(pSafeArrProp);

	return hres;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
HRESULT	GetClassMethodsWMI(IN IWbemClassObject* piappObj,
	OUT vector<CString>&	csarrMethods)
{

	if (NULL == piappObj)
		return E_INVALIDARG;

	csarrMethods.clear();

	USES_CONVERSION;

	HRESULT	hres;
	hres = piappObj->BeginMethodEnumeration(0);

	while (WBEM_S_NO_ERROR == hres)
	{
		CComBSTR	bstrMethodName;
		hres = piappObj->NextMethod(0, &bstrMethodName, NULL, NULL);

		if (WBEM_S_NO_ERROR == hres)
			csarrMethods.push_back(OLE2T(bstrMethodName));
	}

	return piappObj->EndMethodEnumeration();
}
