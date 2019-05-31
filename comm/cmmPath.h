/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


//////////////////////////////////////////////////////////////////////////
// 常用文件和目录转换工具函数
#ifndef _CMMPATH_H_
#define _CMMPATH_H_

// 由于本类使用MAX_PATH作为缓冲区，所以，当长度超过MAX_PATH的场合，或者需要保存大量路径的场合，都不适合使用本对象
class CFilePathName {
public:
	CFilePathName();
	CFilePathName(const class CFilePathName& src);
	CFilePathName(const wchar_t* nszPathName); // 如果传入的是目录，最后一个字符一定要是L'\\'
	~CFilePathName(){}

	// 动态创建对象
	static CFilePathName* Create(const wchar_t* nszPathName); // 如果传入的是目录，最后一个字符一定要是L'\\'

	// 动态创建对象
	CFilePathName* Duplicate(void);

	// 附加目录或者文件名，返回false失败，可能的原因无法转换位置，或者结果超长；nbForceBePath参数==true，当转换的结果不携带'\\'时，增加结尾'\\'
	//	实用用法：取同层目录传入L".\\"；取上层目录传入"..\\'
	bool Transform(const wchar_t* nszTransTo,bool nbForceBePath=false);
	bool Transform(const CFilePathName& roPath);

	// 获得当前目录
	bool SetByCurrentPath(void);

	// 获得模块所在目录和文件名
	bool SetByModulePathName(void);

	// 获得当前用户的AppData\Roaming 目录 例如：C:\Users\UserName\AppData\Roaming
	bool SetByUserAppData(void);

	// 设置为目录，如果结尾没有'\\'就加上
	void AssurePath(void);

	// 通过下面两个函数，可以直接修改内容；缓冲区可容纳的字符数为MAX_PATH
	wchar_t* LockBuffer(){	// 此函数可以被多次调用以获得缓冲区指针，如果改变了Buf的内容，必须调用UnlockBuffer才能使对象回复有效
		msiLength = 0;
		return mszPathName;
	}
	void UnlockBuffer();

	// 清除
	void Clean(void);

	// 获得完整路径名
	const wchar_t* GetPathName(void) const {
		return mszPathName;
	}

	// 取文件名，如果当前保存的是一个目录，则返回L"\0"
	const wchar_t* GetFileName(
		int* npLength=NULL	// 返回名字的字符数，不包括'\0'结尾
		)const;

	// 取扩展名，如果当前保存的不是一个文件，或者不具有扩展名，则返回L"\0"
	const wchar_t* GetExtName(void)const;

	// 复制
	__inline void operator=(const class CFilePathName& src);
	// 从字符串赋值
	__inline void operator=(const wchar_t* nswSrc);

	// 只复制全部目录，结果会保留'\\'
	bool CopyPath(const class CFilePathName& src);

	// 返回完整长度，不包括'\0'结尾
	int GetLength(void)const{
		return msiLength;
	}

	// 判断文件扩展名是否等于指定的类型，使用\0分割类型名，两个\0表示类型结束，返回值<0不相符合，否则返回的是相符合的类型编号，从0开始
	int ExtNameMatch(const wchar_t* nszFilter)const;

	// 直接附加字符串
	void operator+=(const class CFilePathName& src) throw(...);
	void operator+=(const wchar_t* nszAppend)throw(...);

	// 比较，是否相等
	bool operator==(const class CFilePathName& right);
	bool operator!=(const class CFilePathName& right);

private:
	short msiLength;
	wchar_t mszPathName[MAX_PATH];
};


#endif//_CMMPATH_H_