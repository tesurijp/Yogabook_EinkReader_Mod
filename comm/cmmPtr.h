/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _CMM_PTR_
#define _CMM_PTR_

template<class CCmmContent>
class cmmDeletePtr {
public:
	virtual ~cmmDeletePtr() {
		freePtr();
	}

	cmmDeletePtr() {
		mPtr = NULL;
	}

	// 用指向类型的指针构造
	cmmDeletePtr(CCmmContent* src) {
		mPtr = src;
	}

	// 用智能指针构造，发生转移
	cmmDeletePtr(cmmDeletePtr<CCmmContent>& src) {
		move(src);
	}

	// new 分配一组对象
	bool allocate(int size) {
		freePtr();

		mPtr = new CCmmContent[size];

		return mPtr != NULL;
	}

	// 拷贝复制函数，自动释放前一次保存的指针
	void operator = (CCmmContent* src) {
		freePtr();

		mPtr = src;
	}

	// 无类型复制，用于特殊的场合，将不同类型的指针强制放入
	void set_by_void(void* pt) {
		freePtr();

		mPtr = (CCmmContent*)src;
	}

	// 复制函数，自动释放前一次保存的指针，获取新对象所有权
	void operator ()(cmmDeletePtr<CCmmContent>& src) {
		take(src);
	}

	// 复制函数，自动释放前一次保存的指针，获取新对象所有权，注意src的对象必须是正确的获得方法获得的，以适应 delete/release/HeapFree的释放要求
	void take(CCmmContent* src) {
		freePtr();

		mPtr = src;
	}

	// 复制函数，自动释放前一次保存的指针，获取新对象所有权
	void take(cmmDeletePtr<CCmmContent>& src) {
		freePtr();

		mPtr = src.mPtr;
		src.mPtr = NULL;
	}

	// 获取保存在指针, xxx()
	CCmmContent* operator ()(){
		return mPtr;
	}

	// 获取保存的指针，xxx.ptr()
	CCmmContent* ptr() {
		return mPtr;
	}

	// 获取只读的数据缓冲区
	CCmmContent* conPtr() const{
		return mPtr;
	}

	// 获取保存的指针，xxx(false)不放弃对象(不转移所有权，对象将会随着本对象而被删除），xxx(true)放弃对象（转移对象所有权，对象不随本对象被删除）
	CCmmContent* operator ()(bool move) {
		if (move != false)
			return notfree();
		return mPtr;
	}

	// 获取保存的指针，xxx.notfree放弃对象
	CCmmContent* notfree() {
		auto rev = mPtr;
		mPtr = NULL;
		return rev;
	}

	// 重置对象，释放之前保存的指针
	void reset() {
		freePtr();
	}

	// 判断是否指向同一个目标
	bool operator ==(const CCmmContent* src) {
		return mPtr == src;
	}

	// 判断是否指向不同一个目标
	bool operator !=(const CCmmContent* src) {
		return mPtr != src;
	}

	// 判断是否指向同一个目标
	bool  operator == (const cmmDeletePtr<CCmmContent>& src) {
		return mPtr == src.mPtr;
	}

	// 获取保存的指针指向的对象
	CCmmContent& operator *() {
		return *mPtr;
	}

	// 获取保存的指针指向的对象
	CCmmContent& object() {
		return *mPtr;
	}

	// 获取指针本身的地址，用于接收外部的赋值
	CCmmContent**  addressOfPtr() {
		return &mPtr;
	}

	// 直接调用指针的成员对象
	CCmmContent* operator ->() {
		return mPtr;
	}


protected:
	CCmmContent* mPtr;

	virtual void freePtr() {
		if (mPtr != NULL)
		{
			delete[] mPtr;
			mPtr = NULL;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// 释放时，自动调用xxx->Release();
template<class CCmmContent>
class cmmReleasePtr :public cmmDeletePtr<CCmmContent> {
public:
	cmmReleasePtr() : cmmDeletePtr<CCmmContent>() {}
	virtual ~cmmReleasePtr() {	// 必须自己实现，基类的析构函数不能调用虚函数，无法通过虚函数freePtr释放
		freePtr();
	}
	cmmReleasePtr(CCmmContent* src) : cmmDeletePtr<CCmmContent>(src) {	}

	virtual void freePtr() {
		if (mPtr != NULL)
		{
			mPtr->Release();
			mPtr = NULL;
		}
	}

	void operator = (CCmmContent* src) {
		freePtr();
		mPtr = src;
	}
};

//////////////////////////////////////////////////////////////////////////
// 释放时，自动调用堆释放 heapfree(xxx);
template<class CCmmContent>
class cmmHeapPtr :public cmmDeletePtr<CCmmContent> {
public:
	cmmHeapPtr() : cmmDeletePtr<CCmmContent>() {}
	virtual ~cmmHeapPtr() {
		freePtr();
	}
	virtual void freePtr() {
		if (mPtr != NULL)
		{
			HeapFree(GetProcessHeap(), 0, mPtr);
			mPtr = NULL;
		}
	}
	void operator = (CCmmContent* src) {
		freePtr();
		mPtr = src;
	}

	// 从堆中申请一块内存
	bool allocate(int size) {
		freePtr();
		mPtr = (CCmmContent*)HeapAlloc(GetProcessHeap(), 0, size);
		return mPtr != NULL;
	}
};


//cmmDeletePtr<classX> xx;
//xx = new classX;
//xx->XFun();
//
//return xx(true);	// return classX*，and discard xx;
//
//return xx();		// return classX*, and keep going of xx
//
//return *xx;		// return classX
//
//cmmDeletePtr<classX> bb;
//
//bb(xx);		// move classX* from xx to bb，and discard xx
//













#endif//_CMM_PTR_