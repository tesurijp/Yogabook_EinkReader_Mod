#include "stdafx.h"
#include "cmmBaseObj.h"
#include "Einkui.h"
#include "XCtl.h"
#include "ElementImp.h"
#include "EvButtonImp.h"
#include "EvCheckButtonImp.h"


//using namespace D2D1;
DEFINE_BUILTIN_NAME(CheckButton)

// 只用于变量设置初始值，如指针设为NULL，所有可能失败的如分配之类的运算都应该在InitOnCreate中进行
CEvCheckButton::CEvCheckButton()
{

}

// 用于释放成员对象
CEvCheckButton::~CEvCheckButton()
{

}

ULONG CEvCheckButton::InitOnCreate(
	IN IEinkuiIterator* npParent,	// 父对象指针
	IN ICfKey* npTemplete,		// npTemplete的Key ID就是EID，值就是类型EType
	IN ULONG nuEID	// 如果不为0和MAXULONG32，则指定该元素的EID; 否则，取上一个参数的模板内设置的值作为EID，如果模板也没有设置EID，则使用XUI系统自动分配
)
{
	ERESULT leResult = ERESULT_UNSUCCESSFUL;

	do
	{
		//首先调用基类
		leResult = CXuiElement::InitOnCreate(npParent, npTemplete, nuEID);
		if (leResult != ERESULT_SUCCESS)
			break;

		//设置自己的类型
		mpIterator->ModifyStyles(/*EITR_STYLE_CONTROL|*/EITR_STYLE_DRAG);

		LoadResource();

		leResult = ERESULT_SUCCESS;

	} while (false);

	return leResult;
}

//初始建立，当一个元素被建立时调用，注意：子元素会先于父元素收到这条消息，从而确保父元素有一个在子元素初始化之后完成全部初始化的机会
ERESULT CEvCheckButton::OnElementCreate(IEinkuiIterator* npIterator)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		if (CXuiElement::OnElementCreate(npIterator) != ERESULT_SUCCESS)
			break;

		//计算文字位置
		if (mpBgBitmap != NULL && mpTextBitmap != NULL)
		{
			mdTextDestRect.left = mdFrameSize.width + 5.0f;
			int liTemp = mpTextBitmap->GetHeight();
			mdTextDestRect.top = (mdFrameSize.width - mpTextBitmap->GetHeight()) / 2.0f - 1.0f;
			mdTextDestRect.right = mdTextDestRect.left + mpTextBitmap->GetWidth();
			mdTextDestRect.bottom = mdTextDestRect.top + mpTextBitmap->GetHeight();

			//设置感应区域
			mdAcionSize.width = mdTextDestRect.right;
			mdAcionSize.height = mdTextDestRect.bottom;
		}

		if (mfTextTop - 1.0f >= 0.5f)
			mdTextDestRect.top = mfTextTop;

		ICfKey* lpCheckedKey = mpTemplete->OpenKey(TF_ID_CHECK_BT_CHECKED);
		if (lpCheckedKey != NULL)
		{
			//默认选中
			SetChecked(true);
			CMM_SAFE_RELEASE(lpCheckedKey);
		}

		lResult = ERESULT_SUCCESS;
	} while (false);

	return lResult;
}


//绘制
ERESULT CEvCheckButton::OnPaint(IEinkuiPaintBoard* npPaintBoard)
{
	ERESULT lResult = ERESULT_UNSUCCESSFUL;

	do
	{
		BREAK_ON_NULL(npPaintBoard);

		if (mpBgBitmap != NULL)
		{
			//绘制背景图
			float lfX = 0;
			ULONG lulMethod = ESPB_DRAWBMP_LINEAR;
			LONG llIndex = GetCurrentStatesArrayIndex();		//获取当前状态图片信息所在的数组下标

			if (llIndex != -1)
			{
				lfX = (mdArrayFrame[llIndex].Index + mlCurrentPage) * mdFrameSize.width; //从哪个位置开始显示

				npPaintBoard->DrawBitmap(D2D1::RectF(0.0f, 0.0f, mpIterator->GetSizeX(), mpIterator->GetSizeY()),
					D2D1::RectF(lfX, 0, lfX + mdFrameSize.width, mdFrameSize.height),
					mpBgBitmap,
					ESPB_DRAWBMP_EXTEND
				);
			}
		}

		if (mpTextBitmap != NULL)
		{
			//绘制文字
			FLOAT lfValue = 0.0f;
			npPaintBoard->DrawBitmap(D2D1::RectF(mdTextDestRect.left + lfValue, mdTextDestRect.top + lfValue, mdTextDestRect.right + lfValue, mdTextDestRect.bottom + lfValue),
				mpTextBitmap,
				ESPB_DRAWBMP_NEAREST
			);
		}

		lResult = ERESULT_SUCCESS;

	} while (false);

	return lResult;
}


// 鼠标落点检测
ERESULT CEvCheckButton::OnMouseOwnerTest(const D2D1_POINT_2F& rPoint)
{
	ERESULT luResult = ERESULT_SUCCESS;

	do
	{
		if (mpIterator->IsVisible() == false)
			break;


		if (mdAcionSize.width > 1.0f && mdAcionSize.height > 1.0f)
		{
			//如果有感应区，就以感应区为主
			if (rPoint.x < 0.0f || rPoint.x >= mdAcionSize.width || rPoint.y < 0.0f || rPoint.y >= mdAcionSize.height)
				break;
		}
		else if (mpBgBitmap != NULL)
		{
			//有背景图的时候
			if (rPoint.x < 0.0f || (UINT)rPoint.x >= mpIterator->GetSizeX() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpIterator->GetSizeY())
				break;

			float lfX = 0;
			LONG llIndex = GetCurrentStatesArrayIndex();		//获取当前状态图片信息所在的数组下标
			if (llIndex < 0)
				break;
			lfX = (mdArrayFrame[llIndex].Index + mlCurrentPage) * mdFrameSize.width;  	//从哪个位置开始显示

			D2D1_POINT_2F ldPoint = CExPoint::BigToOldPoint(mdFrameSize, mpIterator->GetSize(), D2D1::Point2(rPoint.x, rPoint.y), ESPB_DRAWBMP_EXTEND, D2D1::Point2((FLOAT)mpBgBitmap->GetExtnedLineX(), (FLOAT)mpBgBitmap->GetExtnedLineY()));
			//通过像素Alpha值检测????
			DWORD luPixel;
			if (ERESULT_SUCCEEDED(mpBgBitmap->GetPixel(DWORD(lfX + ldPoint.x), (DWORD)ldPoint.y, luPixel)))
			{
				if (luPixel != 1)
					break;
			}
		}
		else if (mpTextBitmap != NULL)
		{
			//只有文字的时候
			if (rPoint.x < 0.0f || (UINT)rPoint.x >= mpTextBitmap->GetWidth() || rPoint.y < 0.0f || (UINT)rPoint.y >= mpTextBitmap->GetHeight())
				break;
		}
		else
		{
			break;
		}

		luResult = ERESULT_MOUSE_OWNERSHIP;

	} while (false);

	return luResult;
}

//定位文字图片显示位置
void CEvCheckButton::RelocateText(void)
{

}