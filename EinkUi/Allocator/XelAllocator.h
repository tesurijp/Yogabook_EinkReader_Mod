/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _EOB_OBJECT_MGR_
#define _EOB_OBJECT_MGR_

class CXelDataMgr;
class CXelFactoryMgr;

DECLARE_BUILTIN_NAME(CXelAllocator)
class CXelAllocator : public cmmBaseObject<CXelAllocator, IXelAllocator, GET_BUILTIN_NAME(CXelAllocator)>
{
public:
	CXelAllocator();
	virtual ~CXelAllocator();
	
	// 初始化
	ULONG InitOnCreate(const wchar_t* nswRegPath);

	DEFINE_CUMSTOMIZE_CREATE(CXelAllocator,(const wchar_t* nswRegPath),(nswRegPath))

public:
	// 从配置文件中创建对象
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,	// 父对象指针
		IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
		IN ULONG nuEID=MAXULONG32		// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);

	// 通过类名，创建对象,所创建的对象不能携带参数
	virtual IEinkuiIterator* __stdcall CreateElement(
		IN IEinkuiIterator* npParent,		// 父对象指针
		IN const wchar_t* nswClassName,		// 类名
		IN ULONG nuEID=MAXULONG32	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
		);



	// 加载一幅图片文件
	virtual IEinkuiBitmap* __stdcall LoadImageFile(
		IN wchar_t* nswRelativePathName,		//该值为相对路径时，起点路径为该模块Dll所在目录
		IN bool nbIsFullPath = false			//该的真表示nswRelativePathName为全路径，否则为相对路径
		);

	// 从指定的PE文件中，提取位图资源，生成一个位图对象
	virtual IEinkuiBitmap* __stdcall LoadImageFromPE(
		IN wchar_t *npPeFileName,				// 指定PE文件全路径
		IN int dwDummy,						// 资源索引
		IN int niXSize = 64,					// 指定目标图标的宽度
		IN int niYSize = 64						// 指定目标图标的高度
		);

	//从文字生成图片
	virtual IEinkuiBitmap* __stdcall CreateImageByText(STETXT_BMP_INIT& rdBmpInit);

	// 指定大小，以及位图数据，创建一个位图对象，程序返回后npRawData即可有调用者释放
	virtual IEinkuiBitmap* __stdcall CreateBitmapFromMemory(
		LONG nuWidth,					// 位图宽度
		LONG nuHeight,					// 位图高度
		LONG PixelSize,					// 像素的位宽，3 or 4
		LONG Stride,
		void* npRawData				// 位图原始数据
		);

public:
	// 提供给XUI主模块的方法，用于获取指定DLL输出的工厂类接口
	IElementFactory* LoadFactory(/*const wchar_t* nswRelativePathName*/);

protected:
	IElementFactory* PrepareAllocator(const wchar_t* nswClsName);

	CXelDataMgr*	mopDataMgr;
	CXelFactoryMgr*	mopFactoryMgr;
	
	LONG mlTotalBmp;

};


#endif