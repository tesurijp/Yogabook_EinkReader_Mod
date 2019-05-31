/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


// ObjectMgr.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "CommonHeader.h"

#include "XelAllocator.h"


DEFINE_BUILTIN_NAME(CXelAllocator)
CXelAllocator::CXelAllocator()
{
	mlTotalBmp = 0;

}


CXelAllocator::~CXelAllocator()
{
	if (mopDataMgr != NULL)
		mopDataMgr->Release();
	
	if (mopFactoryMgr != NULL)
		mopFactoryMgr->Release();

}


ULONG CXelAllocator::InitOnCreate(const wchar_t* nswRegPath)
{
	mopDataMgr = NULL;
	mopFactoryMgr = NULL;

	// 获取配置管理器对象	
	mopDataMgr = CXelDataMgr::SingleInstance(nswRegPath);
	if (mopDataMgr == NULL)
		return 0x80000000;
	
	mopFactoryMgr = CXelFactoryMgr::CreateInstance();
	if (mopFactoryMgr == NULL)
		return 0x80000000;	

	return 0;

}

// 提供给XUI主模块的方法，用于获取指定DLL输出的工厂类接口
IElementFactory* CXelAllocator::LoadFactory(
	/*const wchar_t* nswRelativePathName*/
	)
{
	IElementFactory* lpElementFact = NULL;
	CFilePathName loPath;
	const wchar_t*	lswWidgetPath = NULL;

	lswWidgetPath = EinkuiGetSystem()->GetCurrentWidget()->GetWidgetDefaultPath();
	if (lswWidgetPath == NULL)
		return NULL;

	loPath = lswWidgetPath;
	loPath.AssurePath();
	loPath.Transform(EinkuiGetSystem()->GetCurrentWidget()->GetModuleName());


	lpElementFact = mopDataMgr->HasFactoryBeenLoaded(loPath.GetPathName());
	if (lpElementFact == NULL)
	{
		SetDllDirectory(lswWidgetPath);

		lpElementFact = mopFactoryMgr->GetFactInstance(loPath.GetPathName());

		SetDllDirectory(NULL);
	}

	return lpElementFact;
}


IElementFactory* CXelAllocator::PrepareAllocator(
	const wchar_t* nswClsName
	)
{

	IElementFactory* lpELementFact = NULL;
	wchar_t lwszDllPath[MAX_PATH] = {0};
	REGSTATUS lStateEnum;

	// 查看是否注册 
	lStateEnum = mopDataMgr->IsRegisted(nswClsName, lwszDllPath, &lpELementFact);

	// 没有注册，返回NULL
	if (lStateEnum == Invalid)
		return NULL;
	
	// LOAD情况下，会返回一个有效的IElementFactory接口指针
	if (lStateEnum == LOAD)
		return lpELementFact;

	// UNLOAD,没有加载，则调用工厂管理类方法获获取工厂类接口
	return mopFactoryMgr->GetFactInstance(lwszDllPath);


}


// 加载一幅图片文件
IEinkuiBitmap* __stdcall CXelAllocator::LoadImageFile( 
	IN wchar_t* nswRelativePathName,		//该值为相对路径时，起点路径为该模块Dll所在目录
	IN bool nbIsFullPath					//该的真表示nswRelativePathName为全路径，否则为相对路径
	)
{
	// 获取当前widget所在的路径，间接获取ImageFile的FullPath
	const wchar_t*	lswWidgetPath = NULL;
	lswWidgetPath = EinkuiGetSystem()->GetCurrentWidget()->GetWidgetDefaultPath();
	if (lswWidgetPath == NULL)
		return NULL;

	CFilePathName	lopFilePath(lswWidgetPath);
	lopFilePath.AssurePath();
	lopFilePath.Transform(nswRelativePathName, false);

	// 加载ImageFile,返回接口指针
	IEinkuiBitmap* lpElBitmap = CXD2dBitmap::CreateInstance(nbIsFullPath==false?lopFilePath.GetPathName():nswRelativePathName);
	if (lpElBitmap == NULL)
		return NULL;
	
	InterlockedIncrement(&mlTotalBmp);
	//CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(lpElBitmap); //在位图建立处注册 ax nov.28,17
	
	return lpElBitmap;

}


// 从指定的PE文件中，提取位图资源，生成一个位图对象
IEinkuiBitmap* __stdcall CXelAllocator::LoadImageFromPE( 
	IN wchar_t *npPeFileName,
	IN int niDummy,
	IN int niXSize,
	IN int niYSize
	)
{
	if (NULL == npPeFileName)
		return NULL;

	IEinkuiBitmap* lpElBitmap = CXD2dBitmap::CreateInstance(npPeFileName,niDummy, niXSize, niYSize);
	if (lpElBitmap == NULL)
		return NULL;

	InterlockedIncrement(&mlTotalBmp);
	//CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(lpElBitmap); //在位图建立处注册 ax nov.28,17

	return lpElBitmap;

}


//从文字生成图片
IEinkuiBitmap* __stdcall CXelAllocator::CreateImageByText(STETXT_BMP_INIT& rdBmpInit)
{
	IEinkuiBitmap* lpElBitmap = CXuiTextBitmap::CreateInstance(rdBmpInit);
	if (lpElBitmap == NULL)
		return NULL;

	InterlockedIncrement(&mlTotalBmp);
	//CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(lpElBitmap); //在位图建立处注册 ax nov.28,17

	return lpElBitmap;
}


// 指定大小，以及位图数据，创建一个位图对象，程序返回后npRawData即可有调用者释放
IEinkuiBitmap* __stdcall CXelAllocator::CreateBitmapFromMemory(
	LONG nuWidth,					// 位图宽度
	LONG nuHeight,					// 位图高度
	LONG PixelSize,					// 像素的位宽，3 or 4
	LONG Stride,
	void* npRawData				// 位图原始数据
	)
{
	IEinkuiBitmap* lpBitmap = NULL;
	do 
	{
		BREAK_ON_NULL(npRawData);

		lpBitmap = CXD2dBitmap::CreateInstance(nuWidth, nuHeight,PixelSize,Stride,npRawData);
		BREAK_ON_NULL(lpBitmap);

		InterlockedIncrement(&mlTotalBmp);
		//CEinkuiSystem::gpXuiSystem->GetBitmapList().RegisteBitmap(lpBitmap); //在位图建立处注册 ax nov.28,17

	} while (false);

	return lpBitmap;

}

// 从配置文件中创建对象
IEinkuiIterator* __stdcall CXelAllocator::CreateElement( 
	IN IEinkuiIterator* npParent, 
	IN ICfKey* npTemplete,
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	wchar_t lswClsName[MAX_PATH] ={0};
	IElementFactory*	lpClsFactory = NULL;
	IEinkuiIterator*			lpIterator = NULL;
	int liStatus;

	// 获取要创建的对象类名
	liStatus = npTemplete->GetValue(lswClsName, MAX_PATH*sizeof(wchar_t));
	if (liStatus <= 0)
		return NULL;
	
	// 首先尝试调用当前widget的工厂类接口创建对象
	lpClsFactory = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory();
	if (lpClsFactory != NULL)
	{
		lpIterator = lpClsFactory->CreateElement(npParent, npTemplete,nuEID);
		if (lpIterator != NULL)
			return lpIterator;
	}

	lpClsFactory = PrepareAllocator(lswClsName);
	if (lpClsFactory == NULL)
		return NULL;

	// 创建对象
	return lpClsFactory->CreateElement(npParent, npTemplete,nuEID);


}


// 通过类名，创建对象
IEinkuiIterator* __stdcall CXelAllocator::CreateElement(
	IN IEinkuiIterator* npParent,		// 父对象指针
	IN const wchar_t* nswClassName,		// 类名
	IN ULONG nuEID					// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	IElementFactory*	lpClsFactory = NULL;
	IEinkuiIterator*			lpIterator = NULL;
	ERESULT luResult = ERESULT_UNSUCCESSFUL;


	do 
	{
		BREAK_ON_NULL(nswClassName);

		// 首先尝试调用当前widget的工厂类接口创建对象
		lpClsFactory = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory();
		BREAK_ON_NULL(lpClsFactory);

		lpIterator = lpClsFactory->CreateElement(npParent, nswClassName,nuEID);
		BREAK_ON_NULL(lpIterator);

		luResult = ERESULT_SUCCESS;

	} while (false);



	do 
	{	// 检测，如果默认接口创建不成功，则查找注册的类，重新加载对应的工厂类，进行创建
		if (luResult == ERESULT_SUCCESS)
			break;
		
		lpClsFactory = PrepareAllocator(nswClassName);
		BREAK_ON_NULL(lpClsFactory);
			
		// 创建对象
		lpIterator = lpClsFactory->CreateElement(npParent, nswClassName,nuEID);

	} while (false);

	return lpIterator;
}

