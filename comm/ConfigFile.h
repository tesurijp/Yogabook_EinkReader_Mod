/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#ifndef _CONFIGBASE_H_
#define _CONFIGBASE_H_
/*
	modified by Ax Mar.15,2011	本类派生之普通配置文件类，同前者的区别是，增加了副本支持，具体如下：
							 配置文件将存在前后两次打开的不同的两个，它们交替使用，互为备份，从而防止某次的使用过程中破坏了其中一个文件；
							 如：如果配置文件名为abc.set，那么在同一个目录下将可能存在一个abc.set.dup的文件，它就是副本；

*/
#include "CfgIface.h"
#include "cmmstruct.h"

#define CF_LOOKASIDE_UNUSEDKEY_COUNT 5


#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// 存储到文件中的数据结构
typedef struct _CF_FILE_HEAD_SHORT{
		ULONG Signature;  // must be CF_SIGNATURE
		ULONG Version;	  // current version is 0x00010003
		ULONG SequenceA;
		ULONG SizeOfHead;		// 文件头的字节数
		ULONG SizeOfReserved;	// 保留区的字节数
		ULONG SizeOfKeys;		// 存储的全部键总字节数
		ULONG CheckSum;		// 保存Key存储区校验和
		ULONG Duplicate;		// 副本序号，如果这个值为0表示没有副本
}CF_FILE_HEAD_SHORT,* PCF_FILE_HEAD_SHORT;
typedef union _CF_FILE_HEAD {
	struct _CF_FILE_HEAD_SHORT ShortHead;
	struct  
	{
		UCHAR Reserved[0x200-4];
		ULONG SequenceB;
	}Tail;
}CF_FILE_HEAD,* PCF_FILE_HEAD;

#define CF_SIGNATURE 'CFFL'
#define CF_VERSION 0x00010005

typedef struct _CF_KEY_ENTRY{
	UCHAR NameLength;
	UCHAR Flag;
	USHORT ValueSize;	// 值的长度
	LONG SubEntryCount;	// 拥有的子项数
}CF_KEY_ENTRY,* PCF_KEY_ENTRY;

#define CFKEY_VALUE_TYPE(_X) ((IConfigFile::VALUETYPE)(_X&0x3))
#define CFKEY_SET_VALUETYPE(_X,_T) (_X=((_X&0xFC)|(_T&0x3)))
#define CFKEY_GET_EXTFLAG(_X) (_X&0xF0)
#define CFKEY_SET_EXTFLAG(_X,_T) (_X=((_X&0xF)|(_T&0xF0)))

#define CFKEY_INDEX_AVAILABLE 4		// 具有序号排序的索引表
#define CFKEY_NODE_OPENED 8 // 这个Key被打开了
#define CFKEY_HAS_CHILD(P_NODE) ((P_NODE->Flag&CFKEY_INDEX_AVAILABLE)!=0)

#pragma pack()

struct _CFKEY_NODE;
class CCfKeyInterface;

class CCfKeyHash{
public:
	ULONG HashValue;
	struct _CFKEY_NODE* KeyObj;
	CCfKeyHash(){}
	void operator=(const class CCfKeyHash& src){
		HashValue = src.HashValue;
		KeyObj = src.KeyObj;
	}
	// 用字符串生成Hash码,返回的是名字的有效长度
	int GenerateHashCode(
		IN const wchar_t* nszName,
		IN int niNameLen=-1	// -1 indicate that the name was terminated by '\0' or '\\' or '/'
		);
};

//////////////////////////////////////////////////////////////////////////
// 排序数组用排序比较类
class CKeyHashCriterion	// 默认的判断准则
{
public:
	bool operator () (const CCfKeyHash& Obj1,const CCfKeyHash& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.HashValue < Obj2.HashValue);
	}
};

class CCfKeyOpened{
public:
	struct _CFKEY_NODE* KeyNode;
	CCfKeyInterface* Interface;
	CCfKeyOpened(){}
	void operator=(const class CCfKeyOpened& src){
		KeyNode = src.KeyNode;
		Interface = src.Interface;
	}
};

//////////////////////////////////////////////////////////////////////////
// 打开键的管理队列用排序比较类
class CKeyInterfaceCriterion	// 默认的判断准则
{
public:
	bool operator () (const CCfKeyOpened& Obj1,const CCfKeyOpened& Obj2)const // 一定要用内联函数
	{
		// 当对象Ob1小于对象Obj2时，返回True，否则返回false
		return (Obj1.KeyNode < Obj2.KeyNode);
	}
};

typedef cmmMultiSequence<CCfKeyHash,CKeyHashCriterion,0,16> TCFKEYSEQUENCE; 
typedef cmmStack<CCfKeyInterface*,15> TCFKEYINTERFACESTACK;
typedef cmmMultiSequence<CCfKeyOpened,CKeyInterfaceCriterion,32,32> TCFKEYOPENED;


//////////////////////////////////////////////////////////////////////////
// 存储在内存中的一项
#pragma pack(1)

// 键节点
typedef struct _CFKEY_NODE{
	UCHAR Flag;			// 指令本项包含的内容	
	UCHAR NameLength;	// 名字的字数，不包括结尾的0，如果没有名字，本值为0，后面的Name无定义
	USHORT ValueLength; // 某值的大小
}CFKEY_NODE,* PCFKEY_NODE;

// 带有子键值的节点
typedef struct _CFKEY_BRANCH {
	UCHAR Flag;			// 指令本项包含的内容
	UCHAR NameLength;	// 名字的字数，不包括结尾的0，如果没有名字，本值为0
	USHORT ValueLength; // 某值的大小
	TCFKEYSEQUENCE* mpSubKeys;	// 保存子键值的对象的指针
	wchar_t Name[1];	// 名字字符串，携带一个结尾的0
}CFKEY_BRANCH,* PCFKEY_BRANCH;

// 不带有子键值的节点
typedef struct _CFKEY_LEAF {
	UCHAR Flag;			// 指令本项包含的内容
	UCHAR NameLength;	// 名字的字数，不包括结尾的0，如果没有名字，本值为0
	USHORT ValueLength; // 某人值的大小
	wchar_t Name[1];	// 名字字符串，携带一个结尾的0
}CFKEY_LEAF,* PCFKEY_LEAF;

#pragma pack()

//////////////////////////////////////////////////////////////////////////
// 一个Key存放在文件中和内存中的情况如下
//	文件中 |Head|Name|Value|Hash|
//  内存中 |Head|Name|0|Value|
//			 ┕ SubKeys
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// 配置文件类
//	本类用于实现一个配置文件的驱动器，这个配置文件具有以下的属性：
//		1. 存放的数据由Key构成，每个Key具有一个名字和一个默认值，默认值的数据类型为字符串或者二进制数据；2.用树形结构来组织全部的Key，其中根Root是系统默认的Key，其他所有的Key都是它的子Key或者它子Key的子Key；
//		3. 对子Key的名字用散列值排序提供索引，能够很快地找寻和打开子Key，适合存储大量的子Key。4. 可以给每个Key指定序号，同一个Key下的直接子Key序号不能重复，是否使用序号需要在这个父Key建立时指定，对于使用序号
//		的Key下新建的子key，如果没有提供序号，系统将自动给予目前最大序号的下一个序号。
DECLARE_BUILTIN_NAME(CConfigFile)
class CConfigFile: public cmmBaseObject<CConfigFile,IConfigFile,GET_BUILTIN_NAME(CConfigFile)>
{
	friend class CCfKeyInterface;
public:
	CConfigFile();
	virtual ~CConfigFile();

	ULONG InitOnCreate(
		IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
		IN ULONG nuCreationDisposition			// 同CreateFile API类似，见CfgIface.h文件中的相关定义
		);

	DEFINE_CUMSTOMIZE_CREATE(CConfigFile,(const wchar_t* nszPathName=NULL,ULONG nuCreationDisposition=CF_OPEN_EXISTING),(nszPathName,nuCreationDisposition))

	// 保存修改到文件
	virtual bool __stdcall SaveFile(
		IN const wchar_t* nszOtherFile=NULL	// 可以指定保存为其他的文件，如果为NULL，这保存到刚才打开或者新建的文件
		);


	// 获得根键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetRootKey(void);

	// 打开指定的键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall OpenKey(
		IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
		IN bool nbCreateIf=false			// 如果这个键不存在，则建立它
		);

	// 判断字符串值的长度
	static int GetValueLengthOfTypeString(IConfigFile::VALUETYPE nuValueType,const void* npValueBuf);
protected:
	//int miReferenceCount;
	wchar_t mszFileName[256];
// 	ULONG muFileHeadSequence;
	CF_FILE_HEAD_SHORT mdFileHead;
	PCFKEY_BRANCH mpRoot;
	CCfKeyInterface* mpRootInterface;
	TCFKEYOPENED moKeyOpened;	// 用这个来存放打开的Key
	TCFKEYINTERFACESTACK moReleasedKey;	// 为了减少内存分配和释放的次数，将一些释放的Key打开对象存放在这儿，供打开其他Key时使用
	CExclusiveAccess moExclusive;
	TCFKEYINTERFACESTACK moUnusedKeyInterface;	// 保存最近释放的未使用的KeyInterface对象，以减少频繁打开关闭Key时带来的内存分配和释放

	// 分配一个新的Node
	PCFKEY_NODE AllocateNode(
		IN const wchar_t* nszName,	// 子键的名字
		IN int niNameLen,		// count in wchar, if be -1 indicate that the nszName is terminated by '\0' or '\\' or '/'
		IN UCHAR nchFlag = 0,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
		IN const void* npValueBuf = NULL,
		IN int  niValuelen = 0,
		IN int  niSubKeyCount = 0
		);

	// 递归调用，装载指定键和他的子键，返回时nrKeyEntry会修改指向下一个Entry
	PCFKEY_NODE LoadKey(PCF_KEY_ENTRY& nrKeyEntry,const PCF_KEY_ENTRY npEndEntry);

	// 递归调用，保存本键和子键的值，返回0表示失败。返回式nrKeyEntry会越过已经存放的数据。如果输入时nrKeyEntry==NULL，则只返回需要的存储区大小
	int SaveKey(PCFKEY_NODE npKey,PCF_KEY_ENTRY& nrKeyEntry);

	// 删除键值和它的子键值
	bool RemoveKey(PCFKEY_NODE npKey);

	// 询问是否可以删除一个子键值，包括它的下层某个键值
	bool QueryRemove(PCFKEY_NODE npKey);

	// 打开一个Key
	CCfKeyInterface* GetKeyInterface(CCfKeyInterface* npParents,int niPosition);

	// 分配一个打开Key的打开对象
	CCfKeyInterface* AllocateKeyInterface(PCFKEY_NODE npKeyNode);

	// 增加对一个Key的引用
	int AddReferToKeyInterface(CCfKeyInterface* npKeyInterface);

	// 减少对一个Key的引用，如果引用为0就释放它
	int ReleaseKeyInterface(CCfKeyInterface* npKeyInterface);

	// 查找一个打开并且有效的KEY，因为可能存在指向同一个npKeyNode的不同npKeyInterface，其中只有一个是有效的，其他的指向的都可能是已经删除过得其他Key值
	CCfKeyInterface* FindValidKeyInterface(PCFKEY_NODE npKeyNode);

	//// 查找一个第一个符合条件的KeyInterface，注意，可能是对应的Key已经被删除但Interface还没有被释放
	//int FindKeyInterface(PCFKEY_NODE npKeyNode);

	// 查找一个KeyInterface是否在保存数组中存在
	int FindKeyInterface(CCfKeyInterface* npKeyInterface);

	// 更新一个打开的键的Node指针
	bool UpdateInterface(CCfKeyInterface* npInterface,PCFKEY_NODE npKeyNode);

	// 更新一个打开的键的SubNodes数组，因为数组发生了可能导致其他Interface（它们指向某个SubNode)失效，所以需要更新
	void UpdateSubNodes(CCfKeyInterface* npInterface);

};

DECLARE_BUILTIN_NAME(CCfKeyInterface)
class CCfKeyInterface: public cmmBaseObject<CCfKeyInterface,ICfKey,GET_BUILTIN_NAME(CCfKeyInterface)>
{
	friend class CConfigFile;
public:
	CCfKeyInterface(){
//		miReferenceCount = 1;	// 相当于记录了一次引用
		mbDeleted = false;
	}
	virtual ~CCfKeyInterface();

	ULONG InitOnCreate(CConfigFile* npConfigFile){
		mpConfigFile = npConfigFile;
		return 0;
	}

	void Reuse(CConfigFile* npConfigFile){
		mbDeleted = false;
		mpConfigFile = npConfigFile;
		cmmBaseObject<CCfKeyInterface,ICfKey,GET_BUILTIN_NAME(CCfKeyInterface)>::Reuse();	
	}


	DEFINE_CUMSTOMIZE_CREATE(CCfKeyInterface,(CConfigFile* npConfigFile),(npConfigFile))

	// 增加引用
	virtual int __stdcall AddRefer(void);

	//释放键，新建，打开，或者任何一种方法得到的本类对象都必须释放
	virtual int __stdcall Release(void);

	//删除这个键，注意，调用删除后的接口仍然需要Release；调用过Delete后，这个接口也就没有用了，出Release外，其他功能都不支持了
	virtual bool __stdcall Delete(void);	

	// 新建一个键，返回NULL表示失败，失败的原因可能是已经具有相同的键值，或者分配内存失败
	virtual ICfKey* __stdcall NewSubKey(
		IN const wchar_t* nszName,	// 子键的名字，不能为空
		IN IConfigFile::VALUETYPE nuValueType = IConfigFile::Invalid,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
		IN const void* npValueBuf = NULL,
		IN LONG  niValuelen = 0	// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
		);

	// 新建一个无名键，返回NULL表示失败；如果同时存在命名键值，将有可能存在命名键值夹在无名键中间的情况
	virtual ICfKey* __stdcall NewSubKey(
		IN ULONG nuID,		// 指定无名键的标识ID，尽可以不要使用相同标识
		IN bool nbAhead=false,	// 当遇到相同ID的子键时，在相同ID子键之前建立新键；否则，在相同ID子键之后建立新键
		IN IConfigFile::VALUETYPE nuValueType = IConfigFile::Invalid,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
		IN const void* npValueBuf = NULL,
		IN LONG  niValuelen = 0		// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
		);

	// 获得本键的父键，如果本键是根键，则返回NULL
	virtual ICfKey* __stdcall GetParentsKey(void);

	// 重新设置父节点，将本节点从当前父节点移除，插入新的父节点下
	virtual bool __stdcall SetParentKey(
		IN ICfKey* npNewParent,
		IN bool nbAhead
		);

	// 获得下一个键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetNextKey(void);

	// 获得第一个子键，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall GetSubKey(void);

	// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的GetSubKey区别是，上面的按名字打开键值，而本函数直接按ID打开，只有通过本函数才能打开没有名字的键值
	virtual ICfKey* __stdcall GetSubKey(
		IN ULONG nuID,		// 子键的标识ID
		IN int niPos=0		// 在相同的ID的键值中的先后位置，< 0表示取最后一个
		);

	// 打开多层路径指定的子键，获得的对象当不再访问时需要Release；本函数同下面的GetSubKey区别是，本函数可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
	virtual ICfKey* __stdcall OpenKey(
		IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
		IN bool nbCreateIf=false			// 如果这个键不存在，则建立它
		);

	// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的OpenKey区别是，OpenKey可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
	virtual ICfKey* __stdcall GetSubKey(
		IN const wchar_t* nszName,	// 按照名字去获取子键
		IN int niNameLen=-1,	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		OUT int* npNameLen=NULL,	// 返回名字的有效长度
		IN bool nbCreateIf=false	// 如果该键不存在，则建立它
		);

	// 获得下一个键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToParentsKey(void);

	// 获得下一个键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToNextKey(void);

	// 获得下一个键，删除并释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToNextKey(bool nbDelete);

	// 获得第一个子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(void);

	// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(
		IN ULONG nuID		// 子键的标识ID
		);

	// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
	virtual ICfKey* __stdcall MoveToSubKey(
		IN const wchar_t* nszName,	// 按照名字去获取子键
		IN int niNameLen=-1	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		);

	// 获得本键的默认值类型
	virtual IConfigFile::VALUETYPE __stdcall GetValueType(void);

	// 获得本键的扩展标志，定义在下面
	virtual UCHAR __stdcall GetExtFlag(void);

	// 获得本键的默认值长度，字节单位
	virtual LONG __stdcall GetValueLength(void);

	// 获得本键的名字
	virtual int __stdcall GetName(wchar_t* npNameBuff,int niBufLenByWchar);

	// 改名，
	virtual bool __stdcall Rename(
		IN const wchar_t* nszName,
		IN bool FailIfExist=true	//	FailIfExist==true如果改名会导致同一节点下出现同名节点，将失败;  ==false 将自动增加附加字符
		);

	// 设置或改变当前的默认值
	virtual bool __stdcall SetValue(
		IN IConfigFile::VALUETYPE nuValueType,	// 子键的默认值类型
		IN const void* npValueBuf,
		IN LONG  niValuelen=-1
		);

	// 获得当前默认值，返回获得的值的字节数
	virtual int __stdcall GetValue(
		OUT PVOID npValueBuf,
		IN  LONG  niBufLen
		);

	// 获得ID，对于命名键值而言获得的是Hash值
	virtual ULONG __stdcall GetID(
		OUT int* npPos=NULL		// 在相同的ID的键值中的先后位置
		);

	// 获得本键所在的序号
	virtual int __stdcall GetPosition(void);

	// 修改扩展标志
	virtual bool __stdcall SetExtFlag(
		IN UCHAR nchFlag
		);

	// 直接将值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QueryValueAsLONG(
		IN LONG niDefault= -1
		);

	// 直接将值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QueryValueAsFLOAT(
		IN FLOAT nfDefault = 0.0f
		);

	// 直接将值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QueryValueAsPVOID(
		IN PVOID npDefault = NULL
		);

	// 按名字查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QuerySubKeyValueAsLONG(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN LONG niDefault= -1
		);

	// 按ID查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
	virtual LONG __stdcall QuerySubKeyValueAsLONG(
		IN ULONG nuID,		// 子键的标识ID
		IN LONG niDefault= -1
		);

	// 按名字查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QuerySubKeyValueAsFLOAT(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN FLOAT nfDefault = 0.0f
		);

	// 按ID查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
	virtual FLOAT __stdcall QuerySubKeyValueAsFLOAT(
		IN ULONG nuID,		// 子键的标识ID
		IN FLOAT nfDefault = 0.0f
		);

	// 按名字查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QuerySubKeyValueAsPVOID(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		IN PVOID npDefault = NULL
		);

	// 按ID查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
	virtual PVOID __stdcall QuerySubKeyValueAsPVOID(
		IN ULONG nuID,		// 子键的标识ID
		IN PVOID npDefault = NULL
		);

	// 按名字查找子键，直接获取子键的值，返回小于零表示目标键不存在，等于0表示目标键值为空，大于0表示返回的Value长度
	virtual int __stdcall QuerySubKeyValue(
		IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
		OUT PVOID npValueBuf,
		IN  LONG  niBufLen
		);

	// 获取指定的值存储New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QueryValueAsBuffer();

	// 获取指定的子健的值存储在New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QuerySubKeyValueAsBuffer(
		IN const wchar_t* nswSubKeyName
		); 

	// 获取指定的子健的值存储在New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
	virtual PVOID __stdcall QuerySubKeyValueAsBuffer(
		IN ULONG nuID									// 子键的标识ID
		); 

	// 释放由Query***AsBuffer系列函数返回的Buffer  ——Colin 加。
	virtual void _stdcall ReleaseBuffer(OUT PVOID npvoBufferToRelease);

protected:
	//int miReferenceCount;
	bool mbDeleted;

	union {
		PCFKEY_NODE BaseNode;
		PCFKEY_BRANCH Branch;
		PCFKEY_LEAF Leaf;
	};
	CCfKeyInterface* mpParentsKey;
	int miPositionInBrothers;	// 本节点在父节点的子节点排序组中的位置

	CConfigFile* mpConfigFile;

	// 查找子节点，返回子节点在Hash数组中的序号，返回-1表示没有找到
	int FindSubKey(
		IN const wchar_t* nszName,
		IN int niNameLen=-1,	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		OUT int* npNameLen=NULL	// 返回名字的有效长度
		);

	// 查找子节点，返回子节点在Hash数组中的序号，返回-1表示没有找到
	int FindSubKey(
		ULONG nuHashCode,
		int niPos=0		// 在相同的ID的键值中的先后位置，< 0表示取最后一个
		);

	// 插入一个新的子节点，返回插入的位置
	inline int InsertSubNode(
		PCFKEY_NODE npNode,
		ULONG nuHashCode=0,
		IN bool nbAhead=false	// 当遇到相同ID的子键时，在相同ID子键之前建立新键；否则，在相同ID子键之后建立新键
		);

	// 获得子节点
	inline PCFKEY_NODE GetSubNode(
		IN int niPosition
		);

	// 移除子节点
	inline bool RemoveSubNode(
		IN int niPosition
		);

	// 更新子节点，这不会改变排序次序
	inline bool UpdateSubNode(
		PCFKEY_NODE npNode,
		IN int niPosition
		);
};



//////////////////////////////////////////////////////////////////////////
// 配置文件类2
//	本类派生之普通配置文件类，同前者的区别是，增加了副本支持，具体如下：
// 配置文件将存在前后两次打开的不同的两个，它们交替使用，互为备份，从而防止某次的使用过程中破坏了其中一个文件；
// 需要注意，备份的是打开前的内容，当一个打开的配置被多次改写，而最后一次写入的文件最后损坏，将来会自动回复的不是前一次写入的数据，而是前次对象打开前最后保存的数据。
// 如：如果配置文件名为abc.set，那么在同一个目录下将可能存在一个abc_dup.set的文件，它就是副本；
DECLARE_BUILTIN_NAME(CStableConfigFile)
class CStableConfigFile:public CConfigFile
{
public:
	CStableConfigFile(){
		mbRestored = false;
	}

	ULONG InitOnCreate(
		IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
		IN ULONG nuCreationDisposition			// 同CreateFile API类似，见CfgIface.h文件中的相关定义
		);

	~CStableConfigFile(){
	}

	// 重载创建工具函数
	DEFINE_CUMSTOMIZE_CREATE(CStableConfigFile,(const wchar_t* nszPathName=NULL,ULONG nuCreationDisposition=CF_OPEN_EXISTING),(nszPathName,nuCreationDisposition))

	// 重载类型识别
	DEFINE_DERIVED_TYPECAST(CStableConfigFile,CConfigFile)

	// 判断打开配置时，是否发生了最新配置损坏，从备份中恢复的行为；???尚未支持
	bool IsRestored(){
		return mbRestored;
	}

private:
	bool mbRestored;

};



#endif//_CONFIGBASE_H_