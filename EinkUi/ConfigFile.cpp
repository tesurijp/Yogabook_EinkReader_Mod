#ifdef KERNEL_CODE
#include "ntddk.h"
#else
#include "Windows.h"
#include "time.h"
#endif


#include "ConfigFile.h"


#ifndef KERNEL_CODE
#define COPYSTR_S(_D,_L,_S) wcscpy_s(_D,_L,_S)
#else
#define COPYSTR_S(_D,_L,_S) wcscpy(_D,_S)
#endif

DEFINE_BUILTIN_NAME(CConfigFile)
DEFINE_BUILTIN_NAME(CCfKeyInterface)
DEFINE_BUILTIN_NAME(CStableConfigFile)

#ifdef __cplusplus
extern "C" {
#endif


// 建立或者打开一个配置文件，返回配置文件对象或者返回NULL表示失败，失败的原因可能是文件不存在或者文件格式错误
IConfigFile* __stdcall CfCreateConfig(
		IN const wchar_t* nszPathName,				// 文件的完整路径名
		IN ULONG nuCreationDisposition			// 同CreateFile API类似，见CfgIface.h文件中的相关定义
		)
{
	return  CConfigFile::CreateInstance(nszPathName,nuCreationDisposition);
}

// 同上面的函数的区别是，StableConfig会自动为文件建立一个备份副本，防止操作文件或者其他原因导致单一文件破坏
// 配置文件将存在前后两次打开的不同的两个，它们交替使用，互为备份，从而防止某次的使用过程中破坏了其中一个文件；
// 需要注意，备份的是打开前的内容，当一个打开的配置被多次改写，而最后一次写入的文件最后损坏，将来会自动回复的不是前一次写入的数据，而是前次对象打开前最后保存的数据。
// 如：如果配置文件名为abc.set，那么在同一个目录下将可能存在一个abc_dup.set的文件，它就是副本；
IConfigFile* __stdcall CfCreateStableConfig(
	IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
	IN ULONG nuCreationDisposition	// 同CreateFile API类似，但仅包括CREATE_ALWAYS、CREATE_NEW、OPEN_ALWAYS、OPEN_EXISTING，定义见下面
	)
{
	return CStableConfigFile::CreateInstance(nszPathName,nuCreationDisposition);
}
#ifdef __cplusplus
}
#endif

CConfigFile::CConfigFile()
{
	miReferenceCount = 1;	// 相当于记录了一次引用
	mpRootInterface = NULL;
	mpRoot = NULL;
}

CConfigFile::~CConfigFile()
{
	CMMASSERT(miReferenceCount<=0);
	while(moUnusedKeyInterface.Size()>0)
	{
		CCfKeyInterface* lpKey = moUnusedKeyInterface.Top();
		moUnusedKeyInterface.Pop();

		if(lpKey!=NULL)
			delete lpKey;
	}
	if(mpRootInterface != NULL)
		mpRootInterface->Release();
	// 释放内存
	if(mpRoot != NULL)
		RemoveKey((PCFKEY_NODE)mpRoot);
}

ULONG CConfigFile::InitOnCreate(
		IN const wchar_t* nszPathName,				// 文件的完整路径名
		IN ULONG nuCreationDisposition			// 同CreateFile API类似，见CfgIface.h文件中的相关定义
	)
{
	bool lbReval = false;
#ifdef KERNEL_CODE
	OBJECT_ATTRIBUTES ObjectAttributes;
	UNICODE_STRING FileName;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
#endif
	PCF_FILE_HEAD lpHead = NULL;
	PVOID lpBuffer = NULL;
	PCF_KEY_ENTRY lpEntry = NULL;
	PCF_KEY_ENTRY lpEndEntry = NULL;
	HANDLE lhFile = NULL;
	bool lbResetConfig = false;

	do{
		// 如果nszPathName为NULL，表示建立一个新的文件
		if(nszPathName != NULL)
		{
			COPYSTR_S(mszFileName,256,nszPathName);
			// 打开文件
	#ifndef KERNEL_CODE
			lhFile = ::CreateFileW(mszFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,nuCreationDisposition,NULL,NULL);
			if(lhFile == INVALID_HANDLE_VALUE)
			{
				lhFile = NULL;
				break;
			}
			{
				LARGE_INTEGER lxFileSize;
				if(GetFileSizeEx(lhFile,&lxFileSize)==FALSE)
					break;
				if(lxFileSize.QuadPart == 0)
					lbResetConfig = true;
			}
	#else
			RtlInitUnicodeString(&FileName,mszFileName);
			InitializeObjectAttributes(
				&ObjectAttributes
				,&FileName
				,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE
				,NULL 
				,NULL );

			Status = ZwCreateFile(&lhFile,FILE_READ_DATA|SYNCHRONIZE,&ObjectAttributes,&IoStatus,NULL,FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,nuCreationDisposition,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
			if(!NT_SUCCESS(Status))
				break;

			if(IoStatus.Information == FILE_CREATED || IoStatus.Information == FILE_DOES_NOT_EXIST)
				lbResetConfig = true;

	#endif

			// 判断文件是否是新建立的，如果是新建的lbResetConfig将会是true
			if(lbResetConfig == false)
			{
				bool lbLoadOk = false;
				do 
				{
					// 读入文件头
					lpHead = new CF_FILE_HEAD;
					if(lpHead == NULL)
						break;

			#ifndef KERNEL_CODE
					ULONG luRead;
					if(ReadFile(lhFile,lpHead,sizeof(CF_FILE_HEAD),&luRead,NULL)==FALSE)
						break;
			#else
					if(STATUS_SUCCESS != ZwReadFile(lhFile,NULL,NULL,NULL,&IoStatus,lpHead,sizeof(CF_FILE_HEAD),NULL,NULL))
						break;
			#endif

					// 检查版本
					if(lpHead->ShortHead.Signature != CF_SIGNATURE || lpHead->ShortHead.Version > CF_VERSION)
						break;

					// 检查文件是否有失败的修改
					if(lpHead->ShortHead.SequenceA != lpHead->Tail.SequenceB)
						break;	// 上次的修改没有正确被保存，当前文件可能损坏

					if (lpHead->ShortHead.SizeOfKeys > 0)
					{
						// 一次行读入全部数据
						lpBuffer = new UCHAR[lpHead->ShortHead.SizeOfKeys];
		#ifndef KERNEL_CODE
						if(ReadFile(lhFile,lpBuffer,lpHead->ShortHead.SizeOfKeys,&luRead,NULL)==FALSE)
							break;
		#else
						if(STATUS_SUCCESS != ZwReadFile(lhFile,NULL,NULL,NULL,&IoStatus,lpBuffer,lpHead->ShortHead.SizeOfKeys,NULL,NULL))
							break;
		#endif
						// 判断是否有正确的校验和
						if(lpHead->ShortHead.Version >= CF_VERSION)
						{
							ULONG luCheckSum = 0;
							for (ULONG i=0;i< lpHead->ShortHead.SizeOfKeys;i++)
							{
								luCheckSum += *((UCHAR*)lpBuffer+i);
							}
							if(luCheckSum != lpHead->ShortHead.CheckSum)
								break;	// 校验和不对
						}

						// 递归调用执行键装载
						lpEntry = (PCF_KEY_ENTRY)lpBuffer;
						lpEndEntry = (PCF_KEY_ENTRY)((UCHAR*)lpBuffer + lpHead->ShortHead.SizeOfKeys);

						mpRoot = (PCFKEY_BRANCH)LoadKey(lpEntry,lpEndEntry);
						if(mpRoot == NULL)
							break;

						// 复制当前文件头
						RtlCopyMemory(&mdFileHead,&lpHead->ShortHead,sizeof(mdFileHead));

						lbLoadOk = true;
					}
				} while(false);
				if(lbLoadOk == false)
				{
					if (mpRoot!=NULL)
						RemoveKey((PCFKEY_NODE)mpRoot);
					lbResetConfig = true;
				}
			}
		}
		else
		{
			mszFileName[0] = UNICODE_NULL;
			lbResetConfig = true;
		}


		if(lbResetConfig != false)
		{
			// 准备默认文件头
			mdFileHead.Signature = CF_SIGNATURE;
			mdFileHead.Version = CF_VERSION;
			mdFileHead.SizeOfHead = sizeof(CF_FILE_HEAD);
			mdFileHead.SizeOfReserved = 0;
			mdFileHead.SequenceA = 1;

			// 是一个新建的文件，建立一个空的Root节点就好了
			mpRoot = (PCFKEY_BRANCH)new UCHAR[sizeof(CFKEY_BRANCH)+sizeof(wchar_t)*4];
			if(mpRoot == NULL)
				break;
			mpRoot->Flag = IConfigFile::Binary|CFKEY_INDEX_AVAILABLE;
			CFKEY_SET_VALUETYPE(mpRoot->Flag,IConfigFile::Invalid);

			mpRoot->NameLength = 4;
			COPYSTR_S(mpRoot->Name,5,L"Root");
			mpRoot->mpSubKeys = new TCFKEYSEQUENCE;
			if(mpRoot->mpSubKeys == NULL)
				break;
			mpRoot->ValueLength = 0;
		}

		// 获取一次RootKey
		{
			ICfKey* lpKey = GetRootKey();
			if(lpKey == NULL)
				break;
			lpKey->Release();
		}

		lbReval = true;

	}while(false);


	if(lpHead != NULL)
		delete lpHead;

	if(lpBuffer != NULL)
		delete lpBuffer;

	if(lhFile != NULL)
	{
#ifndef KERNEL_CODE
		CloseHandle(lhFile);
#else
		ZwClose(lhFile);
#endif
	}

	return (lbReval?0:(ULONG)-1);
}

// 保存修改到文件
bool CConfigFile::SaveFile(
		IN const wchar_t* nszOtherFile	// 可以指定保存为其他的文件，如果为NULL，这保存到刚才打开或者新建的文件
	)
{
	bool lbReval = false;
#ifdef KERNEL_CODE
	OBJECT_ATTRIBUTES ObjectAttributes;
	UNICODE_STRING FileName;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
#endif
	PCF_FILE_HEAD lpHead = NULL;
	PVOID lpBuffer = NULL;
	PCF_KEY_ENTRY lpEntry = NULL;
	HANDLE lhFile = NULL;


	do{
		if(nszOtherFile != NULL)
		{
			COPYSTR_S(mszFileName,256,nszOtherFile);
		}

		if(mszFileName[0]==UNICODE_NULL)
			break;

		// 打开文件
#ifndef KERNEL_CODE
		lhFile = ::CreateFileW(mszFileName,GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_ALWAYS,NULL,NULL);
		if(lhFile == INVALID_HANDLE_VALUE)
		{
			lhFile = NULL;
			break;
		}
#else
		RtlInitUnicodeString(&FileName,mszFileName);
		InitializeObjectAttributes(
			&ObjectAttributes
			,&FileName
			,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE
			,NULL 
			,NULL );

		Status = ZwCreateFile(&lhFile,FILE_WRITE_DATA|SYNCHRONIZE,&ObjectAttributes,&IoStatus,NULL,FILE_ATTRIBUTE_NORMAL,0,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
		if(!NT_SUCCESS(Status))
			break;

#endif

		// 准备文件头
		lpHead = new CF_FILE_HEAD;
		if(lpHead == NULL)
			break;

		RtlZeroMemory(lpHead,sizeof(CF_FILE_HEAD));
		RtlCopyMemory(&lpHead->ShortHead,&mdFileHead,sizeof(mdFileHead));
		lpHead->ShortHead.Version = CF_VERSION;

		// 设定一个错误的顺序值，作为正在修改文件的标志
		lpHead->ShortHead.SequenceA = lpHead->Tail.SequenceB + 1;

		LARGE_INTEGER lxFilePos;
		lxFilePos.QuadPart = 0;

#ifndef KERNEL_CODE
		ULONG luWritten;
		if(SetFilePointerEx(lhFile,lxFilePos,NULL,FILE_BEGIN)==FALSE)
			break;
		if(WriteFile(lhFile,lpHead,sizeof(CF_FILE_HEAD),&luWritten,NULL)==FALSE)
			break;
#else
		Status = ZwWriteFile(lhFile,NULL,NULL,NULL,&IoStatus,lpHead,sizeof(CF_FILE_HEAD),&lxFilePos,NULL);
		if(STATUS_SUCCESS != Status)
			break;
#endif

		// 尝试确定所有Key将占用的尺寸
		moExclusive.Enter();
		lpHead->ShortHead.SizeOfKeys = (LONG)SaveKey((PCFKEY_NODE)mpRoot,lpEntry);
		if(lpHead->ShortHead.SizeOfKeys == 0)
		{
			moExclusive.Leave();
			break;
		}

		// 准备全部存储空间
		// 准备存储区
		lpBuffer = new UCHAR[lpHead->ShortHead.SizeOfKeys];

		// 递归灌入数据
		lpEntry = (PCF_KEY_ENTRY)lpBuffer;
		//moExclusive.Enter();
		LONG liSize = (LONG)SaveKey((PCFKEY_NODE)mpRoot,lpEntry);
		moExclusive.Leave();
		CMMASSERT(((UCHAR*)lpEntry - (UCHAR*)lpBuffer) <= liSize);
		if(liSize != lpHead->ShortHead.SizeOfKeys)
			break;

		// 计算校验和
		lpHead->ShortHead.CheckSum = 0;
		for(ULONG i=0;i< lpHead->ShortHead.SizeOfKeys;i++)
		{
			lpHead->ShortHead.CheckSum += *((UCHAR*)lpBuffer+i);
		}

		// 写入文件
		lxFilePos.QuadPart = sizeof(CF_FILE_HEAD);
#ifndef KERNEL_CODE
		if(WriteFile(lhFile,lpBuffer,lpHead->ShortHead.SizeOfKeys,&luWritten,NULL)==FALSE)
			break;
#else
		Status = ZwWriteFile(lhFile,NULL,NULL,NULL,&IoStatus,lpBuffer,lpHead->ShortHead.SizeOfKeys,&lxFilePos,NULL);
		if(STATUS_SUCCESS != Status)
			break;
#endif

		// 更正文件头，重新写入数据
		lpHead->ShortHead.SequenceA++;
		lpHead->Tail.SequenceB = lpHead->ShortHead.SequenceA;
		lxFilePos.QuadPart = 0;
#ifndef KERNEL_CODE
		if(SetFilePointerEx(lhFile,lxFilePos,NULL,FILE_BEGIN)==FALSE)
			break;
		if(WriteFile(lhFile,lpHead,sizeof(CF_FILE_HEAD),&luWritten,NULL)==FALSE)
			break;
#else
		Status = ZwWriteFile(lhFile,NULL,NULL,NULL,&IoStatus,lpHead,sizeof(CF_FILE_HEAD),&lxFilePos,NULL);
		if(STATUS_SUCCESS != Status)
			break;
#endif

		lbReval = true;

	}while(false);


	if(lpHead != NULL)
		delete lpHead;

	if(lpBuffer != NULL)
		delete lpBuffer;

	if( lhFile != NULL)
	{
#ifndef KERNEL_CODE
		CloseHandle(lhFile);
#else
		ZwClose(lhFile);
#endif
	}

	return lbReval;
}


// 获得根键，获得的对象当不再访问时需要Release
ICfKey* CConfigFile::GetRootKey(void)
{

	moExclusive.Enter();
	if(mpRootInterface == NULL)
	{
		mpRootInterface = AllocateKeyInterface((PCFKEY_NODE)mpRoot);
		if(mpRootInterface == NULL)
		{
			moExclusive.Leave();
			return NULL;
		}

		mpRootInterface->mpParentsKey = NULL;
		mpRootInterface->miPositionInBrothers = -1;
		// 还需要初始化访问数据和名称的快捷
	}

	mpRootInterface->miReferenceCount++;
	

	moExclusive.Leave();

	return dynamic_cast<ICfKey*>(mpRootInterface);
}

// 分配一个新的Node
PCFKEY_NODE CConfigFile::AllocateNode(
	IN const wchar_t* nszName,	// 子键的名字
	IN int niNameLen,		// count in wchar, if be -1 indicate that the nszName is terminated by '\0' or '\\' or '/'
	IN UCHAR nchFlag,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
	IN const void* npValueBuf,
	IN int  niValuelen,
	IN int  niSubKeyCount
	)
{
	int liSize;
	PCFKEY_NODE lpRetrun = NULL;

	if(nszName == NULL)
		niNameLen = 0;
	else
	{
		if(niNameLen < 0)
			niNameLen = 255;

		// the max length is 255
		for(liSize = 0;liSize<niNameLen;liSize++)
		{
			if(/*nszName[liSize] == L'\\' || nszName[liSize] == L'/' || */nszName[liSize] == L'<' || nszName[liSize] == L'>')
				return NULL;	// 名字中出现非法字符

			if(nszName[liSize]==L'\0' || nszName[liSize] == L'\\' || nszName[liSize] == L'/' )
				break;
		}

		niNameLen = liSize;
	}

	if(niValuelen == -1)
		niValuelen = GetValueLengthOfTypeString(CFKEY_VALUE_TYPE(nchFlag),npValueBuf);

	// 去掉是否已打开标志，现在是无意义的
	nchFlag &= ~CFKEY_NODE_OPENED;

	// 区分是否具有子节点
	if(niSubKeyCount > 0)
	{	// 具有子节点
		PCFKEY_BRANCH lpBranch = NULL;
		ULONG* lpHash = NULL;

		do 
		{
			// 计算当前节点需要的存储大小
			liSize = sizeof(CFKEY_BRANCH) + niNameLen*sizeof(wchar_t) + niValuelen;

			// 分配当前节点
			lpBranch = (PCFKEY_BRANCH)new UCHAR[liSize];
			if(lpBranch == NULL)
				break;

			lpBranch->Flag = (nchFlag|CFKEY_INDEX_AVAILABLE);
			lpBranch->mpSubKeys = new TCFKEYSEQUENCE;
			if(lpBranch->mpSubKeys == NULL)
				break;

			// 填入名称
			lpBranch->NameLength = (UCHAR)niNameLen;
			if(niNameLen > 0 )
			{
				RtlCopyMemory(lpBranch->Name,nszName,niNameLen*sizeof(wchar_t));
				lpBranch->Name[niNameLen] = UNICODE_NULL;
			}

			// 填入数据
			lpBranch->ValueLength = (USHORT)niValuelen;
			if(lpBranch->ValueLength > 0)
			{
				RtlCopyMemory(&lpBranch->Name[lpBranch->NameLength]+1,npValueBuf,lpBranch->ValueLength);
			}

			// 成功返回
			lpRetrun = (PCFKEY_NODE)lpBranch;
			lpBranch = NULL;

		} while(false);

		if(lpBranch != NULL)
		{
			if(lpBranch->mpSubKeys != NULL)
				delete lpBranch->mpSubKeys;
			delete lpBranch;
		}
	}
	else
	{	// 没有子节点
		PCFKEY_LEAF lpLeaf = NULL;

		do 
		{
			// 计算当前节点需要的存储大小
			liSize = sizeof(CFKEY_LEAF) + niNameLen*sizeof(wchar_t) + niValuelen;

			// 分配当前节点
			lpLeaf = (PCFKEY_LEAF)new UCHAR[liSize];
			if(lpLeaf == NULL)
				break;

			lpLeaf->Flag = nchFlag;

			// 填入名称
			lpLeaf->NameLength = (UCHAR)niNameLen;
			if(niNameLen > 0 )
			{
				RtlCopyMemory(lpLeaf->Name,nszName,niNameLen*sizeof(wchar_t));
				lpLeaf->Name[niNameLen] = UNICODE_NULL;
			}

			// 填入数据
			lpLeaf->ValueLength = (USHORT)niValuelen;
			if(lpLeaf->ValueLength > 0)
			{
				RtlCopyMemory(&lpLeaf->Name[lpLeaf->NameLength]+1,npValueBuf,niValuelen);
			}

			// 成功返回
			lpRetrun = (PCFKEY_NODE)lpLeaf;
			lpLeaf = NULL;

		} while(false);

		if(lpLeaf != NULL)
			delete lpLeaf;
	}

	return lpRetrun;
}


// 打开指定的键，获得的对象当不再访问时需要Release
ICfKey* CConfigFile::OpenKey(
		IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
		IN bool nbCreateIf			// 如果这个键不存在，则建立它
	)
{
	ICfKey* lpRoot = GetRootKey();
	ICfKey* lpSub = lpRoot->OpenKey(nszKeyName,nbCreateIf);
	lpRoot->Release();
	return lpSub;
}


// 递归调用，装载指定键和他的子键，返回时nrKeyEntry会修改指向下一个Entry
PCFKEY_NODE CConfigFile::LoadKey(PCF_KEY_ENTRY& nrKeyEntry,const PCF_KEY_ENTRY npEndEntry)
{
	PCFKEY_NODE lpRetrun = NULL;

	if(nrKeyEntry >= npEndEntry)	// 超过尾部了，肯定是有错误发生
		return NULL;

	if(nrKeyEntry->NameLength > 255 )	// 有错误，这不是这个版本合法的标志值
		return NULL;


	// 区分是否具有子节点
	if((nrKeyEntry->Flag&CFKEY_INDEX_AVAILABLE) != 0)
	{	// 具有子节点
		PCFKEY_BRANCH lpBranch = NULL;
		ULONG* lpHash = NULL;

		do 
		{
			// 分配当前节点
			lpBranch = (PCFKEY_BRANCH)AllocateNode((wchar_t*)(nrKeyEntry+1),nrKeyEntry->NameLength,nrKeyEntry->Flag,(UCHAR*)((wchar_t*)(nrKeyEntry+1)+nrKeyEntry->NameLength),nrKeyEntry->ValueSize,nrKeyEntry->SubEntryCount);
			if(lpBranch == NULL)
				break;

			lpBranch->mpSubKeys = new TCFKEYSEQUENCE;
			if(lpBranch->mpSubKeys == NULL)
				 break;

			// 循环读入每一个子节点，并将子节点的Hash值与子节点一同插入索引表中
			int liSubCount = nrKeyEntry->SubEntryCount;
			lpHash = (ULONG*)((UCHAR*)nrKeyEntry + sizeof(CF_KEY_ENTRY) + nrKeyEntry->NameLength*sizeof(wchar_t) + nrKeyEntry->ValueSize);	// 指向Hash表
			// 输入数据后移，指向下一条记录
			nrKeyEntry = (PCF_KEY_ENTRY)(lpHash + nrKeyEntry->SubEntryCount);	// 指向下一条记录
			while(--liSubCount >=  0)
			{
				CCfKeyHash loNewSubKey;

				loNewSubKey.HashValue = *lpHash;
				loNewSubKey.KeyObj = LoadKey(nrKeyEntry,npEndEntry);

				if(loNewSubKey.KeyObj == NULL)
				{
					break;
				}
				lpBranch->mpSubKeys->Push_Back(loNewSubKey);
				lpHash++;
			}
			//////////////////////////////////////////////////////////////////////////
			// discarded by AX. Do not terminate the loading on the event that a bad key is found for fastboot version.
			//if(liSubCount >= 0)
			//	break;	// 出错了

			// 成功返回
			lpRetrun = (PCFKEY_NODE)lpBranch;
			lpBranch = NULL;

		} while(false);

		if(lpBranch != NULL)
			delete lpBranch;
	}
	else
	{	// 没有子节点
		PCFKEY_LEAF lpLeaf = NULL;

		do 
		{
			// 分配当前节点
			lpLeaf = (PCFKEY_LEAF)AllocateNode((wchar_t*)(nrKeyEntry+1),nrKeyEntry->NameLength,nrKeyEntry->Flag,(UCHAR*)((wchar_t*)(nrKeyEntry+1)+nrKeyEntry->NameLength),nrKeyEntry->ValueSize);
			if(lpLeaf == NULL)
				break;

			// 输入数据后移，指向下一条记录
			nrKeyEntry = (PCF_KEY_ENTRY)((UCHAR*)nrKeyEntry + sizeof(CF_KEY_ENTRY) + nrKeyEntry->NameLength*sizeof(wchar_t) + nrKeyEntry->ValueSize);

			// 成功返回
			lpRetrun = (PCFKEY_NODE)lpLeaf;
			lpLeaf = NULL;

		} while(false);

		if(lpLeaf != NULL)
			delete lpLeaf;
	}

	return lpRetrun;
}

// 递归调用，保存本键和子键的值，返回0表示失败。返回式nrKeyEntry会越过已经存放的数据。如果输入时nrKeyEntry==NULL，则只返回需要的存储区大小
int CConfigFile::SaveKey(PCFKEY_NODE npKey,PCF_KEY_ENTRY& nrKeyEntry)
{
	int liSize;
	int liReval = 0;

	// 区分是否具有子节点
	if((npKey->Flag&CFKEY_INDEX_AVAILABLE) != 0)
	{	// 具有子节点
		PCFKEY_BRANCH lpBranch = (PCFKEY_BRANCH)npKey;
		ULONG* lpHash = NULL;

		do 
		{
			// 计算当前节点需要的存储大小
			liSize = sizeof(CF_KEY_ENTRY) + lpBranch->NameLength*sizeof(wchar_t) + lpBranch->ValueLength + lpBranch->mpSubKeys->Size()*sizeof(ULONG);

			// 如果nrKeyEntry==NULL，只需要计算存储这个键和他的子键需要的空间就可以了
			if(nrKeyEntry != NULL)
			{
				nrKeyEntry->Flag = lpBranch->Flag;
				if(lpBranch->mpSubKeys->Size() == 0)
					nrKeyEntry->Flag &= (~CFKEY_INDEX_AVAILABLE);	// 说明，已经没有了子键，所以不能有子键标志

				// 填入名称
				nrKeyEntry->NameLength = lpBranch->NameLength;
				RtlCopyMemory((wchar_t*)(nrKeyEntry+1),lpBranch->Name,lpBranch->NameLength*sizeof(wchar_t));

				// 填入数据
				nrKeyEntry->ValueSize = lpBranch->ValueLength;
				if(lpBranch->ValueLength > 0)
				{
					RtlCopyMemory((UCHAR*)((wchar_t*)(nrKeyEntry+1)+nrKeyEntry->NameLength),&lpBranch->Name[lpBranch->NameLength]+1,lpBranch->ValueLength);
				}

				// 填入Hash表
				lpHash = (ULONG*)((UCHAR*)nrKeyEntry + sizeof(CF_KEY_ENTRY) + nrKeyEntry->NameLength*sizeof(wchar_t) + nrKeyEntry->ValueSize);
				nrKeyEntry->SubEntryCount = 0;
				for(int i=0;i<lpBranch->mpSubKeys->Size();i++)
				{
					lpHash[i] = (*lpBranch->mpSubKeys)[i].HashValue;
					nrKeyEntry->SubEntryCount++;
				}

				// 输入数据后移，指向下一条记录
				nrKeyEntry = (PCF_KEY_ENTRY)(lpHash + nrKeyEntry->SubEntryCount);	// 指向下一条记录
			}

			// 循环保存每一个子节点
			int i;
			for(i=0;i<lpBranch->mpSubKeys->Size();i++)
			{
				int liSubSize = SaveKey((*lpBranch->mpSubKeys)[i].KeyObj,nrKeyEntry);
				if(liSubSize == 0)
					break;
				liSize += liSubSize;
			}
			if(i < lpBranch->mpSubKeys->Size())
				break;

			// 成功返回
			liReval = liSize;

		} while(false);
	}
	else
	{	// 没有子节点
		PCFKEY_LEAF lpLeaf = (PCFKEY_LEAF)npKey;

		do 
		{
			// 计算当前节点需要的存储大小
			liSize = sizeof(CF_KEY_ENTRY) + lpLeaf->NameLength*sizeof(wchar_t) + lpLeaf->ValueLength;

			// 如果nrKeyEntry==NULL，只需要计算存储这个键和他的子键需要的空间就可以了
			if(nrKeyEntry != NULL)
			{
				nrKeyEntry->Flag = lpLeaf->Flag;

				// 填入名称
				nrKeyEntry->NameLength = lpLeaf->NameLength;
				RtlCopyMemory((wchar_t*)(nrKeyEntry+1),lpLeaf->Name,lpLeaf->NameLength*sizeof(wchar_t));

				// 填入数据
				nrKeyEntry->ValueSize = lpLeaf->ValueLength;
				if(lpLeaf->ValueLength > 0)
				{
					RtlCopyMemory((UCHAR*)((wchar_t*)(nrKeyEntry+1)+nrKeyEntry->NameLength),&lpLeaf->Name[lpLeaf->NameLength]+1,lpLeaf->ValueLength);
				}

				// 输入数据后移，指向下一条记录
				nrKeyEntry = (PCF_KEY_ENTRY)((UCHAR*)nrKeyEntry + sizeof(CF_KEY_ENTRY) + nrKeyEntry->NameLength*sizeof(wchar_t) + nrKeyEntry->ValueSize);
			}

			// 成功返回
			liReval = liSize;
		} while(false);

	}

	return liReval;
}

// 删除键值和它的子键值
bool CConfigFile::RemoveKey(PCFKEY_NODE npKey)
{
	if((npKey->Flag&CFKEY_NODE_OPENED)!=0)
	{
		CCfKeyInterface* lpInterface = FindValidKeyInterface(npKey);
		// 使其无效
		lpInterface->mbDeleted = true;
	}

	// 是否存在子节点
	if(CFKEY_HAS_CHILD(npKey))
	{
		PCFKEY_BRANCH lpBranch = (PCFKEY_BRANCH)npKey;
		for (int i=0;i<lpBranch->mpSubKeys->Size();i++)
		{
			if(RemoveKey((*lpBranch->mpSubKeys)[i].KeyObj)==false)
				return false;
		}
		delete lpBranch->mpSubKeys;
	}

	// 释放内存
	delete npKey;
	
	return true;
}

// 询问是否可以删除一个子键值，包括它的下层某个键值
bool CConfigFile::QueryRemove(PCFKEY_NODE npKey)
{
	// 是否存在子节点
	if(CFKEY_HAS_CHILD(npKey))
	{
		PCFKEY_BRANCH lpBranch = (PCFKEY_BRANCH)npKey;
		for (int i=0;i<lpBranch->mpSubKeys->Size();i++)
		{
			if(QueryRemove((*lpBranch->mpSubKeys)[i].KeyObj)==false)
				return false;	// 如果某个子键不能被删除，则自身也不能被删除
		}
	}
	// 如果被锁定，则不能被删除
	if((npKey->Flag&CDKEY_FLAG_LOCKED)!=0)
		return false;

	return true;
}

// 打开一个Key
CCfKeyInterface* CConfigFile::GetKeyInterface(CCfKeyInterface* npParents,int niPosition)
{
	CCfKeyInterface* lpInterface;

	if (niPosition < 0 || npParents->mbDeleted != false ||  CFKEY_HAS_CHILD(npParents->BaseNode)==false || niPosition >= npParents->Branch->mpSubKeys->Size())
		return NULL;

	// 检查是否已经打开
	if(((*npParents->Branch->mpSubKeys)[niPosition].KeyObj->Flag&CFKEY_NODE_OPENED)!=0)
	{
		lpInterface = FindValidKeyInterface((*npParents->Branch->mpSubKeys)[niPosition].KeyObj);
		CMMASSERT(lpInterface->miPositionInBrothers == niPosition);
		if(lpInterface != NULL)
		{
			// 增加引用记录
			lpInterface->miReferenceCount++;
			return lpInterface;
		}
	}
	lpInterface = AllocateKeyInterface((*npParents->Branch->mpSubKeys)[niPosition].KeyObj);
	if(lpInterface != NULL)
	{
		lpInterface->miPositionInBrothers = niPosition;
		lpInterface->mpParentsKey = npParents;
		npParents->miReferenceCount++;	// 增加对父节点的引用记录

		// 标识该节点被打开
		lpInterface->BaseNode->Flag |= CFKEY_NODE_OPENED;
	}

	return lpInterface;
}

// 分配一个打开Key的打开对象
CCfKeyInterface* CConfigFile::AllocateKeyInterface(PCFKEY_NODE npKeyNode)
{
	CCfKeyInterface* lpInterface;

	if(moUnusedKeyInterface.Size() > 0 )
	{
		lpInterface = moUnusedKeyInterface.Top();
		moUnusedKeyInterface.Pop();

		lpInterface->Reuse(this);
	}
	else
	{
		lpInterface = CCfKeyInterface::CreateInstance(this);
	}

	if(lpInterface == NULL)
		return NULL;

	lpInterface->BaseNode = npKeyNode;

	CCfKeyOpened loAdd;

	loAdd.KeyNode = npKeyNode;
	loAdd.Interface = lpInterface;

	moKeyOpened.Insert(loAdd);

	return lpInterface;
}

// 增加对一个Key的引用，注意，由于进入SpinLock，所以这个函数只给CCfKeyInterface调用，本类代码处于SpinLock中时，直接对该值加1
int CConfigFile::AddReferToKeyInterface(CCfKeyInterface* npKeyInterface)
{
	int liValue;

	moExclusive.Enter();

	liValue = npKeyInterface->miReferenceCount++;	

	moExclusive.Leave();

	return liValue;
}

// 减少对一个Key的引用，如果引用为0就释放它，注意，这个函数只给CCfKeyInterface调用
int CConfigFile::ReleaseKeyInterface(CCfKeyInterface* npKeyInterface)
{
	TCFKEYINTERFACESTACK loKeysToDelete;
	int liIndex;
	int liReval = -1;

	moExclusive.Enter();

	// 检查它是否存在于已打开队列中
	while(npKeyInterface != NULL && (liIndex = FindKeyInterface(npKeyInterface)) >= 0)
	{
		// 减少引用计数
		npKeyInterface->miReferenceCount--;
		if(liReval == -1)
			liReval = npKeyInterface->miReferenceCount;

		if(npKeyInterface->miReferenceCount <= 0)	// 需要删除了
		{
			// 从打开队列中拿掉
			moKeyOpened.RemoveByIndex(liIndex);

			// 清除在Node上的打开标志
			if(npKeyInterface->mbDeleted == false)
				npKeyInterface->BaseNode->Flag &= ~CFKEY_NODE_OPENED;

			// 插入到释放栈中
			loKeysToDelete.Push(npKeyInterface);

			// 减去对父节点的引用
			npKeyInterface = npKeyInterface->mpParentsKey;
		}
		else
			break;
	}

	moExclusive.Leave();

	// 释放所有的KeyInterface
	while(loKeysToDelete.Size() != 0)
	{
		npKeyInterface = loKeysToDelete.Top();
		loKeysToDelete.Pop();
		if(npKeyInterface!=NULL)
		{
			moExclusive.Enter();

			if(moUnusedKeyInterface.Size() < CF_LOOKASIDE_UNUSEDKEY_COUNT)
				moUnusedKeyInterface.Push(npKeyInterface);
			else
				delete npKeyInterface;

			moExclusive.Leave();
		}
	}
	return liReval;
}

//// 查找一个打开的KEY
//CCfKeyInterface* CConfigFile::FindKeyInterface(PCFKEY_NODE npKeyNode,bool nbTakeOff)
//{
//	CCfKeyOpened loToFind;
//
//	loToFind.Interface = NULL;
//	loToFind.KeyNode = npKeyNode;
//
//	int liIndex = moKeyOpened.Find(loToFind);
//	if(liIndex >= 0)
//	{
//		CCfKeyInterface* lpReval = moKeyOpened[liIndex].Interface;
//		if(nbTakeOff != false)
//			moKeyOpened.RemoveByIndex(liIndex);
//		return lpReval;
//	}
//
//	return NULL;
//}

// 查找一个打开并且有效的KEY，因为可能存在指向同一个npKeyNode的不同npKeyInterface，其中只有一个是有效的，其他的指向的都可能是已经删除过得其他Key值
CCfKeyInterface* CConfigFile::FindValidKeyInterface(PCFKEY_NODE npKeyNode)
{
	CCfKeyOpened loToFind;

	loToFind.Interface = NULL;
	loToFind.KeyNode = npKeyNode;

	int liIndex = moKeyOpened.Find(loToFind);
	if(liIndex < 0)
		return NULL;

	for(;liIndex < moKeyOpened.Size() && moKeyOpened[liIndex].KeyNode == npKeyNode ;liIndex++)
	{
		// 只会有一个未被删除的有效的KeyInterface存在
		if(moKeyOpened[liIndex].Interface->mbDeleted == false)
			return moKeyOpened[liIndex].Interface;
	}

	return NULL;
}

//// 查找一个第一个符合条件的KeyInterface，注意，可能是对应的Key已经被删除但Interface还没有被释放；返回它存在的位置，小于0表示没找到
//int CConfigFile::FindKeyInterface(PCFKEY_NODE npKeyNode)
//{
//	CCfKeyOpened loToFind;
//
//	loToFind.Interface = NULL;
//	loToFind.KeyNode = npKeyNode;
//
//	return moKeyOpened.Find(loToFind);
//}

// 查找一个KeyInterface是否在保存数组中存在，返回它存在的位置，小于0表示没找到
int CConfigFile::FindKeyInterface(CCfKeyInterface* npKeyInterface)
{
	CCfKeyOpened loToFind;

	loToFind.Interface = NULL;
	loToFind.KeyNode = npKeyInterface->BaseNode;

	int liIndex = moKeyOpened.Find(loToFind);
	if(liIndex >= 0)
	{
		for(;liIndex < moKeyOpened.Size();liIndex++)
		{
			// npKeyInterface 是不会重复出现的地址
			if(moKeyOpened[liIndex].Interface == npKeyInterface)
				break;
		}
	}
	return liIndex;
}

// 更新一个打开的键的Node指针
bool CConfigFile::UpdateInterface(CCfKeyInterface* npInterface,PCFKEY_NODE npKeyNode)
{
	CCfKeyOpened loToFind;

	int liIndex = FindKeyInterface(npInterface);
	CMMASSERT(liIndex >= 0);

	if(moKeyOpened.RemoveByIndex(liIndex)==false)
		return false;

	if((PCFKEY_BRANCH)npInterface->BaseNode == mpRoot)
		mpRoot = (PCFKEY_BRANCH)npKeyNode;

	delete npInterface->BaseNode;

	npKeyNode->Flag |= CFKEY_NODE_OPENED;
	npInterface->BaseNode = npKeyNode;
	loToFind.Interface = npInterface;
	loToFind.KeyNode = npKeyNode;

	return moKeyOpened.Insert(loToFind)>=0;

}

// 更新一个打开的键的SubNodes数组，因为数组发生了可能导致其他Interface（它们指向某个SubNode)失效，所以需要更新
void CConfigFile::UpdateSubNodes(CCfKeyInterface* npInterface)
{
	CCfKeyInterface* lpSubIntf;
	for (int i=0;i< npInterface->Branch->mpSubKeys->Size();i++)
	{
		lpSubIntf = FindValidKeyInterface(npInterface->GetSubNode(i));
		if(lpSubIntf != NULL)
			lpSubIntf->miPositionInBrothers = i;
	}
}

// 判断字符串值的长度
int CConfigFile::GetValueLengthOfTypeString(IConfigFile::VALUETYPE nuValueType,const void* npValueBuf)
{
	int liValuelen = 0;
	if(nuValueType == IConfigFile::AnsiString)
		while(((char*)npValueBuf)[liValuelen++]!=0);
	else
		if(nuValueType == IConfigFile::UnicodeString)
		{
			while(((wchar_t*)npValueBuf)[liValuelen++]!=0);
			liValuelen *= 2;
		}

	return liValuelen;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// 增加引用
int CCfKeyInterface::AddRefer(void)
{
	return mpConfigFile->AddReferToKeyInterface(this);
}

//释放键，新建，打开，或者任何一种方法得到的本类对象都必须释放
int CCfKeyInterface::Release(void)
{
	return mpConfigFile->ReleaseKeyInterface(this);
}


//删除这个键，注意，调用删除后的接口仍然需要Release；调用过Delete后，这个接口也就没有用了，出Release外，其他功能都不支持了
bool CCfKeyInterface::Delete(void)
{
	bool lbResult = false;
	mpConfigFile->moExclusive.Enter();

	do 
	{
		if(mbDeleted != false)
			break ;

		// 确认父键有效
		if(mpParentsKey == NULL || mpParentsKey->mbDeleted != false || CFKEY_HAS_CHILD(mpParentsKey->BaseNode)==false)
			break;
		
		CMMASSERT(miPositionInBrothers >= 0 && miPositionInBrothers < mpParentsKey->Branch->mpSubKeys->Size());


		PCFKEY_NODE lpKeyNode = mpParentsKey->GetSubNode(miPositionInBrothers);

		// 确认是否能够被删除
		if(mpConfigFile->QueryRemove(lpKeyNode)==false)
			break;

		// 首先从父键的子键列表中拿掉本键
		if(mpParentsKey->RemoveSubNode(miPositionInBrothers)==false)
		{
			break;
		}

		// 更新同级的所有打开的键
		mpConfigFile->UpdateSubNodes(mpParentsKey);

		// 删除对应的Node和全部它的子Node
		lbResult = mpConfigFile->RemoveKey(lpKeyNode);

	} while(false);

	mpConfigFile->moExclusive.Leave();

	return lbResult;
}
	

// 新建一个键，返回NULL表示失败，失败的原因可能是已经具有相同的键值，或者分配内存失败
ICfKey* CCfKeyInterface::NewSubKey(
	IN const wchar_t* nszName,	// 子键的名字，不能为空
	IN IConfigFile::VALUETYPE nuValueType,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
	IN const void* npValueBuf,
	IN LONG  niValuelen	// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
	)
{
	PCFKEY_LEAF lpLeaf;
	ICfKey* lpReval = NULL;

	mpConfigFile->moExclusive.Enter();
	do 
	{
		if(mbDeleted != false)
			break;

		// 判断是否已经存在
		if(CFKEY_HAS_CHILD(BaseNode)!=false && FindSubKey(nszName,-1) >= 0)
			break;

		// 分配节点
		lpLeaf = (PCFKEY_LEAF)mpConfigFile->AllocateNode(nszName,-1,nuValueType,npValueBuf,niValuelen);
		if(lpLeaf == NULL)
			break;

		// 插入
		int liPos = InsertSubNode((PCFKEY_NODE)lpLeaf);
		if(liPos < 0)
		{
			CMMASSERT(0);
			break;	// 由于不知道发生了什么情况，所以，不能释放这个可能是碎片的内存
		}

		// 更新同级的所有打开的键
		mpConfigFile->UpdateSubNodes(this);

		// 打开这个子键
		lpReval = mpConfigFile->GetKeyInterface(this,liPos);

	} while(false);

	mpConfigFile->moExclusive.Leave();

	return lpReval;
}

// 新建一个无名键，返回NULL表示失败；如果同时存在命名键值，将有可能存在命名键值夹在无名键中间的情况
ICfKey* CCfKeyInterface::NewSubKey(
	IN ULONG nuID,		// 指定无名键的标识ID，尽可以不要使用相同标识
	IN bool nbAhead,	// 当遇到相同ID的子键时，在相同ID子键之前建立新键；否则，在相同ID子键之后建立新键
	IN IConfigFile::VALUETYPE nuValueType,	// 子键的默认值类型，如果为Invalid，则忽略后两个参数
	IN const void* npValueBuf,
	IN LONG  niValuelen	// 字符串类型时，可以通过结尾的\0来决定数值的长度，此时设置为-1
	)
{
	PCFKEY_LEAF lpLeaf;
	ICfKey* lpReval = NULL;

	if(nuID == 0)
		return NULL;

	mpConfigFile->moExclusive.Enter();
	do 
	{
		if(mbDeleted != false)
			break;

		//// 判断是否已经存在
		//if(CFKEY_HAS_CHILD(BaseNode)!=false && FindSubKey(nuID) >= 0)
		//	break;

		// 分配节点
		lpLeaf = (PCFKEY_LEAF)mpConfigFile->AllocateNode(NULL,0,nuValueType,npValueBuf,niValuelen);
		if(lpLeaf == NULL)
			break;

		// 插入
		int liPos = InsertSubNode((PCFKEY_NODE)lpLeaf,nuID,nbAhead);
		if(liPos < 0)
		{
			CMMASSERT(0);
			break;	// 由于不知道发生了什么情况，所以，不能释放这个可能是碎片的内存
		}

		// 更新同级的所有打开的键
		mpConfigFile->UpdateSubNodes(this);

		// 打开这个子键
		lpReval = mpConfigFile->GetKeyInterface(this,liPos);

	} while(false);

	mpConfigFile->moExclusive.Leave();

	return lpReval;
}

// 获得本键的父键，如果本键是根键，则返回NULL
ICfKey* CCfKeyInterface::GetParentsKey(void)
{
	if(mpParentsKey!=NULL)
		mpParentsKey->AddRefer();
	return mpParentsKey;
}

// 重新设置父节点，将本节点从当前父节点移除，插入新的父节点下
bool __stdcall CCfKeyInterface::SetParentKey(
	IN ICfKey* npNewParent,
	IN bool nbAhead
	)
{
	wchar_t lswName[MAX_PATH];
	bool lbHasName = false;
	bool lbResult = false;
	CCfKeyInterface* lpNewParent;
	ULONG luID;

	if(GetName(lswName,MAX_PATH)>0)
	{
		lbHasName = true;
		ICfKey* lpExist = npNewParent->GetSubKey(lswName);
		if(lpExist != NULL)
		{	// 命名键不能存在相同名称
			lpExist->Release();
			return false;
		}
	}
	else
		luID = GetID(NULL);

	lpNewParent = dynamic_cast<CCfKeyInterface*>(npNewParent);

	mpConfigFile->moExclusive.Enter();
	do 
	{
		lbResult = false;

		if(mbDeleted != false)
			break;

		// 从父节点移除
		if(mpParentsKey->RemoveSubNode(miPositionInBrothers)==false)
			break;

		// 更新父节点
		mpConfigFile->UpdateSubNodes(mpParentsKey);

		// 在新的父节点上插入子节点
		if(lbHasName!=false)
		{
			if(lpNewParent->InsertSubNode(BaseNode)<0)
				break;
		}
		else
		{
			if(lpNewParent->InsertSubNode(BaseNode,luID,nbAhead)<0)
				break;
		}

		// 更新新父节点
		mpConfigFile->UpdateSubNodes(lpNewParent);

		lbResult = true;

	} while(false);

	mpConfigFile->moExclusive.Leave();

	return lbResult;
}

// 获得下一个键，获得的对象当不再访问时需要Release,按照获取当前Key用的同一排序规则
ICfKey* CCfKeyInterface::GetNextKey(void)
{
	ICfKey* lpReval = NULL;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
		lpReval =  mpConfigFile->GetKeyInterface(mpParentsKey,miPositionInBrothers+1);

	mpConfigFile->moExclusive.Leave();

	return lpReval;
}

// 获得下一个键，并且释放当前键，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToParentsKey(void)
{
	ICfKey* lpNext = GetParentsKey();

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}

// 获得下一个键，并且释放当前键，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToNextKey(void)
{
	ICfKey* lpNext = GetNextKey();

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}

// 获得下一个键，删除并释放当前键!!!，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToNextKey(bool nbDelete)
{
	ICfKey* lpNext = GetNextKey();

	if(nbDelete != false)
		Delete();

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}

// 获得第一个子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToSubKey(void)
{
	ICfKey* lpNext = GetSubKey();

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}

// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToSubKey(
	IN ULONG nuID		// 子键的标识ID
	)
{
	ICfKey* lpNext = GetSubKey(nuID);

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}

// 获得指定子键，并且释放当前键!!!，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::MoveToSubKey(
	IN const wchar_t* nszName,	// 按照名字去获取子键
	IN int niNameLen	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
	)
{
	ICfKey* lpNext = GetSubKey(nszName,niNameLen);

	mpConfigFile->ReleaseKeyInterface(this);

	return lpNext;
}



// 获得第一个子键，获得的对象当不再访问时需要Release
ICfKey* CCfKeyInterface::GetSubKey(void)
{
	ICfKey* lpReval = NULL;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
		lpReval =  mpConfigFile->GetKeyInterface(this,0);

	mpConfigFile->moExclusive.Leave();

	return lpReval;
}

// 打开多层路径指定的子键，获得的对象当不再访问时需要Release；本函数同下面的GetSubKey区别是，本函数可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
ICfKey* CCfKeyInterface::OpenKey(
	IN const wchar_t* nszKeyName,		// 用'/'分割父子键名，根键不用指定，如：xx/yy/zz
	IN bool nbCreateIf			// 如果这个键不存在，则建立它
	)
{
	ICfKey* lpParents;
	ICfKey* lpCurrent;
	const wchar_t* lpName;
	int liLength;

	if(nszKeyName==NULL)
		return NULL;

	lpParents = this;
	lpParents->AddRefer();

	if(nszKeyName[0] == UNICODE_NULL)
		return lpParents;

	lpName = nszKeyName;
	lpCurrent = NULL;

	do 
	{
		// 打开这个目录
		lpCurrent = lpParents->GetSubKey(lpName,-1,&liLength,nbCreateIf);

		if(liLength < 0 || lpCurrent == NULL)
		{
			lpParents->Release();
			break;
		}

		// 继续打开下一级
		lpParents->Release();
		lpParents = lpCurrent;

		if(lpName[liLength]==UNICODE_NULL)
			break;

		lpName += liLength+1;

	} while(*lpName != UNICODE_NULL);

	return lpCurrent;
}


// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的OpenKey区别是，OpenKey可以打开一个路径如：xx/yy/zz指定的最下层的zz子键，而GetSubKey只会打开第一层的xx子键
ICfKey* CCfKeyInterface::GetSubKey(
		IN const wchar_t* nszName,	// 按照名字去获取子键
		IN int niNameLen,	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
		OUT int* npNameLen,	// 返回名字的有效长度
		IN bool nbCreateIf		// 如果该键不存在，则建立它
	)
{
	int liPos;
	ICfKey* lpReval=NULL;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false && CFKEY_HAS_CHILD(BaseNode)!=false)
	{
		liPos = FindSubKey(nszName,niNameLen,npNameLen);
		if(liPos >= 0)
			lpReval =  mpConfigFile->GetKeyInterface(this,liPos);
	}

	mpConfigFile->moExclusive.Leave();

	if(lpReval == NULL && nbCreateIf)
	{
		lpReval = NewSubKey(nszName);
		if(lpReval != NULL && npNameLen != NULL)
			*npNameLen = lpReval->GetName(NULL,0);
	}

	return lpReval;
}

// 获得子键，获得的对象当不再访问时需要Release；本函数同上面的GetSubKey区别是，上面的按名字打开键值，而本函数直接按ID打开，只有通过本函数才能打开没有名字的键值
ICfKey* CCfKeyInterface::GetSubKey(
	IN ULONG nuID,		// 子键的标识ID
	IN int niPos		// 在相同的ID的键值中的先后位置，< 0表示取最后一个
	)
{
	int liPos;
	ICfKey* lpReval=NULL;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false && CFKEY_HAS_CHILD(BaseNode)!=false)
	{
		liPos = FindSubKey(nuID,niPos);
		if(liPos >= 0)
			lpReval =  mpConfigFile->GetKeyInterface(this,liPos);
	}

	mpConfigFile->moExclusive.Leave();

	return lpReval;
}


// 获得本键的默认值类型
IConfigFile::VALUETYPE CCfKeyInterface::GetValueType(void)
{
	IConfigFile::VALUETYPE luValue = IConfigFile::Invalid;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
		luValue =  CFKEY_VALUE_TYPE(BaseNode->Flag);

	mpConfigFile->moExclusive.Leave();

	return luValue;
}

// 获得本键的扩展标志，定义在下面
UCHAR CCfKeyInterface::GetExtFlag(void)
{
	UCHAR lcFlag = 0;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
		lcFlag =  CFKEY_GET_EXTFLAG(BaseNode->Flag);

	mpConfigFile->moExclusive.Leave();

	return lcFlag;
}


// 获得本键的默认值长度，字节单位
LONG CCfKeyInterface::GetValueLength(void)
{
	LONG liLength = 0;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
		liLength =  BaseNode->ValueLength;

	mpConfigFile->moExclusive.Leave();

	return liLength;
}

// 获得本键所在的序号
int CCfKeyInterface::GetPosition(void)
{
	if(mbDeleted != false)
		return -1;
	return miPositionInBrothers;
}

// 获得ID，对于命名键值而言获得的是Hash值
ULONG CCfKeyInterface::GetID(
	OUT int* npPos		// 在相同的ID的键值中的先后位置
	)
{
	ULONG luID;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
	{
		luID = (*mpParentsKey->Branch->mpSubKeys)[miPositionInBrothers].HashValue;
		if(npPos!=NULL)
		{
			*npPos = 0;
			for(int i=miPositionInBrothers-1 ; i >= 0;i--,*npPos += 1)
			{
				if((*mpParentsKey->Branch->mpSubKeys)[i].HashValue != luID)
					break;
			}
		}
	}

	mpConfigFile->moExclusive.Leave();

	return luID;
}


// 获得本键的名字
int CCfKeyInterface::GetName(wchar_t* npNameBuff,int niBufLenByWchar)
{

	wchar_t* lszName;
	int liReval = -1;

	mpConfigFile->moExclusive.Enter();

	do 
	{
		if(mbDeleted != false)
			break;

		if(npNameBuff == NULL)
		{
			liReval = Branch->NameLength;
			break;
		}

		if(niBufLenByWchar < Branch->NameLength+1)
		{
			liReval = -1;
			break;
		}

		if(Branch->NameLength == 0)
		{
			liReval = 0;
			break;
		}

		if(CFKEY_HAS_CHILD(BaseNode))
			lszName = Branch->Name;
		else
			lszName = Leaf->Name;

		COPYSTR_S(npNameBuff,niBufLenByWchar,lszName);

		liReval = Branch->NameLength;

	} while(false);

	mpConfigFile->moExclusive.Leave();

	return liReval;
}


// 改名，如果改名会导致同一节点下出现同名节点，将失败
bool __stdcall CCfKeyInterface::Rename(
	IN const wchar_t* nszName,
	IN bool FailIfExist	//	FailIfExist==true如果改名会导致同一节点下出现同名节点，将失败;  ==false 将自动增加附加字符
	)
{
	wchar_t lswName[MAX_PATH];
	PCFKEY_NODE lpNewNode;
	bool lbReval = false;
	int i;

	if(nszName == NULL || nszName[0] == UNICODE_NULL)
		return false;	// 名字不能为空

	mpConfigFile->moExclusive.Enter();
	do 
	{
		if(mbDeleted != false|| mpParentsKey == NULL)
			break;

		// 锁定键不能修改属性
		if((BaseNode->Flag&CDKEY_FLAG_LOCKED)!=0)
			break;

		//if(BaseNode->NameLength == 0)	// 没有名字的键值不能设定名字		modified by AX 2011.10.13
		//	break;

		// 确认这个新名字是否出现冲突
		if(mpParentsKey->FindSubKey(nszName,-1) >= 0)
		{
			if(FailIfExist != false)
				break;

			for (i=0;i< 1000;i++)
			{
				wsprintf(lswName,L"%s%d",nszName,i);
				if(mpParentsKey->FindSubKey(lswName,-1) < 0)
				{
					nszName = lswName;
					break;
				}
			}
			if(i >= 1000)
				break;
		}

		// to allocate a new node
		if(CFKEY_HAS_CHILD(BaseNode)!=false)
		{
			lpNewNode = mpConfigFile->AllocateNode(nszName,-1,CFKEY_VALUE_TYPE(BaseNode->Flag),&Branch->Name[Branch->NameLength]+1,BaseNode->ValueLength,Branch->mpSubKeys->Size());
			if(lpNewNode != NULL)
			{
				// 将子节点记录都复制过来
				for (int i=0;i<Branch->mpSubKeys->Size();i++)
				{
					((PCFKEY_BRANCH)lpNewNode)->mpSubKeys->Push_Back((*Branch->mpSubKeys)[i]);
				}
			}
		}
		else
		{
			lpNewNode = mpConfigFile->AllocateNode(nszName,-1,CFKEY_VALUE_TYPE(BaseNode->Flag),&Leaf->Name[Leaf->NameLength]+1,BaseNode->ValueLength);
		}
		if(lpNewNode == NULL)
			break;

		// 替换自己在父节点中的指向记录
		mpParentsKey->RemoveSubNode(miPositionInBrothers);
		mpParentsKey->InsertSubNode(lpNewNode);

		mpConfigFile->UpdateInterface(this,lpNewNode);

		// 更新同级的所有打开的键
		mpConfigFile->UpdateSubNodes(mpParentsKey);

		lbReval = true;

	} while(false);

	mpConfigFile->moExclusive.Leave();

	
	return lbReval;
}

// 设置或改变当前的默认值
bool CCfKeyInterface::SetValue(
	IN IConfigFile::VALUETYPE nuValueType,	// 子键的默认值类型
	IN const void* npValueBuf,
	IN LONG  niValuelen
	)
{
	PCFKEY_NODE lpNewNode;
	bool lbReval = false;

	if(niValuelen == -1)
	{
		niValuelen = CConfigFile::GetValueLengthOfTypeString(nuValueType,npValueBuf);
	}

	mpConfigFile->moExclusive.Enter();
	do 
	{
		if(mbDeleted != false || mpParentsKey == NULL || mpParentsKey->mbDeleted != false)
			break;

		// 锁定键不能修改属性
		if((BaseNode->Flag&CDKEY_FLAG_LOCKED)!=0)
			break;

		if(niValuelen <= BaseNode->ValueLength)
		{
			if(CFKEY_HAS_CHILD(BaseNode)!=false)
			{
				RtlCopyMemory(&Branch->Name[Branch->NameLength]+1,npValueBuf,niValuelen);
				Branch->ValueLength = (USHORT)niValuelen;
			}
			else
			{
				RtlCopyMemory(&Leaf->Name[Leaf->NameLength]+1,npValueBuf,niValuelen);
				Leaf->ValueLength = (USHORT)niValuelen;
			}
			CFKEY_SET_VALUETYPE(BaseNode->Flag,nuValueType);
			lbReval = true;
			break;
		}

		// to allocate a new node
		if(CFKEY_HAS_CHILD(BaseNode)!=false)
		{
			lpNewNode = mpConfigFile->AllocateNode(Branch->Name,Branch->NameLength,nuValueType,npValueBuf,niValuelen,Branch->mpSubKeys->Size());
			if(lpNewNode != NULL)
			{
				// 将子节点记录都复制过来
				for (int i=0;i<Branch->mpSubKeys->Size();i++)
				{
					((PCFKEY_BRANCH)lpNewNode)->mpSubKeys->Push_Back((*Branch->mpSubKeys)[i]);
				}
			}
		}
		else
		{
			lpNewNode = mpConfigFile->AllocateNode(Leaf->Name,Leaf->NameLength,nuValueType,npValueBuf,niValuelen);
		}
		if(lpNewNode == NULL)
			break;

		// 替换自己在父节点中的指向记录
		mpParentsKey->UpdateSubNode(lpNewNode,miPositionInBrothers);

		mpConfigFile->UpdateInterface(this,lpNewNode);

		lbReval = true;

	} while(false);

	mpConfigFile->moExclusive.Leave();


	return lbReval;
}

// 修改扩展标志
bool CCfKeyInterface::SetExtFlag(
	IN UCHAR nchFlag
	)
{

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false)
	{
		CFKEY_SET_EXTFLAG(BaseNode->Flag,nchFlag);
	}

	mpConfigFile->moExclusive.Leave();

	return true;
}

// 获得当前默认值，返回获得的值的字节数
int CCfKeyInterface::GetValue(
	OUT PVOID npValueBuf,
	IN  LONG  niBufLen
	)
{
	int liSize = -1;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted ==false && npValueBuf != NULL)
	{
		if(niBufLen >= BaseNode->ValueLength)
		{
			if(CFKEY_HAS_CHILD(BaseNode)!=false)
			{
				RtlCopyMemory(npValueBuf,&Branch->Name[Branch->NameLength]+1,Branch->ValueLength);
			}
			else
			{
				RtlCopyMemory(npValueBuf,&Leaf->Name[Leaf->NameLength]+1,Leaf->ValueLength);
			}
			liSize = BaseNode->ValueLength;
		}
		else
		{
			liSize = -1;	// overflow
		}
	}

	mpConfigFile->moExclusive.Leave();

	return liSize;
}

CCfKeyInterface::~CCfKeyInterface()
{
	CMMASSERT(miReferenceCount<=0);
}

// 查找子节点，返回子节点在Hash数组中的序号，返回-1表示没有找到
int CCfKeyInterface::FindSubKey(
	IN const wchar_t* nszName,
	IN int niNameLen,	// -1 indicate nszName is terminated by '\0' or '\\' or '/', >=0 is the charactar count of nszName
	OUT int* npNameLen	// 返回名字的有效长度
	)
{
	CCfKeyHash loFind;
	
	niNameLen = loFind.GenerateHashCode(nszName,niNameLen);


	int liPos = Branch->mpSubKeys->Find(loFind);
	if(liPos >= 0)
	{
		for(;liPos < Branch->mpSubKeys->Size() && (*Branch->mpSubKeys)[liPos].HashValue == loFind.HashValue;liPos++)
		{
			if(niNameLen != (*Branch->mpSubKeys)[liPos].KeyObj->NameLength)
				continue;
			wchar_t* lpNodeName;
			if(CFKEY_HAS_CHILD((*Branch->mpSubKeys)[liPos].KeyObj))
				lpNodeName = ((PCFKEY_BRANCH)((*Branch->mpSubKeys)[liPos].KeyObj))->Name;
			else
				lpNodeName = ((PCFKEY_LEAF)((*Branch->mpSubKeys)[liPos].KeyObj))->Name;

			int i;
			for (i=0;i<niNameLen;i++)
			{
				if(nszName[i] != lpNodeName[i] && ((nszName[i]|0x20) != (lpNodeName[i]|0x20) || (lpNodeName[i]|0x20) < L'a' || (lpNodeName[i]|0x20) > L'z'))
					break;
			}
			if(i >= niNameLen)
				break;
		}
		if(liPos >= Branch->mpSubKeys->Size() || (*Branch->mpSubKeys)[liPos].HashValue != loFind.HashValue)
			liPos = -1;
	}

	if(npNameLen != NULL)
		*npNameLen = niNameLen;

	return liPos;
}

// 查找子节点，返回子节点在Hash数组中的序号，返回-1表示没有找到
int CCfKeyInterface::FindSubKey(
	ULONG nuHashCode,
	IN int niPos	// 在相同的ID的键值中的先后位置，< 0表示取最后一个
	)
{
	CCfKeyHash loFind;
	int liEnd;

	loFind.HashValue = nuHashCode;

	int liPos = Branch->mpSubKeys->Find(loFind);
	if(liPos >= 0 && niPos != 0)	// niPos == 0 表示第一个就行了
	{
		if(niPos < 0)
		{
			// 找最后一个
			liEnd = Branch->mpSubKeys->Size();
		}
		else
		{
			liEnd = liPos+niPos+1;
		}

		while(liPos < Branch->mpSubKeys->Size() && liPos < liEnd && (*Branch->mpSubKeys)[liPos].HashValue == loFind.HashValue)
			liPos++;

		liPos--;	// 退回到刚才有效的位置
	}

	return liPos;
}



// 插入一个新的子节点，返回插入的位置
int CCfKeyInterface::InsertSubNode(
	PCFKEY_NODE npNode,
	ULONG nuHashCode,
	bool nbAhead
	)
{
	if(mbDeleted != false)
		return 0;
	// 检查当前节点的类别，如果不是Branch类型，我们就需要重新构造Node节点
	if(CFKEY_HAS_CHILD(BaseNode)==false)
	{
		PCFKEY_NODE lpNewNode;
		lpNewNode = mpConfigFile->AllocateNode(Leaf->Name,Leaf->NameLength,CFKEY_VALUE_TYPE(BaseNode->Flag),&Leaf->Name[Leaf->NameLength]+1,BaseNode->ValueLength,1);
		if(lpNewNode == NULL)
			return -1;

		// 替换自己在父节点中的指向记录
		if(mpParentsKey != NULL)
			(*mpParentsKey->Branch->mpSubKeys)[miPositionInBrothers].KeyObj = lpNewNode;

		mpConfigFile->UpdateInterface(this,lpNewNode);
	}

	CCfKeyHash loNew;
	if(nuHashCode == 0 )
	{
		// 判断插入的对象类型
		if(CFKEY_HAS_CHILD(npNode)!=false)
			loNew.GenerateHashCode(((PCFKEY_BRANCH)npNode)->Name,((PCFKEY_BRANCH)npNode)->NameLength);
		else
			loNew.GenerateHashCode(((PCFKEY_LEAF)npNode)->Name,((PCFKEY_LEAF)npNode)->NameLength);
	}
	else
		loNew.HashValue = nuHashCode;

	loNew.KeyObj = (PCFKEY_NODE)npNode;

	return Branch->mpSubKeys->Insert(loNew,nbAhead);
}


// 获得子节点
PCFKEY_NODE CCfKeyInterface::GetSubNode(
	IN int niPosition
	)
{
	return (*Branch->mpSubKeys)[niPosition].KeyObj;
}

// 移除子节点
bool CCfKeyInterface::RemoveSubNode(
	IN int niPosition
	)
{
	return Branch->mpSubKeys->RemoveByIndex(niPosition);
}


// 用字符串生成Hash码,返回的是名字的有效长度
int CCfKeyHash::GenerateHashCode(
	IN const wchar_t* nszName,
	IN int niNameLen	// -1 indicate that the name was terminated by '\0' or '\\' or '/'
	)
{
	LONG liIndex = 0;
	HashValue = 0;

	if(niNameLen < 0)
	{
		niNameLen = 255;
	}

	wchar_t lcCrt;
	while (nszName[liIndex] != 0 && nszName[liIndex] != L'\\' && nszName[liIndex] != L'/' && liIndex < niNameLen)
	{
		lcCrt = nszName[liIndex++];
		if((lcCrt|0x20) >= L'a' && (lcCrt|0x20) <= L'z')
			lcCrt |= 0x20;
		HashValue += lcCrt;
		HashValue = (HashValue<<1) | ((HashValue&0x010000)>>16);
	}

	return liIndex;
}

// 更新子节点，这不会改变排序次序
bool CCfKeyInterface::UpdateSubNode(
	PCFKEY_NODE npNode,
	IN int niPosition
	)
{
	(*Branch->mpSubKeys)[niPosition].KeyObj = npNode;
	return true;
}



ULONG CStableConfigFile::InitOnCreate(
	IN const wchar_t* nszPathName,				// 文件的完整路径名；设置为NULL，表示建立一个空文件暂时不指定文件名
	IN ULONG nuCreationDisposition			// 同CreateFile API类似，见CfgIface.h文件中的相关定义
	)
{
	ULONG luResult = (ULONG)-1;
	ULONG luDuplicateNumberArr[2]={0};
	wchar_t* lpFileNameArr[2];
	int i;
	bool lbReval = false;
#ifdef KERNEL_CODE
	OBJECT_ATTRIBUTES ObjectAttributes;
	UNICODE_STRING FileName;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
#endif
	PCF_FILE_HEAD lpHead = NULL;
	HANDLE lhFile = NULL;
	ULONG luNewDupNumber;

	if(nszPathName == NULL)
		return CConfigFile::InitOnCreate(NULL,nuCreationDisposition);

	do 
	{
		// 准备文件名
		for (i=0;i<2;i++)
		{
			lpFileNameArr[i] = new wchar_t[256];
			if(lpFileNameArr[i]==NULL)
				break;
			COPYSTR_S(lpFileNameArr[i],256,nszPathName);
		}
		if(lpFileNameArr[0] == NULL || lpFileNameArr[1] == NULL)
			break;

		// 修改第二文件名，在后缀名前面加上'_dup'
		{
			wchar_t* lpExtName = NULL;
			for(i=0;i<256;i++)
			{
				if(lpFileNameArr[1][i] == L'.')
					lpExtName = lpFileNameArr[1]+i;
				if(lpFileNameArr[1][i] == UNICODE_NULL)
				{
					if(lpExtName == NULL)
						lpExtName = lpFileNameArr[1]+i;
					break;
				}
			}
			// 是否还能够将名字增长？
			if(lpExtName - lpFileNameArr[1] >= 256 - 8)
				break;
			*(lpExtName+4) = *lpExtName;
			*(lpExtName+5) = *(lpExtName+1);
			*(lpExtName+6) = *(lpExtName+2);
			*(lpExtName+7) = *(lpExtName+3);
			*(lpExtName+8) = UNICODE_NULL;
			*(lpExtName) = L'_';
			*(lpExtName+1) = L'd';
			*(lpExtName+2) = L'u';
			*(lpExtName+3) = L'p';
		}

		lpHead = new CF_FILE_HEAD;
		if(lpHead == NULL)
			break;

		for(int i=0;i<2;i++)
		{
			// 打开文件，读入文件头
			do 
			{
	#ifndef KERNEL_CODE
				lhFile = ::CreateFileW(lpFileNameArr[i],GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,nuCreationDisposition,NULL,NULL);
				if(lhFile == INVALID_HANDLE_VALUE)
				{
					lhFile = NULL;
					break;
				}
	#else
				RtlInitUnicodeString(&FileName,lpFileNameArr[i]);
				InitializeObjectAttributes(
					&ObjectAttributes
					,&FileName
					,OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE
					,NULL 
					,NULL );

				Status = ZwCreateFile(&lhFile,FILE_READ_DATA|SYNCHRONIZE,&ObjectAttributes,&IoStatus,NULL,FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,nuCreationDisposition,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);
				if(!NT_SUCCESS(Status))
				{
					lhFile = NULL;
					break;
				}
	#endif

				// 读入文件头
	#ifndef KERNEL_CODE
				ULONG luRead;
				if(ReadFile(lhFile,lpHead,sizeof(CF_FILE_HEAD),&luRead,NULL)==FALSE)
					break;
	#else
				if(STATUS_SUCCESS != ZwReadFile(lhFile,NULL,NULL,NULL,&IoStatus,lpHead,sizeof(CF_FILE_HEAD),NULL,NULL))
					break;
	#endif

				// 检查完整性
				if(lpHead->ShortHead.Signature != CF_SIGNATURE || lpHead->ShortHead.Version > CF_VERSION || lpHead->ShortHead.SequenceA != lpHead->Tail.SequenceB)
					break;

				// 副本记录
				luDuplicateNumberArr[i] = lpHead->ShortHead.Duplicate;

			} while (false);

			if(lhFile != NULL)
			{
#ifndef KERNEL_CODE
				CloseHandle(lhFile);
#else
				ZwClose(lhFile);
#endif
			}
		}

		// 判断哪个文件是最后保存的，将文件名的存放顺序调换
		if( luDuplicateNumberArr[0] == (ULONG)-1 && luDuplicateNumberArr[1]== 1 ||
			luDuplicateNumberArr[0] < luDuplicateNumberArr[1] && (luDuplicateNumberArr[0]!=1 || luDuplicateNumberArr[1]!=(ULONG)-1)
			) 
		{
			wchar_t* lpTemp = lpFileNameArr[0];
			lpFileNameArr[0] = lpFileNameArr[1];
			lpFileNameArr[1] = lpTemp;
			luNewDupNumber = luDuplicateNumberArr[1]+1;
		}
		else
			luNewDupNumber = luDuplicateNumberArr[0]+1;

		if(luNewDupNumber == 0)
			luNewDupNumber = 1;

		// 尝试打开最后保存的文件
		luResult = CConfigFile::InitOnCreate(lpFileNameArr[0],nuCreationDisposition);
		if(luResult == 0)
		{
			// 修改基类的文件名为另外一个文件，从而使得保存时将记录存入另外一个文件
			COPYSTR_S(mszFileName,256,lpFileNameArr[1]);
		}
		else// 失败，则尝试打开另外一个文件
		{
			luResult = CConfigFile::InitOnCreate(lpFileNameArr[1],nuCreationDisposition);
			if(luResult == 0)
			{
				// 修改基类的文件名为另外一个文件，从而使得保存时将记录存入另外一个文件
				COPYSTR_S(mszFileName,256,lpFileNameArr[0]);
			}
		}
		mdFileHead.Duplicate = luNewDupNumber;

	} while (false);

	if(lpHead != NULL)
		delete lpHead;

	if(lpFileNameArr[0]!=NULL)
		delete lpFileNameArr[0];
	if(lpFileNameArr[1]!=NULL)
		delete lpFileNameArr[1];

	return luResult;
}

// 直接将值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
LONG __stdcall CCfKeyInterface::QueryValueAsLONG(
	IN LONG niDefault
	)
{
	LONG liValue;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false && BaseNode->ValueLength == sizeof(LONG))
	{
		if(CFKEY_HAS_CHILD(BaseNode)!=false)
			liValue = *(LONG*)(&Branch->Name + Branch->NameLength + 1);
		else
			liValue = *(LONG*)(&Leaf->Name + Leaf->NameLength + 1);
	}
	else
		liValue = niDefault;

	mpConfigFile->moExclusive.Leave();

	return liValue;
}

// 直接将值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
FLOAT __stdcall CCfKeyInterface::QueryValueAsFLOAT(
	IN FLOAT nfDefault
	)
{
	FLOAT lfValue;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false && BaseNode->ValueLength == sizeof(FLOAT))
	{
		if(CFKEY_HAS_CHILD(BaseNode)!=false)
			lfValue = *(FLOAT*)(&Branch->Name + Branch->NameLength + 1);
		else
			lfValue = *(FLOAT*)(&Leaf->Name + Leaf->NameLength + 1);
	}
	else
		lfValue = nfDefault;

	mpConfigFile->moExclusive.Leave();

	return lfValue;
}

// 按名字查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
LONG __stdcall CCfKeyInterface::QuerySubKeyValueAsLONG(
	IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
	IN LONG niDefault
	)
{
	LONG liValue;
	ICfKey* lpKey = OpenKey(nszSubKeyName);

	if(lpKey != NULL)
	{
		liValue = lpKey->QueryValueAsLONG(niDefault);
		lpKey->Release();
	}
	else
		liValue = niDefault;

	return liValue;
}

// 按ID查找子键，直接将子键的值解释为LONG获取，如果值的类型不是4字节，函数将返回默认值
LONG __stdcall CCfKeyInterface::QuerySubKeyValueAsLONG(
	IN ULONG nuID,		// 子键的标识ID
	IN LONG niDefault
	)
{
	LONG liValue;
	ICfKey* lpKey = GetSubKey(nuID);

	if(lpKey != NULL)
	{
		liValue = lpKey->QueryValueAsLONG(niDefault);
		lpKey->Release();
	}
	else
		liValue = niDefault;

	return liValue;
}

// 按名字查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
FLOAT __stdcall CCfKeyInterface::QuerySubKeyValueAsFLOAT(
	IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
	IN FLOAT nfDefault
	)
{
	FLOAT lfValue;
	ICfKey* lpKey = OpenKey(nszSubKeyName);

	if(lpKey != NULL)
	{
		lfValue = lpKey->QueryValueAsFLOAT(nfDefault);
		lpKey->Release();
	}
	else
		lfValue = nfDefault;

	return lfValue;
}

// 按ID查找子键，直接将子键的值解释为FLOAT获取，如果值的类型不是sizeof(FLOAT)字节，函数将返回默认值
FLOAT __stdcall CCfKeyInterface::QuerySubKeyValueAsFLOAT(
	IN ULONG nuID,		// 子键的标识ID
	IN FLOAT nfDefault
	)
{
	FLOAT lfValue;
	ICfKey* lpKey = GetSubKey(nuID);

	if(lpKey != NULL)
	{
		lfValue = lpKey->QueryValueAsFLOAT(nfDefault);
		lpKey->Release();
	}
	else
		lfValue = nfDefault;

	return lfValue;
}

// 按名字查找子键，直接获取子键的值，返回小于零表示目标键不存在，等于0表示目标键值为空，大于0表示返回的Value长度
int __stdcall CCfKeyInterface::QuerySubKeyValue(
	IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
	OUT PVOID npValueBuf,
	IN  LONG  niBufLen
	)
{
	LONG liSize;
	ICfKey* lpKey = OpenKey(nszSubKeyName);

	if(lpKey != NULL)
	{
		liSize = lpKey->GetValue(npValueBuf,niBufLen);

		lpKey->Release();
	}
	else
		liSize = -1;

	return liSize;
}


// 直接将值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
PVOID __stdcall CCfKeyInterface::QueryValueAsPVOID(
	IN PVOID npDefault
	)
{
	PVOID lpValue;

	mpConfigFile->moExclusive.Enter();

	if(mbDeleted == false && BaseNode->ValueLength == sizeof(PVOID))
	{
		if(CFKEY_HAS_CHILD(BaseNode)!=false)
			lpValue = *(PVOID*)(&Branch->Name + Branch->NameLength + 1);
		else
			lpValue = *(PVOID*)(&Leaf->Name + Leaf->NameLength + 1);
	}
	else
		lpValue = npDefault;

	mpConfigFile->moExclusive.Leave();

	return lpValue;
}

// 按名字查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
PVOID __stdcall CCfKeyInterface::QuerySubKeyValueAsPVOID(
	IN const wchar_t* nszSubKeyName,	// 指定要获取的值的键名，不能为NULL或者nszSubKeyName[0]==UNICODE_NULL
	IN PVOID npDefault
	)
{
	PVOID lpValue;
	ICfKey* lpKey = OpenKey(nszSubKeyName);

	if(lpKey != NULL)
	{
		lpValue = lpKey->QueryValueAsPVOID(npDefault);
		lpKey->Release();
	}
	else
		lpValue = npDefault;

	return lpValue;
}

// 按ID查找子键，直接将子键的值解释为PVOID获取，如果值的类型不是sizeof(PVOID)字节，函数将返回默认值
PVOID __stdcall CCfKeyInterface::QuerySubKeyValueAsPVOID(
	IN ULONG nuID,		// 子键的标识ID
	IN PVOID npDefault
	)
{
	PVOID lpValue;
	ICfKey* lpKey = GetSubKey(nuID);

	if(lpKey != NULL)
	{
		lpValue = lpKey->QueryValueAsPVOID(npDefault);
		lpKey->Release();
	}
	else
		lpValue = npDefault;

	return lpValue;
}

//// 获取指定的值存储New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
//PVOID __stdcall CCfKeyInterface::QueryValueAsBuffer()
//{
//	PVOID lpRetValue = NULL;
//	mpConfigFile->moExclusive.Enter();
//	do 
//	{
//		if(this->GetValueType() == IConfigFile::Invalid)
//			break;
//
//		int liValueLength = this->GetValueLength();
//		lpRetValue = new BYTE[liValueLength];
//		memset(lpRetValue, 0, liValueLength);
//
//		if(this->GetValue(lpRetValue, liValueLength) <= 0)
//			break;
//
//	} while (false);
//
//	mpConfigFile->moExclusive.Leave();
//	return lpRetValue;
//}

//// 获取指定的子健的值存储在New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
//PVOID __stdcall CCfKeyInterface::QuerySubKeyValueAsBuffer(
//	IN const wchar_t* nswSubKeyName
//	)
//{
//	PVOID lpRetValue = NULL;
//	mpConfigFile->moExclusive.Enter();
//	
//	ICfKey* lpSubKey = this->OpenKey(nswSubKeyName);
//	do 
//	{
//		if(NULL == lpSubKey || mbDeleted != false)
//			break;
//
//		lpRetValue = lpSubKey->QueryValueAsBuffer();
//
//	} while (false);
//	CMM_SAFE_RELEASE(lpSubKey);
//
//	mpConfigFile->moExclusive.Leave();
//	return lpRetValue;
//}
//
//// 获取指定的子健的值存储在New出来的内存中返回来，失败返回NULL，注意要释放返回的指针, ——Colin 加。
//PVOID __stdcall CCfKeyInterface::QuerySubKeyValueAsBuffer(
//	IN ULONG nuID									// 子键的标识ID
//	)
//{
//	PVOID lpRetValue = NULL;
//	mpConfigFile->moExclusive.Enter();
//
//	ICfKey* lpSubKey = this->GetSubKey(nuID);
//	do 
//	{
//		if(NULL == lpSubKey || mbDeleted != false)
//			break;
//
//		lpRetValue = lpSubKey->QueryValueAsBuffer();
//
//	} while (false);
//	CMM_SAFE_RELEASE(lpSubKey);
//
//	mpConfigFile->moExclusive.Leave();
//	return lpRetValue;
//} 