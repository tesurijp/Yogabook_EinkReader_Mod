// EInkPngToPdf.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#pragma once

#include "pch.h"
#include "EdPdf.h"

int wmain(int argc, wchar_t **argv)
{
	if (argc == 3)
	{
		EdCreatePdf(*(argv + 1), *(argv + 2));
	}

	return 0;
}
