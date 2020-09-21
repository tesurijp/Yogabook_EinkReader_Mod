#pragma once


#include <Wbemidl.h>
#include <atlstr.h>
#include <vector>

#pragma comment(lib, "WbemUuid.Lib")
using namespace std;

class CAccessWmi
{
public:
	CAccessWmi();
	~CAccessWmi();

	HRESULT ConnectServer(const CString&	csNamespace,
		const CString&	csUsername,
		const CString&	csPassword);

	HRESULT EnumInstances(const CString& csInstName,
		OUT IEnumWbemClassObject**	ppiWmiEnum);

	HRESULT QueryInstances(const CString& csQuery,
		OUT IEnumWbemClassObject**	ppiWmiEnum);

	HRESULT ExecMethodWMI(OUT IWbemClassObject** ppiWmiOut,
		const CString& csMethodName,
		const CString& csClassName,
		const CString& csObjectPath,
		const vector<CString>& csarrPropNames,
		IN VARIANT *arrVarInArg,
		const int size);

	void	ReleaseWMI();

	static
		void	GetLastErrorWMI(HRESULT hresErr, OUT CString& cStrErr);

public:
	CString					m_strNameSpace;

	//protected:
	CComPtr<IWbemLocator>	m_pWmiLoc;
	CComPtr<IWbemServices>	m_pWmiServ;
};


HRESULT	EnumInstPropNameWMI(IN IWbemClassObject* piappObj,
	OUT vector<CString>& psarPropRef);

HRESULT	EnumInstPropNameWMI(IN IWbemClassObject* piappObj,
	OUT LPSAFEARRAY* psarProp);

HRESULT	GetClassMethodsWMI(IN IWbemClassObject* piappObj,
	OUT vector<CString>&	csarrMethods);

