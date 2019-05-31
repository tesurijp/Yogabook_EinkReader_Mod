/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"

#include "XsBmpList.h"


int gliCount=0; // ??? for test

void CXsBmpList::RegisteBitmap(IEinkuiBitmap* npBmpIntf)
{
	gliCount++;
	moLock.Enter();

	moBitmaps.Insert(npBmpIntf);

	moLock.Leave();
}

void CXsBmpList::UnregisteBitmap(IEinkuiBitmap* npBmpIntf)
{
	moLock.Enter();

	moBitmaps.Remove(npBmpIntf);

	moLock.Leave();
}

void CXsBmpList::ReleaseDeviceResource(void)
{
	moLock.Enter();

	TEBitmapTree::iterator itr;

	for(itr = moBitmaps.Begin();itr != moBitmaps.End();itr++)
	{
		if ((*itr) != NULL)
		{
			__try {
				(*itr)->DiscardsBitmapResource();
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
			}
		}
	}

	moLock.Leave();
}

