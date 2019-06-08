/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


/* Copyright 2015 the SumatraPDF project authors (see AUTHORS file).
   License: GPLv3 */

/* Copied from EngineManager.h*/

/* 
modify:
add the loading progress callback function 
*/

enum EngineType {
    Engine_None = 0,
    // the EngineManager tries to create a new engine
    // in the following order (types on the same line
    // share common code and reside in the same file)
    Engine_PDF, Engine_XPS,
    Engine_DjVu,
    Engine_Image, Engine_ImageDir, Engine_ComicBook,
    Engine_PS,
    Engine_Epub, Engine_Fb2, Engine_Mobi, Engine_Pdb,
        Engine_Chm, Engine_Html, Engine_Txt,
};

typedef bool(__stdcall *PEDSMT_THREAD_CALLBACK)(uint32 loadingStep, void* context);

namespace EngineLoad {

	bool IsSupportedFile(const WCHAR *filePath, bool sniff = false);
	bool CreateEngine(const WCHAR *filePath, BaseEngine ** engineObj, PasswordUI *pwdUI = nullptr, EngineType *typeOut = nullptr, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL);

}



