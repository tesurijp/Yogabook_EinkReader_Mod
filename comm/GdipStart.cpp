/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "windows.h"
#include "GdipStart.h"

using namespace Gdiplus;

#pragma comment(lib,"Gdiplus.lib")

CGdipStart::CGdipStart()
{
	gdipToken = NULL;
}


CGdipStart::~CGdipStart()
{
	if (gdipToken != NULL)
		UnInit();
}

bool CGdipStart::Init()
{
	GdiplusStartupInput gdipStart;

	if (gdipToken != NULL)
		return false;

	GdiplusStartup(&gdipToken, &gdipStart, NULL);

	return true;
}

void CGdipStart::UnInit()
{
	GdiplusShutdown(gdipToken);
	gdipToken = NULL;
}
