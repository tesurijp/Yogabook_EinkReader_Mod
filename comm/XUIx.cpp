/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"

#include "cmmBaseObj.h"
#include "cmmstruct.h"
#include "Einkui.h"
#include "Xuix.h"
#include "ElementImp.h"
#include "XCtl.h"






//////////////////////////////////////////////////////////////////////////
// CExMessage
IXelManager* CExMessage::gpElementManager = NULL;
void* CExMessage::DataInvalid=NULL;

DEFINE_BUILTIN_NAME(CExWinPromptBox)

// Post模式发送消息，并且携带一个字符串，注意，字符串一定要带有\0结尾
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
ERESULT CExMessage::PostMessageWithText(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const wchar_t* nswText,ULONG nuFast)
{
	ERESULT luResult;
	IEinkuiMessage* lpMsgIntf;
	int liBytes;

	if(nswText != NULL)
		liBytes = (int)(wcslen(nswText)+1)*sizeof(wchar_t);
	else
		liBytes = 0;

	lpMsgIntf = MakeUpMessage(npSender,false,nuMsgID,nswText,liBytes,NULL,0);

	if(lpMsgIntf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luResult = gpElementManager->PostMessage(npReceiver,lpMsgIntf,nuFast);

	lpMsgIntf->Release();

	return luResult;
}

// Send模式发送消息，并且携带一个字符串，注意，字符串一定要带有\0结尾
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid
ERESULT CExMessage::SendMessageWithText(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const wchar_t* nswText,void* npBufferForReceive,int niBytesOfBuffer)
{
	ERESULT luResult;
	IEinkuiMessage* lpMsgIntf;
	int liBytes;

	if(nswText != NULL)
		liBytes = (int)(wcslen(nswText)+1)*sizeof(wchar_t);
	else
		liBytes = 0;

	lpMsgIntf = MakeUpMessage(npSender,true,nuMsgID,nswText,liBytes,npBufferForReceive,niBytesOfBuffer);

	if(lpMsgIntf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luResult = gpElementManager->SendMessage(npReceiver,lpMsgIntf);

	lpMsgIntf->Release();

	return luResult;
}

// Post模式发送消息，并且携带缓冲区数据
// 如果没有数据发送，rDataToPost填写CExMessage::DataInvalid;		nuFast消息的优先级，EMSG_POST_NORMAL,EMSG_POST_FAST,EMSG_POST_REVERSE
ERESULT CExMessage::PostMessageWithBuffer(IEinkuiIterator* npReceiver,IEinkuiIterator* npSender,ULONG nuMsgID,const void* npBuffer,int niBytes,ULONG nuFast)
{
	ERESULT luResult;
	IEinkuiMessage* lpMsgIntf;

	lpMsgIntf = MakeUpMessage(npSender,false,nuMsgID,npBuffer,niBytes,NULL,0);

	if(lpMsgIntf == NULL)
		return ERESULT_INSUFFICIENT_RESOURCES;

	luResult = gpElementManager->PostMessage(npReceiver,lpMsgIntf,nuFast);

	lpMsgIntf->Release();

	return luResult;
}




// 显示Windows的MessageBox，参数同Windows的完全一致
int CExWinPromptBox::MessageBox(const wchar_t* nswText,const wchar_t* nswTitle,UINT nuType)
{
	ST_MESSAGEBOX ldContext;

	ldContext.Text = nswText;
	ldContext.Title = nswTitle;
	ldContext.Type = nuType;
	ldContext.Result = 0;

	EinkuiGetSystem()->CallBackByWinUiThread(NULL,(PXUI_CALLBACK)&CExWinPromptBox::MessageBoxCallBack,0,&ldContext);

	return ldContext.Result;
}


ERESULT __stdcall CExWinPromptBox::MessageBoxCallBack(ULONG nuFlag,LPVOID npContext)
{
	ST_MESSAGEBOX* lpContext = (ST_MESSAGEBOX*)npContext;

	lpContext->Result = ::MessageBox(EinkuiGetSystem()->GetMainWindow(),lpContext->Text,lpContext->Title,lpContext->Type);

	return ERESULT_SUCCESS;
}


// 显示Windows的MessageBox，参数同Windows的完全一致
int CExWinPromptBox::MessageBoxEx(const wchar_t* nswText,const wchar_t* nswTitle,UINT nuType,WORD nsuLanguage)
{
	ST_MESSAGEBOX ldContext;

	ldContext.Text = nswText;
	ldContext.Title = nswTitle;
	ldContext.Type = nuType;
	ldContext.Language = nsuLanguage;
	ldContext.Result = 0;

	EinkuiGetSystem()->CallBackByWinUiThread(NULL,(PXUI_CALLBACK)&CExWinPromptBox::MessageBoxExCallBack,0,&ldContext);

	return ldContext.Result;
}


ERESULT __stdcall CExWinPromptBox::MessageBoxExCallBack(ULONG nuFlag,LPVOID npContext)
{
	ST_MESSAGEBOX* lpContext = (ST_MESSAGEBOX*)npContext;

	lpContext->Result = ::MessageBoxEx(EinkuiGetSystem()->GetMainWindow(),lpContext->Text,lpContext->Title,lpContext->Type,lpContext->Language);

	return ERESULT_SUCCESS;
}

// 设置鼠标光标
HCURSOR CExWinPromptBox::SetCursor(HCURSOR nhCursor)
{
	if(EinkuiGetSystem()->CallBackByWinUiThread(NULL,(PXUI_CALLBACK)&CExWinPromptBox::SetCursorCallBack,0,&nhCursor,true)!= ERESULT_SUCCESS)
		nhCursor =NULL;

	return nhCursor;
}

ERESULT __stdcall CExWinPromptBox::SetCursorCallBack(ULONG nuFlag,LPVOID npContext)
{
	HCURSOR* lpCursor = (HCURSOR*)npContext;

	*lpCursor = ::SetCursor(*lpCursor);

	return ERESULT_SUCCESS;
}






//用于计算被放大图上的坐标对应于原图的坐标
//rdSrcSize原图大小
//rdDestSize当前的显示大小
//rdPoint要转换的坐标
//nuMethod使用的放大方式：1、ESPB_DRAWBMP_EXTEND（延展线方式） 2、其它值表示直接缩放 
//rdExtendLine如果使用的是ESPB_DRAWBMP_EXTEND放大方式，需要给出延展线坐标
D2D1_POINT_2F CExPoint::BigToOldPoint(D2D1_SIZE_F& rdSrcSize,D2D1_SIZE_F& rdDestSize,D2D1_POINT_2F& rdPoint,ULONG nuMethod,D2D1_POINT_2F& rdExtendLine)
{
	D2D1_POINT_2F ldPoint = rdPoint;

	do 
	{
		if((rdDestSize.width - rdSrcSize.width) < 1.0f && (rdDestSize.height - rdSrcSize.height) < 1.0f)
			break; //没有放大

		if (nuMethod == ESPB_DRAWBMP_EXTEND)
		{
			//延展线方式
			if (rdPoint.x > rdExtendLine.x)
			{
				ldPoint.x = rdPoint.x - (rdDestSize.width-rdSrcSize.width);
				if(ldPoint.x < rdExtendLine.x)
					ldPoint.x = rdExtendLine.x;
			}
			else
			{
				ldPoint.x = rdPoint.x;
			}

			if (rdPoint.y > rdExtendLine.y)
			{
				ldPoint.y = rdPoint.y - (rdDestSize.height-rdSrcSize.height);
				if(ldPoint.y < rdExtendLine.y)
					ldPoint.y = rdExtendLine.y;
			}
			else
			{
				ldPoint.y = rdPoint.y;
			}
		}
		else
		{
			//直接放大的方式
			rdPoint.x *= rdDestSize.width / rdSrcSize.width;
			rdPoint.y *= rdDestSize.height / rdSrcSize.height;
		}

	} while (false);

	return ldPoint;
}


bool CExHotkey::RegisterHotKey(
	IEinkuiIterator* npFocusOn,	// 关注此对象和它的子对象，NULL表示根对象
	IEinkuiIterator* npReceiver,	// 用来接收快捷键的对象
	ULONG nuSysHotkeyID		// 系统默认的ID，见lwUI文件IXelManager定义末尾的系统默认快捷键ID表
	)
{
	bool lbOK = false;

	switch(nuSysHotkeyID)
	{
	case EHOTKEY_COPY:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_COPY,'C',true,false,false,npFocusOn);
		break;
	case EHOTKEY_CUT:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_CUT,'X',true,false,false,npFocusOn);
		break;
	case EHOTKEY_PASTE:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_PASTE,'V',true,false,false,npFocusOn);
		break;
	case EHOTKEY_SELECT_ALL:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_SELECT_ALL,'A',true,false,false,npFocusOn);
		break;
	case EHOTKEY_ESC:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_ESC,VK_ESCAPE,false,false,false,npFocusOn);
		break;
	case EHOTKEY_ENTER:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_ENTER,VK_RETURN,false,false,false,npFocusOn);
		break;
	case EHOTKEY_DELETE:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_DELETE,VK_DELETE,false,false,false,npFocusOn);
		break;
	case EHOTKEY_ALTF4:
		EinkuiGetSystem()->GetElementManager()->RegisterHotKey(npReceiver,EHOTKEY_ALTF4,VK_F4,false,false,true,npFocusOn);
		break;
	default:;
	}

	return lbOK;
}

