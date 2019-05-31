/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

class CErInputHold
{
public:
	CErInputHold(ULONG nuInterval);
	~CErInputHold();

	bool InputIntervalTest(void);
protected:
	ULONG muRecentTick;
	ULONG muInterval;
};

