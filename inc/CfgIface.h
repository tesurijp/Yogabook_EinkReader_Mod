/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#ifndef _CONFIGINTF_H_
#define _CONFIGINTF_H_

#include "cmmBaseObj.h"

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif



__interface ICfKey;
//////////////////////////////////////////////////////////////////////////
// 配置文件接口
//	本类用于实现一个配置文件的驱动器，这个配置文件具有以下的属性：
//		1. 存放的数据由Key构成，每个Key具有一个名字和一个默认值，默认值的数据类型为字符串或者二进制数据；2.用树形结构来组织全部的Key，其中根Root是系统默认的Key，其他所有的Key都是它的子Key或者它子Key的子Key；
//		3. 对子Key的名字用散列值排序提供索引，能够很快地找寻和打开子Key，适合存储大量的子Key。
__interface IConfigFile:public IBaseObject{
public:
	// 获得根键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetRootKey(void)=NULL;

	// 打开指定的键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall OpenKey(
		IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
		IN bool nbCreateIf=false			// 如果这个键不存在，则建立它
		)=NULL;

	// 保存修改到文件
	virtual bool __stdcall SaveFile(
		IN const wchar_t* nszOtherFile=NULL	// 可以指定保存为其他的文件，如果为NULL，这保存到刚才打开或者新建的文件
		)=NULL;


	enum VALUETYPE{
		Invalid=0,	// 无有效数据
		Binary=1,	// 二进制类型
		AnsiString=2,	// Ansi编码字符串
		UnicodeString=3	// Unicode编码字符串
	};
};



//////////////////////////////////////////////////////////////////////////
// 键值类
__interface ICfKey:public IBaseObject{
public:
	//删除这个键，注意，调用删除后的接口仍然需要Release；调用过Delete后，这个接口也就没有用了，出Release外，其他功能都不支持了
	virtual bool __stdcall Delete(void)=NULL;	

	// 新建一个键，返回NULL表示失败，失败的原因可能是已经具有相同的键值，或者分配内存失败
	virtual ICfKey* __stdcall NewSubKey(
		IN const wchar_t* nszName,	// 子键的名字，不能为空，名字中不能使用下列4个字符" < > / \ " 不包括引号
		IN IConfigFile::VALUETYPE nuValueType = IConfigFile::Invalid,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
		IN const void* npValueBuf = NULL,
		IN LONG  niValuelen = 0		// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
		)=NULL;

	// 新建一个无名键，返回NULL表示失败；如果同时存在命名键值，将有可能存在命名键值夹在无名键中间的情况
	virtual ICfKey* __stdcall NewSubKey(
		IN ULONG nuID,		// 指定无名键的标识ID，尽可能不要使用相同标识，不能为0
		IN bool nbAhead=false,	// 当遇到相同ID的子键时，在相同ID子键之前建立新键；否则，在相同ID子键之后建立新键
		IN IConfigFile::VALUETYPE nuValueType = IConfigFile::Invalid,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
		IN const void* npValueBuf = NULL,
		IN LONG  niValuelen = 0		// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
		)=NULL;

	// 打开多层路径指定的子键，获得的对象当不再访问时需要Release；本函数同下面的GetSubKey区别是，本函数可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
	virtual ICfKey* __stdcall OpenKey(
		IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
		IN bool nbCreateIf=false			// 如果这个键不存在，则建立它
		)=NULL;

	// 获得本键的父键，如果本键是根键，则返回NULL,获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetParentsKey(void)=NULL;

	// 重新设置父节点，将本节点从当前父节点移除，插入新的父节点下
	virtual bool __stdcall SetParentKey(
		IN ICfKey* npNewParent,
		IN bool nbAhead
		)=NULL;

	// 获得下一个键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetNextKey(void)=NULL;

	// 获得第一个子键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetSubKey(void)=NULL;

	// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的OpenKey区别是，OpenKey可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
	virtual ICfKey* __stdcall GetSubKey(
		IN const wchar_t* nszName,	// 按照名字去获取子键
		IN int niNameLen=-1,	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		OUT int* npNameLen=NULL,	// 返回名字的有效长度
		IN bool nbCreateIf=false	// 如果该键不存在，则建立它
		)=NULL;

	// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的GetSubKey区别是，上面的按名字打开键值，而本函数直接按ID打开，只有通过本函数才能打开没有名字的键值
	virtual ICfKey* __stdcall GetSubKey(
		IN ULONG nuID,		// 子键的标识ID
		IN int niPos=0		// 在相同的ID的键值中的先后位置，< 0表示取最后一个
		)=NULL;

	// 获得父键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToParentsKey(void)=NULL;

	// 获得下一个键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToNextKey(void)=NULL;

	// 获得下一个键，删除并释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToNextKey(bool nbDelete)=NULL;

	// 获得第一个子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(void)=NULL;

	// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(
		IN ULONG nuID		// 子键的标识ID
		)=NULL;

	// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(
		IN const wchar_t* nszName,	// 按照名字去获取子键
		IN int niNameLen=-1	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		)=NULL;

	// 获得本键的默认值类型
	virtual IConfigFile::VALUETYPE __stdcall GetValueType(void)=NULL;

	// 获得本键的扩展标志，定义在下面
	virtual UCHAR __stdcall GetExtFlag(void)=NULL;

	// 获得本键的默认值长度，字节单位
	virtual LONG __stdcall GetValueLength(void)=NULL;

	// 获得本键的名字，返回的是名字长度，如果npNameBuff为NULL，返回名字的长度(wchar_t为单位），不包括字符串结尾的0。返回-1表示缓冲区溢出
	virtual int __stdcall GetName(wchar_t* npNameBuff,int niBufLenByWchar)=NULL;	//注意，niBufLenByWchar参数存放的是wchar_t的数，不是字节数

	// 改名
	virtual bool __stdcall Rename(
		IN const wchar_t* nszName,
		IN bool FailIfExist=true	//	FailIfExist==true如果改名会导致同一节点下出现同名节点，将失败;  ==false 将自动增加附加字符
		)=NULL;

	// 获得ID，对于命名键值而言获得的是Hash值
	virtual ULONG __stdcall GetID(
		OUT int* npPos=NULL		// 在相同的ID的键值中的先后位置
		)=NULL;

	// 获得本键所在的父键下的存储位置
	virtual int __stdcall GetPosition(void)=NULL;

	// 设置或改变当前的默认值
	virtual bool __stdcall SetValue(
		IN IConfigFile::VALUETYPE nuValueType,	// 子键的默认值类型
		IN const void* npValueBuf,
		IN LONG  niValuelen=-1		// 字符串类型时，可以通过结尾的\0来决定数值的长度
		)=NULL;

	// 获得当前默认值，返回获得的值的字节数
	virtual int __stdcall GetValue(
		OUT PVOID npValueBuf,
		IN  LONG  niBufLen
		)=NULL;

	// 修改扩展标志
	virtual bool __stdcall SetExtFlag(
		IN UCHAR nchFlag
		)=NULL;

	// 直接将值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QueryValueAsLONG(
		IN LONG niDefault= -1
		)=NULL;

	// 直接将值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QueryValueAsFLOAT(
		IN FLOAT nfDefault = 0.0f
		)=NULL;

	// 直接将值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QueryValueAsPVOID(
		IN PVOID npDefault = NULL
		)=NULL;

	// 按名字查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QuerySubKeyValueAsLONG(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN LONG niDefault= -1
		)=NULL;

	// 按ID查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QuerySubKeyValueAsLONG(
		IN ULONG nuID,		// 子键的标识ID
		IN LONG niDefault= -1
		)=NULL;

	// 按名字查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QuerySubKeyValueAsFLOAT(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN FLOAT nfDefault = 0.0f
		)=NULL;

	// 按ID查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QuerySubKeyValueAsFLOAT(
		IN ULONG nuID,		// 子键的标识ID
		IN FLOAT nfDefault = 0.0f
		)=NULL;

	// 按名字查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QuerySubKeyValueAsPVOID(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN PVOID npDefault = NULL
		)=NULL;

	// 按ID查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QuerySubKeyValueAsPVOID(
		IN ULONG nuID,		// 子键的标识ID
		IN PVOID npDefault = NULL
		)=NULL;

	// 按名字查找子键，直接获取子键的值，返回小于零表示目标键不存在，等于0表示目标键值为空，大于0表示返回的Value长度
	virtual int __stdcall QuerySubKeyValue(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		OUT PVOID npValueBuf,
		IN  LONG  niBufLen
		)=NULL;

	// 获取指定的值存储New出来的内存中返回来，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QueryValueAsBuffer()=NULL;

	// 获取指定的子健的值存储在New出来的内存中返回来，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QuerySubKeyValueAsBuffer(
		IN const wchar_t* nswSubKeyName
		)=NULL;

	// 获取指定的子健的值存储在New出来的内存中返回来，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QuerySubKeyValueAsBuffer(
		IN ULONG nuID									// 子键的标识ID
		)=NULL;

	// 释放由Query***AsBuffer系列函数返回的Buffer  ——Colin 加。
	virtual void _stdcall ReleaseBuffer(OUT PVOID npvoBufferToRelease)=NULL;

};

#define CDKEY_FLAG_LOCKED	16		// 本键不可以被修改或者删除
#define CFKEY_FLAG_DISABLE	32		// 设置本键为禁用，失效，或不可见；此标志只是供应用程序识别使用，配置系统只负责保存和读取
#define CDKEY_FLAG_CUSTOM1	64		// 自定义数据1，配置数据的使用场景解释它的用途
#define CDKEY_FLAG_CUSTOM2	128		// 自定义数据2，配置数据的使用场景解释它的用途

#ifdef __cplusplus
extern "C" {
#endif

// nuCreationDisposition refer to the function CfCreateConfig
#if defined(KERNEL_CODE)||defined(NATIVE_CODE)
// 核心态
#define CF_CREATE_ALWAYS	FILE_OVERWRITE_IF
#define CF_CREATE_NEW		FILE_CREATE
#define CF_OPEN_ALWAYS	FILE_OPEN_IF
#define CF_OPEN_EXISTING	FILE_OPEN
#else
// 用户态
#define CF_CREATE_ALWAYS	CREATE_ALWAYS
#define CF_CREATE_NEW		CREATE_NEW
#define CF_OPEN_ALWAYS	OPEN_ALWAYS
#define CF_OPEN_EXISTING	OPEN_EXISTING
#endif//KERNEL_CODE

// 建立或者打开一个配置文件，返回配置文件对象或者返回NULL表示失败，失败的原因可能是文件不存在或者文件格式错误
IConfigFile* __stdcall CfCreateConfig(
		IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
		IN ULONG nuCreationDisposition=CF_OPEN_EXISTING	// 同CreateFile API类似，但仅包括CREATE_ALWAYS、CREATE_NEW、OPEN_ALWAYS、OPEN_EXISTING，定义见下面
		);

// 同上面的函数的区别是，StableConfig会自动为文件建立一个备份副本，防止操作文件或者其他原因导致单一文件破坏
// 配置文件将存在前后两次打开的不同的两个，它们交替使用，互为备份，从而防止某次的使用过程中破坏了其中一个文件；
// 需要注意，备份的是打开前的内容，当一个打开的配置被多次改写，而最后一次写入的文件最后损坏，将来会自动回复的不是前一次写入的数据，而是前次对象打开前最后保存的数据。
// 如：如果配置文件名为abc.set，那么在同一个目录下将可能存在一个abc_dup.set的文件，它就是副本；
IConfigFile* __stdcall CfCreateStableConfig(
	IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
	IN ULONG nuCreationDisposition=CF_OPEN_EXISTING	// 同CreateFile API类似，但仅包括CREATE_ALWAYS、CREATE_NEW、OPEN_ALWAYS、OPEN_EXISTING，定义见下面
	);


#ifdef __cplusplus
}
#endif




#endif//_CONFIGINTF_H_