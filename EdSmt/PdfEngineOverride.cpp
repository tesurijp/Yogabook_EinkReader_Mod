/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "PdfEngine.cpp"
#include "EngineOverride.h"



class PdfEngineOverride : public PdfEngineImpl
{
protected:
	PEDSMT_THREAD_CALLBACK mCallBackFun;
	void* mCallBackContext;

public:
	PdfEngineOverride(PEDSMT_THREAD_CALLBACK callFun, void* callContext):PdfEngineImpl(){
		mCallBackContext = callContext;
		mCallBackFun = callFun;
	}

	bool Load(const WCHAR *fileName, PasswordUI *pwdUI) {
		InvokeCallbackFunction(0);
		auto rev = PdfEngineImpl::Load(fileName, pwdUI);

		InvokeCallbackFunction(MAXULONG32);

		return rev;
	}

	void InvokeCallbackFunction(uint32 loadingStep) {
		if (mCallBackFun != NULL)
		{
			try {
				mCallBackFun(loadingStep, mCallBackContext);
			}
			catch (...) {}
		}
	}
};


BaseEngine *PdfCreateFromFile(const wchar_t *fileName, PasswordUI *pwdUI, BaseEngine ** engineObj, PEDSMT_THREAD_CALLBACK callFun = NULL, void* callContext = NULL)
{
	PdfEngineOverride *engine = new PdfEngineOverride(callFun, callContext);
	InterlockedExchangePointer((void**)engineObj, engine);
	if (!engine || !fileName || !engine->Load(fileName, pwdUI)) {
		delete engine;
		InterlockedExchangePointer((void**)engineObj, NULL);
		return nullptr;
	}
	return engine;
}
