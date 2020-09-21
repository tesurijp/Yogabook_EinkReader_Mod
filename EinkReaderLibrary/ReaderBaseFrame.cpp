/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include <sstream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <cctype>
#include "ReaderBaseFrame.h"
#include "CYesNoPromptDlg.h"
#include "PDFOverwriteDlg.h"
#include "ConvertProgressDlg.h"
#include "OpenFileLockedDlg.h"
#include "XCtl.h"
#include <shellapi.h>

#include "SmCuveImp.h"
#include "stdio.h"
#include "HellFun.h"
#include <tlhelp32.h>
#include "winsock.h"

#include "cmmStrHandle.h"
#include "math.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <Shlobj.h>
#include "time.h"
#include "util.h"

DEFINE_BUILTIN_NAME(Reader_BaseFrame)

const wchar_t* kReaderTempPath = LR"###(Lenovo\Reader)###";
const wchar_t* kCommandLineParamFile = L"cmdparam";

using std::wostringstream;
using std::unique_ptr;
using std::make_unique;

CReaderBaseFrame::CReaderBaseFrame(void)
{
	mpFileHistoryDlg = NULL;
	mpTipFrame = NULL;
	mpPdfPicture = NULL;
	mpJumpPage = NULL;
	mpIterBackground = NULL;
	mpThumbnailDlg = NULL;
	mpPreNextButton = NULL;
	mpSnapShot = NULL;
	mpIterToast = NULL;
	mlHoldInput = 0;
	mxLastGcUiNumber = 0;
	muLastGcTick = 0;
	mbGcMode = false;
	mpToolbarH = NULL;
	mbIsTxt = false;
	mpLoadingView = NULL;
	mbIsLoadingSuccess = false;
	mpHighlight = NULL;
	mbIsScreenAuto = true;

	promptDlg = nullptr;
	pdfOverwriteDlg = nullptr;
	convertProgressDlg = nullptr;
	askConvertDlg = nullptr;

	//txt字号
	mdwFontsizeIndex = 1;
	mdwFontSizeArray[0] = 9;
	mdwFontSizeArray[1] = 12;
	mdwFontSizeArray[2] = 15;
	mdwFontSizeArray[3] = 18;
	mdwFontSizeArray[4] = 21;

	//笔宽度
	mdwPenWidthIndex = 1;
	mfArrayPenWidthArray[0] = 0.5f;
	mfArrayPenWidthArray[1] = 1.0f;
	mfArrayPenWidthArray[2] = 2.0f;
	mfArrayPenWidthArray[3] = 4.0f;
	mfArrayPenWidthArray[4] = 8.0f;
	mfArrayFwPenWidthArray[0] = 0x1;
	mfArrayFwPenWidthArray[1] = 0x1;
	mfArrayFwPenWidthArray[2] = 0x2;
	mfArrayFwPenWidthArray[3] = 0x4;
	mfArrayFwPenWidthArray[4] = 0x7;
	mfArrayFwFingerWidthArray[0] = 0x1;
	mfArrayFwFingerWidthArray[1] = 0x1;
	mfArrayFwFingerWidthArray[2] = 0x2;
	mfArrayFwFingerWidthArray[3] = 0x4;
	mfArrayFwFingerWidthArray[4] = 0x7;

	mdwPenColorIndex = 0;

	mszTempFile[0] = UNICODE_NULL;
	mszSrcFile[0] = UNICODE_NULL;

	mbIsSetPartial = true;
	mulPageIndex = 0;
	mulPageCount = 0;
	

	mulPenMode = PEN_MODE_NONE;
	mbIsHand = false;
	//mhFile = INVALID_HANDLE_VALUE;
	enableCapacitivepen = true;

	//Sleep(1000 * 20);

	wostringstream pathBuffer;
	const int bufferSize = 2048;
	unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(bufferSize);
	GetTempPathW(bufferSize, buffer.get());
	pathBuffer << buffer.get() << kReaderTempPath << L"\\" << kCommandLineParamFile;
	if (GetFileAttributesW(pathBuffer.str().c_str()) == INVALID_FILE_ATTRIBUTES) return;

	FILE* f = nullptr;
	_wfopen_s(&f, pathBuffer.str().c_str(), L"rt,ccs=utf-8");
	if (f == nullptr) return;
	fgetws(buffer.get(), bufferSize, f);
	if (wcsstr(buffer.get(), L"-open") == buffer.get())
	{
		wstring s = buffer.get();
		size_t i;
		for (i = wcslen(L"-open "); i < s.size() && s[i] == ' '; i++);
		m_cmdLineOpenFileName = s.substr(i);
	}

	fclose(f);
	DeleteFileW(pathBuffer.str().c_str());
}


CReaderBaseFrame::~CReaderBaseFrame(void)
{
	while (mdHistroyPath.Size() > 0)
	{
		delete mdHistroyPath.GetEntry(0);
		mdHistroyPath.RemoveByIndex(0);
	}

	//if (mhFile != INVALID_HANDLE_VALUE)
	//	CloseHandle(mhFile);
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CReaderBaseFrame::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		// 设置风格支持拖拽
		mpIterator->ModifyStyles(EITR_STYLE_POPUP);
		

		EinkuiGetSystem()->SetEinkUpdatingCallBack((PXUI_EINKUPDATING_CALLBACK)&EinkUpdating, this);
		EinkuiGetSystem()->EnablePaintboard();
#ifdef SHOW_BCOVER_WINDOW
		EinkuiGetSystem()->ShowMainWindow(true);
#endif

		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_ACTIVATE, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnActivity);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_HOMEBAR_MODE_CHANGE, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnHomebarChanged);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_LAPTOP_MODE_CHANGE, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnModeChanged);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_RESET_TP_AREA, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnNotifyResetTpArea);
		// Register listener to Open Office Docs with 'TimeStamp' method [zhuhl5@20200121]
		//EinkuiGetSystem()->CaptureWindowsMessage(WM_EI_READER_CONVERT_OFFICE_FILE_WITHTS, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnOpenOfficeFileWithTS);
		EinkuiGetSystem()->CaptureWindowsMessage(WM_POWERBROADCAST, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnPowerChangeMsg);
		
		/*RAWINPUTDEVICE Rid = {};

		Rid.usUsagePage = 0x01;
		Rid.usUsage = 0x06;
		Rid.dwFlags = RIDEV_INPUTSINK;
		Rid.hwndTarget = EinkuiGetSystem()->GetMainWindow();

		auto regResult = RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE));
		if (!regResult)
		{

		}
		EinkuiGetSystem()->CaptureWindowsMessage(WM_INPUT, this, (PWINMSG_CALLBACK)&CReaderBaseFrame::OnInputChangeMsg);*/
		//mpIterator->SetTimer(EINK_SETTINGS_TIMER_ID_PAINT, MAXULONG32, 1000, NULL);
		//ImmDisableIME(-1);	//关闭输入法

		//mpIterator->SetTimer(1,MAXULONG32,3000,NULL);

		EiSetWaveformMode(GI_WAVEFORM_DU2);
		EiCleanupScreen(0xff);
		Sleep(585); //DU260+15*5   分16帧刷，每帧间隔5ms
		EiSetWaveformMode(GI_WAVEFORM_GC16);

		ShowToolBar(false);
		mpIterator->SetTimer(RBF_TIMER_ENABL_PARTIAL, 1, 1800, NULL);
		//mpIterator->SetTimer(RBF_TIMER_INIT, 1, 100, NULL);
		Init();

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

ULONG CReaderBaseFrame::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;
	ICfKey* lpSubKey = NULL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;

		EI_SYSTEM_INFO ldSysInfo;
		EiGetSystemInfo(&ldSysInfo);
		EiSetScreenOrient(ldSysInfo.ulOrient);
		

		//EI_SYSTEM_INFO ldInfo;
		//EiGetSystemInfo(&ldInfo);
		EiSetHandWritingMode(GIHW_OWNER_DRAW);
		

		//EiGetSystemInfo(&ldInfo);

		mpIterToast = mpIterator->GetSubElementByID(2);
		BREAK_ON_NULL(mpIterToast);
		mpIterToast->SetVisible(false);
		mpIterToast->SetDefaultZOrder(900000);
		
		mpIterBackground = mpIterator->GetSubElementByID(1);
		BREAK_ON_NULL(mpIterBackground);

		//翻页
		lpSubKey = mpTemplete->OpenKey(L"PreNextButton");
		mpPreNextButton = CPreNextButton::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPreNextButton);

		//高亮选择界面
		lpSubKey = mpTemplete->OpenKey(L"Highlight");
		mpHighlight = CHighlight::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpHighlight);
		mpHighlight->GetIterator()->SetVisible(false);

		//PDF图片
		lpSubKey = mpTemplete->OpenKey(L"PicturePreview");
		mpPdfPicture = CPdfPicture::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpPdfPicture);
		mpPreNextButton->SetPdfPicture(mpPdfPicture,mpHighlight);
		mpHighlight->SetPdfPicture(mpPdfPicture);

		//横屏工具栏
		lpSubKey = mpTemplete->OpenKey(L"ToolbarH");
		mpToolbarH = CToolbarH::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpToolbarH);
		

		//底部工具栏
		lpSubKey = mpTemplete->OpenKey(L"ToolbarBottom");
		mpToolbarBottom = CToolbarBottom::CreateInstance(mpIterator, lpSubKey);
		CMM_SAFE_RELEASE(lpSubKey);
		BREAK_ON_NULL(mpToolbarBottom);
		mpToolbarBottom->GetIterator()->BringToTop();

		

		OnRotated(ldSysInfo.ulOrient);

		leResult = ERESULT_SUCCESS;


	} while (false);

	CMM_SAFE_RELEASE(lpSubKey);

	// 向系统注册需要收到的消息
	return leResult;
}


//进入系统消息
ERESULT __stdcall CReaderBaseFrame::OnPowerChangeMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		if (wParam == PBT_POWERSETTINGCHANGE)
		{
			POWERBROADCAST_SETTING* lpSetting = (POWERBROADCAST_SETTING*)lParam;
			if (lpSetting->PowerSetting == GUID_CONSOLE_DISPLAY_STATE)
			{
				DWORD ldwStatus = 0;
				memcpy_s(&ldwStatus, sizeof(DWORD), lpSetting->Data, sizeof(DWORD));
				/*if (ldwStatus == 0x1)
				{
					EiSetPartialUpdate(FALSE);
					EinkuiGetSystem()->ClearEinkBuffer();
					EinkuiGetSystem()->UpdateView(true);
				}*/
				
			}
		}
		else if (wParam == PBT_APMRESUMEAUTOMATIC)
		{
			OutputDebugString(L"PBT_APMRESUMEAUTOMATIC a");
			//ExitProcess(0);
		}
		else if (wParam == PBT_APMSUSPEND)
		{
			OutputDebugString(L"PBT_APMSUSPEND ");
		}



	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

ERESULT __stdcall CReaderBaseFrame::OnInputChangeMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & Result)
{
	/*do
	{
		BYTE buffer[0x1000]{};
		HRAWINPUT hRawInput = (HRAWINPUT)lParam;
		PRAWINPUT pri = (PRAWINPUT)buffer;
		UINT uint_size = 0x1000;
		uint_size = GetRawInputData(hRawInput, RID_INPUT, pri, &uint_size, sizeof(RAWINPUTHEADER));
		mpPdfPicture->SvcDebugOutFmt("Press Button: key = %d . Flag = %d  . Type = %d .", pri->data.keyboard.VKey, pri->data.keyboard.Flags, pri->header.dwType);
		//LOGW(L"Key = %d Flag = %d", pri->data.keyboard.VKey, pri->data.keyboard.Flags);
		if (pri->header.dwType == RIM_TYPEKEYBOARD)
		{

			if (pri->data.keyboard.VKey == VK_F19)
			{
				//双击笔帽按钮
				if (mpPdfPicture != nullptr && enableCapacitivepen)
				{
					mpPdfPicture->SvcDebugOutFmt("Double press Button.");
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_PRE);
				}

			}
			else if (pri->data.keyboard.VKey == VK_F20)
			{
				//单击笔帽按钮
				if (mpPdfPicture != nullptr && enableCapacitivepen)
				{
					mpPdfPicture->SvcDebugOutFmt("Signle Press Button.");
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_PRE_NEXT_CLICKED, PNB_BT_NEXT);
				}
			}
		}

	} while (false);*/

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//机器形态改变的通知
ERESULT __stdcall CReaderBaseFrame::OnModeChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & Result)
{
	do
	{
		CExMessage::SendMessage(mpIterator, mpIterator, EEVT_HB_MODE_CHANGE, (ULONG)wParam);

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

ERESULT __stdcall CReaderBaseFrame::OnOpenOfficeFileWithTS(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT & Result)
{
	//通过右键打开office文档
	CExMessage::PostMessage(mpIterator, mpIterator, EEVT_CONVERT_OPEN_OFFICE_FILE, static_cast<int64_t>(wParam));
	return ERESULT_SUCCESS;
}

void CReaderBaseFrame::InternalOpenOfficeFile(uint64_t timeStamp)
{
	wostringstream pathBuffer;

	const int bufferSize = 2048;
	unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(bufferSize);
	GetTempPathW(bufferSize, buffer.get());
	pathBuffer << buffer.get() << kReaderTempPath << L"\\openfile" << timeStamp;
	auto tempFileName = pathBuffer.str();

	FILE* f = nullptr;
	_wfopen_s(&f, tempFileName.c_str(), L"rt,ccs=utf-8");
	if (f == nullptr) return;
	fgetws(buffer.get(), bufferSize, f);
	fclose(f);

	DeleteFileW(tempFileName.c_str());

	bool convertSuccess, canceled;
	wstring resourceName;

	mpToolbarH->HideMoreMenu();

	if (EInkReaderUtil::IsOfficeFileName(buffer.get()))
	{
		if (showAskDlg())
		{
			//开始转换
			std::tie(convertSuccess, resourceName, canceled) = ConvertAndOpenOfficeFile(buffer.get());

			if (!convertSuccess)//未进行转换 or 转换失败
			{
				if (!canceled)//不是取消导致的失败
					ShowFileOpenFailDialog(resourceName.c_str());
				if (mpPdfPicture != nullptr && mpPdfPicture->GetDoc() == nullptr)
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			}
		}

	}
	else
	{
		//看一下要打开的文件是不是在历史记录里
		bool lbRet = false;
		wchar_t lpszFilePath[2048] = { 0 };
		wcscpy_s(lpszFilePath, 2047, buffer.get());
		CharLowerW(lpszFilePath);
		HISTORY_FILE_ATTRIB* lpFileAttrib = NULL;
		for (int i = 0; i < mdHistroyPath.Size(); i++)
		{
			if (_wcsicmp(lpszFilePath, mdHistroyPath.GetEntry(i)->FilePath) == 0)
			{
				lpFileAttrib = mdHistroyPath.GetEntry(i);
				break;
			}
		}
		lbRet = OpenFile(lpszFilePath, lpFileAttrib);
	}
}

//服务通知应用切换到前台或后台
ERESULT __stdcall CReaderBaseFrame::OnActivity(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		CExMessage::SendMessage(mpIterator, mpIterator, EEVT_HB_ACTIVITE, (ULONG)wParam);

	} while (false);

	return ERESULT_SUCCESS;
}

//homebar状态变化通知
ERESULT __stdcall CReaderBaseFrame::OnHomebarChanged(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		CExMessage::SendMessage(mpIterator, mpIterator, EEVT_ER_HOMEBAR_CHANGED, (ULONG)wParam);

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//Tp被reset了，需要重新设置tp area
ERESULT __stdcall CReaderBaseFrame::OnNotifyResetTpArea(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& Result)
{
	do
	{
		SET_TP_AREA ldArea;
		ldArea.Rect.x = 0;
		ldArea.Rect.w = 1920;
		ldArea.Rect.y = 0;
		ldArea.Rect.h = 1080;
		ldArea.Index = 0;
		ldArea.Flag = 0;
		EiSetTpArea(ldArea);

		ldArea.Flag = SET_SP_AREA_TOUCH_PEN;
		EiSetTpArea(ldArea);

	} while (false);

	return ERESULT_WINMSG_SENDTO_NEXT;
}

//按钮单击事件
ERESULT CReaderBaseFrame::OnCtlButtonClick(IEinkuiIterator* npSender)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;


	return lResult;
}


void CReaderBaseFrame::HoldInput(bool nbSet)
{
	if (nbSet != false)
	{
		EinkuiGetSystem()->GetElementManager()->CleanHumanInput(true);
		InterlockedExchange(&mlHoldInput, 1);
		mpIterator->SetTimer(RBF_TIMER_HOLDINPUT, 1, 2000, NULL);	// 如果没有刷新，2秒后也将自动解除
	}
	else
	{
		if (InterlockedExchange(&mlHoldInput, 0) == 1) // 只有当前处于锁定状态，才调用管理器的清理服务
			EinkuiGetSystem()->GetElementManager()->CleanHumanInput(false);
	}
}

//重置隐藏定时器
void CReaderBaseFrame::ResetHideTime(bool lbIsOpen)
{
	//mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
	//if(lbIsOpen != false)
	//	mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR, 1, 5000, NULL);
}

//显示文件打开页面
void CReaderBaseFrame::ShowFileOpenDlg(void)
{
	if (mpFileHistoryDlg != NULL)
	{
		mpFileHistoryDlg->ExitModal();
		mpFileHistoryDlg = NULL;
	}

	if (mpHighlight != NULL)
		mpHighlight->HideSelect();

	ICfKey* lpSubKey = mpTemplete->OpenKey(L"OpenFileHistory");
	mpFileHistoryDlg = CFileHistoryDlg::CreateInstance(mpIterator, lpSubKey);;
	CheckHistoryList();
	mpFileHistoryDlg->SetHistoryList(&mdHistroyPath);
	mpFileHistoryDlg->GetIterator()->SetSize(mpIterator->GetSize());

	//EiSetPartialUpdate(TRUE);
	//如果当前没有打开文件，文件打开对话框的取消按钮就置灰
	bool lbEnable = true;
	if (mszSrcFile[0] == UNICODE_NULL || GetFileAttributes(mszSrcFile) == INVALID_FILE_ATTRIBUTES)
		lbEnable = false;

	mpPreNextButton->SetPenMode(PEN_MODE_NONE);
	wstring selectedFile;
	HISTORY_FILE_ATTRIB* selectedHistoryItem = nullptr;
	std::tie(selectedHistoryItem, selectedFile) = mpFileHistoryDlg->DoModal(lbEnable);
	mpPreNextButton->SetPenMode(mulPenMode);

	if (selectedFile.size() > 0 || selectedHistoryItem != nullptr)
	{
		const wstring& fileName = selectedFile.size() > 0 ? selectedFile : selectedHistoryItem->FilePath;
		// 打开的是现在已经打开的文件，什么都不做了。
		//if (mdHistroyPath.Size() > 0 && _wcsicmp(fileName.c_str(), mdHistroyPath[0]->FilePath) == 0)
		//	return;

		if (selectedHistoryItem != nullptr)
		{
			//从列表里打开office文档，需要转换
			if (EInkReaderUtil::IsOfficeFileName(selectedHistoryItem->FilePath))
			{
				if (showAskDlg())
				{
					bool convertResult, canceled;
					wstring resourceName;
					std::tie(convertResult, resourceName, canceled) = ConvertAndOpenOfficeFile(selectedHistoryItem->FilePath);
					if (!convertResult)
					{
						if (!canceled)
							ShowFileOpenFailDialog(resourceName.c_str());
						if (mpPdfPicture != nullptr && mpPdfPicture->GetDoc() == nullptr)
							CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
					}
				}
			}
			else
			{
				this->OpenFile(selectedHistoryItem->FilePath, selectedHistoryItem);
			}
		}
		else
		{
			const int kBufferSize = 2048;
			unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(kBufferSize);
			wcscpy_s(buffer.get(), 2048, fileName.c_str());
			if (EInkReaderUtil::IsOfficeFileName(fileName))
			{
				if (showAskDlg())
				{
					if (mpFileHistoryDlg != NULL)
					{
						mpFileHistoryDlg->ExitModal();
						CMM_SAFE_RELEASE(lpSubKey);
						mpFileHistoryDlg = NULL;
					}

					bool convertResult, canceled;
					wstring resourceName;
					std::tie(convertResult, resourceName, canceled) = ConvertAndOpenOfficeFile(fileName.c_str());
					if (!convertResult)
					{
						if (!canceled)
							ShowFileOpenFailDialog(resourceName.c_str());
						if (mpPdfPicture != nullptr && mpPdfPicture->GetDoc() == nullptr)
							CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
					}
				}
			}
			else
			{
				this->OpenFile(buffer.get(), nullptr);
			}
		}
	}

	//EiSetPartialUpdate(FALSE);
	CMM_SAFE_RELEASE(lpSubKey);
	mpFileHistoryDlg = NULL;

	/*
	if (mpPdfPicture->GetPageCount() == 0)
	{
		//重新打开文件
		if(mbIsTxt == false || GetFileSize(mszSrcFile)>0)
			Init();
	}
	*/
}

//消息处理函数
ERESULT CReaderBaseFrame::ParseMessage(IEinkuiMessage* npMsg)
{
	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	switch (npMsg->GetMessageID())
	{
	case EMSG_QUERY_SWGT_ROTATED:
	{
		ULONG luOrient = 0;
		luResult = CExMessage::GetInputData(npMsg, luOrient);
		if (luResult != ERESULT_SUCCESS || npMsg->GetOutputBufferSize() < sizeof(ULONG))
			break;

		if (mbIsScreenAuto == false)
		{
			if (lockvertical == true)
			{
				luOrient = MAXULONG32; //屏幕锁定
			}
			else if (luOrient == GIR_90 || luOrient == GIR_270)
			{
				//TBS 允许180度，否则开盖时reader方向不对
				luOrient = MAXULONG32; //屏幕锁定
			}
		}

		*(ULONG*)npMsg->GetOutputBuffer() = luOrient;

		npMsg->SetOutputDataSize(sizeof(ULONG));

		luResult = ERESULT_SUCCESS;
		break;
	}
	break;
	case EMSG_SWGT_ROTATED:
	{
		ULONG luOrient = 0;
		luResult = CExMessage::GetInputData(npMsg, luOrient);
		if (luResult != ERESULT_SUCCESS)
			break;

		OnRotated(luOrient);

		luResult = ERESULT_SUCCESS;
		break;
	}
	case EEVT_ER_OPEN_FILE:
	{
		//选择文件
		ShowFileOpenDlg();
	
		break;
	}
	case EEVT_ENTER_THUMBNAIL_DLG:
	{
		//进入缩略图界面
		if(mpHighlight != NULL)
			mpHighlight->HideSelect();

		if (mpPdfPicture->GetDocType() == DOC_TYPE_PDF)
		{
			mpPdfPicture->RefreshThumbnail();
			mpPdfPicture->LoadThumbnails(mszSrcFile, false); //PDF格式需要再刷新一下，可能有变化
			Sleep(500);
		}
		

		if (mpThumbnailDlg != NULL)
		{
			mpThumbnailDlg->ExitModal();
			mpThumbnailDlg = NULL;
		}

		ICfKey* lpSubKey = mpTemplete->OpenKey(L"Thumbnail");
		mpThumbnailDlg = CThumbnailDlg::CreateInstance(mpIterator, lpSubKey);
		mpThumbnailDlg->SetPdfPicture(mpPdfPicture);

		mpThumbnailDlg->GetIterator()->SetSize(mpIterator->GetSize());
		mpThumbnailDlg->SetDoctype(mpPdfPicture->GetDocType());

		mpPreNextButton->SetPenMode(PEN_MODE_NONE);
		mpThumbnailDlg->DoModal();
		mpPreNextButton->SetPenMode(mulPenMode);

		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpThumbnailDlg = NULL;

		break;
	}
	case EEVT_ER_OPEN_PAGE_JUMP:
	{
		//页码跳转
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"JumpPage");
		mpJumpPage = CJumpPage::CreateInstance(mpIterator, lpSubKey);
		ULONG secondPage;
		mpJumpPage->SetCurrentPage(mpPdfPicture->GetCurrentPageNumber(secondPage), mpPdfPicture->GetPageCount());
		SetJumpPagePos();

		//EiSetPartialUpdate(TRUE);
		mpPreNextButton->SetPenMode(PEN_MODE_NONE);
		mpJumpPage->DoModal();
		mpPreNextButton->SetPenMode(mulPenMode);
		
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpJumpPage = NULL;
		break;
	}
	case EEVT_ER_SHOW_TIP:
	{
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"TipFrame");
		mpTipFrame = CTipFrame::CreateInstance(mpIterator, lpSubKey);;
		mpTipFrame->GetIterator()->SetSize(mpIterator->GetSize());
		//EiSetPartialUpdate(TRUE);
		
		mpPreNextButton->SetPenMode(PEN_MODE_NONE);
		mpTipFrame->DoModal();
		mpPreNextButton->SetPenMode(mulPenMode);
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpTipFrame = NULL;

		//mpTipFrame->GetIterator()->SetVisible(false);
		break;
	}
	case EEVT_HB_ACTIVITE:
	{
		//应用切换到前台或后台
		ULONG lulActivity = 0;
		luResult = CExMessage::GetInputData(npMsg, lulActivity);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lulActivity == 1)
		{
			//切换到前台
			//mpIterator->KillTimer(RBF_TIMER_EXIT);
			if (mpFileHistoryDlg == NULL && mpThumbnailDlg == NULL && mpSnapShot == NULL)
			{
				ShowToolBar(true); //只有在正常界面才显示
			}
			if (mpFileHistoryDlg != NULL || mpThumbnailDlg != NULL)
			{
				//在选择文档页需要隐藏电池
				EiSetBatteryStatus(GI_BATTERY_HIDE);
			}
			else
			{
				EiSetHomebarStatus(GI_HOMEBAR_SHOW);
				EiSetBatteryStatus(GI_BATTERY_SHOW);
			}

			EiSetWaveformMode(GI_WAVEFORM_DU2);
			EiCleanupScreen(0xff);
			Sleep(585); //DU260+15*5   分16帧刷，每帧间隔5ms
			EiSetWaveformMode(GI_WAVEFORM_GC16);
			EiSetPartialUpdate(FALSE);

			mpIterator->SetTimer(RBF_TIMER_ENABL_PARTIAL, 1, 1800, NULL);
			//mpIterator->SetTimer(RBF_TIMER_TSHOW_TOOLBAR, 1, 5000, NULL);

			//mpToolbarH->SetBCoverState();//恢复B面显示状态

			//如果刚才打开的文件被删除了，就进入打开文件对话框
			if (mszSrcFile[0] != UNICODE_NULL && GetFileAttributes(mszSrcFile) == INVALID_FILE_ATTRIBUTES)
			{
				mpPdfPicture->CloseFile();
				if(mpFileHistoryDlg == NULL) //只有在没有弹出的情况下才打开
					CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			}
			else if (mszSrcFile[0] != UNICODE_NULL)
			{
				//mhFile = CreateFile(mszSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
			}

			enableCapacitivepen = true;
			EinkuiGetSystem()->ClearEinkBuffer();
			EinkuiGetSystem()->UpdateView(true);
		}
		else
		{
			//切换到后台
			//保存数据
			//ExitProcess(0); //????这个代码是测试代码，正式发布时需要删除

			/*if (mhFile != INVALID_HANDLE_VALUE)
				CloseHandle(mhFile);*/

			mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);
			//mpToolbarH->GetIterator()->SetVisible(true);
			//这样就可以在切换过来第一帧时关闭partial刷新一次
			enableCapacitivepen = false;
			
			//mpIterator->SetTimer(RBF_TIMER_EXIT, 1, 400, NULL);
			//ExitProcess(0);
		}

		break;
	}
	case EEVT_HB_MODE_CHANGE:
	{
		//机器形态改变
		// GIR_MODE_LAPTOP  2
        // GIR_MODE_TENT 3
        // GIR_MODE_TABLET 4
		ULONG lumodestatus = 0;
		luResult = CExMessage::GetInputData(npMsg, lumodestatus);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lumodestatus == 2)
		{
			CHellFun::SetRegData(RbF_REG_B_COVER, 1);
		}
		else
		{
			CHellFun::SetRegData(RbF_REG_B_COVER, 0);
		}
		//mpToolbarH->UpdateBCoverState();
		break;
	}
	case EEVT_ER_HOMEBAR_CHANGED:
	{
		//homebar状态发生变化
		ULONG lulHomebarMode = 0;
		luResult = CExMessage::GetInputData(npMsg, lulHomebarMode);
		if (luResult != ERESULT_SUCCESS)
			break;

		//EiSetPartialUpdate(TRUE);
		if (lulHomebarMode == GI_HOMEBAR_EXPAND)
		{
			//homebar展开就隐藏自己的工具栏
			mpToolbarH->GetIterator()->SetVisible(false);
			mpToolbarBottom->ShowItem(false);
		}
		else if(lulHomebarMode == GI_HOMEBAR_COLLAPSE)
		{
			//homebar收缩就显示自己的工具栏
			if (mpFileHistoryDlg == NULL)
			{
				//在文件打开窗口不显示
				mpToolbarH->GetIterator()->SetVisible(true);
				mpToolbarBottom->ShowItem(true);
			}
			
			ResetHideTime();
		}
		else if (lulHomebarMode == GI_HOMEBAR_HIDE)
		{
			//隐藏了
		}
		//EiSetPartialUpdate(FALSE);

		break;
	}
	case EEVT_ER_PAGE_JUMP:
	{
		//页码跳转
		if (mszSrcFile[0] == UNICODE_NULL)
			break;
		int liPage = 1;
		luResult = CExMessage::GetInputData(npMsg, liPage);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (mZoomStatus == ZoomStatus::AUTO_ZOOM)
		{
			mpPdfPicture->GoToPage(liPage, false);
			PageAutoZoom();
		}
		else
		{
			mpPdfPicture->GoToPage(liPage, true);
		}
		ShowPageInfo();

		RECT ldRect;
		mpPdfPicture->CalcMovalbe(ldRect);
		mpToolbarBottom->ShowMoveButton(ldRect);
		//GotoPage(liPage);

		break;
	}
	case EEVT_ER_OPEN_FILE_PATH:
	{
		//要打开的文件路径
		wchar_t* lpszFilePath = (wchar_t*)npMsg->GetInputData();

		bool lbRet = false;
		if (lpszFilePath != NULL)
		{
			//看一下要打开的文件是不是在历史记录里
			HISTORY_FILE_ATTRIB* lpFileAttrib = NULL;
			for (int i = 0; i < mdHistroyPath.Size(); i++)
			{
				if (_wcsicmp(lpszFilePath, mdHistroyPath.GetEntry(i)->FilePath) == 0)
				{
					lpFileAttrib = mdHistroyPath.GetEntry(i);
					break;
				}
			}
			lbRet = OpenFile(lpszFilePath, lpFileAttrib);
		}
		

		// 设置输出数据
		bool* lpOut = (bool*)npMsg->GetOutputBuffer();
		*lpOut = lbRet;
		npMsg->SetOutputDataSize(sizeof(bool));

		break;
	}
	case EEVT_ER_OPEN_HISTORY_FILE_PATH:
	{
		//要打开历史记录中的文件
		HISTORY_FILE_ATTRIB* lpFileAttrib = NULL;
		luResult = CExMessage::GetInputData(npMsg, lpFileAttrib);
		if (luResult != ERESULT_SUCCESS)
			break;

		bool lbRet = OpenFile(lpFileAttrib->FilePath, lpFileAttrib);

		// 设置输出数据
		bool* lpOut = (bool*)npMsg->GetOutputBuffer();
		*lpOut = lbRet;
		npMsg->SetOutputDataSize(sizeof(bool));

		break;
	}

	case EEVT_ER_PRE_NEXT_CLICKED:
	{
		//页码跳转
		ULONG lulFlag = 1;
		luResult = CExMessage::GetInputData(npMsg, lulFlag);
		if (luResult != ERESULT_SUCCESS)
			break;
		do 
		{
			if (mszSrcFile[0] == UNICODE_NULL)
				break;
			if (lulFlag == PNB_BT_PRE)
			{
				if (mZoomStatus == ZoomStatus::AUTO_ZOOM)
				{
					mpPdfPicture->PageFoward(false, false);
					PageAutoZoom();
				}
				else
				{
					mpPdfPicture->PageFoward(false, true);
				}
				ShowPageInfo();
				mpHighlight->HideSelect();
			}
			else if (lulFlag == PNB_BT_NEXT)
			{
				if (mZoomStatus == ZoomStatus::AUTO_ZOOM)
				{
					mpPdfPicture->PageFoward(true, false);
					PageAutoZoom();
				}
				else
				{
					mpPdfPicture->PageFoward(true, true);
				}
				ShowPageInfo();
				mpHighlight->HideSelect();
			}
			else if (lulFlag == PNB_BT_MIDDLE)
			{
				//显示或隐藏工作栏
				ShowToolBar(!mpToolbarH->GetIterator()->IsVisible());
			}
			else if (lulFlag == PNB_BT_MIDDLE_ZOOM)
			{
				//隐藏翻页滚动条
				mpToolbarBottom->HidePageProcess(false);
			}
			else
				break;

			if (lulFlag == PNB_BT_PRE || lulFlag == PNB_BT_NEXT)
			{
				RECT ldRect;
				mpPdfPicture->CalcMovalbe(ldRect);
				mpToolbarBottom->ShowMoveButton(ldRect);
			}

			HoldInput(true);

		} while (false);

		//???niu 增加对PageFoward()失败的判断和处理

		break;
	}
	case EEVT_ER_TWO_SCREEN:
	{
		//切换双屏或单屏
		bool lbIsDoubleScreen = mpToolbarH->GetDuopageStatus();

		mpPdfPicture->EnableDuopageView(lbIsDoubleScreen);
		ShowPageInfo();

		CHellFun::SetRegData(RbF_REG_DOUBLE_SCREEN, lbIsDoubleScreen==false?0:1);

		ResetHideTime();

		HoldInput(true);

		RECT ldRect;
		mpPdfPicture->CalcMovalbe(ldRect);
		mpToolbarBottom->ShowMoveButton(ldRect);
		mpToolbarBottom->EnableAutoZoomButton(!lbIsDoubleScreen);

		mpHighlight->HideSelect();

		break;
	}
	case EEVT_CLOSE_B_SCREEN:
	{
		//bool BScreenStatus = mpToolbarH->GetBCoverState();


		//CHellFun::SetRegData(RbF_REG_B_COVER, BScreenStatus == false ? 0 : 1);

		//BScreenStatus = !BScreenStatus;

		//EiCloseBCover(BScreenStatus);

		//break;
	}
	case EEVT_ER_ENTER_ZOOM:
	{
		//进入缩放模式
		bool lbIsZoom = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsZoom);
		if (luResult != ERESULT_SUCCESS)
			break;

		mZoomStatus = ZoomStatus::ZOOM;
		mdHistroyPath.GetEntry(0)->autoZoom = 0; 
		wchar_t lszText[MAX_PATH];
		swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_AUTO_ZOOM, 0);
		CHellFun::SetRegData(lszText, 0);
		mpPreNextButton->SetZoomStatus(lbIsZoom ? ZoomStatus::ZOOM : ZoomStatus::NONE);
		if (lbIsZoom && !mbIsTxt)
			mpToolbarBottom->SetFatRatio(mpPdfPicture->GetFatRatio());

		if (!lbIsZoom)
		{
			mdHistroyPath.GetEntry(0)->scalingLevel = 0;
			mpToolbarBottom->ShowMoveButton(RECT{ 0,0,0,0 });
			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGLEVEL, 0);
			CHellFun::SetRegData(lszText, 0);
		}
		
		break;
	}
	case EEVT_ER_SET_PAGE_MOVE:
	{
		//页面移动
		POINT ldPos;
		luResult = CExMessage::GetInputData(npMsg, ldPos);
		if (luResult != ERESULT_SUCCESS)
			break;

		RECT ldRect;
		mpPdfPicture->MovePage(ldPos.x, ldPos.y, ldRect);
		if (mZoomStatus == ZoomStatus::AUTO_ZOOM)
		{
			if (mMoveForward == MoveForward::HORIZONTAL)
			{
				ldRect.top = 0;
				ldRect.bottom = 0;
			}
			else if (mMoveForward == MoveForward::VERTICAL)
			{
				ldRect.left = 0;
				ldRect.right = 0;
			}
		}
		// 设置输出数据
		RECT* lpOut = (RECT*)npMsg->GetOutputBuffer();

// [zhuhl5@20200116:ZoomRecovery, Position recovery, not done.]
// Make judgement if outer caller doesn't want result rect.
		if (lpOut)
		{
			*lpOut = ldRect;
		}
		npMsg->SetOutputDataSize(sizeof(RECT));

		//获取真实大小
		D2D1_SIZE_F ldMaxSize;
		D2D1_RECT_F ldViewRect;
		mpPdfPicture->GetRectOfViewportOnPage(ldMaxSize, ldViewRect);
		mpToolbarBottom->ShowMoveButton(ldRect);

		mpHighlight->Relocation();

// [zhuhl5@20200116:ZoomRecovery]
// update position to history, but this is just offset, should be absolute position.
		mdHistroyPath.GetEntry(0)->scalingPosX = (float)ldPos.x;
		mdHistroyPath.GetEntry(0)->scalingPosY = (float)ldPos.y;
		break;
	}
	case EEVT_ER_SET_ZOOM:
	{
		//设置放大比例
		double lfRotio = 1.0;
		luResult = CExMessage::GetInputData(npMsg, lfRotio);
		if (luResult != ERESULT_SUCCESS)
			break;

		mZoomStatus = ZoomStatus::ZOOM;
		mdHistroyPath.GetEntry(0)->autoZoom = 0;
		mpPreNextButton->SetZoomStatus(ZoomStatus::ZOOM);

		mpToolbarBottom->SetMoveForward(MoveForward::HORIZONTAL_VERTICAL);
		mpPreNextButton->SetMoveForward(MoveForward::HORIZONTAL_VERTICAL);

		RECT ldRect;
		mpPdfPicture->SetScaleRatio(lfRotio, ldRect);
		CHellFun::SetRegData(RbF_REG_RATIO, DWORD(lfRotio*100));

		mpToolbarBottom->SetRatioString(lfRotio);
		mpToolbarBottom->ShowMoveButton(ldRect);
		
		// 设置输出数据
		RECT* lpOut = (RECT*)npMsg->GetOutputBuffer();
		*lpOut = ldRect;
		npMsg->SetOutputDataSize(sizeof(RECT));

		//获取真实大小
		D2D1_SIZE_F ldMaxSize;
		D2D1_RECT_F ldViewRect;
		mpPdfPicture->GetRectOfViewportOnPage(ldMaxSize, ldViewRect);

		SetHandWriteWidth();

		mpHighlight->Relocation();

// [zhuhl5@20200116:ZoomRecovery]
// update scaling ratio to history
		mdHistroyPath.GetEntry(0)->scaling = lfRotio;

		wchar_t lszText[MAX_PATH];
		swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_AUTO_ZOOM, 0);
		CHellFun::SetRegData(lszText, 0);

		break;
	}
// [zhuhl5@20200116:ZoomRecovery]
// Notify current zoom level, outer should record this, and use this to recover zoom status, not specific zoom ratio.
	case EEVT_ER_AUTO_ZOOM:
	{
		if (mszSrcFile[0] == UNICODE_NULL)
			break;
		mZoomStatus = ZoomStatus::AUTO_ZOOM;
		mdHistroyPath.GetEntry(0)->autoZoom = 1;
		mpPreNextButton->SetZoomStatus(ZoomStatus::AUTO_ZOOM);
		PageAutoZoom();
		wchar_t lszText[MAX_PATH];
		swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_AUTO_ZOOM, 0);
		CHellFun::SetRegData(lszText, 1);
		break;
	}
	case EEVT_ER_SET_ZOOMLEVEL:
	{
		int scalingLevel = 0;
		luResult = CExMessage::GetInputData(npMsg, scalingLevel);
		if (luResult != ERESULT_SUCCESS)
			break;
		mdHistroyPath.GetEntry(0)->scalingLevel = scalingLevel;
		wchar_t lszText[MAX_PATH];
		swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGLEVEL, 0);
		CHellFun::SetRegData(lszText, scalingLevel);
		break;
	}
	case EEVT_ER_ENTER_SNAPSHOT:
	{
		//截屏
		ICfKey* lpSubKey = mpTemplete->OpenKey(L"SnapShot");
		mpSnapShot = CSnapShot::CreateInstance(mpIterator, lpSubKey);
		mpSnapShot->SetToolBarHeight(mpToolbarH->GetIterator()->GetSizeY(), mpToolbarBottom->GetIterator()->GetSizeY());
		mpSnapShot->GetIterator()->SetSize(mpIterator->GetSize());
		//EiSetPartialUpdate(TRUE);

		EiSetBatteryStatus(GI_BATTERY_HIDE);
		ShowToolBar(false);
		EinkuiGetSystem()->ClearEinkBuffer();
		mpPreNextButton->SetPenMode(PEN_MODE_NONE);
		mpSnapShot->DoModal();
		mpPreNextButton->SetPenMode(mulPenMode);
		ShowToolBar(true);
		EiSetBatteryStatus(GI_BATTERY_SHOW);
		EinkuiGetSystem()->ClearEinkBuffer();
		//EiSetPartialUpdate(FALSE);
		CMM_SAFE_RELEASE(lpSubKey);
		mpSnapShot = NULL;

		break;
	}
	case EEVT_ER_SNAPSHOT_TO_CLIPBRD:
	{
		D2D1_RECT_F ldRect;
		luResult = CExMessage::GetInputData(npMsg, ldRect);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->CopyToClipboard(ldRect);
		ShowToast(L"SnapShot");

		break;
	}
	case EEVT_ER_SET_TXT_ZOOM:
	{
		//设置txt放大
		luResult = CExMessage::GetInputData(npMsg, mdwFontsizeIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->SetFontSize(mdwFontSizeArray[mdwFontsizeIndex]);
		ShowPageInfo();

		//把设置保存到注册表中
		CHellFun::SetRegData(RbF_REG_TXT_FONT_SIZE_INDEX, mdwFontsizeIndex);

		break;
	}
	case EEVT_TXT_ARRANGED_START:
	{
		//txt开始页面加载
		mbIsLoadingSuccess = false;
		
		break;
	}
	case EEVT_TXT_ARRANGED_DOING:
	{
		//txt页面正在加载中
		ShowPageInfo();
		mbIsSetPartial = false;

		int32 liPageIndex = 0;
		luResult = CExMessage::GetInputData(npMsg, liPageIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPdfPicture->SetLoadingPageIndex(liPageIndex);

		break;
	}
	case EEVT_ARRANGED_COMPLETE:
	{
		//页面加载完毕
		int32 liPagecount = 0;
		luResult = CExMessage::GetInputData(npMsg, liPagecount);
		if (luResult != ERESULT_SUCCESS)
			break;

		mbIsSetPartial = true;
		mbIsLoadingSuccess = true;
		mpToolbarH->DocmentLoadComplete();
		if (liPagecount > 0)
			ShowPageInfo();

		break;
	}
	case EEVT_THUMBNAIL_COMPLETE:
	{
		//缩略图加载完毕
		mpToolbarH->ThumbnailsLoadComplete();
		if(mpThumbnailDlg != NULL)
			mpThumbnailDlg->SetPdfPicture(mpPdfPicture); //重新刷新一下

		break;
	}
	case EEVT_RESET_HIDE_TIME:
	{
		//重置隐藏任务栏定时器
		bool lbIsClose = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsClose);
		if (luResult != ERESULT_SUCCESS)
			break;

		ResetHideTime(lbIsClose);

		break;
	}
	case EEVT_SHOW_TOOLBAR:
	{
		//显示或隐藏工作栏
		bool lbIsShow = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsShow);
		if (luResult != ERESULT_SUCCESS)
			break;
		ShowToolBar(lbIsShow);

		break;
	}
	case EEVT_DELETE_READ_HISTORY:
	{
		//清空阅读历史
		while (mdHistroyPath.Size() > 0)
		{
			delete mdHistroyPath.GetEntry(0);
			mdHistroyPath.RemoveByIndex(0);
		}

		if (mpFileHistoryDlg != NULL)
			mpFileHistoryDlg->SetHistoryList(&mdHistroyPath);

		//清空记录
		wchar_t lszText[MAX_PATH];
		DWORD ldwLen = 0;
		for (int i = 0; i < RBF_HISTROY_MAX; i++)
		{
			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_PATH, i);
			CHellFun::SetRegData(lszText, 0, L"");
		}

		ClearThumbnail(NULL);
		//mpToolbarH->SetDoctype(mpPdfPicture->GetDocType());

		break;
	}
	case EEVT_SET_PEN_WIDTH:
	{
		//设置笔宽
		luResult = CExMessage::GetInputData(npMsg, mdwPenWidthIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		SetHandWriteWidth();

		mpPdfPicture->SetPenWidth(mfArrayPenWidthArray[mdwPenWidthIndex]);

		CHellFun::SetRegData(RbF_REG_PEN_WIDTH_INDEX, mdwPenWidthIndex);
		
		break;
	}
	case EEVT_SET_PEN_COLOR:
	{
		//设置笔颜色
		luResult = CExMessage::GetInputData(npMsg, mdwPenColorIndex);
		if (luResult != ERESULT_SUCCESS)
			break;

		
		mpPdfPicture->SetPenColor(mdwPenColorIndex);
		CHellFun::SetRegData(RbF_REG_PEN_COLOR_INDEX, mdwPenColorIndex);

		break;
	}
	case EEVT_PEN_MODE:
	{
		//设置笔状态
		if (npMsg->GetOutputBufferSize() >= sizeof(ULONG))
		{
			//如果发送者想要得到前一个状态
			ULONG* lpOut = (ULONG*)npMsg->GetOutputBuffer();
			*lpOut = mulPenMode;
			npMsg->SetOutputDataSize(sizeof(ULONG));
		}

		luResult = CExMessage::GetInputData(npMsg, mulPenMode);
		if (luResult != ERESULT_SUCCESS)
			break;
		mpPreNextButton->SetPenMode(mulPenMode);
		mpPdfPicture->SetPenmode(mulPenMode);

		if (mulPenMode == PEN_MODE_SELECT)
		{
			EinkuiGetSystem()->EnablePaintboard(false);

			//mpPreNextButton->GetIterator()->SetVisible(false);
			mpHighlight->GetIterator()->SetVisible(true);
			mpHighlight->GetIterator()->BringToTop();
		}
		else if(mulPenMode == PEN_MODE_PEN || mulPenMode == PEN_MODE_ERASER)
		{
			//mpPreNextButton->GetIterator()->SetVisible(true);
			mpHighlight->GetIterator()->SetVisible(false);

		}

		mpToolbarH->SetPenStatusSelect(mulPenMode);

		break;
	}
	case EEVT_HAND_WRITE_MUTE:
	{
		//是否开启手画线,静默方式
		bool lbIsHand = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsHand);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPreNextButton->SetHandWrite(lbIsHand);
		SetHandWriteWidth();

		ResetHideTime();
		break;
	}
	case EEVT_HAND_WRITE:
	{
		//是否开启手画线
		bool lbIsHand = false;
		luResult = CExMessage::GetInputData(npMsg, lbIsHand);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPreNextButton->SetHandWrite(lbIsHand);
		SetHandWriteWidth();

		if (lbIsHand != false)
		{
			ShowToast(L"HandOpen");
		}
		else
		{
			ShowToast(L"HandClose");
		}

		
		ResetHideTime();

		break;
	}
	case EEVT_UNDO:
	{
		//undo操作
		int liStackSize = mpPdfPicture->RedoUndoProc(false);
		mpHighlight->HideSelect();
		// 设置输出数据
		/*int* lpOut = (int*)npMsg->GetOutputBuffer();
		*lpOut = liStackSize;
		npMsg->SetOutputDataSize(sizeof(int));*/

		break;
	}
	case EEVT_REDO:
	{
		//redo操作
		int liStackSize = mpPdfPicture->RedoUndoProc(true);
		mpHighlight->HideSelect();
		// 设置输出数据
		/*int* lpOut = (int*)npMsg->GetOutputBuffer();
		*lpOut = liStackSize;
		npMsg->SetOutputDataSize(sizeof(int));*/

		break;
	}
	case EEVT_UPDATE_PAGE_STATUS:
	{
		// 通知工具栏更新页面状态
		PAGE_STATUS ldStatus;
		luResult = CExMessage::GetInputData(npMsg, ldStatus);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpToolbarH->UpdatePageStatus(ldStatus);

		break;
	}
	case EEVT_PAGE_CHANGED:
	{
		//页码发生变化
		ShowPageInfo();
		break;
	}
	case EEVT_MENU_ERASER_ALL:
	{
		//清除本页所有标注
		mpPdfPicture->ClearCurrentPageAllInk();

		ShowToast(L"ClearAllInk");

		break;
	}
	case EEVT_CLEAR_FULL_SCREEN:
	{
		//清全屏
		EiSetPartialUpdate(FALSE);
		muLastGcTick = 0;
		mxLastGcUiNumber = 49;

		EinkuiGetSystem()->ClearEinkBuffer();
		EinkuiGetSystem()->UpdateView(true);

		EiSetPartialUpdate(TRUE);
		

		break;
	}
	case EEVT_FILE_MODIFY:
	{
		//文件被修改
		if (mdHistroyPath.Size() > 0)
		{
			CHellFun::SetRegData(L"historyModify0", 1);

			mdHistroyPath.GetEntry(0)->IsModify = 1;
		}

		break;
	}
	case EEVT_SET_FW_LINE_RECT:
	{
		//画线区域
		ED_RECT ldRect;
		luResult = CExMessage::GetInputData(npMsg, ldRect);
		if (luResult != ERESULT_SUCCESS)
			break;

		mpPreNextButton->SetFwLineRect(ldRect);
		break;
	}
	case EEVT_SELECT_HIGHLIGHT:
	{
		//选中指定区域的文字
		if(mpPdfPicture == NULL)
			break;

		D2D1_RECT_F ldRect;
		luResult = CExMessage::GetInputData(npMsg, ldRect);
		if (luResult != ERESULT_SUCCESS)
			break;

		SELECT_HIGHLIGHT ldHighltght;
		ZeroMemory(&ldHighltght, sizeof(ldHighltght));
		int liCount = mpPdfPicture->SelectHighlight(ldRect, ldHighltght);
		// 设置输出数据
		SELECT_HIGHLIGHT* lpOut = (SELECT_HIGHLIGHT*)npMsg->GetOutputBuffer();
		*lpOut = ldHighltght;
		npMsg->SetOutputDataSize(sizeof(int));

		break;
	}
	case EEVT_GET_SELECT_HIGHLIGHT_INFO:
	{
		// 获取选中区域信息
		SELECT_HIGHLIGHT ldHighltght;
		ZeroMemory(&ldHighltght, sizeof(SELECT_HIGHLIGHT));
		ED_RECTF ldRect;
		mpPdfPicture->GetSelectHighlightInfo(ldHighltght, ldRect);
		// 设置输出数据
		SELECT_HIGHLIGHT* lpOut = (SELECT_HIGHLIGHT*)npMsg->GetOutputBuffer();
		*lpOut = ldHighltght;
		npMsg->SetOutputDataSize(sizeof(int));

		break;
	}
	case EEVT_HIGHLIGHT_BT_EVENT:
	{
		//高亮工具条消息
		int liEvent = 0;
		luResult = CExMessage::GetInputData(npMsg, liEvent);
		if (luResult != ERESULT_SUCCESS)
			break;
		mpPdfPicture->HighlightEvent(liEvent);
		if (liEvent == HIGHLIGHT_BT_COPY)
			ShowToast(L"CopyText");

		break;
	}
	case EEVT_SET_SCREEN_STATUS:
	{
		//屏幕方向
		ULONG lulScreen = 0;
		luResult = CExMessage::GetInputData(npMsg, lulScreen);
		if (luResult != ERESULT_SUCCESS)
			break;

		DWORD ldwOrient = GIR_NONE;
		mbIsScreenAuto = false;
		lockvertical = false;
		DWORD ldwRegOri = (DWORD)lulScreen;
		if (lulScreen == SCREEN_STATUS_AUTO)
		{
			EI_SYSTEM_INFO ldInfo;
			EiGetSystemInfo(&ldInfo);
			ldwOrient = ldInfo.ulOrient;
			mbIsScreenAuto = true;
		}
		else if (lulScreen == SCREEN_STATUS_H)
		{
			EI_SYSTEM_INFO ldInfo;
			EiGetSystemInfo(&ldInfo);
			ldwOrient = ldInfo.ulOrient;
			if (ldwOrient == GIR_90 || ldwOrient == GIR_270)
			{
				//TBS 允许180度，否则开盖时reader方向不对
				ldwOrient = GIR_NONE;
			}
		}
		else if (lulScreen == SCREEN_STATUS_V)
		{
			ldwOrient = GIR_90;
			lockvertical = true;
		}

		EiSetScreenOrient(ldwOrient);
		EinkuiGetSystem()->ResetPaintboard();
		OnRotated(ldwOrient);

		CHellFun::SetRegData(RbF_REG_SCREEN_ORI, ldwRegOri);

		break;
	}
	case EEVT_PARTIAL_ENABLE:
	{
		//打开或关闭partail
		bool lbEnable = false;
		luResult = CExMessage::GetInputData(npMsg, lbEnable);
		if (luResult != ERESULT_SUCCESS)
			break;

		if (lbEnable == false)
		{
			EiSetPartialUpdate(FALSE);
			OutputDebugString(L"enter gc false partial");

			muLastGcTick = 0;
			mxLastGcUiNumber = 49;
		}
		else
		{
			EiSetPartialUpdate(TRUE);
		}

		break;
	}
	case EEVT_UPDATE_THUMBNAIL_PATH:
	{
		//更新缩略图目录
		wchar_t* lpszPath = (wchar_t*)npMsg->GetInputData();
		if (lpszPath != NULL && lpszPath[0] != UNICODE_NULL && mdHistroyPath.Size() > 0)
		{
			wcscpy_s(mdHistroyPath.GetEntry(0)->ThumbanilFilePath, MAX_PATH, lpszPath);
			CHellFun::SetRegData(L"historyThumbanilPath0", 0, mdHistroyPath.GetEntry(0)->ThumbanilFilePath);
		}

		break;
	}
	case EEVT_CONVERT_OPEN_OFFICE_FILE:
	{
		int64_t timeStamp = *reinterpret_cast<const int64_t*>(npMsg->GetInputData());
		InternalOpenOfficeFile(timeStamp);
		break;
	}
	case EEVT_HIDE_HIGHLIGHT:
	{
		if (mpHighlight != NULL)
			mpHighlight->HideSelect();
		break;
	}
	default:
		luResult = ERESULT_NOT_SET;
		break;
	}

	if (luResult == ERESULT_NOT_SET)
	{
		luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
	}

	return luResult;
}

//设置笔宽
void CReaderBaseFrame::SetHandWriteWidth()
{
	do
	{
		BREAK_ON_NULL(mpPdfPicture);

		BYTE ld = BYTE(mfArrayFwFingerWidthArray[mdwPenWidthIndex] * mpPdfPicture->GetRealRatio());
		if (mpPreNextButton->GetHandWrite() == false)
			EiSetHandwritingSetting(BYTE(mfArrayFwPenWidthArray[mdwPenWidthIndex] * mpPdfPicture->GetRealRatio()));
		else
			EiSetHandwritingSetting(BYTE(mfArrayFwFingerWidthArray[mdwPenWidthIndex] * mpPdfPicture->GetRealRatio()));

	} while (false);

}

//绘制
ERESULT CReaderBaseFrame::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	HoldInput(false);

	return CXuiElement::OnPaint(npPaintBoard);
}

////切页
//void CReaderBaseFrame::GotoPage(ULONG nulPage)
//{
//	mpPdfPicture->GoToPage(nulPage);
//
//	wchar_t lszString[MAX_PATH] = { 0 };
//	swprintf_s(lszString, MAX_PATH, L"(第%d页/共%d页)", nulPage, mpPdfPicture->GetPageCount());
//	mpToolbarH->SetPage(lszString);
//
//	mulCurrentPageNumber = nulPage;
//}

//检查历史文件状态，如果不存在的文件就清掉
void CReaderBaseFrame::CheckHistoryList(void)
{
	int liIndex = 0;
	while (liIndex < mdHistroyPath.Size())
	{
		HISTORY_FILE_ATTRIB * lpTemp = mdHistroyPath.GetEntry(liIndex);
		if (GetFileAttributes(lpTemp->FilePath) == INVALID_FILE_ATTRIBUTES)
		{
			//文件不存在了
			delete lpTemp;
			mdHistroyPath.RemoveByIndex(liIndex);
		}
		else
		{
			liIndex++;
		}
	}
}

//显示或隐藏工具栏
void CReaderBaseFrame::ShowToolBar(bool nbIsShow)
{
	if (nbIsShow == false)
	{
		//隐藏
		mpToolbarH->GetIterator()->SetVisible(false);
		mpToolbarBottom->ShowItem(false);
		mpIterator->KillTimer(RBF_TIMER_TSHOW_TOOLBAR);

		mpPreNextButton->SetToolbarHeight(0, 0);
		mpHighlight->SetToolbarHeight(0, 0);
	}
	else
	{
		//显示
		mpToolbarH->GetIterator()->SetVisible(true);
		mpToolbarBottom->ShowItem(true);
		
		ResetHideTime();

		//EiSetHomebarStatus(GI_HOMEBAR_COLLAPSE);

		mpPreNextButton->SetToolbarHeight((int)mpToolbarH->GetIterator()->GetSizeY(), (int)mpToolbarBottom->GetIterator()->GetSizeY());
		mpHighlight->SetToolbarHeight((int)mpToolbarH->GetIterator()->GetSizeY(), (int)mpToolbarBottom->GetIterator()->GetSizeY());
	}
}

void CReaderBaseFrame::ShowPageInfo(void)
{
	wchar_t lszString[MAX_PATH] = { 0 };
	ULONG pageNo1=0, pageNo2=0;

	do
	{
		BREAK_ON_NULL(mpPdfPicture);
		if (mpPdfPicture->GetDoc() == NULL)
			break;

		pageNo1 = mpPdfPicture->GetCurrentPageNumber(pageNo2);
		if (pageNo1 == 0)
			pageNo1 = 1;

		//页码数据
		mpToolbarBottom->SetPage(pageNo2 <= 0 ? pageNo1 : pageNo2, mpPdfPicture->GetPageCount() == 0 ? 1 : mpPdfPicture->GetPageCount());


		//当前页面标注数量
		mpToolbarH->SetCurrentPageInkCount(mpPdfPicture->GetCurrentPageInkCount());

		if (mulPageIndex != pageNo1 || mulPageCount != mpPdfPicture->GetPageCount())
		{
			//更新记录数据
			mulPageIndex = pageNo1;
			mulPageCount = mpPdfPicture->GetPageCount();
			if (mulPageCount == 0)
				mulPageCount = 1;

			PAGE_PDF_CONTEXT ldContent;
			ZeroMemory(&ldContent, sizeof(ldContent));
			mpPdfPicture->GetCrtPageContext(&ldContent);

			//阅读进度
			if (mdHistroyPath.Size() > 0 && mbIsLoadingSuccess != false)
			{
				DWORD ldwProgress = DWORD(floor((float(pageNo1) / mulPageCount) * 100));
				if (ldwProgress <= 0)
					ldwProgress = 1; //进度最小1
				if (ldwProgress > 100)
					ldwProgress = 100; //进度最大100

				mdHistroyPath.GetEntry(0)->ReadProgress = ldwProgress;
				CHellFun::SetRegData(L"historyProgress0", ldwProgress);

				time_t lTimeNow;
				time(&lTimeNow);
				CHellFun::SetRegData(L"historyTimestamp0", (DWORD)lTimeNow);

				mdHistroyPath.GetEntry(0)->ReadProgress = ldwProgress;
				mdHistroyPath.GetEntry(0)->TimeStamp = (DWORD)lTimeNow;

				mdHistroyPath.GetEntry(0)->PageContext = ldContent.pageContext;
				CHellFun::SetRegData(L"PageContent0", ldContent.pageContext);

				mdHistroyPath.GetEntry(0)->PageContext2 = ldContent.pageContext2;
				CHellFun::SetRegData(L"PageContent20", ldContent.pageContext2);

				mdHistroyPath.GetEntry(0)->PageNumber = pageNo1;
				CHellFun::SetRegData(L"PageNumber0", pageNo1);
			}


		}

	} while (false);

}

//显示toast
void CReaderBaseFrame::ShowToast(wchar_t* npszKeyName)
{
	IConfigFile* lpProfile = NULL;

	do 
	{
		BREAK_ON_NULL(npszKeyName);

		//获取多语言字符串
		//为了翻译方便，字符串存放在root/string
		wchar_t lszText[MAX_PATH] = { 0 };
		lpProfile = EinkuiGetSystem()->GetCurrentWidget()->GetDefaultFactory()->GetTempleteFile();
		ICfKey* lpCfKey = NULL;
		if (lpProfile != NULL)
		{
			swprintf_s(lszText, MAX_PATH, L"String/%s", npszKeyName);
			lpCfKey = lpProfile->OpenKey(lszText);
			if (lpCfKey != NULL)
			{
				lpCfKey->GetValue(lszText, MAX_PATH * sizeof(wchar_t));
				CMM_SAFE_RELEASE(lpCfKey);
				CExMessage::SendMessageWithText(mpIterToast, mpIterator, EACT_LABEL_SET_TEXT, lszText);
				FLOAT lfX = (mpIterator->GetSizeX() - mpIterToast->GetSizeX()) / 2;
				mpIterToast->SetPosition(lfX, mpIterator->GetPositionY() + 100);
				//mpIterToast->SetPosition(lfX, mpIterator->GetSizeY() - 200);

				mpIterToast->SetVisible(true);
				mpIterToast->BringToTop();
				EinkuiGetSystem()->UpdateView(true);
				mpIterator->SetTimer(RBF_TIMER_TOAST, 1, 2000, NULL);
			}
		}
		CMM_SAFE_RELEASE(lpCfKey);

	} while (false);

	CMM_SAFE_RELEASE(lpProfile);
}


// 用于给eink工作线程发送消息，避免阻塞服务主线程
bool CReaderBaseFrame::CopyFileThread(LPVOID npData)
{
	CReaderBaseFrame* lpThis = (CReaderBaseFrame*)npData;
	
	CopyFile(lpThis->mszSrcFile, lpThis->mszTempFile, FALSE);

	return true;
}

bool CReaderBaseFrame::GetAskConvertStatus()
{
	bool result = true;

	HKEY lhKey = NULL;
	DWORD ldwRes = 0;
	DWORD ldwValue = 0;
	DWORD ldwLen = sizeof(DWORD);

	ldwRes = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
	if (ldwRes == ERROR_SUCCESS)
	{
		RegQueryValueEx(lhKey, L"AskConvertOrNot", NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
	}

	if (ldwValue == 1)
	{
		//1: 不询问
		result = true;
	}
	else
	{
		//0： 询问
		result = false;
	}
	if (lhKey != NULL)
		RegCloseKey(lhKey);

	return result;
}

bool CReaderBaseFrame::showAskDlg()
{
	if (askConvertDlg != NULL)
	{
		askConvertDlg->ExitModal(PromptDialogAskResult::No);
		askConvertDlg = NULL;
	}

	if (!(GetAskConvertStatus()))
	{

		ICfKey* lpSubKey_1 = mpTemplete->OpenKey(L"AskConvertPrompt");
		askConvertDlg = CAskConvertDlg::CreateInstance(mpIterator, lpSubKey_1);

		PlaceChildInCenter(askConvertDlg->GetIterator());
		//弹出菜单时就关闭输入
		CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_NONE);
		auto AskResult = askConvertDlg->DoModal();
		CMM_SAFE_RELEASE(lpSubKey_1);
		askConvertDlg = NULL;

		switch (AskResult)
		{
		case PromptDialogAskResult::Yes: //继续
		{
			return true;
		}
		case PromptDialogAskResult::No: //取消
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return false;
		}
		default:
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return false;
		}
		}
	}

	if (askConvertDlg != NULL)
	{
		askConvertDlg->ExitModal(PromptDialogAskResult::Yes);
		askConvertDlg = NULL;
	}
	return true;
}

//把要打开的文件copy到临时目录
void CReaderBaseFrame::CopyFileToTemp(IN wchar_t* npszSrc, OUT wchar_t* npszDest, IN LONG nlLen)
{
	do 
	{
		BREAK_ON_NULL(npszSrc);
		BREAK_ON_NULL(npszDest);
		int liLastChar = int(wcslen(npszDest) - 1);
		if (npszDest[0] != UNICODE_NULL && npszDest[liLastChar] != 'f')
		{
			wchar_t lszTemp[MAX_PATH] = { 0 };
			wcscpy_s(lszTemp, MAX_PATH, npszDest);
			lszTemp[wcslen(lszTemp) + 1] = UNICODE_NULL; //双0结尾
			//如此已存在，先删除
			SHFILEOPSTRUCT ldShfile;
			memset(&ldShfile, 0, sizeof(ldShfile));
			ldShfile.pFrom = lszTemp;
			ldShfile.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
			ldShfile.wFunc = FO_DELETE;
			SHFileOperation(&ldShfile);
		}

		liLastChar = int(wcslen(npszSrc) - 1);
		if (npszSrc[liLastChar] == 'f')
		{
			//如果是pdf文件就不copy
			wcscpy_s(npszDest, nlLen, npszSrc);
			break;
		}
		SHGetSpecialFolderPath(NULL, npszDest, CSIDL_COMMON_APPDATA, FALSE);
		wcscat_s(npszDest, nlLen, L"\\EinkSrv\\");
		wcscat_s(npszDest, nlLen, wcsrchr(npszSrc, L'\\') + 1);


		DWORD ldwProtectThreadID = 0;
		HANDLE lhCopyfileThread = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)&CReaderBaseFrame::CopyFileThread,
			this,
			0,
			&ldwProtectThreadID
		);

		if (WaitForSingleObject(lhCopyfileThread, 2000) == WAIT_TIMEOUT)
		{
			ICfKey* lpSubKey = mpTemplete->OpenKey(L"PicturePreview/Loading");
			mpLoadingView = CLoadingView::CreateInstance(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), lpSubKey);
			wchar_t* lpszFileName = wcsrchr(npszSrc, L'\\');
			if (lpszFileName != NULL)
				lpszFileName = lpszFileName + 1;
			mpLoadingView->SetData(lpszFileName);
			mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());

			mpPreNextButton->SetPenMode(PEN_MODE_NONE);
			mpLoadingView->DoModal(NULL, lhCopyfileThread);
			mpPreNextButton->SetPenMode(mulPenMode);
			
			mpLoadingView = NULL;
			CMM_SAFE_RELEASE(lpSubKey);

			CloseHandle(lhCopyfileThread);
		}

	} while (false);

}

//用户选择了要打开的文件,npFileAttrib!=NULL表示是历史记录中的文件
bool CReaderBaseFrame::OpenFile(wchar_t* npszFilePath, HISTORY_FILE_ATTRIB* npFileAttrib)
{
	bool lbRet = false;

	do 
	{
		BREAK_ON_NULL(npszFilePath);
		mpPdfPicture->CloseFile();
		mbIsLoadingSuccess = false;
		/*if (mhFile != INVALID_HANDLE_VALUE)
			CloseHandle(mhFile);*/

		//while (EInkReaderUtil::IsFileLocked(npszFilePath))
		//{
		//	COpenFileLockedDlg* fileLockedDialog = nullptr;
		//	ICfKey* lpSubKey = mpTemplete->OpenKey(L"ConvertFailFileLocked");
		//	fileLockedDialog = COpenFileLockedDlg::CreateInstance(mpIterator, lpSubKey);
		//	PlaceChildInCenter(fileLockedDialog->GetIterator());
		//	if (!fileLockedDialog->DoModal())
		//	{
		//		fileLockedDialog = NULL;
		//		return false;
		//	}
		//	fileLockedDialog = NULL;
		//}

		wcscpy_s(mszSrcFile, MAX_PATH, npszFilePath);

		CopyFileToTemp(npszFilePath, mszTempFile, MAX_PATH);
		
		mulPageIndex = 0;

		wchar_t* lpszFileName = wcsrchr(npszFilePath, L'\\');
		if (lpszFileName == NULL)
			lpszFileName = npszFilePath;
		else
			lpszFileName = lpszFileName + 1;
		if (lpszFileName[wcslen(lpszFileName) - 1] == 't')
			mbIsTxt = true;
		else
			mbIsTxt = false;
		
		PAGE_PDF_CONTEXT ldPageContent;
		ldPageContent.pageContext = 0;
		ldPageContent.pageContext2 = 0;
		ldPageContent.pageIndex = 0;
		if (npFileAttrib != NULL)
		{
			//还原进度
			ldPageContent.pageContext = npFileAttrib->PageContext;
			ldPageContent.pageContext2 = npFileAttrib->PageContext2;
			ldPageContent.pageIndex = npFileAttrib->PageNumber;
		}

		FILE* lfile = NULL;
		bool lbIsUsed = true;
		try
		{
			_wfopen_s(&lfile, npszFilePath, L"rb+");
		}
		catch (...)
		{
		}

		if (lfile != NULL)
		{
			lbIsUsed = false;
			fclose(lfile);
		}

		ULONG lulRet = mpPdfPicture->OpenFile(mszTempFile, ldPageContent.pageContext<=0?NULL:&ldPageContent, npszFilePath);
		mbIsSetPartial = true;
		if (lulRet != EDERR_SUCCESS/* && lulRet != EDERR_EMPTY_CONTENT*/)
		{
			if (mbIsTxt == false || GetFileSize(npszFilePath) > 0)
			{
				//打开文件失败
				ShowToast(L"OpenFileFail");
				mszSrcFile[0] = UNICODE_NULL;
				mpToolbarBottom->GetIterator()->SetVisible(false);
				//CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
				if (mdHistroyPath.Size() > 0)
					break;
			}
		}

		//mhFile = CreateFile(mszSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);

		mpIterToast->SetVisible(false);
		DWORD ldwAttrib = GetFileAttributes(mszTempFile);
		if (lbIsUsed != false)
			ldwAttrib |= FILE_ATTRIBUTE_READONLY; //如果其它软件正在打开这个pdf，我们是保存不了的，所以这种情况就认为文件是只读的
		mpToolbarH->SetFileAttrib(ldwAttrib);
		mpToolbarH->SetDoctype(mpPdfPicture->GetDocType());
		mpPreNextButton->SetFileAttrib(ldwAttrib);

		mpToolbarBottom->SetDoctype(mpPdfPicture->GetDocType());

		if (mszSrcFile[0] == UNICODE_NULL && mpToolbarH != NULL && mpToolbarBottom != NULL)
		{
			ShowToast(L"OpenFileFail");
			mpToolbarH->GetIterator()->SetVisible(true);
			break;
		}

		bool lbIsDoubleScreen = mpToolbarH->GetDuopageStatus();
		mpPdfPicture->EnableDuopageView(lbIsDoubleScreen);
		mpToolbarBottom->EnableAutoZoomButton(!lbIsDoubleScreen);

		//不能变换这个代码和上面代码的位置，切记
		//if (ldPageContent.pageIndex > 1 && ldPageContent.pageContext == 0 && ldPageContent.pageContext2 == 0)
		if (mpPdfPicture->GetDocType() == DOC_TYPE_PDF)
		{
			if (ldPageContent.pageIndex <= 0)
				ldPageContent.pageIndex = 1;
			mpPdfPicture->GoToPage(ldPageContent.pageIndex, true);
		}
		else
		{
			//只有txt格式文件才适用于这种方式
			mpPdfPicture->GoToPage((IEdPage_ptr)NULL, true);
		}

		//记录打开历史
		//先看下这个是不是已经记录过了
		bool lbIsHave = false;
		wchar_t lszOldThumbanilFilePath[MAX_PATH] = { 0 };
		wchar_t lszNewThumbanilFilePath[MAX_PATH] = { 0 };
		mpPdfPicture->GetThumbanilsPath(lszNewThumbanilFilePath, MAX_PATH);

		for (int i=0;i<mdHistroyPath.Size();i++)
		{
			HISTORY_FILE_ATTRIB* lpHistoryFile = mdHistroyPath.GetEntry(i);
			if (_wcsicmp(npszFilePath, lpHistoryFile->FilePath) == 0)
			{
				//已经有了，把它提到第一就行了
				mdHistroyPath.RemoveByIndex(i);
				mdHistroyPath.Insert(0, lpHistoryFile);

				wcscpy_s(lszOldThumbanilFilePath, MAX_PATH, lpHistoryFile->ThumbanilFilePath);

				wcscpy_s(lpHistoryFile->ThumbanilFilePath, MAX_PATH, lszNewThumbanilFilePath);

				lbIsHave = true;
				break;
			}
		}

		if (lbIsHave == false)
		{
			//增加到列表
			HISTORY_FILE_ATTRIB* lpHistoryFile = new HISTORY_FILE_ATTRIB();
			wchar_t* lpszPath = new wchar_t[MAX_PATH];
			wcscpy_s(lpHistoryFile->FilePath, MAX_PATH, npszFilePath);
			wcscpy_s(lpHistoryFile->ThumbanilFilePath, MAX_PATH, lszNewThumbanilFilePath);
			lpHistoryFile->IsModify = 0;
			lpHistoryFile->ReadProgress = 1;
			time_t lTimeNow;
			time(&lTimeNow);
			lpHistoryFile->TimeStamp = (DWORD)lTimeNow;

			mdHistroyPath.Insert(0, lpHistoryFile);

			if (mdHistroyPath.Size() > RBF_HISTROY_MAX)
			{
				//超出最大数了，删除最后一个
				//清除它的缩略图
				HISTORY_FILE_ATTRIB* lpFileAttrib = mdHistroyPath.GetEntry(mdHistroyPath.Size() - 1);
				ClearThumbnail(lpFileAttrib->ThumbanilFilePath);

				delete[] lpFileAttrib;
				mdHistroyPath.RemoveByIndex(mdHistroyPath.Size() - 1);
			}
		}

		//查看是否需要删除掉的缩略图目录
		if (lszOldThumbanilFilePath != NULL && lszOldThumbanilFilePath[0] != UNICODE_NULL && _wcsicmp(lszOldThumbanilFilePath,lszNewThumbanilFilePath) != 0)
		{
			ClearThumbnail(lszOldThumbanilFilePath);
		}

		//保存到注册表
		wchar_t lszText[MAX_PATH];
		DWORD ldwLen = 0;
		for (int i = 0; i < mdHistroyPath.Size(); i++)
		{
			HISTORY_FILE_ATTRIB* lpHistoryFile = mdHistroyPath.GetEntry(i);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_PATH, i);
			CHellFun::SetRegData(lszText, 0, lpHistoryFile->FilePath);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_THUMBNAIL_PATH, i);
			CHellFun::SetRegData(lszText, 0, lpHistoryFile->ThumbanilFilePath);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_MODIFY, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->IsModify);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_PROGRESS, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->ReadProgress);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_TIMESTAMP, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->TimeStamp);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_CONTENT, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->PageContext);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_CONTENT2, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->PageContext2);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_NUMBER, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->PageNumber);

			// Saving scaling data [zhuhl5@20200116]
			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_AUTO_ZOOM, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->autoZoom);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALING, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->scaling);
			
			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGLEVEL, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->scalingLevel);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGPOSX, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->scalingPosX);

			swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGPOSY, i);
			CHellFun::SetRegData(lszText, lpHistoryFile->scalingPosY);
		}

		ShowPageInfo();

		ShowToolBar(true);

		mpHighlight->HideSelect();

		// Recover scaling params [zhuhl5@20200116]
		if (npFileAttrib)
		{
			if (npFileAttrib->autoZoom == 1 && !mbIsTxt)
				mpToolbarBottom->SetZoomStatus(ZoomStatus::AUTO_ZOOM);
			else if (npFileAttrib->scalingLevel == 0 && !mbIsTxt)
				mpToolbarBottom->SetZoomStatus(ZoomStatus::NONE);
			else
				mpToolbarBottom->SetZoomStatus(ZoomStatus::ZOOM, npFileAttrib->scalingLevel);
		}
		else
		{
			mpToolbarBottom->SetZoomStatus(ZoomStatus::NONE);
		}
		// Recover zoomed position [zhuhl5@20200116]
/*		Disable for now, and this part is not done developing. Acquiring absolute zoom position needs to be done if we have recovering zoom position as requirement.
		if(npFileAttrib)
		{
			POINT pt{ LONG(npFileAttrib->scalingPosX), LONG(npFileAttrib->scalingPosY) };
			//RECT ldRect;
			CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_SET_PAGE_MOVE, pt);
			//CExMessage::SendMessage(mpIterator->GetParent()->GetParent(), mpIterator, EEVT_ER_SET_PAGE_MOVE, ldPos, &ldRect, sizeof(RECT));
		}
*/
		lbRet = true;

	} while (false);

	return lbRet;
}

//定时器
void CReaderBaseFrame::OnTimer(
	PSTEMS_TIMER npStatus
	)
{
	if (npStatus->TimerID == RBF_TIMER_TOAST)
	{
		mpIterator->KillTimer(RBF_TIMER_TOAST);
		mpIterToast->SetVisible(false);
	}
	else if (npStatus->TimerID == RBF_TIMER_TSHOW_TOOLBAR)
	{
		//ShowToolBar(false); //不再自动隐藏工具栏
	}
	else if (npStatus->TimerID == RBF_TIMER_ENABL_PARTIAL)
	{
		mpIterator->KillTimer(npStatus->TimerID);
		EiSetPartialUpdate(TRUE);
	}
	else if (npStatus->TimerID == RBF_TIMER_INIT)
	{
		mpIterator->KillTimer(npStatus->TimerID);
		Init();
	}
	else if (npStatus->TimerID == RBF_TIMER_EXIT)
	{
		ExitProcess(0);
	}
	else if (npStatus->TimerID == RBF_TIMER_HOLDINPUT)
	{
		HoldInput(false);
	}
	else if (npStatus->TimerID == RBF_TIMER_ACTIVITY)
	{
		mpPreNextButton->SetPenMode(mulPenMode);
	}
}



//系统屏幕发生旋转
void CReaderBaseFrame::OnRotated(ULONG nuOrient)
{
	EI_SIZE ldPaintSize;

	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
	
	if (mpPdfPicture != NULL)
	{
		//mpPdfPicture->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);上一行设置本窗体大小后，本窗体的大小改变相应函数已经对pdfpicture对象做了大小调整了
		mpPdfPicture->SetRotation(nuOrient);
	}

	mpIterator->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);


	if (mpPreNextButton != NULL)
	{
		mpPreNextButton->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		//mpPreNextButton->SetRotation(nuOrient);
	}

	if (mpToolbarH != NULL)
		mpToolbarH->GetIterator()->SetSize((FLOAT)ldPaintSize.w, 80.0f);

	if (mpToolbarBottom != NULL)
	{
		mpToolbarBottom->GetIterator()->SetSize((FLOAT)ldPaintSize.w, 80.0f);
		mpToolbarBottom->SetFatRatio(mpPdfPicture->GetFatRatio());

		RECT ldRect;
		mpPdfPicture->CalcMovalbe(ldRect);
		mpToolbarBottom->ShowMoveButton(ldRect);
	}
		
	if (mpHighlight != NULL)
	{
		mpHighlight->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		mpHighlight->HideSelect();
	}
	
	if (pdfOverwriteDlg != NULL)
	{
		PlaceChildInCenter(pdfOverwriteDlg->GetIterator());
	}
	if (promptDlg != NULL)
	{
		PlaceChildInCenter(promptDlg->GetIterator());
	}
	if (convertProgressDlg != NULL)
	{
		PlaceChildInCenter(convertProgressDlg->GetIterator());
	}
	if (m_fileOpenFailDlg != NULL)
	{
		PlaceChildInCenter(m_fileOpenFailDlg->GetIterator());
	}

	if (askConvertDlg != NULL)
	{
		PlaceChildInCenter(askConvertDlg->GetIterator());
	}

	if (nuOrient == GIR_90 || nuOrient == GIR_270)
	{
		//坚屏
	}
	else
	{
		//横屏
			
	}

	ShowPageInfo();
	////重绘自己
	//EinkuiGetSystem()->UpdateView();
}

//元素参考尺寸发生变化
ERESULT CReaderBaseFrame::OnElementResized(D2D1_SIZE_F nNewSize)
{
	do 
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		//if(mbIsInit == false)
		//	break;

		if (mpPdfPicture != NULL)
		{
			mpPdfPicture->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		}

		if (mpFileHistoryDlg != NULL)
		{
			mpFileHistoryDlg->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		}

		if (mpThumbnailDlg != NULL)
		{
			mpThumbnailDlg->GetIterator()->SetSize((FLOAT)ldPaintSize.w, (FLOAT)ldPaintSize.h);
		}

		if (mpLoadingView != NULL)
			mpLoadingView->GetIterator()->SetSize(mpIterator->GetSize());

		if(mpIterBackground != NULL)
			mpIterBackground->SetSize(nNewSize);

		if(mpSnapShot != NULL)
			mpSnapShot->GetIterator()->SetSize(mpIterator->GetSize());

		if (mZoomStatus == ZoomStatus::AUTO_ZOOM)
			CExMessage::SendMessage(mpIterator, mpIterator, EEVT_ER_AUTO_ZOOM, NULL, NULL, NULL);
	
	} while (false);

	return ERESULT_SUCCESS;
}

//设置页码跳转窗口的位置
void CReaderBaseFrame::SetJumpPagePos(void)
{
	if (mpJumpPage != NULL)
	{
		EI_SIZE ldPaintSize;
		EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);

		FLOAT lfX = (ldPaintSize.w - mpJumpPage->GetIterator()->GetSizeX()) / 2;
		lfX = lfX - mpIterator->GetPosition().x;
		D2D1_POINT_2F ldPos;
		ldPos.x = lfX;
		if (ldPaintSize.w > ldPaintSize.h)
		{
			ldPos.y = 200.0f;
		}
		else
		{
			ldPos.y = 522.0f;
		}
		mpJumpPage->GetIterator()->SetPosition(ldPos);
	}

}

//清除缩略图文件
void CReaderBaseFrame::ClearThumbnail(const wchar_t* npszPath)
{
	CFilePathName lCachePath;
	if (npszPath == NULL)
	{
		//清除所有
		lCachePath.SetByUserAppData();
		lCachePath.AssurePath();
		lCachePath += L"EinkReader\\";
	}
	else
	{
		lCachePath.SetPathName(npszPath);
	}

	wchar_t lszTemp[MAX_PATH] = { 0 };
	wcscpy_s(lszTemp, MAX_PATH, lCachePath.GetPathName());
	lszTemp[wcslen(lszTemp) + 1] = UNICODE_NULL; //双0结尾
												 //如此已存在，先删除
	SHFILEOPSTRUCT ldShfile;
	memset(&ldShfile, 0, sizeof(ldShfile));
	ldShfile.pFrom = lszTemp;
	ldShfile.fFlags = FOF_NO_UI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
	ldShfile.wFunc = FO_DELETE;
	SHFileOperation(&ldShfile);
}

void CReaderBaseFrame::ShowFileOpenFailDialog(const wchar_t* resourceName)
{
	if (m_fileOpenFailDlg != NULL)
	{
		m_fileOpenFailDlg->ExitModal();
		m_fileOpenFailDlg = NULL;
	}
	//显示转换失败的弹框
	ICfKey* lpSubKey = mpTemplete->OpenKey(resourceName);
	m_fileOpenFailDlg = CFileOpenFailDlg::CreateInstance(mpIterator, lpSubKey);;
	bool lbEnable = true;
	PlaceChildInCenter(m_fileOpenFailDlg->GetIterator());
	m_fileOpenFailDlg->DoModal(&lbEnable);

	CMM_SAFE_RELEASE(lpSubKey);
	m_fileOpenFailDlg = nullptr;
	//开启输入
	CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
}

void CReaderBaseFrame::PlaceChildInCenter(IEinkuiIterator * childIterator)
{
	if (!childIterator) return;
	auto size = GetIterator()->GetSize();
	auto childSize = childIterator->GetSize();
	decltype(childIterator->GetPosition()) position;
	position.x = (size.width - childSize.width) / 2;
	position.y = (size.height - childSize.height) / 2;
	childIterator->SetPosition(position);
}

//获取文件大小
ULONG CReaderBaseFrame::GetFileSize(wchar_t* npszFilePath)
{
	ULONG lulRet = 1;
	HANDLE lhTxtFile = INVALID_HANDLE_VALUE;
	LARGE_INTEGER lFileSize;

	do 
	{
		lhTxtFile = CreateFile(npszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);
		if (lhTxtFile == INVALID_HANDLE_VALUE)
			break;

		if (GetFileSizeEx(lhTxtFile, &lFileSize) == FALSE)
			break;

		lulRet = lFileSize.LowPart;

	} while (false);

	if (lhTxtFile != INVALID_HANDLE_VALUE)
		CloseHandle(lhTxtFile);

	return lulRet;
}

//初始化
void CReaderBaseFrame::Init(void)
{
	DWORD ldwRet = 0, ldwLen = 0;;
	HKEY lhKey = NULL;
	HKEY lhKey2 = NULL;
	wchar_t lszText[MAX_PATH] = { 0 };
	
	do 
	{
		/*if (mhFile != INVALID_HANDLE_VALUE)
			CloseHandle(mhFile);*/

		DWORD ldwValue = 0;
		//日志开关
		ldwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Lenovo\\EinkSDK", 0, KEY_READ | KEY_WOW64_64KEY, &lhKey2);
		if (ldwRet == ERROR_SUCCESS)
		{
			ldwLen = sizeof(DWORD);
			ldwValue = 0;
			RegQueryValueEx(lhKey2, RbF_REG_LOG, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			mpPdfPicture->SetLogStatus(ldwValue == 0 ? false : true);
		}

		if (lhKey2 != NULL)
			RegCloseKey(lhKey2);

		ldwRet = RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &lhKey);
		if (ldwRet == ERROR_SUCCESS)
		{
			//是否第一次打开
			ldwLen = sizeof(DWORD);
			ldwValue = 0;
			RegQueryValueEx(lhKey, RbF_REG_ONCE, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			if (ldwValue == 0)
			{
				ldwValue = 1;
				RegSetValueEx(lhKey, RbF_REG_ONCE, NULL, REG_DWORD, (BYTE*)&ldwValue, sizeof(DWORD));

				CFilePathName ldPath;
				ldPath.SetByModulePathName();
				ldPath.Transform(L".\\EInkHelp.exe");
				//验证exe是否签名
				FileCVCheckVaule filecheckvalue;
				wcscpy_s(filecheckvalue.CVPath, ldPath.GetPathName());
				DWORD resultCheck = EiIsFileCertificateValidation(filecheckvalue);
				if (filecheckvalue.checkResult != 0)
				{
					//启动reader介绍页
					ShellExecute(NULL, L"open", ldPath.GetPathName(), L"/first-time-Reader", NULL, SW_SHOW);
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//方向设置
			ldwLen = sizeof(DWORD);
			ldwValue = 10;
			RegQueryValueEx(lhKey, RbF_REG_SCREEN_ORI, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			mpToolbarH->SetScreenOriButton((ULONG)ldwValue);
			DWORD ldwOrient = GIR_NONE;
			mbIsScreenAuto = false;
			if (ldwValue == SCREEN_STATUS_AUTO)
			{
				EI_SYSTEM_INFO ldInfo;
				EiGetSystemInfo(&ldInfo);
				ldwOrient = ldInfo.ulOrient;
				mbIsScreenAuto = true;
			}
			else if (ldwValue == SCREEN_STATUS_H)
			{
				EI_SYSTEM_INFO ldInfo;
				EiGetSystemInfo(&ldInfo);
				ldwOrient = ldInfo.ulOrient;
				if (ldwOrient == GIR_90 || ldwOrient == GIR_270)
				{
					//TBS 允许180度，否则开盖时reader方向不对
					ldwOrient = GIR_NONE;
				}
			}
			else if (ldwValue == SCREEN_STATUS_V)
			{
				ldwOrient = GIR_90;
			}
			EiSetScreenOrient(ldwOrient);
			EinkuiGetSystem()->ResetPaintboard();
			OnRotated(ldwOrient);
			//////////////////////////////////////////////////////////////////////////

			//笔宽度
			ldwLen = sizeof(DWORD);
			RegQueryValueEx(lhKey, RbF_REG_PEN_WIDTH_INDEX, NULL, NULL, (BYTE*)&mdwPenWidthIndex, &ldwLen);
			mpPdfPicture->SetPenWidth(mfArrayPenWidthArray[mdwPenWidthIndex]);
			EiSetHandwritingSetting(mfArrayFwPenWidthArray[mdwPenWidthIndex]);

			//笔颜色
			ldwLen = sizeof(DWORD);
			RegQueryValueEx(lhKey, RbF_REG_PEN_COLOR_INDEX, NULL, NULL, (BYTE*)&mdwPenColorIndex, &ldwLen);
			mpPdfPicture->SetPenColor(mdwPenColorIndex);
			if (mpToolbarH != NULL)
				mpToolbarH->SetPenData(mdwPenWidthIndex, mdwPenColorIndex);

			//上次设置的txt字号
			mdwFontsizeIndex = 1;
			ldwLen = sizeof(DWORD);
			RegQueryValueEx(lhKey, RbF_REG_TXT_FONT_SIZE_INDEX, NULL, NULL, (BYTE*)&mdwFontsizeIndex, &ldwLen);

			//单双页
			ldwLen = sizeof(DWORD);
			ldwValue = 0;
			RegQueryValueEx(lhKey, RbF_REG_DOUBLE_SCREEN, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			if (ldPaintSize.w > ldPaintSize.h)
			{
				mpToolbarH->SetDuopageButton(ldwValue == 0 ? false : true);
			}
			else
			{
				mpToolbarH->SetDuopageButton(false);
			}

			//获取历史记录
			if (mdHistroyPath.Size() <= 0)
			{
				for (int i = 0; i < RBF_HISTROY_MAX; i++)
				{
					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_PATH, i);
					ldwLen = MAX_PATH * sizeof(wchar_t);
					HISTORY_FILE_ATTRIB* lpHistoryPath = NULL;
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lszText, &ldwLen) == ERROR_SUCCESS)
					{
						if (lszText[0] == UNICODE_NULL)
						{
							break; //没有有效记录
						}

						lpHistoryPath = new HISTORY_FILE_ATTRIB();
						wcscpy_s(lpHistoryPath->FilePath, MAX_PATH, lszText);
						mdHistroyPath.Insert(-1, lpHistoryPath);
					}

					if (lpHistoryPath == NULL)
						break;

					ldwLen = MAX_PATH * sizeof(wchar_t);
					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_THUMBNAIL_PATH, i);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lszText, &ldwLen) == ERROR_SUCCESS)
						wcscpy_s(lpHistoryPath->ThumbanilFilePath, MAX_PATH, lszText);

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_MODIFY, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->IsModify, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->IsModify = 0;


					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_PROGRESS, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->ReadProgress, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->ReadProgress = 1;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_TIMESTAMP, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->TimeStamp, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->TimeStamp = 0;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_NUMBER, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->PageNumber, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->PageNumber = 0;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_CONTENT, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->PageContext, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->PageContext = 0;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RbF_REG_PAGE_CONTENT2, i);
					ldwLen = sizeof(DWORD);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->PageContext2, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->PageContext2 = 0;

					// Loading scaling data [zhuhl5@20200116]
					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_AUTO_ZOOM, i);
					ldwLen = sizeof(float);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->autoZoom, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->autoZoom = 0;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALING, i);
					ldwLen = sizeof(float);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->scaling, &ldwLen) != ERROR_SUCCESS)
					{
						lpHistoryPath->scaling = 1.0f;
					}
					else
					{
						if (lpHistoryPath->scaling < 1.0f)	// We don't have zoom out.
						{
							lpHistoryPath->scaling = 1.0f;
						}
					}

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGLEVEL, i);
					ldwLen = sizeof(float);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->scalingLevel, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->scalingLevel = 0;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGPOSX, i);
					ldwLen = sizeof(float);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->scalingPosX, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->scalingPosX = 0.0f;

					swprintf_s(lszText, MAX_PATH, L"%s%d", RBF_REG_HISTORY_SCALINGPOSY, i);
					ldwLen = sizeof(float);
					if (RegQueryValueEx(lhKey, lszText, NULL, NULL, (BYTE*)&lpHistoryPath->scalingPosY, &ldwLen) != ERROR_SUCCESS)
						lpHistoryPath->scalingPosY = 0.0f;

				}
				if (mdHistroyPath.Size() <= 0)
					EiSetBatteryStatus(GI_BATTERY_HIDE);
			}
		}
		else
		{
			//preload后无此注册表路径，启动reader需显示介绍页
			ldwRet = RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Lenovo\\Eink-PdfReader", &lhKey);
			if (ldwRet == ERROR_SUCCESS)
			{
				DWORD showhelp = 1;
				ldwRet = RegSetValueEx(lhKey, RbF_REG_ONCE, NULL, REG_DWORD, (BYTE*)&showhelp, sizeof(DWORD));

				CFilePathName ldPath;
				ldPath.SetByModulePathName();
				ldPath.Transform(L".\\EInkHelp.exe");

				//验证exe是否签名
				FileCVCheckVaule filecheckvalue;
				wcscpy_s(filecheckvalue.CVPath, ldPath.GetPathName());
				DWORD resultCheck = EiIsFileCertificateValidation(filecheckvalue);
				if (filecheckvalue.checkResult != 0)
				{
				//启动reader介绍页
					ShellExecute(NULL, L"open", ldPath.GetPathName(), L"/first-time-Reader", NULL, SW_SHOW);
				}
			}
		}
		//笔宽度
		mpPdfPicture->SetPenWidth(mfArrayPenWidthArray[mdwPenWidthIndex]);
		EiSetHandwritingSetting(mfArrayFwPenWidthArray[mdwPenWidthIndex]);

		//笔颜色
		mpPdfPicture->SetPenColor(mdwPenColorIndex);
		if (mpToolbarH != NULL)
			mpToolbarH->SetPenData(mdwPenWidthIndex, mdwPenColorIndex);

		//txt字号
		if (mdwFontsizeIndex >= ZCT_FONTSIZE_LEVEL || mdwFontsizeIndex < 0)
			mdwFontsizeIndex = 1;
		mpToolbarH->SetTxtFontSizeIndex(mdwFontsizeIndex);

		if (mdHistroyPath.Size() > 0)
		{
			//说明打开过文件,恢复上次打开状态
			mpPdfPicture->SetFontSize(mdwFontSizeArray[mdwFontsizeIndex]);

			
			if (GetFileAttributes(mdHistroyPath.GetEntry(0)->FilePath) == INVALID_FILE_ATTRIBUTES)
			{
				//文件不存在了
				CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
				break;
			}

			//mpToolbarH->SetBCoverState();

			if (!m_cmdLineOpenFileName.empty() && m_cmdLineOpenFileName.size() >= 4) // sizeof ".pdf" == 4
			{
				unique_ptr<wchar_t[]> extName = make_unique<wchar_t[]>(10);
				wcscpy_s(extName.get(), 10, m_cmdLineOpenFileName.c_str() + m_cmdLineOpenFileName.length() - 4);
				for (int i = 0; extName[i] != 0; i++)
				{
					extName[i] = std::tolower(extName[i]);
				}

				if (wcscmp(extName.get(), L".pdf") == 0) // Open PDF files
				{
					unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(2048);
					wcscpy_s(buffer.get(), 2048, m_cmdLineOpenFileName.c_str());
					OpenFile(buffer.get(), nullptr);
				}
				else
				{
					PDFConvert::SubmitConvertTask(m_cmdLineOpenFileName);
					while (!PDFConvert::IsConvertingCompleted()) Sleep(100);
					auto convertResult = PDFConvert::GetConvertResult();
					if (convertResult == PDFConvert::ConvertResult::OK)
					{
						auto pdfFileName = PDFConvert::GetPDFFileFullPath();
						unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(2048);
						wcscpy_s(buffer.get(), 2048, pdfFileName.c_str());
						OpenFile(buffer.get(), nullptr);
					}
				}
			}
			else
			{
				OpenFile(mdHistroyPath.GetEntry(0)->FilePath, mdHistroyPath.GetEntry(0));
			}


			//wcscpy_s(mszSrcFile, MAX_PATH, mdHistroyPath.GetEntry(0)->FilePath);
			//CopyFileToTemp(mdHistroyPath.GetEntry(0)->FilePath, mszTempFile, MAX_PATH);
			//

			////文件名
			//wchar_t* lpszFileName = wcsrchr(mdHistroyPath.GetEntry(0)->FilePath, L'\\');
			//if (lpszFileName == NULL)
			//	lpszFileName = mdHistroyPath.GetEntry(0)->FilePath;
			//else
			//	lpszFileName = lpszFileName + 1;

			//if (lpszFileName[wcslen(lpszFileName) - 1] == 't')
			//	mbIsTxt = true;
			//else
			//	mbIsTxt = false;

			//PAGE_PDF_CONTEXT ldPageContent;
			//ldPageContent.pageContext = mdHistroyPath.GetEntry(0)->PageContext;
			//ldPageContent.pageContext2 = mdHistroyPath.GetEntry(0)->PageContext2;
			//ldPageContent.pageIndex = mdHistroyPath.GetEntry(0)->PageNumber;
			//mulPageIndex = 0;
			//ULONG lulRet = mpPdfPicture->OpenFile(mszTempFile, &ldPageContent, mdHistroyPath.GetEntry(0)->FilePath);
			//mbIsSetPartial = true;
			//if (lulRet != EDERR_SUCCESS/* && lulRet != EDERR_EMPTY_CONTENT*/)
			//{
			//	if (mbIsTxt == false || GetFileSize(mdHistroyPath.GetEntry(0)->FilePath) > 0)
			//	{
			//		//打开失败了，可能文件不存在了
			//		mszSrcFile[0] = UNICODE_NULL;
			//		CExMessage::PostMessage(mpIterator, mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			//		break;
			//	}
			//}

			////mhFile = CreateFile(mszSrcFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, NULL, NULL);

			////是否双屏
			//ldwLen = sizeof(DWORD);
			//RegQueryValueEx(lhKey, RbF_REG_DOUBLE_SCREEN, NULL, NULL, (BYTE*)&ldwValue, &ldwLen);
			//mpToolbarH->SetDuopageButton(ldwValue == 0 ? false : true);
			//DWORD ldwAttrib = GetFileAttributes(mszTempFile);
			//mpPreNextButton->SetFileAttrib(ldwAttrib);
			//mpToolbarH->SetFileAttrib(ldwAttrib);
			//mpToolbarH->SetDoctype(mpPdfPicture->GetDocType());
			//
			//mpToolbarBottom->SetDoctype(mpPdfPicture->GetDocType());

			/*EI_SIZE ldPaintSize;
			EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
			if (ldPaintSize.w > ldPaintSize.h)
			{
				mpToolbarH->SetDuopageButton(ldwValue == 0 ? false : true);
			}
			else
			{
				mpToolbarH->SetDuopageButton(false);
			}*/

			//mpPdfPicture->EnableDuopageView(mpToolbarH->GetDuopageStatus());

			////不能变换这个代码和上面代码的位置，切记
			////if (ldPageContent.pageIndex > 1 && ldPageContent.pageContext == 0 && ldPageContent.pageContext2 == 0)
			////ldPageContent.pageIndex = 2;
			//if(mpPdfPicture->GetDocType() == DOC_TYPE_PDF)
			//{
			//	if (ldPageContent.pageIndex <= 0)
			//		ldPageContent.pageIndex = 1;
			//	mpPdfPicture->GoToPage(ldPageContent.pageIndex);
			//}
			//else
			//{
			//	//只有txt格式文件才适用于这种方式
			//	mpPdfPicture->GoToPage((IEdPage_ptr)NULL);
			//}

			//ShowPageInfo();
			//ShowToolBar(true);
			//mpHighlight->HideSelect();
		}
		else
		{
			//说明是第一次打开，直接弹出打开文件框，其它的就不需要初始化了
			//ShowFileOpenDlg();
			CExMessage::PostMessage(mpIterator,mpIterator, EEVT_ER_OPEN_FILE, CExMessage::DataInvalid);
			break;
		}

	} while (false);
}

ERESULT __stdcall CReaderBaseFrame::EinkUpdating(ULONGLONG nxNumber, CReaderBaseFrame* npThis)	// nuNumber是更新序号，每次更新加一，达到最大值后回到零
{
	do 
	{
		if (npThis->mbIsSetPartial == false)
		{
			//npThis->mbIsSetPartial = true;
			break; //此时不要关闭partial,正在处于界面频繁变化阶段
		}
			

		ULONG luCrtTick = GetTickCount();

		if ((nxNumber - npThis->mxLastGcUiNumber >= 50 && luCrtTick - npThis->muLastGcTick > 1000*5) || luCrtTick - npThis->muLastGcTick > 1000 * 60 * 5)	// 十帧或者五分钟
		{
			// 设置进入GC模式 ???niu
			EinkuiGetSystem()->ClearEinkBuffer();
			EiSetPartialUpdate(FALSE);
			OutputDebugString(L"enter gc false partial");
			npThis->mbGcMode = true;
			npThis->muLastGcTick = luCrtTick;
			npThis->mxLastGcUiNumber = nxNumber;
		}
		else
		{
			if (npThis->mbGcMode != FALSE)
			{
				// 设置退出GC模式 ???niu
				EiSetPartialUpdate(TRUE);
				OutputDebugString(L"enter gc true partial");
				npThis->mbGcMode = false;
			}
		}

	} while (false);
	

	return ERESULT_SUCCESS;
}

tuple<bool, wstring, bool> CReaderBaseFrame::ConvertAndOpenOfficeFile(const wchar_t * filePath)
{
	//实际转换文档
	static const std::unordered_map<PDFConvert::ConvertResult, const wchar_t*> kErrorDialogMap {
		{ PDFConvert::ConvertResult::OK, L""},//hy
		{ PDFConvert::ConvertResult::OfficeComponentNotInstalled, L"ConvertFailMissingOfficeComponent" },
		{ PDFConvert::ConvertResult::UnsupportedFileType, L"ConvertFailUnsupportedFileType" },
		{ PDFConvert::ConvertResult::FileLocked, L"ConvertFailFileLocked" },
		{ PDFConvert::ConvertResult::ComError, L"ConvertFailConvertFail" },
		{ PDFConvert::ConvertResult::NeedPassword, L"ConvertFailOpenFail" },
		{ PDFConvert::ConvertResult::InvalidFile, L"ConvertFailOpenFail" },
		{ PDFConvert::ConvertResult::ConvertFailed, L"ConvertFailConvertFail" },
		{PDFConvert::ConvertResult::PDFFileLocked, L"ConvertFailPDFFileLocked"}
	};

	auto AskYesNo = [this](const wchar_t* dialogResourceName)->PromptDialogResult {
		if (!dialogResourceName) 
			return PromptDialogResult::No;

		if (promptDlg != NULL)
		{
			promptDlg->ExitModal(PromptDialogResult::No);
			promptDlg = NULL;
		}

		ICfKey* lpSubKey = mpTemplete->OpenKey(dialogResourceName);

		promptDlg = CYesNoPromptDlg::CreateInstance(mpIterator, lpSubKey);
		PlaceChildInCenter(promptDlg->GetIterator());
		auto dialogResult = promptDlg->DoModal();
		CMM_SAFE_RELEASE(lpSubKey);
		promptDlg = nullptr;
		return dialogResult;
	};

	wstring resultFilePath = wstring(filePath) + L".pdf";

	if (EInkReaderUtil::IsFileExists(resultFilePath))//提示覆盖
	{
		if (pdfOverwriteDlg != NULL)
		{
			pdfOverwriteDlg->ExitModal(OverwriteDialogResult::Cancel);
			pdfOverwriteDlg = NULL;
		}

		ICfKey* lpSubKey = mpTemplete->OpenKey(L"PDFOverwritePrompt");
		pdfOverwriteDlg = CPDFOverwriteDlg::CreateInstance(mpIterator, lpSubKey);
		PlaceChildInCenter(pdfOverwriteDlg->GetIterator());
		auto dialogResult = pdfOverwriteDlg->DoModal();
		CMM_SAFE_RELEASE(lpSubKey);
		pdfOverwriteDlg = NULL;

		switch (dialogResult)
		{
		case OverwriteDialogResult::Overwrite://覆盖，跳出switch执行转换操作
			break;

		case OverwriteDialogResult::Cancel://取消，返回
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return std::make_tuple(false, L"", true);
		}

		case OverwriteDialogResult::OpenExisting://打开，不执行转换
		{
			if (mpThumbnailDlg != NULL)
			{
				//如果是在缩略图界面转换的，则退出缩略图界面
				mpThumbnailDlg->ExitModal();
				mpThumbnailDlg = NULL;
			}

			auto buffer = std::make_unique<wchar_t[]>(2048);
			wcscpy_s(buffer.get(), 2048, resultFilePath.c_str());
			OpenFile(buffer.get(), nullptr);
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return std::make_tuple(true, L"", false);
		}

		default:
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return std::make_tuple(false, L"", false);
		}
		
		}
	}

	const int64_t kWarningSize = 5 * 1024 * 1024; //文件过大时（5MB），提醒用户

	if (EInkReaderUtil::GetFileSize(filePath) >= kWarningSize)
	{
		auto dialogResult = AskYesNo(L"BigOfficeFilePrompt");
		if (dialogResult == PromptDialogResult::No)
		{
			CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
			return std::make_tuple(false, L"", true);//文件过大时，用户选择取消
		}
	}

	if (pdfOverwriteDlg != NULL)
	{
		pdfOverwriteDlg->ExitModal(OverwriteDialogResult::Overwrite);
		pdfOverwriteDlg = NULL;
	}

	if (convertProgressDlg != NULL)
	{
		convertProgressDlg->ExitModal(PDFConvert::ConvertResult::ConvertFailed);
		convertProgressDlg = NULL;
	}
	if (mpThumbnailDlg != NULL)
	{
		//如果是在缩略图界面转换的，则退出缩略图界面
		mpThumbnailDlg->ExitModal();
		mpThumbnailDlg = NULL;
	}

	ICfKey* lpSubKey = mpTemplete->OpenKey(L"ConvertProgress");
	convertProgressDlg = CConvertProgressDlg::CreateInstance(mpIterator, lpSubKey);;
	PlaceChildInCenter(convertProgressDlg->GetIterator());

	wstring pdfFileName;
	PDFConvert::ConvertResult convertResult;
	std::tie(convertResult, pdfFileName) = convertProgressDlg->DoModal(filePath);

	CMM_SAFE_RELEASE(lpSubKey);
	convertProgressDlg = NULL;

	wstring errorMessage;
	if (convertResult == PDFConvert::ConvertResult::OK)
	{
		unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(2048);
		wcscpy_s(buffer.get(), 2048, pdfFileName.c_str());
		if (mpFileHistoryDlg != nullptr) mpFileHistoryDlg->ExitModal();
		OpenFile(buffer.get(), nullptr);
		errorMessage = kErrorDialogMap.at(PDFConvert::ConvertResult::OK);
		CExMessage::SendMessage(EinkuiGetSystem()->GetCurrentWidget()->GetHomePage(), mpIterator, EEVT_PEN_MODE, PEN_MODE_PEN);
		return std::make_tuple(true, errorMessage, false); //三个参数分别代表（是否成功，errormessage,用户是否取消）
	}

	errorMessage = kErrorDialogMap.at(convertResult);
	return std::make_tuple(false, errorMessage, false);
}

void CReaderBaseFrame::PageAutoZoom()
{
	EI_SIZE ldPaintSize;
	EinkuiGetSystem()->GetPaintboardSize(&ldPaintSize);
	ldPaintSize.h -= 162;

	ED_RECTF contentRect = mpPdfPicture->CalImageContentRect();
	float wRatio = ldPaintSize.w / (contentRect.right - contentRect.left);
	float hRatio = 0.0f;//ldPaintSize.h / (contentRect.bottom - contentRect.top);
	float largerRotio = wRatio > hRatio ? wRatio : hRatio;

	RECT ldRect;
	float scaleRatio = largerRotio / mpPdfPicture->GetBaseRatio();
	scaleRatio = mpToolbarBottom->AjustAutoZoomLevel(scaleRatio);
	mpPdfPicture->SetScaleRatio(scaleRatio, ldRect);

	float32 realRatio = mpPdfPicture->GetRealRatio();
	if (wRatio < hRatio)
	{
		ldRect.top = 0;
		ldRect.bottom = 0;
		mMoveForward = MoveForward::HORIZONTAL;
		mpPreNextButton->SetMoveForward(MoveForward::HORIZONTAL);
		mpToolbarBottom->SetMoveForward(MoveForward::HORIZONTAL);
	}
	else
	{
		ldRect.left = 0;
		ldRect.right = 0;
		mMoveForward = MoveForward::VERTICAL;
		mpPreNextButton->SetMoveForward(MoveForward::VERTICAL);
		mpToolbarBottom->SetMoveForward(MoveForward::VERTICAL);
	}
	mpToolbarBottom->ShowMoveButton(ldRect);

	float32 posX = contentRect.left * realRatio + ldPaintSize.w / 2.0f;
	float32 posY = contentRect.top * realRatio + ldPaintSize.h / 2.0f;
	mpPdfPicture->MovePageTo(posX, posY);

	SetHandWriteWidth();
	mpHighlight->Relocation();
}

ZoomStatus CReaderBaseFrame::GetZoomStatus()
{
	return mZoomStatus;
}
