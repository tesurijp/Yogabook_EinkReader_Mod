#ifndef PDF_CONVERT_H_
#define PDF_CONVERT_H_

#include <string>

using std::wstring;

namespace PDFConvert
{
	enum class ConvertResult
	{
		OK,
		OfficeComponentNotInstalled,
		UnsupportedFileType,
		FileLocked,
		ComError,
		NeedPassword,
		InvalidFile,
		ConvertFailed,
		PDFFileLocked
	};

	ConvertResult GetConvertResult();   //3.实际返回code
	bool IsConvertingCompleted();  //2.不断通过这个函数获取，返回ture是真的完成了
	void SubmitConvertTask(const wstring& sourceFileName); //1.文件路径通过该接口传进去
	wstring GetPDFFileFullPath();  //4.返回转换后的pdf文档
}


#endif