/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */

#ifndef _EDPDFIM_H_
#define _EDPDFIM_H_
#include "PdfmImp.h"



DECLARE_BUILTIN_NAME(CpdfBitmap)
class CpdfBitmap : public cmmBaseObject<CpdfBitmap, IEdBitmap, GET_BUILTIN_NAME(CpdfBitmap)>
{
	friend 	class CpdfPage;

public:
	bin_ptr GetBuffer();
	int32 GetWidth();
	int32 GetHeight();

protected:
	// 内部变量
	int32 mWidth;
	int32 mHeight;
	fz_pixmap* mfzImage;

	CpdfBitmap();
	~CpdfBitmap();

	DEFINE_CUMSTOMIZE_CREATE(CpdfBitmap, (fz_pixmap* fzImage), (fzImage))

	// 初始化函数，可以实现各种不同参数的初始化函数，注意，派生类重载InitOnCreate函数后，一定要调用基类的InitOnCreate函数
	// 返回0表示成功；返回值最高位为1表示发生严重错误，应该终止初始化过程，返回的就是错误码；返回其他值表示其他非错误返回码
	ULONG InitOnCreate(fz_pixmap* fzImage);

};



#endif//_EDPDFIM_H_