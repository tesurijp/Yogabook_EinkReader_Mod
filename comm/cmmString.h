/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#ifndef _CMM_STRING_
#define _CMM_STRING_
#include "type_traits"
#include "cmmptr.h"



// only for char or wchar_t
template<class CCharType>
class cmmString
{
private:
	template<class ... tail>
	static int call_sprintf(wchar_t* buf, int len, tail... args) {
		return swprintf_s(buf, len, args...);
	}
	template<class ... tail>
	static int call_sprintf(char* buf, int len, tail... args) {
		return sprintf_s(buf, len, args...);
	}

	template<class ... tail>
	static int call_sscanf(wchar_t* buf, const wchar_t* fmtStr, tail... args) {
		return swscanf_s(buf, fmtStr, args...);
	}
	template<class ... tail>
	static int call_sscanf(char* buf, const char* fmtStr, tail... args) {
		return sscanf_s(buf, fmtStr, args...);
	}

	static auto call_strcpy(char* s1, int len, const char* s2)
	{
		strcpy_s(s1, len, s2);
	}
	static auto call_strcpy(wchar_t* s1, int len, const wchar_t* s2)
	{
		wcscpy_s(s1, len, s2);
	}

	static auto call_strcat(char* s1, int len, const char* s2)
	{
		strcat_s(s1, len, s2);
	}
	static auto call_strcat(wchar_t* s1, int len, const wchar_t* s2)
	{
		wcscat_s(s1, len, s2);
	}
	inline static void GetBcdChar(int index, char& ch) {
		const auto overall = "0123456789ABCDEF";
		ch= overall[index];
	}
	inline static void GetBcdChar(int index, wchar_t& ch) {
		const auto overall = L"0123456789ABCDEF";
		ch = overall[index];
	}

	static auto call_strcmp(const char* s1,const char* s2)
	{
		return strcmp(s1, s2);
	}
	static auto call_strcmp(const wchar_t* s1, const wchar_t* s2)
	{
		return wcscmp(s1, s2);
	}
	static auto call_stricmp(const char* s1, const char* s2)
	{
		return stricmp(s1, s2);
	}
	static auto call_stricmp(const wchar_t* s1, const wchar_t* s2)
	{
		return wcsicmp(s1, s2);
	}

	void convertFrom(const wchar_t* strIn) {
		if (UnicodeType() == false)
			mContent = (CCharType*)U16ToMulti(strIn);
		else
			mContent = copy((const CCharType*)strIn);
	}

	void convertFrom(const char* strIn) {

		if (UnicodeType() == false)
		{
			mContent = copy((const CCharType*)strIn);
		}
		else
		{
			BOMINFO bom = GetBomInfor(strIn, (long)strlen(strIn));
			switch (bom)
			{
			case BOMINFO::utf8:
				mContent = (CCharType*)U8ToU16(strIn);
				break;
			case BOMINFO::utf7:
				mContent = (CCharType*)ToU16(strIn, CP_UTF7);
				break;
			case BOMINFO::none:
				mContent = (CCharType*)MultiToU16(strIn);
				break;
			case BOMINFO::utf16b:
			case BOMINFO::utf16:
			default:
				throw "unsupported type";
				break;
			}
		}
	}

public:
	~cmmString() {
	}
	cmmString() {
		mContent = NULL;
	}
	cmmString(const CCharType* strIn):cmmString() {
		mContent = copy(strIn);
	}
	cmmString(const cmmString<CCharType>& src):cmmString() {
		*this = src;
	}

	// 复制字符串
	static CCharType* copy(const CCharType* strIn,const CCharType* strAppend=NULL,int bufLen=0) {
		int length = 1;
		if (strIn != NULL)
			length += size(strIn);
		if(strAppend != NULL)
			length += size(strAppend);

		if (bufLen != 0)
		{
			if (length <= bufLen)
				length = bufLen;
			else
				return NULL;
		}
		
		CCharType* ptr = new CCharType[length];

		*ptr = 0;

		if (strIn != NULL)
			call_strcpy(ptr, length, strIn);
		if (strAppend != NULL)
			call_strcat(ptr, length, strAppend);

		return ptr;
	}

	void operator = (wchar_t* strIn){
		convertFrom(strIn);
	}

	void operator = (char* strIn) {
		convertFrom(strIn);
	}

	void operator = (const cmmString<wchar_t>& src) {
		convertFrom(src.conPtr());
	}
	void operator = (const cmmString<char>& src) {
		convertFrom(src.conPtr());
	}

	template<class ... T>
	int format(const CCharType* pstrFormat,T... args) {

		mContent.allocate(1024);

		return call_sprintf(mContent.ptr(), 1024, pstrFormat,args...);
	}

/*
	scanf可以是一个或多个 {%[*] [width] [{h | I | I64 | L}]type | ' ' | '\t' | '\n' | 非%符号}
注：
1、 * 亦可用于格式中, (即 %*d 和 %*s) 加了星号 (*) 表示跳过此数据不读入. (也就是不把此数据读入参数中)
2、{a|b|c}表示a,b,c中选一，[d],表示可以有d也可以没有d。
3、width表示读取宽度。
4、{h | l | I64 | L}:参数的size,通常h表示单字节size，I表示2字节 size,L表示4字节size(double例外),l64表示8字节size。
5、type :这就很多了，就是%s,%d之类。
6、特别的：%*[width] [{h | l | I64 | L}]type 表示满足该条件的被过滤掉，不会向目标参数中写入值
失败返回0 ，否则返回格式化的参数个数
支持集合操作
%[a-z] 表示匹配a到z中任意字符，贪婪性(尽可能多的匹配)
%[aB'] 匹配a、B、'中一员，贪婪性
%[^a] 匹配非a的任意字符，并且停止读入，贪婪性
*/
	// demo "100,200-I am a dog" => scanf("5d,%d-%s",v1,v2,v3,v3.bufsize)
	template<class ... T>
	int scanf(const CCharType* pstrFormat, T... args) {

		if (mContent == NULL)
			return 0;

		return call_sscanf(mContent.ptr(), pstrFormat, args...);
	}

	int DumpBinBuf(const unsigned char* buf, int bufSize) {
		if (bufSize <= 0)
			return 0;

		if (mContent.allocate(bufSize*2+1) == false)
			return 0;

		CCharType* dumpOut = mContent.ptr();
		auto endP = buf + bufSize;
		while (buf < endP)
		{
			GetBcdChar(((*buf) >> 4), *dumpOut++);
			GetBcdChar(((*buf++) & 0x0F), *dumpOut++);
		}
		*dumpOut = 0;

		return bufSize * 2;
	}

	bool operator == (const wchar_t* compareWith) {
		if (mContent.ptr() == NULL)
			return false;
		return compare(compareWith) == 0;
	}

	bool operator == (const char* compareWith) {
		if (mContent.ptr() == NULL)
			return false;
		return compare(compareWith) == 0;
	}

	int compare(const wchar_t* compareWith, bool sensitive = false) {
		if (mContent.ptr() == NULL)
		{
			if (compareWith == NULL)
				return 0;
			return -1;
		}
		if (UnicodeType() == false)
		{
			cmmDeletePtr<char> temp = U16ToMulti(compareWith);

			if(sensitive == false)
				return strcmp((const char*)mContent.ptr(), temp.ptr());
			else
				return _stricmp((const char*)mContent.ptr(), temp.ptr());
		}
		else
		{
			if (sensitive == false)
				return wcscmp((const wchar_t*)mContent.ptr(), compareWith);
			else
				return _wcsicmp((const wchar_t*)mContent.ptr(), compareWith);
		}
	}

	int compare(const char* compareWith,bool sensitive=false) {
		if (mContent.ptr() == NULL)
		{
			if (compareWith == NULL)
				return 0;
			return -1;
		}
		if (UnicodeType() == false)
		{
			if(sensitive == false)
				return strcmp((const char*)mContent.ptr(), compareWith);
			else
				return _stricmp((const char*)mContent.ptr(), compareWith);
		}
		else
		{
			cmmDeletePtr<wchar_t> temp = MultiToU16(compareWith);

			if(sensitive == false)
				return wcscmp((const wchar_t*)mContent.ptr(), temp.ptr());
			else
				return _wcsicmp((const wchar_t*)mContent.ptr(), temp.ptr());
		}
	}

	void copyUtf8(const char* u8Str) {
		auto u16Ptr = U8ToU16(u8Str);
		if (UnicodeType() == false)
		{
			auto multiPtr = U16ToMulti(u16Ptr);
			*this = multiPtr;
		}
		else
		{
			*this = u16Ptr;
		}
	}

	// 不复制，直接获取缓冲区的控制权
	CCharType* take(cmmString<CCharType>& src){

		mContent = src.mContent;
		src.mContent = NULL;
	}

	CCharType operator [](int index) {
		return mContent[index];
	}

	CCharType* operator() (void) {
		return mContent.ptr();
	}

	// 返回数据缓冲区
	CCharType* ptr(void) {
		return mContent.ptr();
	}
	// 返回数据缓冲区
	const CCharType* conPtr(void) const {
		return mContent.conPtr();
	}

	// 返回字符数，不含尾部0
	int size(void) {
		if (mContent == NULL)
			return 0;

		return size(mContent.ptr());
	}

	// 返回包含尾部0的有效数据字节数
	int bufSize(void) {
		auto chars = size();
		if (chars == 0)
			return 0;

		return (chars + 1) * sizeof(CCharType);
	}

	static int size(const CCharType* strIn) {
		if (UnicodeType() == false)
			return (int)strlen((char*)strIn);
		else
			return (int)wcslen((wchar_t*)strIn);
	}

	CCharType* operator +=(const CCharType* strAdd) {
		mContent = copy(mContent.ptr(), strAdd);

		return mContent.ptr();
	}

	// 获取保存的字符串缓存，释放控制权
	CCharType* notfree() {
		return mContent.notfree();
	}

	// 重置对象
	void reset() {
		mContent.reset();
		mTempBuf.reset();
	}

	// 获取存储区指针，并且锁定存储区到指定大小，使用此缓存期间，不要操作本对象的其他函数
	CCharType* lockBuf(int bufLen=0) {
		mContent = copy(mContent.ptr(), NULL, bufLen);

		return mContent.ptr();
	}

	void unlockBuf(void) {
		// 不指定大小的copy函数，将缩减缓冲区到符合尺寸
		mContent = copy(mContent.ptr());
	}

	// 获取的指针在下次操作本对象后可能失效，如需长期使用需复制保存
	wchar_t* CopyToUnicodeBuffer(void) {
		if (UnicodeType() == false)
		{
			mTempBuf = MultiToU16((const char*)mContent.ptr());

			return mTempBuf.ptr();
		}
		else
		{
			return (wchar_t*)mContent.ptr();
		}
	}

	// 获取的指针在下次操作本对象后可能失效，如需长期使用需复制保存
	char* CopyToMultiByteBuffer(int* bufSize=NULL, unsigned int codePage = CP_ACP) {
		if (UnicodeType() == false)
		{
			if (codePage != CP_ACP)
				return NULL;	// 暫時不支持從ansi到utf8

			if (bufSize != NULL)
				*bufSize = (int)strnlen_s((char*)(mContent.ptr()), 2048)+1;
			return (char*)mContent.ptr();
		}
		else
		{
			mTempBuf = U16ToMulti((wchar_t*)mContent.ptr(),bufSize,codePage);

			return mTempBuf.ptr();
		}
	}

	char* U16ToMulti(const wchar_t* u16Str,int* bufSize=NULL,unsigned int codePage= CP_ACP) {
		int byteCount = WideCharToMultiByte(codePage, 0, u16Str, -1, NULL, 0,NULL,FALSE);

		if (byteCount == 0)
			return NULL;

		cmmDeletePtr<char> multiPtr = new char[byteCount];
		if (multiPtr == NULL)
			return NULL;

		if (bufSize != NULL)
			*bufSize = byteCount;

		if (WideCharToMultiByte(codePage, 0, u16Str, -1, multiPtr.ptr(), byteCount,NULL,FALSE) == byteCount)
			return multiPtr.notfree();

		return NULL;
	}

	wchar_t* MultiToU16(const char* multiStr)	{
		return ToU16(multiStr, CP_ACP);
	}

	wchar_t* U8ToU16(const char* u8Str)	{
		return ToU16(u8Str, CP_UTF8);
	}

	

	static bool UnicodeType(void) {
		return std::is_same<CCharType, wchar_t>::value;//https://blog.csdn.net/czyt1988/article/details/52812797
	}

protected:
	cmmDeletePtr<CCharType> mContent;
	cmmDeletePtr<char> mTempBuf;

	enum BOMINFO{
		utf7,
		utf8,
		utf16,
		utf16b,
		none
	}BOM;

	wchar_t* ToU16(const char* packedStr,UINT codePage)
	{
		int byteCount = MultiByteToWideChar(codePage, 0, packedStr, -1, NULL, 0);
		if (byteCount == 0)
			return NULL;

		auto widePtr = new wchar_t[byteCount];
		if (widePtr == NULL)
			return NULL;

		if (MultiByteToWideChar(codePage, 0, packedStr, -1, widePtr, byteCount) == byteCount)
			return widePtr;

		delete[]widePtr;
		return NULL;
	}

	BOMINFO GetBomInfor(const void* npBuffer, long nlSize)
	{
		const unsigned char* const stringHead = (const unsigned char*)npBuffer;
		BOMINFO rev = BOMINFO::none;

		do
		{
			if (nlSize < 3)
				break;

			if (stringHead[0] == 0xFF && stringHead[1] == 0xFE)
			{
				rev = BOMINFO::utf16;
				break;
			}

			if (stringHead[0] == 0xFE && stringHead[1] == 0xFF)
			{
				rev = BOMINFO::utf16b;
				break;
			}

			if (stringHead[0] == 0xEF && stringHead[1] == 0xBB && stringHead[2] == 0xBF)
			{
				rev = BOMINFO::utf8;
				break;
			}

			if (stringHead[0] == 0x2B && stringHead[1] == 0x2F && stringHead[2] == 0x76 )
			{
				rev = BOMINFO::utf7;
				break;
			}

			// 尝试分析一下，这个目标是不是utf8
			if (IsU8Data(npBuffer,nlSize) != false)
			{
				// mBom = u8NoBOM;
				rev = BOMINFO::utf8;
			}

		} while (false);

		return rev;
	}


	//判断内容是否是utf8
	bool IsU8Data(const void* npBuffer, long nlSize)
	{
		bool lbIsUTF8 = false;
		int liUtf8Count = 0;
		unsigned char* lpStart = (unsigned char*)npBuffer;
		unsigned char* lpEnd = (unsigned char*)npBuffer + nlSize;
		while (lpStart < lpEnd)
		{
			if (*lpStart < 0x80) // (10000000): 值小于0x80的为ASCII字符    
			{
				lpStart++;
			}
			else if (*lpStart < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符    
			{
				lbIsUTF8 = false;
				liUtf8Count = 0;
				break;
			}
			else if (*lpStart < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符    
			{
				if (lpStart >= lpEnd - 1)
				{
					break;
				}

				if ((lpStart[1] & (0xC0)) != 0x80)
				{
					lbIsUTF8 = false;
					liUtf8Count = 0;
					break;
				}

				liUtf8Count++;
				lpStart += 2;
			}
			else if (*lpStart < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符    
			{
				if (lpStart >= lpEnd - 2)
				{
					break;
				}

				if ((lpStart[1] & (0xC0)) != 0x80 || (lpStart[2] & (0xC0)) != 0x80)
				{
					lbIsUTF8 = false;
					liUtf8Count = 0;
					break;
				}

				liUtf8Count++;
				lpStart += 3;
			}
			else
			{
				lbIsUTF8 = false;
				liUtf8Count = 0;
				break;
			}
		}

		return liUtf8Count > 0;
	}

};




typedef cmmString<char> cmmStringA;
typedef cmmString<wchar_t> cmmStringW;


#endif//_CMM_STRING_