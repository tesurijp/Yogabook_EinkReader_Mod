/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#ifndef _EDDOC_H_
#define _EDDOC_H_
#include "cmmBaseObj.h"

// 默认定义
#ifndef IN
#define IN
#endif//IN
#ifndef OUT
#define OUT
#endif//OUT


//////////////////////////////////////////////////////////////////////////
// 前置声明，无需关注
//////////////////////////////////////////////////////////////////////////

__interface IEdModule; // 模块
__interface IEdDocument; // 文档
__interface IEdPage;	// 页
__interface IEdBitmap;	// 位图
__interface IEdAnnotManager;	// 注释管理器，每页都有自己的注释管理器
__interface IEdAnnot;				// 标注，这是基础接口，通过内部的接口来转换成实际类型的接口
__interface IEdInkAnnot;			// 墨迹注释
__interface IEdStextAnnot;		// 高亮/删除/下划线注释
__interface IEdTextAnnot;			// 文字标签注释
__interface IEdStructuredTextPage;	// 结构文本页，用于处理structured text信息 		
__interface IEdStextQuadList;		// 结构文本的结构信息
__interface IEdAnnotList;			// Annot列表对象

typedef IEdModule* IEdModule_ptr;
typedef IEdDocument* IEdDocument_ptr;
typedef IEdPage* IEdPage_ptr;
typedef IEdAnnotManager* IEdAnnotManager_ptr;
typedef IEdAnnot* IEdAnnot_ptr;
typedef IEdInkAnnot* IEdInkAnnot_ptr;
typedef IEdStextAnnot* IEdStextAnnot_ptr;
typedef IEdTextAnnot* IEdTextAnnot_ptr;
typedef IEdBitmap* IEdBitmap_ptr;
typedef IEdStextQuadList* IEdStextQuadList_ptr;
typedef IEdStructuredTextPage* IEdStructuredTextPage_ptr;
typedef IEdAnnotList* IEdAnnotList_ptr;


//////////////////////////////////////////////////////////////////////////
// 前置声明结束
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// 数据类型定义
//////////////////////////////////////////////////////////////////////////
typedef unsigned char bin;
typedef unsigned char* bin_ptr;
typedef unsigned long bool32;
typedef unsigned long uint32Eink;
typedef long int32Eink;
#ifndef _INTTYPE_CONFLICT	//$ax$ 此处和Sumatra库冲突，把它改名了，注意，这个EdDoc.h文件和EinkReader处的同名文件存在这个差异
#define uint32 uint32Eink
#define int32 int32Eink
#endif
typedef char* buf_ptr;
typedef wchar_t char16;
typedef wchar_t* char16_ptr;
typedef float float32;
typedef double float64;
typedef void* void_ptr;



#define false32 (int32Eink)		0
#define success32(int32Eink)	1
//////////////////////////////////////////////////////////////////////////
// 函数类型定义
//////////////////////////////////////////////////////////////////////////

typedef void(__stdcall *PEDDOC_CALLBACK)(uint32Eink loadingStep, uint32Eink pageCount, void_ptr npContext);
// loadingStep: 
//		0			start of loading
//		MAXULONG	end of loading
//		otherwise	loading

//typedef ERESULT(__stdcall IBaseObject::*PXUI_CALLBACK)(ULONG nuFlag, LPVOID npContext);
//typedef ERESULT(__stdcall *PXUI_CUSTOM_DRAW_CALLBACK)(unsigned char* npPixels, ULONG nuWidth, ULONG nuHeight, bool nbRefresh);//nbRefresh设置，则必须提交数据

typedef void(__stdcall *PEDDOC_THUMBNAILS_CALLBACK)(uint32Eink pageNum,void_ptr npContext);
// pageNum:
//		the page number that it's thumbnail have rendered.
// this routine will be called as many as the page count of the current document;


//////////////////////////////////////////////////////////////////////////
// 常数定义
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
// 返回值
//////////////////////////////////////////////////////////////////////////
typedef uint32Eink ED_ERR;

#define EDERR_SUCCESS 0
#define EDERR_UNSUCCESSFUL MAXDWORD

#define EDERR_MAKE_ERR(_X) (_X|0x80000000)
#define EDERR_OBJECT_EXISTED EDERR_MAKE_ERR(1)
#define EDERR_WRONG_PARAMETERS EDERR_MAKE_ERR(2)

#define EDERR_FAILED_FZCONTEXT	EDERR_MAKE_ERR(100)
#define EDERR_FIALED_OPENFILE	EDERR_MAKE_ERR(105)
#define EDERR_HAS_PASSWORD		EDERR_MAKE_ERR(106)
#define EDERR_UNKOWN_DOCTYPE	EDERR_MAKE_ERR(107)
#define EDERR_EMPTY_CONTENT		EDERR_MAKE_ERR(110)
#define EDERR_FIALED_LOAD_PAGE	EDERR_MAKE_ERR(111)
#define EDERR_THREAD_ERROR		EDERR_MAKE_ERR(115)
#define EDERR_NOTENOUGH_MEMORY	EDERR_MAKE_ERR(120)
#define EDERR_3RDPART_FAILED	EDERR_MAKE_ERR(130)




#define		EDPDF_ANNOT_NONE	0
#define		EDPDF_ANNOT_INK		1
#define		EDPDF_ANNOT_UNDERL		2
#define		EDPDF_ANNOT_DELETE		3
#define		EDPDF_ANNOT_HIGHLIGHT	4
#define		EDPDF_ANNOT_TEXT		5



// 常用宏
#define BOOL_TRUE(_X) (_X!=0)
#define BOOL_FALSE(_X) (_X==0)

#define ERR_SUCCESS(_X) (_X<=0x7fffffff)
#define ERR_FAILED(_X) (_X>0x7fffffff)


#define ANN_SIGN_TYPE(_X)	(_X>>(64-8))
#define ANN_SIGN_PAGE(_X)	((_X>>(64-8-16))&0xFFFF)
#define ANN_SIGN_ALT(_X)	((_X>>(64-8-16-16))&0xFFFF)
#define ANN_SIGN_CHECKSUM(_X)	(_X&0xFFFFFF)
#define ANN_MAKE(_T,_P,_A,_C) (((ULONGLONG)_T<<(64-8))|((ULONGLONG)_P<<(64-8-16))|((ULONGLONG)_A<<(64-8-16-16))|(_C&0xFFFFFF))

//////////////////////////////////////////////////////////////////////////
// 结构体定义，此处定义的结构体用于跨模块交换数据
//////////////////////////////////////////////////////////////////////////

#pragma pack(4)

//typedef struct _STEMS_TIMER {
//	void* Context;		//
//}STEMS_TIMER, *PSTEMS_TIMER;

typedef struct _ED_RECT {
	int32Eink left;
	int32Eink right;
	int32Eink top;
	int32Eink bottom;
}ED_RECT,* ED_RECT_PTR;
typedef struct _ED_RECTF {
	float32 left;
	float32 right;
	float32 top;
	float32 bottom;
}ED_RECTF,* ED_RECTF_PTR;

#define ED_RECT_WIDTH(_X) (_X.right - _X.left)
#define ED_RECT_HEIGHT(_X) (_X.bottom - _X.top)

typedef struct _ED_SIZE {
	int32Eink x;
	int32Eink y;
}ED_SIZE,* ED_SIZE_PTR;

typedef struct _ED_SIZEF {
	float32 x;
	float32 y;
}ED_SIZEF,* ED_SIZEF_PTR;

#define ED_SIZE_CLEAN(_X) (_X.x=0,_X.y = 0)
#define ED_SIZEF_CLEAN(_X) (_X.x=0.0f,_X.y = 0.0f)

typedef _ED_SIZE ED_POINT,*ED_POINT_PTR;
typedef _ED_SIZEF ED_POINTF,* ED_POINTF_PTR;

typedef struct _ED_LINE {
	int32Eink x1;
	int32Eink x2;
	int32Eink y1;
	int32Eink y2;
}ED_LINE, *ED_LINE_PTR;
typedef struct _ED_LINEF {
	float32 x1;
	float32 x2;
	float32 y1;
	float32 y2;
}ED_LINEF, *ED_LINEF_PTR;


typedef struct _ED_COLOR {
	int32Eink r;
	int32Eink g;
	int32Eink b;
	int32Eink a;
}ED_COLOR,* PED_COLOR;

typedef struct _PAGE_PDF_CONTEXT {
	ULONG pageIndex;	
	ULONG pageContext;	
	ULONG pageContext2;
}PAGE_PDF_CONTEXT, *PPAGE_PDF_CONTEXT;


#pragma pack()


//////////////////////////////////////////////////////////////////////////
// 结构体定义结束
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 接口定义
//////////////////////////////////////////////////////////////////////////

// 模块接口定义
__interface IEdModule :public IBaseObject
{
	// 获取支持的文件格式数量
	virtual int32Eink GetTypeCount(void) = NULL;
	// 获取支持的文件格式的扩展名
	virtual const char16_ptr GetTypeName(int32Eink index=0) = NULL;

	// 打开文档
	virtual ED_ERR OpenDocument(
		IN char16_ptr pathName,
		OUT IEdDocument_ptr* documentPtrPtr
	) = NULL;

};


// 文档对象
__interface IEdDocument :public IBaseObject
{
	virtual bool32 LoadAllPage(PEDDOC_CALLBACK callBackFun, void_ptr contextPtr, PPAGE_PDF_CONTEXT initPage)=NULL;
	virtual bool32 LoadAllThumbnails(PEDDOC_THUMBNAILS_CALLBACK callBack, void_ptr contextPtr,const wchar_t* nphumbnailPathName =NULL) = NULL;
	virtual bool32 GetThumbanilsPath(wchar_t* npszPathBuffer, int niLen) = NULL;
	virtual int32Eink GetMetaData(char* keyName,char* dataBuf, int32Eink bufSize)=NULL;
	virtual int32Eink GetDocType(void) = NULL;
	virtual int32Eink GetPageCount(void) = NULL;
	virtual IEdPage_ptr GetPage(int32Eink pageIndex) = NULL;
	virtual IEdPage_ptr GetPage(PPAGE_PDF_CONTEXT contextPtr) = NULL;	// contextPtr==NULL for the first page of this doc
	virtual IEdPage_ptr GetPage(IEdPage_ptr currentPage, int32Eink pagesOff) = NULL;
	virtual bool32 GetThumbnailPathName(int32Eink pageIndex, char16 pathName[256],bool* hasAnnot=NULL)=NULL;
	virtual int32Eink GetAnnotPageCount(void) = NULL;

	virtual bool32 SaveAllChanges(const char16_ptr pathName = NULL) = NULL;

	//////////////////////////////////////////////////////////////////////////
	// following functions just work on a txt document.

	// This method to update the layout of all pages.
	IEdPage_ptr Rearrange(PPAGE_PDF_CONTEXT contextPtr)=NULL; 

	// This method to update the layout of all pages.
	IEdPage_ptr Rearrange(IEdPage_ptr currentPage)=NULL;

	// This method to change the font used to render this document. 
	virtual void SetFont(const char16_ptr fontName, int32Eink fontSize)=NULL;

	// this method to set the view port to render txt document. 
	virtual void SetViewPort(int32Eink viewWidth, int32Eink viewHeight) = NULL;

};
// keyName
//key: Which meta data key to retrieve...
//	"format"
//	"encryption"
//	"info:Author"
//	"info:Title"
// keyName
//key: Which meta data key to retrieve...

//Basic information:
//	'format'	-- Document format and version.
//	'encryption'	-- Description of the encryption used.

//From the document information dictionary:
//	'info:Title'
//	'info:Author'
//	'info:Subject'
//	'info:Keywords'
//	'info:Creator'
//	'info:Producer'
//	'info:CreationDate'
//	'info:ModDate'


//From the document information dictionary:
//	'info:Title'
//	'info:Author'
//	'info:Subject'
//	'info:Keywords'
//	'info:Creator'
//	'info:Producer'
//	'info:CreationDate'
//	'info:ModDate'


// 页类型
__interface IEdPage :public IBaseObject
{
	virtual bool32 GetMediaBox(
		OUT ED_RECTF_PTR mediaBox
	) = NULL;
	virtual bool32 GetCropBox(
		OUT ED_RECTF_PTR cropBox
	) = NULL;
	virtual bool32 GetBleedBox(
		OUT ED_RECTF_PTR bleedBox
	) = NULL;
	virtual IEdBitmap_ptr Render(
		IN float32 scalRatio,
		IN bool32 cleanUp
	) = NULL;
	virtual int32Eink GetPageIndex(void) = NULL;

	virtual bool32 GetPageContext(
		PPAGE_PDF_CONTEXT contextPtr
	) = NULL;

	virtual IEdAnnotManager_ptr GetAnnotManager(void) = NULL;	// 获得的这个对象不用释放，会自动随着page对象一同销毁

	virtual bool32 GetSelectedText(
		IN ED_RECTF_PTR selBox,
		OUT char16_ptr textBuf,
		IN int32Eink bufSize
	) = NULL;

	virtual IEdStructuredTextPage_ptr GetStructuredTextPage(void) = NULL;	// 返回的对象需要调用release释放
};

// 位图类型
__interface IEdBitmap :public IBaseObject
{
	virtual bin_ptr GetBuffer() = NULL;
	virtual int32Eink GetWidth() = NULL;
	virtual int32Eink GetHeight() = NULL;
};


__interface IEdAnnotManager
{
	// 从存档数据装入一个Annotation
	virtual IEdAnnot_ptr LoadAnnotFromArchive(buf_ptr buf, uint32Eink bufSize) = NULL;

	// 新建文本笔记
	virtual IEdAnnot_ptr AddTextAnnot(
		IN ED_POINT position,
		IN char16_ptr text
	) = NULL;

	// 新建墨水笔记，返回对象需要释放
	virtual IEdAnnot_ptr AddInkAnnot(
		IN ED_POINTF* stroke,
		IN int32Eink pointCount,
		IN ED_COLOR* color=NULL,
		IN float32 border=1.0f
	) = NULL;

	// 新建Highlight笔记，返回对象需要释放
	virtual IEdAnnot_ptr AddHighLightAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 新建删除现笔记，返回对象需要释放
	virtual IEdAnnot_ptr AddDeleteLineAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 新建下划线笔记，返回对象需要释放
	virtual IEdAnnot_ptr AddUnderLineAnnot(
		IN IEdStextQuadList_ptr stext,
		IN ED_COLOR* color = NULL,
		IN int32Eink clrN = 3
	);

	// 通过签名查找一个Annot，返回对象需要释放
	virtual IEdAnnot_ptr GetAnnotBySignature(ULONGLONG signature) = NULL;
	virtual ULONGLONG GetSignature(IEdAnnot_ptr annot) = NULL;

	// 获取第一个笔记，返回对象需要释放
	virtual IEdAnnot_ptr GetFirstAnnot(void) = NULL;

	// 获取下一个笔记，返回对象需要释放
	virtual IEdAnnot_ptr GetNextAnnot(IEdAnnot_ptr crt) = NULL;

	// 获取所有对象的列表，当annotList==NULL时，返回需要的List单元数,当缓冲区不足时返回-1；成功返回获得的IEdAnnot_ptr对象数
	virtual int32Eink GetAllAnnot(IEdAnnot_ptr* annotList,int bufSize) = NULL;

	// 检测一笔画接触到的一系列Ink笔记对象（相交），当annotList==NULL时，返回需要的缓冲区长度,当缓冲区不足时返回-1
	virtual int32Eink DetectInkAnnot(
		IN ED_POINTF* stroke,
		IN int32Eink pointCount,
		OUT	IEdAnnot_ptr* annotList,	// 用于返回所有相交对象的缓冲区，建议一次性分配大的缓冲区，比如 IEdAnnot_ptr buf[256];返回对象需要释放
		IN int32Eink bufSize					// 上述缓冲区的单元数，不是字节数
	) = NULL;

	// 删除一个Annotation，调用此函数后，IEdAnnot_ptr annot，仍然需要释放
	virtual void RemoveAnnot(
		IEdAnnot_ptr annot
	) = NULL;
};

__interface IEdAnnot :public IBaseObject
{
	virtual uint32Eink GetType() = NULL;	// EDPDF_ANNOT_INK ,EDPDF_ANNOT_UNDERL, EDPDF_ANNOT_DELETE,EDPDF_ANNOT_HIGHLIGHT, EDPDF_ANNOT_TEXT
	virtual char* GetTypeName() = NULL;		// "ink" "text" "highlight" "underline" "deleteline" or "Identity"

	virtual IEdTextAnnot_ptr ConvertToTextAnnot(void) = NULL;	// 返回值无需释放
	virtual IEdInkAnnot_ptr ConvertToInkAnnot(void) = NULL;		// 返回值无需释放
	virtual IEdStextAnnot_ptr ConvertToStextAnnot(void) = NULL;	// 返回值无需释放

	// 当buf==NULL时，返回需要的缓冲区字节数，缓冲区不足返回-1
	virtual uint32Eink SaveToAchive(buf_ptr buf, uint32Eink bufSize) = NULL;	// 将此对象保存到存档，供将来从存档恢复存到到Page(通过调用IEdPage的方法 LoadAnnotFromArchive)
};

__interface IEdTextAnnot :public IEdAnnot
{
	virtual const char16_ptr GetText(void) = NULL;

	virtual ED_POINT GetPosition(void) = NULL;

	virtual ED_RECT GetRect(void) = NULL;
};

__interface IEdInkAnnot :public IEdAnnot
{
	virtual void SetColor(ED_COLOR clr) = NULL;		// 设置线条颜色
	virtual ED_COLOR GetColor(void) = NULL;

	virtual void SetBorder(int32Eink border) = NULL;	// 设置线条宽度 1 ~ N
	virtual int32Eink GetBorder(void) = NULL;
};

__interface IEdStextAnnot :public IEdAnnot
{
	virtual void SetColor(ED_COLOR clr) = NULL;		// 设置线条颜色
	virtual ED_COLOR GetColor(void) = NULL;

	virtual IEdStextQuadList_ptr GetQuadsList(void) = NULL; // 获得分块列表
};

__interface IEdStructuredTextPage :public IBaseObject	// 选择，用于处理structured text信息 
{
	// 探测文本选中情况
	virtual bool DetectSelectedText(
		IN const ED_POINTF* aPoint,
		IN const ED_POINTF* bPoint,
		OUT IEdStextQuadList_ptr* stext,	// 返回结构化文本对象，需要释放
		OUT IEdAnnotList_ptr* impactedAnnot=NULL,// 返回碰撞的已有Annot对象列表，需要释放
		IN bool32 includeImpacted = false	// 将碰撞的annot区域也加入选区
	) = NULL;	// 返回的对象需要释放

	// 复制目标区域文本，返回值是复制的字符数，不包含结尾0
	virtual int32Eink CopyText(
		IN ED_RECTF_PTR selBox,
		OUT char16_ptr textBuf,	// 传入NULL，函数返回实际的字符数，不含结尾0
		IN int32Eink bufSize
	) = NULL;

};

__interface IEdStextQuadList :public IBaseObject
{
	// 获得A点（上止点）
	virtual void GetAPoint(
		OUT ED_POINTF* pt
		) = NULL;

	// 获得B点（下止点）
	virtual void GetBPoint(
		OUT ED_POINTF* pt
		) = NULL;

	// 获得分块总数
	virtual int32Eink GetQuadCount(void) = NULL;

	// 获得一个分块的位置
	virtual void GetQuadBound(
		IN int32Eink index,
		OUT ED_RECTF_PTR quad
		) = NULL;

	virtual const ED_RECTF* GetQuad(IN int32Eink index)=NULL;
};

__interface IEdAnnotList :public IBaseObject
{
	virtual int32Eink GetCount(void) = NULL;
	virtual IEdAnnot_ptr GetAnnot(int32Eink index) = NULL;	// 返回的对象需要释放
};

//////////////////////////////////////////////////////////////////////////
// 接口定义结束
//////////////////////////////////////////////////////////////////////////








////////////////////////////////////////////////////////////////////////////
//// 插件入口函数定义
////////////////////////////////////////////////////////////////////////////
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//	// 获取模块对象
//	IEdModule_ptr __stdcall EdGetModule(void);
//
//#ifdef __cplusplus
//}
//#endif
typedef IEdModule_ptr(__stdcall *GetModule_Proc)(void);


//////////////////////////////////////////////////////////////////////////
// 文件结束
//////////////////////////////////////////////////////////////////////////
#endif//_XUI_H_