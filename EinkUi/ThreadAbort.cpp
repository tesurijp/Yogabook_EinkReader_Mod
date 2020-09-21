#include "stdafx.h"
#include "threadabort.h"


__declspec (noreturn) void CThreadAbort::Throw()
{
	throw CThreadAbort();
}

void CThreadAbort::Dummy() throw (...)
{
	volatile int i = 0;
	if (i)
		Throw();
}

bool CThreadAbort::PullOut(HANDLE nhThread)
{
	bool ok = false;

	// Suspend the thread, so that we won't have surprises
	DWORD dwVal = SuspendThread(nhThread);
	if (INFINITE != dwVal)
	{
		// Get its context (processor registers)
		CONTEXT ctx;
		ctx.ContextFlags = CONTEXT_CONTROL;
		if (GetThreadContext(nhThread, &ctx))
		{
			// Jump into our Throw() function
#if defined(_AMD64_)
			ctx.Rip = (DWORD64)(void*)Throw;
#elif defined(_X86_)
			ctx.Eip = (DWORD)(DWORD_PTR)Throw;

#else
#error can't using in platform other than AMD64 and x86
#endif
			if (SetThreadContext(nhThread, &ctx))
				ok = true;
		}
		// proceed
		ResumeThread(nhThread);
	}
	return ok;
}