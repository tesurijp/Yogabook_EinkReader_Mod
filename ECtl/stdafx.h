/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
//#include "compile.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "strsafe.h"

// TODO: reference additional headers your program requires here
#ifdef _DEBUG
#define PrintDebugString(_X) OutputDebugString(_X)
#else
#define PrintDebugString(_X) 
#endif


