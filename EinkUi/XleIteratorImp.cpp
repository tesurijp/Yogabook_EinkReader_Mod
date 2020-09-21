#include "stdafx.h"

#include "CommonHeader.h"

#include "XleIteratorImp.h"




DEFINE_BUILTIN_NAME(CXuiIterator)
CXelManager* CXuiIterator::gpElementManager = NULL;
CExclusiveAccess CXuiIterator::goAttributesLock;


CXuiIterator::CXuiIterator()
{
	mpExtension = NULL;
	muEID = EID_INVALID;
	muStyle = EITR_STYLE_NONE;
	mpElement = NULL;
	mpParent = NULL;
	mdPosition.x = mdPosition.y = 0;
	mdSize.width = mdSize.height = 0;
	mlTabOrder = -1;
	mlZOrder = -1;

	mbInverted = false;
	// 设置向远处移动的矩阵，未绘制时鼠标判断出错
	mdWorldMatrix._11 = 1.0f; mdWorldMatrix._12 = 0.0f;
	mdWorldMatrix._21 = 0.0f; mdWorldMatrix._22 = 1.0f;
	mdWorldMatrix._31 = 1000000.0f; mdWorldMatrix._32 = 1000000.0f;
	mdWMInverted._11 = 1.0f;
	mdWMInverted._12 = 0.0f;
	mdWMInverted._21 = 0.0f;
	mdWMInverted._22 = 1.0f;
	mdWMInverted._31 = 0.0f;
	mdWMInverted._32 = 0.0f;

	cmmBaseObject::SetFlags(EITR_FLAG_VISIBLE, true);
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);
}

CXuiIterator::~CXuiIterator()
{
	if (mpExtension != NULL)
		delete mpExtension;

	if (cmmBaseObject::TestFlag(EITR_FLAG_HOTKEY) != false)
	{
		THotKeyTable* lpHtKyTable = NULL;
		moAtts.GetAttribute('htky', lpHtKyTable);
		CMM_SAFE_DELETE(lpHtKyTable);

	}

	CMM_SAFE_RELEASE(mpElement);
	//if(mpParent != NULL)	// 当对象从父对象移除时调用，而不是子对象释放时调用
	//{
	//	mpParent->KRelease();
	//	mpParent = NULL;
	//}
}

// 销毁元素，用于通知一个元素销毁，注意：父元素首先收到此消息，应该及时调用元素管理器的UnregisterElement方法，
// 从而触发元素管理器向所有下一层元素发送销毁消息，而后再将自己从元素管理器注销，并且释放自身对象
ERESULT CXuiIterator::OnElementDestroy()
{
	ClearTip();

	return ERESULT_SUCCESS;
}

// 启动Iterator，准备接受消息，调用这个方法后，Element首先会收到EMSG_CREATE消息，这个方法通常在Element的实例化函数退出前调用
void __stdcall CXuiIterator::Start(void)
{
	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_INIT) != false)
		return;	// 已经初始化了

	// 设置初始化完成标志，并且调用元素管理器的方法
	cmmBaseObject::SetFlags(EITR_FLAG_INIT);
	gpElementManager->StartMessageReceiver(this);
}

// 获得本Element的EID
ULONG __stdcall CXuiIterator::GetID(void)
{
	ITR_CHECK();
	return muEID;
}

// 获得本Element所属的Etype
const wchar_t* __stdcall CXuiIterator::GetType(void)
{
	ITR_CHECK();
	return mpElement->GetType();
}

// 获得父对象
IEinkuiIterator* __stdcall CXuiIterator::GetParent(void)
{
	ITR_CHECK();

	CMMASSERT(cmmBaseObject::TestFlag(EITR_FLAG_DELETED) == false);

	return mpParent;
}


// 获得本迭代器对应的Element对象
IXsElement* __stdcall CXuiIterator::GetElementObject(void)
{
	ITR_CHECK();
	return mpElement;
}

// 获得下一层的子对象的总数
int __stdcall CXuiIterator::GetSubElementCount(void)
{
	int liCount;

	ITR_CHECK();
	gpElementManager->LockIterators();

	if (mpExtension != NULL)
		liCount = mpExtension->moZOrder.Size();
	else
		liCount = 0;

	gpElementManager->UnlockIterators();

	return liCount;
}

// 询问某个Iterator在层次结构上是不是当前Iterator的祖先
bool __stdcall CXuiIterator::FindAncestor(const IEinkuiIterator* npIsAncestor)
{
	bool lbIsAncestor = false;
	CXuiIterator* lpItrObj;

	ITR_CHECK();

	if (npIsAncestor == this)
		return true;

	lpItrObj = this->mpParent;
	gpElementManager->LockIterators();

	// 到达根节点
	while (lpItrObj != lpItrObj->mpParent)
	{
		if (lpItrObj == npIsAncestor)
		{
			lbIsAncestor = true;
			break;
		}
		lpItrObj = lpItrObj->mpParent;
	}

	gpElementManager->UnlockIterators();

	return lbIsAncestor;
}

// 通过ZOder的排列次序获得子节点，返回的接口需要释放
IEinkuiIterator* __stdcall CXuiIterator::GetSubElementByZOder(
	int niPos	// zero base index value to indicate the position in z-order array
)
{
	IEinkuiIterator* lpReturn = NULL;

	ITR_CHECK();
	gpElementManager->LockIterators();

	if (mpExtension != NULL && mpExtension->moZOrder.Size() > 0 && niPos >= 0 && niPos < mpExtension->moZOrder.Size())
	{
		mpExtension->moZOrder[niPos]->AddRefer();
		lpReturn = mpExtension->moZOrder[niPos];
	}

	gpElementManager->UnlockIterators();

	return lpReturn;
}

// 通过ID获得子节点，返回的接口需要释放
IEinkuiIterator* __stdcall CXuiIterator::GetSubElementByID(ULONG nuEid)
{
	IEinkuiIterator* lpFound = NULL;

	ITR_CHECK();
	gpElementManager->LockIterators();

	if (mpExtension != NULL && mpExtension->moIDOrder.Size() > 0)
	{
		CEoSubItrNode loToFind;
		loToFind.muID = nuEid;
		int liIndex = mpExtension->moIDOrder.Find(loToFind);
		if (liIndex >= 0)
		{
			lpFound = mpExtension->moIDOrder[liIndex].mpIterator;
			lpFound->AddRefer();
		}
	}

	gpElementManager->UnlockIterators();

	return lpFound;
}

// 给此元素发送一条消息，发送模式是Send
// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimplePostMessage方法
ERESULT __stdcall CXuiIterator::SendMessage(IEinkuiMessage* npMsg)
{
	ITR_CHECK();
	return gpElementManager->SendMessage(this, npMsg);
}

// 给此元素发送一条消息，发送模式是Post
// 消息发送后，发送者仍然需要释放；如果希望以更加简单的方式发送消息，参考IXelManager的SimplePostMessage方法
ERESULT __stdcall CXuiIterator::PostMessage(IEinkuiMessage* npMsg)
{
	ITR_CHECK();
	return gpElementManager->PostMessage(this, npMsg);
}

// 给此元素的父元素发送一条消息，发送的模式是FastPost
// 消息发送后，发送者仍然需要释放
// !!!注意!!! 仅用于发送者是本迭代器对应的元素之情况
ERESULT __stdcall CXuiIterator::PostMessageToParent(IEinkuiMessage* npMsg)
{
	IEinkuiIterator* lpFarther;

	ITR_CHECK();
	lpFarther = GetParent();
	if (lpFarther == NULL || lpFarther == this)	// 发生错误，或者抵达根部，没有父节点了
		return ERESULT_ITERATOR_INVALID;

	npMsg->SetMessageSender(this);

	return gpElementManager->PostMessage(lpFarther, npMsg, EMSG_POSTTYPE_FAST);
}

// 申请定时器，对于永久触发的定时器，需要注销
ERESULT __stdcall CXuiIterator::SetTimer(
	IN ULONG nuID,	  // 定时器ID
	IN ULONG nuRepeat,// 需要重复触发的次数，MAXULONG32表示永远重复
	IN ULONG nuDuration,	// 触发周期
	IN void* npContext//上下文，将随着定时器消息发送给申请者
)
{
	ITR_CHECK();
	return CEinkuiSystem::gpXuiSystem->SetTimer(this, nuID, nuRepeat, nuDuration, npContext);
}

// 销毁定时器
ERESULT __stdcall CXuiIterator::KillTimer(
	IN ULONG nuID	  // 定时器ID
)
{
	ITR_CHECK();
	return CEinkuiSystem::gpXuiSystem->KillTimer(this, nuID);
}

// 设置渲染增效器，增效器用于给某个元素和它的子元素提供特定的渲染，增效器可以选择Direct2D，Direct3D技术完善XUI系统的渲染
// 同一个元素在同一时刻只能有一个增效器在工作；并且，通常增效器都是对其父元素发生作用
// 返回ERESULT_ACCESS_CONFLICT表示多个增效器发生冲突；增效器设置，请在接收到EMSG_PREPARE_PAINT时处理，其他地方做设置，有可能导致严重错误
ERESULT __stdcall CXuiIterator::SetEnhancer(
	IN IEinkuiIterator* npEnhancer,
	IN bool nbEnable		// true 启用，false 取消
)
{
	ERESULT luResult = ERESULT_SUCCESS;
	IEinkuiIterator* lpCrtEnhancer;

	ITR_CHECK();
	goAttributesLock.Enter();

	if (nbEnable != false)
	{
		if (moAtts.SetAttribute('ehcr', npEnhancer) != false)
			cmmBaseObject::SetFlags(EITR_FLAG_ENHANCER, true);
		else
			luResult = ERESULT_UNSUCCESSFUL;
	}
	else
	{
		if (moAtts.GetAttribute('ehcr', lpCrtEnhancer) != false)
		{
			if (lpCrtEnhancer != npEnhancer)
				luResult = ERESULT_ACCESS_CONFLICT;
			else
			{
				moAtts.DeleteAttribute('ehcr');
			}
		}
		cmmBaseObject::SetFlags(EITR_FLAG_ENHANCER, false);
	}

	goAttributesLock.Leave();

	return luResult;

}

// 获得增效器
IEinkuiIterator* __stdcall CXuiIterator::GetEnhancer(void)
{
	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_ENHANCER) == false)
		return NULL;

	IEinkuiIterator* lpCrtEnhancer;

	goAttributesLock.Enter();

	if (moAtts.GetAttribute('ehcr', lpCrtEnhancer) == false)
		lpCrtEnhancer = NULL;
	goAttributesLock.Leave();

	return lpCrtEnhancer;
}

// 添加一个子节点
ERESULT CXuiIterator::AddSubElement(
	CXuiIterator* npSubElement
)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;
	bool lbDirty = false;

	do
	{
		// 如果没有扩展结构，分配扩展结构
		if (mpExtension == NULL)
		{
			mpExtension = new CXuiIteratorExtension();
			if (mpExtension == NULL)
				break;
		}

		// 插入ID队列
		{
			CEoSubItrNode loAddIn;
			loAddIn.muID = npSubElement->muEID;//npSubElement->GetID();
			loAddIn.mpIterator = npSubElement;

			// 如果待注册元素没有EID，那么就为他分配一个
			if (loAddIn.muID == 0 || loAddIn.muID == MAXULONG32)
			{
				ULONG i = 0;
				const ULONG luMax = 0x10000;
				do
				{
					union {
						void* p;
						ULONG d;
					}ldValue;
					ldValue.p = npSubElement->mpElement;
					loAddIn.muID = ((ldValue.d + i++) | 0x80000000);	// 最高位置一，是为了防止同将来可能加入的其他指定ID的元素冲突

				} while (mpExtension->moIDOrder.Find(loAddIn) >= 0 && i < luMax);	// 循环16K次，如果还不行就放弃
				if (i >= luMax)
					break;	//失败退出

				npSubElement->muEID = loAddIn.muID;
			}

			if (mpExtension->moIDOrder.Insert(loAddIn) < 0)
				break;
		}
		lbDirty = true;

		// 插入到Z-Order
//		BringSubElementToTop(npSubElement);
		InsertToZOder(npSubElement);

		// 插入到Tab-Order
		if (mpExtension->moTabOder.Insert(-1, npSubElement) < 0)
			break;

		// 设置父对象
		npSubElement->mpParent = this;

		// 增加本对象对象引用
		KAddRefer();

		// 返回成功
		luResult = ERESULT_SUCCESS;

	} while (false);

	if (luResult != ERESULT_SUCCESS && lbDirty != false)
	{
		// 释放资源
		CEoSubItrNode loAddIn;
		loAddIn.muID = npSubElement->GetID();
		mpExtension->moIDOrder.Remove(loAddIn);

		for (int i = 0; i < mpExtension->moZOrder.Size(); i++)
		{
			if (mpExtension->moZOrder[i] == npSubElement)
			{
				mpExtension->moZOrder.RemoveByIndex(i);
				break;
			}
		}
		for (int i = 0; i < mpExtension->moTabOder.Size(); i++)
		{
			if (mpExtension->moTabOder[i] == npSubElement)
			{
				mpExtension->moTabOder.RemoveByIndex(i);
				break;
			}
		}
	}

	return luResult;
}


// 删除一个子节点
void CXuiIterator::RemoveSubElement(
	CXuiIterator* npSubElement
)
{
	CEoSubItrNode loRemove;
	bool lbFound = false;
	loRemove.muID = npSubElement->GetID();
	mpExtension->moIDOrder.Remove(loRemove);

	for (int i = 0; i < mpExtension->moZOrder.Size(); i++)
	{
		if (mpExtension->moZOrder[i] == npSubElement)
		{
			mpExtension->moZOrder.RemoveByIndex(i);
			lbFound = true;
			break;
		}
	}

	for (int i = 0; i < mpExtension->moTabOder.Size(); i++)
	{
		if (mpExtension->moTabOder[i] == npSubElement)
		{
			mpExtension->moTabOder.RemoveByIndex(i);
			lbFound = true;
			break;
		}
	}

	// 指向自身，表示没有父节点了
	npSubElement->mpParent = npSubElement;

	// 如果确实删掉了一个子节点，那么就减少自身的引用
	if (lbFound != false)
		KRelease();
}

// 在子节点中查找携带目标的迭代器
CXuiIterator* CXuiIterator::SeekIteratorInChild(IXsElement* npElement)
{
	CXuiIterator* lpIteratorObj = NULL;

	for (int i = 0; i < mpExtension->moIDOrder.Size(); i++)
	{
		CMMASSERT(mpExtension->moIDOrder[i].mpIterator != NULL);

		if (mpExtension->moIDOrder[i].mpIterator->mpElement == npElement)
		{
			lpIteratorObj = mpExtension->moIDOrder[i].mpIterator;
			break;
		}

		// 递归调用，检查它的子节点
		if (mpExtension->moIDOrder[i].mpIterator->mpExtension != NULL)
		{
			lpIteratorObj = mpExtension->moIDOrder[i].mpIterator->SeekIteratorInChild(npElement);
			if (lpIteratorObj != NULL)
				break;
		}

	}

	return lpIteratorObj;
}

// 关闭一个元素
void __stdcall CXuiIterator::Close(void)
{
	ITR_CHECK();
	// 注销控件
	CExMessage::SendMessage(NULL, this, EMSG_APPLY_DESTROY, CExMessage::DataInvalid, NULL, 0);
}

// Hook目标，当前仅支持单层次的Hook，即，一个元素在同一时刻仅被一个元素Hook；试图Hook一个已经被Hook的元素时，将会返回失败ERESULT_ACCESS_CONFLICT
ERESULT __stdcall CXuiIterator::SetHook(
	IN IEinkuiIterator* npHooker,	// Hook请求者，一旦设置了Hook，本对象的所有消息（EMSG_HOOKED_MESSAGE不会被转发），都会先发送给Hooker处理，Hooker可以修改任意的消息，也可以阻止消息发送给本对象
	IN bool nbSet		// true to set ,false to remove
)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	ITR_CHECK();
	if (nbSet != FALSE)
	{
		if (cmmBaseObject::TestFlag(EITR_FLAG_HOOK) != false)
			return ERESULT_ACCESS_CONFLICT;
	}
	else
	{
		if (cmmBaseObject::TestFlag(EITR_FLAG_HOOK) == false)
			return ERESULT_SUCCESS;
	}

	goAttributesLock.Enter();

	if (nbSet != FALSE)
	{
		if (moAtts.SetAttribute('hook', npHooker) != false)
			luResult = ERESULT_SUCCESS;
	}
	else
	{
		if (moAtts.DeleteAttribute('hook') != false)
			luResult = ERESULT_SUCCESS;
	}

	if (luResult == ERESULT_SUCCESS)
		cmmBaseObject::SetFlags(EITR_FLAG_HOOK, nbSet);

	goAttributesLock.Leave();

	return luResult;
}

// 获得Hooker，获取本元素被谁Hook
IEinkuiIterator* __stdcall CXuiIterator::GetHooker(void)
{
	IEinkuiIterator* lpHooker;

	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_HOOK) == false)
		return NULL;
	goAttributesLock.Enter();

	if (moAtts.GetAttribute('hook', lpHooker) == false)
		lpHooker = NULL;
	goAttributesLock.Leave();

	return lpHooker;

}


//////////////////////////////////////////////////////////////////////////
// 下面是所有与显示和位置相关的方法

// 设置显示/隐藏
void __stdcall CXuiIterator::SetVisible(bool nbVisible)
{
	ITR_CHECK();
	cmmBaseObject::SetFlags(EITR_FLAG_VISIBLE, nbVisible);
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CExMessage::SendMessage(this, NULL, EMSG_SHOW_HIDE, nbVisible);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取显示/隐藏状态
bool __stdcall CXuiIterator::IsVisible(void)
{
	ITR_CHECK();
	return cmmBaseObject::TestFlag(EITR_FLAG_VISIBLE) && cmmBaseObject::TestFlag(EITR_FLAG_INIT);
}

// 设定整体透明度
void __stdcall CXuiIterator::SetAlpha(FLOAT nfAlpha)
{
	ITR_CHECK();
	mfAlpha = nfAlpha;
	cmmBaseObject::SetFlags(EITR_FLAG_ALPHA, nfAlpha > 0.99f ? false : true);	// 如果大于0.99相当于是完全不透明
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取整体透明度
FLOAT __stdcall CXuiIterator::GetAlpha(void)
{
	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_ALPHA) == false)
		return 1.0f;

	return mfAlpha;
}

// 设置平面坐标
void __stdcall CXuiIterator::SetPosition(FLOAT nfX, FLOAT nfY)
{
	ITR_CHECK();
	mdPosition.x = CExFloat::Round(nfX);
	mdPosition.y = CExFloat::Round(nfY);
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CExMessage::SendMessage(this, NULL, EMSG_ELEMENT_MOVED, mdPosition);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}
void __stdcall CXuiIterator::SetPosition(const D2D1_POINT_2F& rPosition)
{
	ITR_CHECK();
	SetPosition(rPosition.x, rPosition.y);
}


// 读取平面坐标
FLOAT __stdcall CXuiIterator::GetPositionX(void)
{
	ITR_CHECK();
	return mdPosition.x;
}
FLOAT __stdcall CXuiIterator::GetPositionY(void)
{
	ITR_CHECK();
	return mdPosition.y;
}

// 读取平面坐标
D2D1_POINT_2F __stdcall CXuiIterator::GetPosition(void)
{
	ITR_CHECK();
	return mdPosition;
}

void __stdcall CXuiIterator::GetRect(D2D1_RECT_F& rRect)
{
	ITR_CHECK();
	rRect.left = mdPosition.x;
	rRect.top = mdPosition.y;
	rRect.right = mdPosition.x + mdSize.width;
	rRect.bottom = mdPosition.y + mdSize.height;
}

// 设置可视区域
void __stdcall CXuiIterator::SetVisibleRegion(
	IN const D2D1_RECT_F& rRegion		// 基于相对坐标的可视区域，此区域之外不会显示本元素及子元素的内容；如果rRegion.left > region.right 表示取消可视区设置
)
{
	ITR_CHECK();
	// 理论上存在某个Element在我们处理绘制并且恰好在绘制它的时间点调用此函数，这样可能会导致严重故障
	if (CEinkuiSystem::gpXuiSystem->GetRenderStep() != CEinkuiSystem::eRenderStop)
		return;

	goAttributesLock.Enter();

	if (rRegion.left > rRegion.right || rRegion.top > rRegion.bottom)
	{
		moAtts.DeleteAttribute('vrgn');
		cmmBaseObject::SetFlags(EITR_FLAG_VREGION, false);
	}
	else
		if (moAtts.SetAttribute('vrgn', rRegion, true) != false)
			cmmBaseObject::SetFlags(EITR_FLAG_VREGION, true);

	goAttributesLock.Leave();

}

// 获取可视区域，返回false表示没有设置可是区域
bool __stdcall CXuiIterator::GetVisibleRegion(
	OUT D2D1_RECT_F& rRegion	// 返回可视区域，如果没有设置可视区域，则不会修改这个对象
)
{
	bool lbReval;

	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_VREGION) == false)
		return false;

	goAttributesLock.Enter();

	lbReval = moAtts.GetAttribute('vrgn', rRegion);

	goAttributesLock.Leave();

	return lbReval;
}


// 设置平面转角
void __stdcall CXuiIterator::SetRotation(FLOAT nfAngle, D2D1_POINT_2F ndCenter)
{
	// 理论上存在某个Element在我们处理绘制并且恰好在绘制它的时间点调用此函数，这样可能会导致严重故障
	if (CEinkuiSystem::gpXuiSystem->GetRenderStep() != CEinkuiSystem::eRenderStop)
		return;

	ITR_CHECK();
	goAttributesLock.Enter();

	if (nfAngle > -1.0f && nfAngle < 1.0)// 如果小于1度相当于是没有旋转
	{
		cmmBaseObject::SetFlags(EITR_FLAG_ROTATION, false);
		moAtts.DeleteAttribute('rtal');
		moAtts.DeleteAttribute('rtcn');
	}
	else
	{
		cmmBaseObject::SetFlags(EITR_FLAG_ROTATION, true);
		moAtts.SetAttribute('rtal', nfAngle, true);
		moAtts.SetAttribute('rtcn', ndCenter, true);
	}

	goAttributesLock.Leave();

	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 设置平面转角，以元素中心为旋转中心
void __stdcall CXuiIterator::SetRotation(
	FLOAT nfAngle			// 角度单位 -359 -> +359度
)
{
	D2D1_POINT_2F ldCenter;

	ITR_CHECK();
	ldCenter.x = mdSize.width / 2.0f;
	ldCenter.y = mdSize.height / 2.0f;

	goAttributesLock.Enter();

	if (nfAngle > -1.0f && nfAngle < 1.0)// 如果小于1度相当于是没有旋转
	{
		cmmBaseObject::SetFlags(EITR_FLAG_ROTATION, false);

		moAtts.DeleteAttribute('rtal');
		moAtts.DeleteAttribute('rtcn');
	}
	else
	{
		cmmBaseObject::SetFlags(EITR_FLAG_ROTATION, true);
		moAtts.SetAttribute('rtal', nfAngle, true);
		moAtts.SetAttribute('rtcn', ldCenter, true);
	}

	goAttributesLock.Leave();

	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取平面转角
FLOAT __stdcall CXuiIterator::GetRotationAngle(void)
{
	FLOAT lfAngle;

	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_ROTATION) == false)
		return 0.0f;

	goAttributesLock.Enter();

	if (moAtts.GetAttribute('rtal', lfAngle) == false)
		lfAngle = 0.0f;

	goAttributesLock.Leave();

	return lfAngle;
}

D2D1_POINT_2F __stdcall CXuiIterator::GetRotationCenter(void)
{
	D2D1_POINT_2F ldCenter = { 0.0f,0.0f };

	ITR_CHECK();
	goAttributesLock.Enter();

	if (cmmBaseObject::TestFlag(EITR_FLAG_ROTATION) == false)
		moAtts.GetAttribute('rtcn', ldCenter);

	goAttributesLock.Leave();

	return ldCenter;
}

FLOAT __stdcall CXuiIterator::GetRotation(D2D1_POINT_2F& rCenter)
{
	FLOAT lfAngle;

	ITR_CHECK();
	if (cmmBaseObject::TestFlag(EITR_FLAG_ROTATION) == false)
		return 0.0f;

	goAttributesLock.Enter();

	if (moAtts.GetAttribute('rtal', lfAngle) == false)
		lfAngle = 0.0f;

	moAtts.GetAttribute('rtcn', rCenter);

	goAttributesLock.Leave();

	return lfAngle;
}

// 设置参考尺寸
void __stdcall CXuiIterator::SetSize(FLOAT nfCx, FLOAT nfCy)
{
	ITR_CHECK();
	mdSize.width = nfCx;
	mdSize.height = nfCy;

	CExMessage::SendMessage(this, NULL, EMSG_ELEMENT_RESIZED, mdSize);

	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

void __stdcall CXuiIterator::SetSize(const D2D1_SIZE_F& rSize)
{
	ITR_CHECK();
	mdSize = rSize;

	CExMessage::SendMessage(this, NULL, EMSG_ELEMENT_RESIZED, mdSize);

	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取参考尺寸
FLOAT __stdcall CXuiIterator::GetSizeX(void)
{
	ITR_CHECK();
	return mdSize.width;
}
FLOAT __stdcall CXuiIterator::GetSizeY(void)
{
	ITR_CHECK();
	return mdSize.height;
}

// 读取参考尺寸
D2D1_SIZE_F __stdcall CXuiIterator::GetSize(void)
{
	ITR_CHECK();
	return mdSize;
}

// 设置是否有效
void __stdcall CXuiIterator::SetEnable(bool nbSet)
{
	ITR_CHECK();
	cmmBaseObject::SetFlags(EITR_FLAG_DISABLE, !nbSet);
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	CExMessage::SendMessage(this, NULL, EMSG_ENALBE_DISABLE, nbSet);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取是否有效
bool __stdcall CXuiIterator::IsEnable(void)
{
	return !cmmBaseObject::TestFlag(EITR_FLAG_DISABLE);
}

// 设置Style
void __stdcall CXuiIterator::SetStyles(ULONG nuStyles)
{
	ITR_CHECK();
	// 理论上存在某个Element在我们处理绘制并且恰好在绘制它的时间点调用此函数，这样可能会导致严重故障
	if (CEinkuiSystem::gpXuiSystem->GetRenderStep() != CEinkuiSystem::eRenderStop)
		return;

	muStyle = nuStyles;
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	if ((nuStyles&EITR_STYLE_TOP) != 0)
		mpParent->BringSubElementToTop(this);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 修改Style，前一个参数是希望增加的Style，后一个参数是希望移除的Style，当前后两个参数中包括相同Style时，该Style不会被移除 
void __stdcall CXuiIterator::ModifyStyles(ULONG nuSet, ULONG nuClear)
{
	ITR_CHECK();
	// 理论上存在某个Element在我们处理绘制并且恰好在绘制它的时间点调用此函数，这样可能会导致严重故障
	if (CEinkuiSystem::gpXuiSystem->GetRenderStep() != CEinkuiSystem::eRenderStop)
		return;

	muStyle &= (~nuClear);
	muStyle |= nuSet;
	cmmBaseObject::SetFlags(EITR_FLAG_DIRTY, true);

	if ((nuSet&EITR_STYLE_TOP) != 0)
		mpParent->BringSubElementToTop(this);

	CEinkuiSystem::gpXuiSystem->UpdateView();
}

// 读取Style
ULONG __stdcall CXuiIterator::GetStyles(void)
{
	ITR_CHECK();
	return muStyle;
}

// 申请键盘焦点，如果该元素具有popup属性，也将被调整到合适的上层
ERESULT __stdcall CXuiIterator::SetKeyBoardFocus(void)
{
	ITR_CHECK();
	return CExMessage::SendMessage(NULL, this, EMSG_SET_KEYBOARD_FOCUS, this, NULL, 0);
}

// 释放键盘焦点，这将导致Tab Order的下一个键盘接收者获得焦点
void __stdcall CXuiIterator::ReleaseKeyBoardFocus(bool nbShiftFocus)
{
	ST_RELEASE_KEYFOCUS loRelease;

	ITR_CHECK();
	loRelease.CrtFocus = this;
	loRelease.ShiftTab = nbShiftFocus;

	CExMessage::SendMessage(NULL, this, EMSG_RELEASE_KEYBOARD_FOCUS, loRelease, NULL, 0);
}

// 设置元素为活跃，如果本元素不具有EITR_STYLE_POPUP或EITR_STYLE_ACTIVE风格，那么最低的一个具有EITR_STYLE_POPUP或EITR_STYLE_ACTIVE风格的上层元素将被激活
void __stdcall CXuiIterator::SetActive(void)
{
	ITR_CHECK();
	CExMessage::SendMessage(NULL, this, EMSG_SET_ACTIVE, this, NULL, 0);
}



// 将本元素调整到同级窗口最高层
void __stdcall CXuiIterator::BringToTop(void)
{
	ITR_CHECK();
	dynamic_cast<CXuiIterator*>(mpParent)->BringSubElementToTop(this);
}

// 将本元素在父元素的Z Order序列中，向下移动一位
bool __stdcall CXuiIterator::MoveDown(void)
{
	bool lbReval;

	ITR_CHECK();
	gpElementManager->LockIterators();

	lbReval = mpParent->MoveOnZOrder(false, this);

	gpElementManager->UnlockIterators();

	return lbReval;
}

// 将本元素在父元素的Z Order序列中，向上移动一位
bool __stdcall CXuiIterator::MoveUp(void)
{
	bool lbReval;

	ITR_CHECK();
	gpElementManager->LockIterators();

	lbReval = mpParent->MoveOnZOrder(true, this);

	gpElementManager->UnlockIterators();

	return lbReval;
}

// 整理所有子元素的叠放次序为原始设置（即配置文件制定的次序）
bool __stdcall CXuiIterator::ResetChildrenByDefualtZOrder(void)
{
	ITR_CHECK();
	UpdateOrder();

	return true;
}

// 重新设置本元素的Z Order的值，如果两个兄弟Element具有相同的Z Order将无法确定它们的先后循序，但系统本身运行不会出错
void __stdcall CXuiIterator::SetDefaultZOrder(const LONG nlDefaultZOrder)
{
	ITR_CHECK();
	mlZOrder = nlDefaultZOrder;
}

// 获得默认的ZOrder值
LONG __stdcall CXuiIterator::GetDefaultZOrder(void)
{
	ITR_CHECK();
	return mlZOrder;
}



// 调整子元素在ZOrder的顺序
bool CXuiIterator::MoveOnZOrder(bool nbUp, CXuiIterator* npChild)
{
	int i;
	CXuiIterator* lpSwap;
	bool lbResult = false;

	for (i = 0; i < mpExtension->moZOrder.Size(); i++)
	{
		if (mpExtension->moZOrder[i] == npChild)
		{
			if (nbUp != false)
			{
				if (i + 1 < mpExtension->moZOrder.Size())
				{
					lpSwap = mpExtension->moZOrder[i];
					mpExtension->moZOrder[i] = mpExtension->moZOrder[i + 1];
					mpExtension->moZOrder[i + 1] = lpSwap;
					lbResult = true;
				}
			}
			else
			{
				if (i > 1)
				{
					lpSwap = mpExtension->moZOrder[i];
					mpExtension->moZOrder[i] = mpExtension->moZOrder[i - 1];
					mpExtension->moZOrder[i - 1] = lpSwap;
					lbResult = true;
				}
			}
			break;
		}
	}

	return lbResult;
}


// 将某个子元素调整到ZOder最高层
void CXuiIterator::BringSubElementToTop(
	CXuiIterator* npSubElement
)
{
	int i;

	gpElementManager->LockIterators();

	for (i = 0; i < mpExtension->moZOrder.Size(); i++)
	{
		if (mpExtension->moZOrder[i] == npSubElement)
		{
			mpExtension->moZOrder.RemoveByIndex(i);
			break;
		}
	}

	for (i = mpExtension->moZOrder.Size() - 1; i >= 0; i--)
	{
		// 待查元素不具有Top属性，或者，本元素也具有Top属性
		if (mpExtension->moZOrder[i]->CheckStyle(EITR_STYLE_TOP) == false || npSubElement->CheckStyle(EITR_STYLE_TOP) != false)
		{
			break;
		}
	}

	mpExtension->moZOrder.Insert(i + 1, npSubElement);

	gpElementManager->UnlockIterators();

}

// 将元素插入到ZOrder中
void CXuiIterator::InsertToZOder(
	CXuiIterator* npSubElement
)
{
	int i;

	gpElementManager->LockIterators();

	for (i = mpExtension->moZOrder.Size() - 1; i >= 0; i--)
	{
		// 待查元素不具有Top属性，或者，本元素也具有Top属性
		if (mpExtension->moZOrder[i]->mlZOrder <= npSubElement->mlZOrder && (mpExtension->moZOrder[i]->CheckStyle(EITR_STYLE_TOP) == false || npSubElement->CheckStyle(EITR_STYLE_TOP) != false))
		{
			break;
		}
	}

	mpExtension->moZOrder.Insert(i + 1, npSubElement);

	gpElementManager->UnlockIterators();

}


// 调整TabOrder和ZOrder，初始化之际有Element管理器调用
void CXuiIterator::UpdateOrder(void)
{
	int liCount;
	int i, j;
	CXuiIterator* lpSwap;
	bool lbClean;

	if (mpExtension == NULL)
		return;

	gpElementManager->LockIterators();

	// 冒泡排序
	liCount = mpExtension->moTabOder.Size();
	for (i = 0; i < liCount - 1; i++)
	{
		lbClean = true;
		for (j = 0; j < liCount - i - 1; j++)
		{
			if (mpExtension->moTabOder[j]->mlTabOrder > mpExtension->moTabOder[j + 1]->mlTabOrder)
			{
				lpSwap = mpExtension->moTabOder[j];
				mpExtension->moTabOder[j] = mpExtension->moTabOder[j + 1];
				mpExtension->moTabOder[j + 1] = lpSwap;
				lbClean = false;
			}
		}
		if (lbClean != false)
			break;
	}

	// 冒泡排序
	liCount = mpExtension->moZOrder.Size();
	for (i = 0; i < liCount - 1; i++)
	{
		lbClean = true;
		for (j = 0; j < liCount - i - 1; j++)
		{
			if (mpExtension->moZOrder[j]->mlZOrder > mpExtension->moZOrder[j + 1]->mlZOrder && mpExtension->moZOrder[j]->CheckStyle(EITR_STYLE_TOP) == mpExtension->moZOrder[j + 1]->CheckStyle(EITR_STYLE_TOP) \
				|| mpExtension->moZOrder[j]->CheckStyle(EITR_STYLE_TOP) != false && mpExtension->moZOrder[j + 1]->CheckStyle(EITR_STYLE_TOP) == false)
			{
				lpSwap = mpExtension->moZOrder[j];
				mpExtension->moZOrder[j] = mpExtension->moZOrder[j + 1];
				mpExtension->moZOrder[j + 1] = lpSwap;
				lbClean = false;
			}
		}
		if (lbClean != false)
			break;
	}

	gpElementManager->UnlockIterators();
}

// 获得下一个EITR_STYLE_KEYBOARD的元素，不会进入Iterator临界区
CXuiIterator* CXuiIterator::GetNextKeyBoardAccepter(CXuiIterator* npCurrent)
{
	int i;
	int liEnd = 0;//hy 20190723
	CXuiIterator* lpFound = NULL;

	if (mpExtension == NULL || mpExtension->moTabOder.Size() <= 0)
		return NULL;

	// 定位到当前元素，并且计算环回后的结束地址
	if (npCurrent != NULL)
	{
		for (i = 0; i < mpExtension->moTabOder.Size(); i++)
			if (mpExtension->moTabOder[i] == npCurrent)
			{
				i++;

				if (CheckStyle(EITR_STYLE_POPUP) != false)
					liEnd = i + mpExtension->moTabOder.Size();
				else
					liEnd = mpExtension->moTabOder.Size();

				break;
			}
	}
	else
	{
		i = 0;
		liEnd = mpExtension->moTabOder.Size();
	}


	for (; i < liEnd && lpFound == NULL; i++)
	{
		npCurrent = mpExtension->moTabOder[i%mpExtension->moTabOder.Size()];
		if (npCurrent->CheckStyle(EITR_STYLE_KEYBOARD) != false)
		{
			lpFound = npCurrent;
		}
		else
			if (npCurrent->mpExtension != NULL)
				lpFound = npCurrent->GetNextKeyBoardAccepter(NULL);	// 递归调用，以使得焦点进入某个子控件内部
	}

	return lpFound;
}

//设置ToolTip，鼠标在本对象上悬停，将会自动显示的单行简短文字提示，鼠标一旦移开显示的ToolTip，它自动消失
void __stdcall CXuiIterator::SetToolTip(const wchar_t* nswTip)
{
	ERESULT luResult = ERESULT_SUCCESS;
	wchar_t* lswTipText;
	int liSize;

	ClearTip();

	if (nswTip == NULL)
		return;

	liSize = (int)wcslen(nswTip);
	if (liSize <= 0)
		return;

	lswTipText = new wchar_t[liSize + 1];
	wcscpy_s(lswTipText, liSize + 1, nswTip);

	goAttributesLock.Enter();

	if (moAtts.SetAttribute('tip', lswTipText) != false)
		cmmBaseObject::SetFlags(EITR_FLAG_TIP, true);

	goAttributesLock.Leave();
	delete[] lswTipText;//hy 20190723
	lswTipText = NULL;
}

void CXuiIterator::ClearTip()
{
	wchar_t* lswTipText;

	if (cmmBaseObject::TestFlag(EITR_FLAG_TIP) == false)
		return;

	goAttributesLock.Enter();

	lswTipText = NULL;
	moAtts.GetAttribute('tip', lswTipText);

	moAtts.DeleteAttribute('tip');
	cmmBaseObject::SetFlags(EITR_FLAG_TIP, false);

	goAttributesLock.Leave();

	CMM_SAFE_DELETE(lswTipText);

}

// 获得Tip
const wchar_t* CXuiIterator::GetToolTip(void)
{
	wchar_t* lswTipText = NULL;

	if (cmmBaseObject::TestFlag(EITR_FLAG_TIP) == false)
		return NULL;

	goAttributesLock.Enter();

	moAtts.GetAttribute('tip', lswTipText);

	goAttributesLock.Leave();

	return lswTipText;
}

//设置IME输入窗口位置，ndPosition是本元素局部坐标中的位置; 当原元素具有EITR_STYLE_DISABLE_IME属性时无效
void __stdcall CXuiIterator::SetIMECompositionWindow(D2D1_POINT_2F ndPosition)
{
	ITR_CHECK();
	if (CheckStyle(EITR_STYLE_DISABLE_IME) == false)
	{
		D2D1_POINT_2F ldPoint;

		if (LocalToWorld(ndPosition, ldPoint) == false)
			return;

		CEinkuiSystem::gpXuiSystem->GetImeContext()->SetImeCompositionWnd(ldPoint);
	}
}


// 从局部地址转换为世界地址
bool __stdcall CXuiIterator::LocalToWorld(const D2D1_POINT_2F& crLocalPoint, D2D1_POINT_2F& rWorldPoint)
{
	ITR_CHECK();
	rWorldPoint = crLocalPoint * mdWorldMatrix;

	return true;
}

// 从世界地址转换为局部地址
bool __stdcall CXuiIterator::WorldToLocal(const D2D1_POINT_2F& crWorldPoint, D2D1_POINT_2F& rLocalPoint)
{
	ITR_CHECK();
	// 计算局部坐标
	if (mbInverted == false)
	{
		mdWMInverted = mdWorldMatrix;
		mbInverted = mdWMInverted.Invert();
	}
	if (mbInverted != false)
	{
		rLocalPoint = crWorldPoint * mdWMInverted;

		return true;
	}

	return false;
}

// 注册快捷键，当快捷键被触发，注册快捷键的Element将会受到；
// 如果普通按键组合（不包含Alt键)按下的当时，存在键盘焦点，按键消息会首先发送给键盘焦点，如果焦点返回ERESULT_KEY_UNEXPECTED才会判断是否存在快捷键行为
bool CXuiIterator::RegisterHotKey(
	IN IEinkuiIterator* npApplicant,	// 注册的元素，将有它收到注册是快捷键消息
	IN ULONG nuHotKeyID,	// 事先定义好的常数，用来区分Hotkey；不能出现相同的ID，试图注册已有的Hotkey将会失败
	IN ULONG nuVkNumber,	// 虚拟键码
	IN bool nbControlKey,	// 是否需要Control组合
	IN bool nbShiftKey,		// 是否需要Shift组合
	IN bool nbAltKey		// 是否需要Alt组合
)
{
	bool lbOk = true;

	//CXuiHotkeyEntry loEntry;
	//THotKeyTable* lpHtKyTable = NULL;

	//do 
	//{
	//	// 如果没有扩展结构，分配扩展结构
	//	goAttributesLock.Enter();

	//	if(moAtts.GetAttribute('htky',lpHtKyTable)==false)
	//		lpHtKyTable = NULL;

	//	goAttributesLock.Leave();

	//	if(lpHtKyTable == NULL)
	//	{
	//		lpHtKyTable = new THotKeyTable;
	//		if(lpHtKyTable == NULL)
	//			break;

	//		goAttributesLock.Enter();

	//		lbOk = moAtts.SetAttribute('htky',lpHtKyTable);

	//		goAttributesLock.Leave();

	//		if(lbOk == false)
	//		{
	//			delete lpHtKyTable;
	//			break;
	//		}

	//		lbOk = false;
	//	}

	//	// 不再检查相同ID，允许使用相同ID注册
	//	//for(i=0;i<lpHtKyTable->Size();i++)
	//	//{
	//	//	if((*lpHtKyTable)[i].muHotKeyID == nuHotKeyID)
	//	//		break;
	//	//}
	//	//if(i<mpExtension->moHotKey.Size())
	//	//	break;

	//	loEntry.mpApplicant = npApplicant;
	//	loEntry.mcuExKey = 0;

	//	if(nbControlKey != false)
	//		loEntry.mcuExKey |= CXuiHotkeyEntry::eControl;
	//	if(nbShiftKey != false)
	//		loEntry.mcuExKey |= CXuiHotkeyEntry::eShit;
	//	if(nbAltKey != false)
	//		loEntry.mcuExKey |= CXuiHotkeyEntry::eAlt;

	//	loEntry.msuVkCode = (USHORT)nuVkNumber;
	//	loEntry.muHotKeyID = nuHotKeyID;

	//	if(lpHtKyTable->Insert(loEntry)<0)
	//		break;

	//	cmmBaseObject::SetFlags(EITR_FLAG_HOTKEY,true);

	//	lbOk = true;
	//	
	//} while (false);

	return lbOk;
}

// 注销快捷键
bool CXuiIterator::UnregisterHotKey(
	IN IEinkuiIterator* npApplicant,	// 注册者
	IN ULONG UnuKeyNumber
)
{
	// 暂不支持
	return false;
}

// 检查是否具有符合的HotKey
bool CXuiIterator::DetectHotKey(
	CXuiHotkeyEntry& rToDetect
)
{
	int liIndex;
	THotKeyTable* lpHtKyTable;

	if (cmmBaseObject::TestFlag(EITR_FLAG_HOTKEY) == false)
		return false;

	goAttributesLock.Enter();

	if (moAtts.GetAttribute('htky', lpHtKyTable) == false)
		lpHtKyTable = NULL;

	goAttributesLock.Leave();

	// 如果没有扩展结构，分配扩展结构
	if (lpHtKyTable == NULL)
		return false;

	liIndex = lpHtKyTable->Find(rToDetect);
	if (liIndex >= 0)
	{
		rToDetect = (*lpHtKyTable)[liIndex];

		return true;
	}

	return false;

}

// 获得下一绘制层
LONG CXuiIterator::GetNextPaintLevel(LONG nlCrtLevel)
{
	if (cmmBaseObject::TestFlag(EITR_FLAG_PAINTLEVEL_HOST) == false)
	{
		if (nlCrtLevel == -1)
			return 0;
		return nlCrtLevel;
	}
	LONG llLevelCount;

	goAttributesLock.Enter();

	if (moAtts.GetAttribute('plvc', llLevelCount) == false)
		llLevelCount = 1;
	goAttributesLock.Leave();

	if (nlCrtLevel + 1 >= llLevelCount)
		return nlCrtLevel;
	return nlCrtLevel + 1;
}


// 对子元素建立绘制层，绘制层是一个改变子元素绘制时叠放次序的方法，通常的子元素按照从属关系排列为树形结构，绘制时也是按照树形结构先根遍历执行；
//		引入绘制层技术后，子元素将在不同层上逐次绘制，同一个层的子元素，仍然按照从属关系的树形结构先根遍历；
//		可以设定任意的绘制层，但不能大于max-ulong；鼠标落点的判定也收到绘制层的影响，层次高的元素首先被判定获得鼠标
//		如果在一个已经设定了绘制层次的子树中的某个元素再次建立绘制层次，那么它的子树中的所有子元素将不受到上一个绘制层次的影响，按照新的层次工作；
//		为了避免错误，请尽可能避免使用嵌套的绘制层次
ERESULT __stdcall CXuiIterator::CreatePaintLevel(LONG nlLevelCount)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	if (cmmBaseObject::TestFlag(EITR_FLAG_PAINTLEVEL_HOST) != false)
		return luResult;	// 已经建立了绘制层次

	goAttributesLock.Enter();

	if (moAtts.SetAttribute('plvc', nlLevelCount) != false)
		luResult = ERESULT_SUCCESS;

	goAttributesLock.Leave();

	if (luResult == ERESULT_SUCCESS)
		cmmBaseObject::SetFlags(EITR_FLAG_PAINTLEVEL_HOST, true);

	return luResult;
}

// 获得子元素绘制层数量
LONG __stdcall CXuiIterator::GetPaintLevelCount(void)
{
	LONG llLevelCount = 0;

	if (cmmBaseObject::TestFlag(EITR_FLAG_PAINTLEVEL_HOST) == false)
		return 0;

	goAttributesLock.Enter();

	moAtts.GetAttribute('plvc', llLevelCount);

	goAttributesLock.Leave();

	return llLevelCount;
}

// 删除绘制层次设定
ERESULT __stdcall CXuiIterator::DeletePaintLevel(void
	//bool nbClearAll=true		// 清除掉所有子元素的绘制层次设定
)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	if (cmmBaseObject::TestFlag(EITR_FLAG_PAINTLEVEL_HOST) == false)
		return true;

	goAttributesLock.Enter();

	if (moAtts.DeleteAttribute('plvc') != false)
		luResult = ERESULT_SUCCESS;

	goAttributesLock.Leave();

	cmmBaseObject::SetFlags(EITR_FLAG_PAINTLEVEL_HOST, false);

	return luResult;
}

// 设定自身的绘制层次
ERESULT __stdcall CXuiIterator::SetPaintLevel(LONG nlLevel)
{
	ERESULT luResult = ERESULT_UNSUCCESSFUL;

	goAttributesLock.Enter();

	if (moAtts.SetAttribute('plvn', nlLevel) != false)
		luResult = ERESULT_SUCCESS;

	goAttributesLock.Leave();

	if (luResult == ERESULT_SUCCESS)
		cmmBaseObject::SetFlags(EITR_FLAG_CRT_PAINTLEVEL, true);

	return luResult;
}

// 获得自身的绘制层次
LONG __stdcall CXuiIterator::GetPaintLevel(void)
{
	LONG llLevel = -1;

	if (cmmBaseObject::TestFlag(EITR_FLAG_CRT_PAINTLEVEL) != false)
	{
		goAttributesLock.Enter();

		moAtts.GetAttribute('plvn', llLevel);

		goAttributesLock.Leave();
	}

	return llLevel;
}
