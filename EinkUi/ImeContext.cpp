/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "CommonHeader.h"
#include "XsWgtContext.h"

#include "ImeContext.h"


DEFINE_BUILTIN_NAME(CXsImeContext)



CXsImeContext::CXsImeContext()
{
	mhPreviousImc = NULL;
	mhImcToHaltIme = NULL;
	mbAssociated = false;
	mdComInfo.dwStyle = CFS_POINT;
	mdComInfo.ptCurrentPos.x = 0;
	mdComInfo.ptCurrentPos.y = 0;
}

CXsImeContext::~CXsImeContext()
{
	if(mbAssociated != false)
		ImmAssociateContext(EinkuiGetSystem()->GetMainWindow(),mhPreviousImc);
	if(mhImcToHaltIme != NULL)
		ImmDestroyContext(mhImcToHaltIme);
}

ERESULT CXsImeContext::InitOnCreate()
{
	mhImcToHaltIme = ImmCreateContext();

	ImmSetOpenStatus(mhImcToHaltIme,FALSE);

	return ERESULT_SUCCESS;
}


LRESULT CXsImeContext::OnImeMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_IME_NOTIFY:	// 当用户切换输入法开启IME的时候，所有的Context都会执行开启动作，所以，只要收到开启消息，就要检查我们的禁用IME的Context是否被开启了。
		if(mhImcToHaltIme != NULL && wParam == IMN_SETOPENSTATUS && ImmGetOpenStatus(mhImcToHaltIme)!=FALSE)	
			ImmSetOpenStatus(mhImcToHaltIme,FALSE);		// 将其关闭
		break;
	case WM_IME_STARTCOMPOSITION:
		{
			HIMC lhImc;

			lhImc = ImmGetContext(EinkuiGetSystem()->GetMainWindow());

			ImmSetCompositionWindow(lhImc,&mdComInfo);

			ImmReleaseContext(EinkuiGetSystem()->GetMainWindow(),lhImc);
		}
		break;
	default:;
	}

	return DefWindowProc(hwnd,message,wParam,lParam);
}

void CXsImeContext::EnableIme(void)
{
	if(mhImcToHaltIme == NULL || mbAssociated == false)
		return ;

	EinkuiGetSystem()->CallBackByWinUiThread(this,(PXUI_CALLBACK)&CXsImeContext::ChangeImeContextCallback,FALSE,NULL,true);
}

void CXsImeContext::DisableIme(void)
{
	if(mhImcToHaltIme == NULL || mbAssociated != false)
		return ;

	EinkuiGetSystem()->CallBackByWinUiThread(this,(PXUI_CALLBACK)&CXsImeContext::ChangeImeContextCallback,TRUE,NULL,true);
}

void CXsImeContext::SetImeCompositionWnd(D2D1_POINT_2F ndPosition)
{
	mdComInfo.dwStyle = CFS_FORCE_POSITION;//CFS_POINT;
	mdComInfo.ptCurrentPos.x = (LONG)(ndPosition.x+0.5f);
	mdComInfo.ptCurrentPos.y = (LONG)(ndPosition.y+0.5f);
}

ERESULT __stdcall CXsImeContext::ChangeImeContextCallback(ULONG nuFlag,LPVOID npContext)
{
	if(nuFlag != FALSE)
	{	// 禁用IME，换用自建的IME上下文
		if(mhImcToHaltIme == NULL || mbAssociated != false)
			return ERESULT_SUCCESS;

		mbAssociated = true;
		mhPreviousImc = ImmAssociateContext(EinkuiGetSystem()->GetMainWindow(),mhImcToHaltIme);
	}
	else
	{
		// 取消禁用，换回默认的IME
		if(mhImcToHaltIme == NULL || mbAssociated == false)
			return ERESULT_SUCCESS;

		mbAssociated = false;
		ImmAssociateContext(EinkuiGetSystem()->GetMainWindow(),mhPreviousImc);
		mhPreviousImc = NULL;
	}

	return ERESULT_SUCCESS;
}



