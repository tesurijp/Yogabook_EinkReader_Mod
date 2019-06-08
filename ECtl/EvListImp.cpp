/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "ElementImp.h"
#include "EvListImp.h"
#include "XCtl.h"
#include "EvButtonImp.h"
#include   <algorithm>  
//using namespace D2D1;


DEFINE_BUILTIN_NAME(List)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvList::CEvList()
{
	mlShowStyle = LIST_VIEW_STYLE_REPORT;
	mbShowVerScroll = true;
	mbShowHorScroll = true;
	mlDocumentHeight = 0;
	mlDocumentWidth = 0;
	mlDocToView0PositionY = 0;
	mbNeedMemoryManager = true;
	mpVScrollBar = NULL;
	mpHScrollBar = NULL;
	mbSetScrollBarPositionAndSize = false;
	mbAcceptDrop = false;
	mpIterMoveItem = NULL;
	mpIterPicMoveForMouse = NULL;
	mpIterInsertMark = NULL;
}

// 用于释放成员对象
CEvList::~CEvList()
{
	//ResetList();

	//CMM_SAFE_RELEASE(mpVScrollBar);
	//CMM_SAFE_RELEASE(mpHScrollBar);

}

ULONG CEvList::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
	)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		//首先调用基类
		leResult = 	CXuiElement::InitOnCreate(npParent,npTemplete,nuEID);
		if(leResult != ERESULT_SUCCESS)
			break;
		//创建纵向滚动条
		ICfKey * lpVScrollKey = npTemplete->GetSubKey(L"VScroll");
		if(lpVScrollKey)
		{
			mpVScrollBar = CEvScrollBar::CreateInstance(mpIterator,lpVScrollKey,V_SCROLL_BAR_ID);			
		}
		//创建横向滚动条
		ICfKey * lpHScrollKey = npTemplete->GetSubKey(L"HScroll");
		if(lpHScrollKey)
		{
			mpHScrollBar = CEvScrollBar::CreateInstance(mpIterator,lpHScrollKey,H_SCROLL_BAR_ID);			
		}
		
		mpIterator->ModifyStyles(EITR_STYLE_ALL_MWHEEL);
		//创建InsertMark
		ICfKey * lpMarkKey = npTemplete->GetSubKey(L"InsertMark");
		if(lpMarkKey)
		{
			mpIterInsertMark = EinkuiGetSystem()->GetAllocator()->CreateElement (mpIterator,lpMarkKey,INSERT_MARK_ID);	
			if(mpIterInsertMark)
				mpIterInsertMark->SetVisible(false);
		}
		
		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvList::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//装载一些必要的配置资源
		LoadResource();

		//设置滚动条位置和大小
		//设置显示区域
		if(mlShowStyle != LIST_VIEW_STYLE_AUTO_FIT_X && mlShowStyle != LIST_VIEW_STYLE_AUTO_FIT_Y)
		{
			D2D1_RECT_F lRectVisibel;
			lRectVisibel.left = 0;
			lRectVisibel.top = 0;
			lRectVisibel.right = lRectVisibel.left + mpIterator->GetSizeX();
			lRectVisibel.bottom = lRectVisibel.top + mpIterator->GetSizeY();
			mpIterator->SetVisibleRegion(lRectVisibel);
			//重新设置滚动条的位置和大小
			SetScrollBarPositionAndSize();
		}
		else
		{
			if(mpVScrollBar != NULL)
				mpVScrollBar->GetIterator()->SetVisible(false);
		}
		lResult = ERESULT_SUCCESS;
	}while(false);

	return lResult;
}

//设置ScrollBar的位置
bool CEvList::SetScrollBarPositionAndSize()
{
	//if(mbSetScrollBarPositionAndSize == false)
	{
		//设置ScrollBar位置
		if(mpVScrollBar)
		{
			//mpVScrollBar->GetIterator()->SetVisible(false);
			FLOAT lfScrollBarWidth = mpVScrollBar->GetVScrollBarWidth() ;
			FLOAT lfX = mpIterator->GetSizeX();
			FLOAT lfY = mpIterator->GetSizeY();
			mpVScrollBar->GetIterator()->SetPosition(lfX-lfScrollBarWidth-1,1 );
			mpVScrollBar->GetIterator()->SetSize(lfScrollBarWidth,lfY-2);
			mpVScrollBar->Relocation();
			mbSetScrollBarPositionAndSize = true;
		}

	}
	return true;
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CEvList::OnElementDestroy()
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		CXuiElement::OnElementDestroy();	//调用基类

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//装载配置资源
ERESULT CEvList::LoadResource()
{

	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	ICfKey* lpValue = NULL;

	do 
	{
		//获取帧信息
		mlMaxFrame = mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LIST_BACKIAMGE_FRAME_COUNT,1);

		if (mlMaxFrame > 0 && mpBgBitmap != NULL)
		{
			//计算每帧大小
			UINT luiWidth = mpBgBitmap->GetWidth();
			UINT luiHeight = mpBgBitmap->GetHeight();
			mlCurrentIndex = 0;

			mpIterator->SetSize(float(luiWidth / mlMaxFrame),(float)luiHeight);
		}
		//获取显示模式

		//获取是否不需要显示滚动条等属性

		// 读取风格设置
		mlShowStyle = (LONG)mpTemplete->QuerySubKeyValueAsLONG(TF_ID_LIST_STYLE,1);

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}
//重载Resize 重新设置显示区域
ERESULT CEvList::OnElementResized(D2D1_SIZE_F nNewSize)
{
	
	if(mlShowStyle != LIST_VIEW_STYLE_AUTO_FIT_X && mlShowStyle != LIST_VIEW_STYLE_AUTO_FIT_Y)
	{
		//设置显示区域
		D2D1_RECT_F lRectVisibel;
		lRectVisibel.left = 0;
		lRectVisibel.top = 0;
		lRectVisibel.right = lRectVisibel.left + mpIterator->GetSizeX();
		lRectVisibel.bottom = lRectVisibel.top + mpIterator->GetSizeY();
		mpIterator->SetVisibleRegion(lRectVisibel);
		//重新设置滚动条的位置和大小
		SetScrollBarPositionAndSize();

		//重新计算区域设置滚动大小
		CalcElementPosition();
		//
		CheckVScrollBarNeedToShow();
	}
	return ERESULT_SUCCESS;
}
// 分解消息，提供消息分解或消息响应的功能，本类的实现是将消息分解为不同的请求后，调用相应的处理虚函数，对于不认识的消息，一律返回ERESULT_UNEXPECTED_MESSAGE
// 本函数的返回值会自动同步设置到npMsg指向的消息对象中
ERESULT CEvList::ParseMessage(IEinkuiMessage* npMsg)
{
	// 实现原则，优先调用自身的分解功能，而后将不处理的消息发给基类

	ERESULT luResult = ERESULT_UNEXPECTED_MESSAGE;

	do 
	{
		BREAK_ON_NULL(npMsg);

		switch (npMsg->GetMessageID())
		{
			
	
			//处理滚动消息
		case EACT_SCROLLBAR_VSCROLL_THUMB_POSITION:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				//FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();			
				FLOAT lTempPos = *(FLOAT*)npMsg->GetInputData();
				if(lTempPos > (mlDocumentHeight - mpIterator->GetSizeY()))
					lTempPos = mlDocumentHeight - mpIterator->GetSizeY();
				else if(lTempPos < 0.0f)
					lTempPos = 0.0f;

				mlDocToView0PositionY = lTempPos;
				CalcElementPosition();

				//需返回给父窗口，链表正在滚动
				PostMessageToParent(EACT_LIST_SCROLLING,mlDocToView0PositionY);
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_SET_PIC_INDEX:	
			{
				//切换显示帧
				if(npMsg->GetInputDataSize() != sizeof(LONG))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				LONG* lpValue = (LONG*)npMsg->GetInputData();
				luResult = OnChangeBackImageIndex(*lpValue);

				break;
			}
		case EACT_LIST_SET_MEMORY_MANAGER:	
			{
				//切换显示帧
				if(npMsg->GetInputDataSize() != sizeof(bool))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				bool* lpValue = (bool*)npMsg->GetInputData();
				mbNeedMemoryManager = *lpValue;
				luResult = ERESULT_SUCCESS;

				break;
			}
		case EACT_LIST_CHANGE_PIC:	
			{
				//更换显示图片			

				wchar_t* lpValue = (wchar_t*)npMsg->GetInputData();

				luResult = OnChangeBackImagePic(lpValue);
				break;
			}
		case EACT_LIST_ITEMCLICK:
		case EACT_LIST_ITEMCLICK_RBUTTON:
		case EEVT_MENUITEM_CLICK:
			{
				//发向父窗口
				mpIterator->PostMessageToParent(npMsg);
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DOCSCROLL:
			{
				if(npMsg->GetInputDataSize() != sizeof(FLOAT))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				if(mpVScrollBar->GetIterator()->IsEnable() == false || mpVScrollBar->GetIterator()->IsVisible() == false)
					break;
				FLOAT* lpValue = (FLOAT*)npMsg->GetInputData();
				FLOAT lTempPos = *lpValue;
				if(lTempPos > (mlDocumentHeight - mpIterator->GetSizeY()))
					lTempPos = mlDocumentHeight - mpIterator->GetSizeY();
				else if(lTempPos < 0.0f)
					lTempPos = 0.0f;
				mlDocToView0PositionY = lTempPos;
				CalcElementPosition();
				CheckVScrollBarNeedToShow();
				//需要设置滚动条的滑块位置
				if(mpVScrollBar)
					EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpVScrollBar->GetIterator(),
					EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION,&mlDocToView0PositionY,sizeof(FLOAT),NULL,0);
				luResult = ERESULT_SUCCESS;

				break;
			}
		case EACT_LIST_GET_SCROLL_RANGE:
			{
				FLOAT lfScrollRange = mlDocumentHeight - mpIterator->GetSizeY();
				if(npMsg->GetOutputBufferSize() == sizeof(FLOAT) && npMsg->GetOutputBuffer() != NULL)
				{
					//RtlCopyMemory(npMsg->GetOutputBuffer(),&lfScrollRange,sizeof(FLOAT));
					*(FLOAT*)npMsg->GetOutputBuffer() = lfScrollRange;
					luResult = ERESULT_SUCCESS;
				}
				else
				{
					luResult = ERESULT_WRONG_PARAMETERS;
				}
				//npMsg->SetOutputBuffer(&lfScrollRange,sizeof(FLOAT));

				break;
			}
		case EACT_LIST_GET_SCROLL_POSITION:
			{
				FLOAT lfScrollPosition = mlDocToView0PositionY;

				if(npMsg->GetOutputBufferSize() == sizeof(FLOAT) && npMsg->GetOutputBuffer() != NULL)
				{
					//RtlCopyMemory(npMsg->GetOutputBuffer(),&lfScrollPosition,sizeof(FLOAT));
					*(FLOAT*)npMsg->GetOutputBuffer() = lfScrollPosition;
					luResult = ERESULT_SUCCESS;
				}
				else
				{
					luResult = ERESULT_WRONG_PARAMETERS;
				}
				break;
			}	
		case EACT_LIST_ADD_ELEMENT:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();
				bool bRet = AddElement(lpElement,mpElementVector.Size());
				
				/*if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;*/
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_ADD_ELEMENT_HEAD:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();
				bool bRet = AddElement(lpElement,0);
				npMsg->SetOutputDataSize(sizeof(bool));
				//npMsg->SetOutputBuffer(&bRet,sizeof(bool));
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DELETE_ELEMENT_NO_CLOSE:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();
				bool bRet = false;
				for(int i = 0; i < mpElementVector.Size();i++)
				{
					if(lpElement == mpElementVector[i])
					{
						bRet = DeleteElement(i,false);
						break;
					}
				}
				npMsg->SetOutputDataSize(sizeof(bool));
				//npMsg->SetOutputBuffer(&bRet,sizeof(bool));
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DELETE_ELEMENT:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();
				bool bRet = DeleteElement(lpElement);
				npMsg->SetOutputDataSize(sizeof(bool));
				//npMsg->SetOutputBuffer(&bRet,sizeof(bool));
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;

				
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DELETE_ELEMENT_INDEX:
			{
				if(npMsg->GetInputDataSize() != sizeof(long))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				long* lpValue = (long*)npMsg->GetInputData();
				bool bRet = DeleteElement(*lpValue);
				npMsg->SetOutputDataSize(sizeof(bool));				
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DELETE_ELEMENT_BIGGER_OR_EQUAL_INDEX_NO_CLOSE:
			{

				if(npMsg->GetInputDataSize() != sizeof(long))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				long* lpValue = (long*)npMsg->GetInputData();
				bool bRet = false;
				for(int i = mpElementVector.Size()-1;  i >= *lpValue;i--)
					bRet = DeleteElement(i,false);

				npMsg->SetOutputDataSize(sizeof(bool));				
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_DELETE_ELEMENT_BIGGER_OR_EQUAL_INDEX:
			{
				if(npMsg->GetInputDataSize() != sizeof(long))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				long* lpValue = (long*)npMsg->GetInputData();
				bool bRet = false;
				for(int i = mpElementVector.Size()-1;  i >= *lpValue;i--)
					bRet = DeleteElement(i);
				
				npMsg->SetOutputDataSize(sizeof(bool));				
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;
				break;
			}
		case  EACT_LIST_RESET:
			{
				ResetList();
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_SET_STYLE:
			{
				if(npMsg->GetInputDataSize() != sizeof(LONG))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				LONG* lpValue = (LONG*)npMsg->GetInputData();
				SetListViewStyle(*lpValue);
				luResult = ERESULT_SUCCESS;
			}
		case EEVT_BUTTON_CLICK:
			{
				//向上转发
				if(npMsg->IsSent())
				{					
					if(mpIterator->GetParent())
						mpIterator->GetParent()->SendMessageW(npMsg);
				}
				else
					mpIterator->GetParent()->PostMessageW(npMsg);
				//mpIterator->PostMessageToParent(npMsg);
			}
			luResult = ERESULT_SUCCESS;
			break;
		case EACT_LIST_SET_VSCROLLBAR_RECT:
			{
				if(npMsg->GetInputDataSize() != sizeof(D2D1_RECT_F))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				D2D1_RECT_F* lpRect = (D2D1_RECT_F*)npMsg->GetInputData();
				if(lpRect && mpVScrollBar)
				{
					luResult = ERESULT_SUCCESS;
					FLOAT lfScrollBarWidth = mpVScrollBar->GetVScrollBarWidth() ;
					FLOAT lfX = lpRect->left;
					FLOAT lfY = lpRect->top;
					mpVScrollBar->GetIterator()->SetPosition(lpRect->left,lpRect->top );
					mpVScrollBar->GetIterator()->SetSize(lfScrollBarWidth,lpRect->bottom - lpRect->top);
					mpVScrollBar->Relocation();
				}				
				luResult = ERESULT_SUCCESS;
				break;
			}
		case EACT_LIST_GET_ITEM_POSITION:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();

				break;
			}
		case EACT_LIST_GET_ITEM_INDEX:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				if(npMsg->GetOutputBufferSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator* lpElement = *(IEinkuiIterator**)npMsg->GetInputData();
				int i = 0;
				for(i = 0 ; i < mpElementVector.Size();i++)
				{
					if(lpElement == mpElementVector[i])
						break;
				}
				// 设置输出数据
				int* lpOut = (int*)npMsg->GetOutputBuffer();
				*lpOut = i;
				npMsg->SetOutputDataSize(sizeof(int));
				break;
			}
		case EACT_LIST_GET_ITEM_NUM:
			{
				
				if(npMsg->GetOutputBufferSize() != sizeof(int))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}				
				int lnNum = mpElementVector.Size();
				
				// 设置输出数据
				int* lpOut = (int*)npMsg->GetOutputBuffer();
				*lpOut = lnNum;
				npMsg->SetOutputDataSize(sizeof(int));
				break;
			}
		case EACT_LIST_RECACULATE:
			{
				
				Recaculate();
				
				luResult = ERESULT_SUCCESS;
				break;
				
			}
		case EACT_LIST_INSER_ITEM_INDEX:
			{
				_STCTL_LIST_INSERT *lIns = NULL;
				if(ERESULT_SUCCESS != CExMessage::GetInputDataBuffer(npMsg, lIns))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				BREAK_ON_NULL(lIns);
				bool bRet = AddElement(lIns->mpElement,lIns->mnIndex);

				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = bRet;
				luResult = ERESULT_SUCCESS;				
				break;
			}
		case EACT_LIST_SWAP_ELEMENT:
			{
				if(npMsg->GetInputDataSize() != sizeof(_STCTL_LIST_SWAP*) )
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				_STCTL_LIST_SWAP *lSwap = (_STCTL_LIST_SWAP*)npMsg->GetInputData();
				BREAK_ON_NULL(lSwap);
				BREAK_ON_NULL(lSwap->mpFromElement);
				BREAK_ON_NULL(lSwap->mpToElement);
				int lPos1,lPos2;
				lPos2 = lPos1 = -1;
				for(int i = 0;i < mpElementVector.Size();i++)
				{
					if(mpElementVector[i] == lSwap->mpFromElement)
						lPos1 = i;
					if(mpElementVector[i] == lSwap->mpToElement)
						lPos2 = i;
				}

				if(lPos2 == -1 || lPos1 == -1)
					break;
				mpElementVector[lPos1] = lSwap->mpToElement;
				mpElementVector[lPos2] = lSwap->mpFromElement;
				CalcElementPosition();
				
				bool lbRet = true;
				if(npMsg->GetOutputBufferSize() == sizeof(bool))
					*(bool*)npMsg->GetOutputBuffer() = lbRet;
				luResult = ERESULT_SUCCESS;		
				break;
			}
		case EACT_LIST_SET_ACCEPT_DROP_ITEM:
			{
				if(npMsg->GetInputDataSize() != sizeof(bool) )
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				bool *lbAccept = (bool*)npMsg->GetInputData();
				mbAcceptDrop = *lbAccept;
				if(mbAcceptDrop)
					mpIterator->ModifyStyles(EITR_STYLE_XUIDRAGDROP);
				else
					mpIterator->ModifyStyles(0,EITR_STYLE_XUIDRAGDROP);
				luResult = ERESULT_SUCCESS;	
				break;
			}
		case EMSG_DRAGDROP_ENTER://Drag进入
			{
				
				break;
			}
		case EMSG_DRAGDROP_LEAVE://drag离开
			{
				OutputDebugString(L"Drop leave");
				if(mpIterInsertMark)
					mpIterInsertMark->SetVisible(false);
				break;
			}
		
		case EMSG_XUI_DROP_TEST:
			{	
			
				if(npMsg->GetInputData()==NULL || npMsg->GetInputDataSize()!=sizeof(STMS_EDRGDRP_REQUEST))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				PSTMS_EDRGDRP_REQUEST lpReq = (PSTMS_EDRGDRP_REQUEST)npMsg->GetInputData();

				if(mbAcceptDrop && IsEqualGUID(lpReq->Reason,EGUID_LIST_DROP_ITEM) != false )
				{				
					
					//mpIterator->SetAlpha(0.4);
					//检测矩形覆盖，显示插入标识
					int nIndex = CheckPos(lpReq->CurrentPos);
					if(nIndex != -1 && mpIterInsertMark)
					{						
						//mpIterInsertMark->ModifyStyles(EITR_STYLE_TOPDRAW);
						if(nIndex == mpElementVector.Size()) //如果达到最后
						{
							IEinkuiIterator * lpLastIte = mpElementVector[nIndex-1];
							mpIterInsertMark->SetPosition(lpLastIte->GetPositionX(),
								lpLastIte->GetPositionY() + lpLastIte->GetSizeY() - 6);
						}
						else
							mpIterInsertMark->SetPosition(mpElementVector[nIndex]->GetPositionX() ,
									mpElementVector[nIndex]->GetPositionY() - 6);
						mpIterInsertMark->SetSize(mpElementVector[nIndex]->GetSizeX(),mpIterInsertMark->GetSizeY());
						mpIterInsertMark->SetVisible(true);
						luResult = ERESULT_EDRAGDROP_ACCEPT;
					}
				}					
					
			
				break;
			}
		case EMSG_XUI_DROP_DOWN:
			{
				OutputDebugString(L"Drop down");
				if(mpIterInsertMark)
					mpIterInsertMark->SetVisible(false);
				//目标在自己身上落下
				if(npMsg->GetInputData()==NULL || npMsg->GetInputDataSize()!=sizeof(STMS_EDRGDRP_REQUEST))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				PSTMS_EDRGDRP_REQUEST lpReq = (PSTMS_EDRGDRP_REQUEST)npMsg->GetInputData();

				if(IsEqualGUID(lpReq->Reason,EGUID_LIST_DROP_ITEM)==false/* || lpReq->Flags != ICON_TYPE_WIDGET*/)
					break;

				break;
			}
		case EACT_LIST_SET_DRAG_FOR_MOUSER_ITERATOR:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				mpIterPicMoveForMouse = *(IEinkuiIterator**)npMsg->GetInputData();
				//if(mpIterPicMoveForMouse)
				//	mpIterPicMoveForMouse->ModifyStyles(EITR_STYLE_TOPDRAW);
				luResult = ERESULT_SUCCESS;	
				break;
			}
		case EACT_LIST_SET_DROP_MARK_ITERATOR:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				mpIterInsertMark = *(IEinkuiIterator**)npMsg->GetInputData();
				//if(mpIterInsertMark)
				//	mpIterInsertMark->ModifyStyles(EITR_STYLE_TOPDRAW);
				luResult = ERESULT_SUCCESS;	
				break;
			}	
		case EACT_LIST_DRAG_ITEM_START:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}

				mpIterMoveItem = *(IEinkuiIterator**)npMsg->GetInputData();
				if(mpIterMoveItem)
					mpIterMoveItem->SetAlpha(0.5);
				
				luResult = ERESULT_SUCCESS;	
				break;
			}
		case EACT_LIST_DRAG_ITEM_DRAGING:
			{

				if(npMsg->GetInputDataSize() != sizeof(STMS_DRAGGING_ELE))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				STMS_DRAGGING_ELE * lpDrageEle = (STMS_DRAGGING_ELE*)npMsg->GetInputData();
				if(lpDrageEle && mpIterPicMoveForMouse)
				{
					mpIterPicMoveForMouse->SetVisible(true);
					mpIterPicMoveForMouse->SetPosition(lpDrageEle->CurrentPos.x,
						lpDrageEle->CurrentPos.y);					
				}
			

				luResult = ERESULT_SUCCESS;	
				break;
			}
		case EACT_LIST_DRAG_ITEM_END:
			{
				if(npMsg->GetInputDataSize() != sizeof(IEinkuiIterator*))
				{
					luResult = ERESULT_WRONG_PARAMETERS;
					break;
				}
				IEinkuiIterator * lpIte = *(IEinkuiIterator**)npMsg->GetInputData();
				if(lpIte)
					lpIte->SetAlpha(1.0);
				if(mpIterPicMoveForMouse)
					mpIterPicMoveForMouse->SetVisible(false);
			
				luResult = ERESULT_SUCCESS;	
				break;
			}
		case EACT_LIST_SHOW_BY_INDEX:
			{
				////把指定项移动到可显示区域
				LONG llIndex = -1;
				if(CExMessage::GetInputData(npMsg,llIndex) != ERESULT_SUCCESS)
					break;

				ShowByIndex(llIndex);

				break;
			}
		default:
			luResult = ERESULT_NOT_SET;
			break;
		}

		if(luResult == ERESULT_NOT_SET)
		{
			luResult = CXuiElement::ParseMessage(npMsg); // 调用基类的同名函数；注意：一定要调用自身直接基类
		}

	} while (false);

	return luResult;
}

//绘制
ERESULT CEvList::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{

	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
		{
			float lfX = mpIterator->GetSizeX() * mlCurrentIndex;

			npPaintBoard->DrawBitmap(D2D1::RectF(0,0,mpIterator->GetSizeX(),mpIterator->GetSizeY()),
				D2D1::RectF(lfX,0,lfX + (float)mpBgBitmap->GetWidth(),(float)mpBgBitmap->GetHeight()),
				mpBgBitmap,
				ESPB_DRAWBMP_EXTEND
				);
		}

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}


//切换显示帧,第一帧为1
ERESULT CEvList::OnChangeBackImageIndex(LONG nlIndex)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if(nlIndex <= 0 || nlIndex > mlMaxFrame)
			break;

		mlCurrentIndex = nlIndex - 1;

		lResult = ERESULT_SUCCESS;

	}while(false);

	return lResult;
}

//更换显示图片
ERESULT CEvList::OnChangeBackImagePic(wchar_t* npswPicPath)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do 
	{
		if(npswPicPath == NULL || npswPicPath[0] == UNICODE_NULL)
			break;

		CMM_SAFE_RELEASE(mpBgBitmap);	//去除原来的图片

		mpBgBitmap = EinkuiGetSystem()->GetAllocator()->LoadImageFile(npswPicPath);
		BREAK_ON_NULL(mpBgBitmap);

		mlMaxFrame = 1;	//目前只支持切换为一帧的图

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

// 鼠标落点检测
ERESULT CEvList::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do 
	{
		if(rPoint.x < 0.0f || (UINT)rPoint.x >= mpIterator->GetSizeX() || 
			rPoint.y < 0.0f || (UINT)rPoint.y >= mpIterator->GetSizeY())
			break;

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}
bool CEvList::CheckVScrollBarNeedToShow()
{
	bool lbRet = false;
	if(mlShowStyle == LIST_VIEW_STYLE_AUTO_FIT_Y || mlShowStyle == LIST_VIEW_STYLE_AUTO_FIT_X)
	{
		if(mpVScrollBar) mpVScrollBar->GetIterator()->SetVisible(false);
		//if(mp)
		return true;
	}


	FLOAT lfDeata = mlDocumentHeight - mpIterator->GetSizeY();

	if(lfDeata > 0)
	{
		if(mpVScrollBar)
		{
			if (mpTemplete->QuerySubKeyValueAsLONG(L"ShowOrDisable",0) == 0)
			{
				if(!mpVScrollBar->GetIterator()->IsEnable())
				{
					mpVScrollBar->GetIterator()->SetEnable(true);			
				}
			}
			else
			{
				if(!mpVScrollBar->GetIterator()->IsVisible())
				{
					mpVScrollBar->GetIterator()->SetVisible(true);
					bool lbShow = true;
					EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpIterator->GetParent(),
						EACT_LIST_VSCROLLBAR_SHOW,&lbShow,sizeof(bool),NULL,0);
				}
			}

			lbRet = true;
			mpVScrollBar->SetDeltaSize(lfDeata);
		}
	}
	else
	{
		if(mpVScrollBar)
		{
			if (mpTemplete->QuerySubKeyValueAsLONG(L"ShowOrDisable",0) == 0)
			{
				if(mpVScrollBar->GetIterator()->IsEnable())
				{
					mpVScrollBar->GetIterator()->SetEnable(false);				
				}
			}
			else
			{
				if(mpVScrollBar->GetIterator()->IsVisible())
				{
					mpVScrollBar->GetIterator()->SetVisible(false);
					bool lbShow = false;
					EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpIterator->GetParent(),
						EACT_LIST_VSCROLLBAR_SHOW,&lbShow,sizeof(bool),NULL,0);
				}
			}

			lbRet = true;
		}
	}

	return lbRet;
}
//根据位置，计算成员变量的位置
bool CEvList::CalcElementPosition()
{
	bool lbRet = false;
	if(mlShowStyle == LIST_VIEW_STYLE_REPORT && mpElementVector.Size() > 0)
	{
		IEinkuiIterator * lpIterator = NULL;
		IEinkuiIterator * lpPreIterator = NULL;

		//位置从-mlDocToView0PositionY开始
		FLOAT lfPositionY = -mlDocToView0PositionY; 
		mlDocumentHeight = 0.0f;
		for(int i = 0; i < mpElementVector.Size();i++)
		{
			lpIterator = mpElementVector[i];
			if(lpIterator && lpIterator->IsVisible())
			{
				mlDocumentHeight += lpIterator->GetSizeY();
				//如果前一个不会空，则当前的位置为（前一个PositionY） + （前一个sizeY）			
				if(lpPreIterator)
					lfPositionY += lpPreIterator->GetSizeY();

				lpIterator->SetPosition(0,lfPositionY);
				lpPreIterator = lpIterator;
			}

		}
	}
	else if(mlShowStyle == LIST_VIEW_STYLE_SMALLITEM && mpElementVector.Size() > 0)
	{
		//获取View的size ，需要减去滚动条的宽度
		FLOAT lfViewSizeX = mpIterator->GetSizeX();
		FLOAT lfViewSizeY = mpIterator->GetSizeY();
		//一行一行排布，得统计一行的最大高度，方便计算docmentHeigth
		FLOAT lfCurrentLineMaxHeigth = 0;
		FLOAT lfCurrentLinePositionX = 0;
		FLOAT lfCurrentLinePositionY = -mlDocToView0PositionY;
		FLOAT lfCurrentLineUsedX = 0; //临时变量，记录一行已经用了多少
		IEinkuiIterator * lpIterator = NULL;
		IEinkuiIterator * lpPreIterator = NULL;
		mlDocumentHeight = 0.0;
		for(int i = 0; i < mpElementVector.Size();i++)
		{
			lpIterator = mpElementVector[i];
			if(lpIterator && lpIterator->IsVisible())
			{
				//看看能不能排在当前行，不行就换行
				bool lbIsChangeLine = false;
				if(lfViewSizeX - lfCurrentLineUsedX < lpIterator->GetSizeX())
				{
					//如果不行就要换行，重新设置当前行变量
					mlDocumentHeight += lfCurrentLineMaxHeigth;
					lfCurrentLineUsedX = 0;
					lfCurrentLinePositionX = 0;
					lfCurrentLinePositionY+= lfCurrentLineMaxHeigth;
					lfCurrentLineMaxHeigth = 0;
					lbIsChangeLine = true;

				}

				//重新check 看看能不能放入当前行，或者新的行，
				//如果这都不行，说明这个element的宽度超出view的宽度，不予处理
				if(lfViewSizeX - lfCurrentLineUsedX >= lpIterator->GetSizeX())
				{
					//可以放在当前行,修改当前行变量
					lfCurrentLineUsedX += lpIterator->GetSizeX();
					if(lfCurrentLineMaxHeigth < lpIterator->GetSizeY())
						lfCurrentLineMaxHeigth = lpIterator->GetSizeY();

					if(!lbIsChangeLine && lpPreIterator)
						lfCurrentLinePositionX += lpPreIterator->GetSizeX();
					lpIterator->SetPosition(lfCurrentLinePositionX,lfCurrentLinePositionY);

				}			
			}//if(lpIterator)
			lpPreIterator = lpIterator;
		}//for
		//加上最后一行的高度
		if(lpIterator)
		   mlDocumentHeight += lfCurrentLineMaxHeigth;
	}
	else if(mlShowStyle == LIST_VIEW_STYLE_AUTO_FIT_Y && mpElementVector.Size() > 0)
	{
		IEinkuiIterator * lpIterator = NULL;
		IEinkuiIterator * lpPreIterator = NULL;

		//位置从-mlDocToView0PositionY开始
		FLOAT lfPositionY = 0; 
		mlDocumentHeight = 0.0;
		FLOAT lfMaxSizeX = 0;
		for(int i = 0; i < mpElementVector.Size();i++)
		{
			lpIterator = mpElementVector[i];
			if(lpIterator && lpIterator->IsVisible())
			{
				mlDocumentHeight += lpIterator->GetSizeY();
				//获取最大宽度
				if(lpIterator->GetSizeX() > lfMaxSizeX)
					lfMaxSizeX = lpIterator->GetSizeX();
				//如果前一个不会空，则当前的位置为（前一个PositionY） + （前一个sizeY）			
				if(lpPreIterator)
					lfPositionY += lpPreIterator->GetSizeY();

				lpIterator->SetPosition(0,lfPositionY);
				lpPreIterator = lpIterator;
			}
		}
		mpIterator->SetSize(lfMaxSizeX,mlDocumentHeight);
	}
	return lbRet;
}
//公开操作
//设置显示模式
bool CEvList::SetListViewStyle(LONG nlShowStyle)
{
	mlShowStyle = nlShowStyle;
	return true;
}
//设置是否显示横向和纵向的滚动条
bool CEvList::ShowScrollBar(bool nbShowVer,bool nbShowHor)
{
	mbShowHorScroll = nbShowHor;
	mbShowVerScroll = nbShowVer;
	return true;
}

//添加一个元素到List容器，list容器将负责释放该元素
bool CEvList::AddElement(IEinkuiIterator * npElement,int nIndex ) 
{
	if(npElement == NULL)
		return false;

	bool lbRet = false;
	mpElementVector.Insert(nIndex,npElement);
	lbRet = CalcElementPosition();
	lbRet = CheckVScrollBarNeedToShow();
	
	return lbRet;
}

//清空List，将释放原有的元素
bool CEvList::ResetList()
{
	//释放对应的内存
	if(mbNeedMemoryManager)
	{
		for(int i = 0; i < mpElementVector.Size();i++)
		{
			mpElementVector[i]->Close();
			//mpElementVector[i]->Release();
		}
	}

	mpElementVector.Clear();
	mlDocumentHeight = mlDocumentWidth = 0;
	//隐藏滚动条
	CheckVScrollBarNeedToShow();
	if(mlShowStyle == LIST_VIEW_STYLE_AUTO_FIT_Y )
		mpIterator->SetSize(mpIterator->GetSizeX(),0.0f);
	if(mlShowStyle == LIST_VIEW_STYLE_AUTO_FIT_X)
		mpIterator->SetSize(0.0f,mpIterator->GetSizeY());

	return true;
}

//根据索引删除元素
bool CEvList::DeleteElement(LONG nlIndex,bool nbClostElement)
{
	bool lbRet = false;
	if(nlIndex >= 0 && nlIndex < mpElementVector.Size())
	{
		FLOAT lfElementHeigth = mpElementVector[nlIndex]->GetSizeY();

		if(nbClostElement)
			mpElementVector[nlIndex]->Close();
		

		lbRet = mpElementVector.RemoveByIndex(nlIndex);
		if(lbRet)
		{
			//减少Doc高度			
			lbRet = CalcElementPosition();
		}

		//check docunment的高度,设置滚动范围
		lbRet = CheckVScrollBarNeedToShow();

	}

	return lbRet;
}

//根据元素指针删除元素
bool CEvList::DeleteElement(IEinkuiIterator * npElement)
{
	int liShow = 0;
	for(int i = 0; i < mpElementVector.Size();i++)
	{
		if(npElement == mpElementVector[i])
		{
			DeleteElement(i);
			liShow = i-2;
			ShowByIndex(liShow>0?liShow:0);
			return true;
		}
	}

	return false;
}

//鼠标滑轮
ERESULT CEvList::OnMouseWheel(const STEMS_MOUSE_WHEEL* npInfo)
{
	if(npInfo && mpVScrollBar->GetIterator()->IsVisible() && mpElementVector.Size() != 0)
	{	
		float lfPosition = -mlDocToView0PositionY;
		float lfOffset = ( (FLOAT)npInfo->Delta * (mlDocumentHeight/mpElementVector.Size()) )/120.0f;
	    //为了统一，只滚动20个像素
		lfPosition += (lfOffset < 0 ? -LIST_MIN_WHEEL_OFFSET :LIST_MIN_WHEEL_OFFSET);
		if(lfPosition <= -(mlDocumentHeight - mpIterator->GetSizeY()))
			lfPosition = -(mlDocumentHeight - mpIterator->GetSizeY());
		if(lfPosition > 0.0f)
			lfPosition = 0.0f;

		mlDocToView0PositionY = -lfPosition;
		/*TCHAR tcBuf[MAX_PATH]= {0};
		swprintf(tcBuf,L"mldoctovir %f i = %f",mlDocToView0PositionY,mlDocumentHeight);
		OutputDebugString(tcBuf);*/
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpIterator,
			EACT_LIST_DOCSCROLL,
			&mlDocToView0PositionY,
			sizeof(float),NULL,0);
	}
	return ERESULT_SUCCESS;
}


//键盘消息
ERESULT CEvList::OnKeyPressed(const STEMS_KEY_PRESSED* npInfo)
{
	ERESULT luResult = ERESULT_SUCCESS;

	bool lbSet = false;
	float lfPosition = -mlDocToView0PositionY;
	switch (npInfo->VirtualKeyCode )
	{
	case VK_LEFT:		
		break;
	case VK_RIGHT:		
		break;
	case VK_UP:	
		if(mpVScrollBar->GetIterator()->IsVisible()
			&& mpElementVector.Size() != 0 
			&& npInfo->IsPressDown)
		{
			lfPosition += mlDocumentHeight/(mpElementVector.Size());
			lbSet = true;
		}
		break;
	case VK_DOWN:
		if(mpVScrollBar->GetIterator()->IsVisible()
			&& mpElementVector.Size() != 0
			&& npInfo->IsPressDown )
		{
			lfPosition -=  mlDocumentHeight/(mpElementVector.Size());
			lbSet = true;
		}
		break;

	default:
		luResult = ERESULT_SUCCESS;
	}
	if(lbSet)
	{	
	
		if(lfPosition <= -(mlDocumentHeight - mpIterator->GetSizeY()))
			lfPosition = -(mlDocumentHeight - mpIterator->GetSizeY());
		if(lfPosition > 0.0f)
			lfPosition = 0.0f;

		mlDocToView0PositionY = -lfPosition;
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
			mpIterator,
			EACT_LIST_DOCSCROLL,
			&mlDocToView0PositionY,
			sizeof(float),NULL,0);
	}

	return luResult;
}

//
////字符输入消息
//ERESULT CEvList::OnChar(const PSTEMS_CHAR_INPUT npChar)
//{
//	ERESULT luResult;
//
//	bool lbSet = false;
//	float lfPosition = -mlDocToView0PositionY;
//	//判断
//	if(npChar->CharIn == VK_UP)
//	{
//		if(mpVScrollBar->GetIterator()->IsVisible()
//			&& mpElementVector.Size() != 0 
//			)
//		{
//			lfPosition += mlDocumentHeight/(mpElementVector.Size());
//			lbSet = true;
//		}
//	}
//	else if(npChar->CharIn == VK_DOWN)
//	{
//		if(mpVScrollBar->GetIterator()->IsVisible()
//			&& mpElementVector.Size() != 0
//			)
//		{
//			lfPosition -=  mlDocumentHeight/(mpElementVector.Size());
//			lbSet = true;
//		}
//	}
//	if(lbSet)
//	{	
//
//		if(lfPosition <= -(mlDocumentHeight - mpIterator->GetSizeY()))
//			lfPosition = -(mlDocumentHeight - mpIterator->GetSizeY());
//		if(lfPosition > 0.0f)
//			lfPosition = 0.0f;
//
//		mlDocToView0PositionY = -lfPosition;
//		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(
//			mpIterator,
//			EACT_LIST_DOCSCROLL,
//			&mlDocToView0PositionY,
//			sizeof(float),NULL,0);
//	}
//	return ERESULT_SUCCESS;
//}

int CEvList::CheckPos(D2D1_POINT_2F nPos)
{
	int lnIndex = -1;
	do 
	{
		D2D1_RECT_F lRect;
		//先check落点，落在那个element上
		for(int i = 0;i < mpElementVector.Size();i++)
		{			
			mpElementVector[i]->GetRect(lRect);
			if(nPos.x > lRect.left && nPos.x < lRect.right
				&& nPos.y > lRect.top && nPos.y < lRect.bottom)
			{
				lnIndex = i;
				break;
			}
		}

		//check 是否落在下半部分，如果下半部分，则Index+1
		if( lnIndex != -1 && nPos.y > lRect.top + (lRect.bottom - lRect.top)/2)
			lnIndex += 1;

	} while (false);
	return lnIndex;
}

////把指定项移动到可显示区域
void CEvList::ShowByIndex(LONG nlIndex)
{
	IEinkuiIterator* lpItem = NULL;

	do 
	{
		if(nlIndex < 0 || nlIndex >= mpElementVector.Size())
			break;	//不合法的序号

		lpItem = mpElementVector.GetEntry(nlIndex);
		BREAK_ON_NULL(lpItem);

		if(lpItem->GetPositionX() >= 0.0f 
			&& lpItem->GetPositionY() >= 0.0f 
			&& (lpItem->GetPositionX()+lpItem->GetSizeX()) <= mpIterator->GetSizeX()
			&& (lpItem->GetPositionY()+lpItem->GetSizeY()) <= mpIterator->GetSizeY())
			break;	//已经在显示区域内了

		D2D1_POINT_2F ldOldPos = lpItem->GetPosition();
		D2D1_POINT_2F ldNewPos = ldOldPos;
		if (ldOldPos.y < 0.0f)
		{
			//在显示区域上面
			ldNewPos.y = 0.0f;
		}
		else
		{
			//在显示区域下面
			ldNewPos.y = mpIterator->GetSizeY() - lpItem->GetSizeY();
		}

		
		mlDocToView0PositionY -= (ldNewPos.y - ldOldPos.y);

		//重新定位元素位置
		CalcElementPosition();

		//需要设置滚动条的滑块位置
		if(mpVScrollBar)
			EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpVScrollBar->GetIterator(),
			EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION,&mlDocToView0PositionY,sizeof(FLOAT),NULL,0);

	} while (false);
}

//重新定位位置
void CEvList::Recaculate(void)
{
	CalcElementPosition();
	CheckVScrollBarNeedToShow();
	if(mpVScrollBar )
	{
		if(mpVScrollBar->GetIterator()->IsEnable())
		{
			//检测是否需要设置Postion位置，因为变动后，原有的位置超出了滑动范围
			if( ( mlDocToView0PositionY + mpIterator->GetSizeY()) > mlDocumentHeight )
				mlDocToView0PositionY = mlDocumentHeight - mpIterator->GetSizeY();	
		}
		else
			mlDocToView0PositionY = 0.0f;
		EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpVScrollBar->GetIterator(),
			EACT_SCROLLBAR_VSCROLL_SET_SLIDER_POSTION,&mlDocToView0PositionY,sizeof(FLOAT),NULL,0);

	}
	CalcElementPosition();
	//需返回给父窗口，链表大小的改变
	//SendMessageToParent(EACT_LIST_LAYOUT_CHANGE,NULL);
	EinkuiGetSystem()->GetElementManager()->SimpleSendMessage(mpIterator->GetParent(),
		EACT_LIST_LAYOUT_CHANGE,NULL,0,NULL,0);
}