/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
/*
File name: ConfigX.h


*/

#include "CfgIface.h"




class CCfgAssistant
{
public:
	// 复制一个键，仅复制这个键本身，不包括它的子键
	static ICfKey* DuplicateKey(ICfKey* npToDuplicate,ICfKey* npInsertTo){
		ICfKey* lpNewKey = NULL;
		wchar_t* lpName = NULL;
		UCHAR* lpDataBuf = NULL;
		int liName;
		int liDateLen;
		IConfigFile::VALUETYPE leVType;

		liDateLen = npToDuplicate->GetValueLength();
		leVType = npToDuplicate->GetValueType();

		if(leVType != IConfigFile::Invalid && liDateLen > 0)
		{
			lpDataBuf = new UCHAR[liDateLen];
			liDateLen = npToDuplicate->GetValue(lpDataBuf,liDateLen);
			if(liDateLen <= 0){
				delete lpDataBuf;
				lpDataBuf = NULL;
			}
		}

		liName = npToDuplicate->GetName(NULL,0);
		if(liName > 0)
		{
			lpName = new wchar_t[liName+1];
			if(lpName != NULL && npToDuplicate->GetName(lpName,liName+1)>0)
			{
				lpNewKey = npInsertTo->NewSubKey(lpName,leVType,lpDataBuf,liDateLen);
			}
		}
		else
		{
			ULONG luID = npToDuplicate->GetID(NULL);

			if(luID != 0)
			{
				lpNewKey = npInsertTo->NewSubKey(luID,false,leVType,lpDataBuf,liDateLen);
			}
		}
		CMM_SAFE_DELETE(lpDataBuf);
		CMM_SAFE_DELETE(lpName);

		return lpNewKey;
	}

	// 复制一个键内的全部内容到另外一个键，包括子键的子键
	static bool CopyContent(ICfKey* npSrcKey,ICfKey* npDestKey){
		ICfKey* lpCrtSrc = NULL;
		ICfKey* lpCrtDst = NULL;
		bool lbResult;

		lpCrtSrc = npSrcKey->GetSubKey();
		while(lpCrtSrc != NULL)
		{
			lpCrtDst = DuplicateKey(lpCrtSrc,npDestKey);
			if(lpCrtDst == NULL)
				break;

			if(CopyContent(lpCrtSrc,lpCrtDst)==false)
				break;

			CMM_SAFE_RELEASE(lpCrtDst);
			lpCrtSrc = lpCrtSrc->MoveToNextKey();
		}

		lbResult = (lpCrtSrc == NULL);

		CMM_SAFE_RELEASE(lpCrtSrc);
		CMM_SAFE_RELEASE(lpCrtDst);

		return lbResult;
	}

	// 清空一个键的子键
	static void ClearSubKeys(ICfKey* npMarKey){
		ICfKey* lpSub;

		lpSub = npMarKey->GetSubKey();
		while(lpSub != NULL)
		{
			lpSub = lpSub->MoveToNextKey(true);
		}
	}

};
































