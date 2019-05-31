/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"

#include "XsBrushList.h"


void CXsBrushList::RegisteBrush(IEinkuiBrush* npBrushIntf)
{

	moLock.Enter();

	moBrushes.Insert(npBrushIntf);

	moLock.Leave();
}

void CXsBrushList::UnregisteBrush(IEinkuiBrush* npBrushIntf)
{
	moLock.Enter();

	moBrushes.Remove(npBrushIntf);

	moLock.Leave();
}


void CXsBrushList::ReleaseDeviceResource(void)
{
	moLock.Enter();

	TEBrushTree::iterator itr;

	for(itr = moBrushes.Begin();itr != moBrushes.End();itr++)
	{
 		if((*itr)!=NULL)
 			(*itr)->DiscardsBrushResource();
	}

	moLock.Leave();
}

