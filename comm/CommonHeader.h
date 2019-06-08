/* Copyright 2019 - present Lenovo */
/* License: COPYING.GPLv3 */



// 这里统一包含引擎所需的所有头文件
#include "gdiplus.h"
using namespace Gdiplus;

#include <d2d1.h>
#include <d3d10_1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <ShellAPI.h>

#include <d3d10_1.h>
//#include <d3dx10math.h>
using namespace D2D1;


#include "resource.h"

// 系统相关
#include "Shlobj.h"

// 状态定义
#include "EResult.h"

// 通用基类
#include "CfgIface.h"
#include "cmmBaseObj.h"

// 通用数据接口
#include "BpTree.h"
#include "cmmstruct.h"
#include "cmmPath.h"
#include "graphy/XgBitArray.h"

// 痕迹追踪
//#include "cmmTrace.h"

// 接口头文件
#include "Einkui.h"


#include "XsWgtContext.h"
#include "XsWidgetImp.h"
#include "XsBmpList.h"
#include "XsBrushList.h"
#include "einkuiimp.h"
#include "ElementImp.h"
#include "XleIteratorImp.h"
#include "XsMsgImp.h"
#include "XsMsgQueue.h"
#include "XleMgrImp.h"

#include "./Graphy/XgD2DBitmap.h"
#include "./Graphy/XgD2DBrush.h"
#include "./Graphy/XgD2DEngine4Eink.h"

#include "graphy/XgTextBitmapImp.h"

#include "./Allocator/XelFactoryMgr.h"
#include "./Allocator/XelDataMgr.h"
#include "./Allocator/XelAllocator.h"



// 
#include "ThreadAbort.h"
#include "Xuix.h"
