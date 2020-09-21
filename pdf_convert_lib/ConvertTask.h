#ifndef CONVERT_TASK_H_
#define CONVERT_TASH_H_

#include "pch.h"
#include "PdfConvert.h"
#include "ScopeGuard.h"
#include <mutex>

namespace PDFConvert
{
	void DoWordConvert_cs(wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock);

	void DoExcelConvert_cs(wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock );

	void DoPowerPointConvert_cs( wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock );

	void DoVisioConvert_cs( wstring sourceFilePath, wstring targetFilePath, ConvertResult* convertResult,
		bool* convertCompleted, std::mutex* setFinishFlagLock );
}

#endif
