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

	ConvertResult GetConvertResult();
	bool IsConvertingCompleted();
	void SubmitConvertTask(const wstring& sourceFileName);
	wstring GetPDFFileFullPath();
}


#endif