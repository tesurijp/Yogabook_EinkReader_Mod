/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
#include "Gdiplus.h"

class CGdipStart
{
public:
	ULONG_PTR gdipToken;
	CGdipStart();
	~CGdipStart();

	bool Init();
	void UnInit();
};

