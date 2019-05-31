/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "ErInputHold.h"


CErInputHold::CErInputHold(ULONG nuInterval)
{
	muInterval = nuInterval;
	muRecentTick = 0;
}


CErInputHold::~CErInputHold()
{
}

bool CErInputHold::InputIntervalTest(void)
{
	ULONG luCurrent = GetTickCount();

	if (luCurrent - muRecentTick < muInterval)
		return false;

	muRecentTick = luCurrent;
	return true;
}
