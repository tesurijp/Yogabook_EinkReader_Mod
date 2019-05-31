/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once 

class CXsWidget;



class CLsWgtcNode{
public:
	CLsWgtcNode(){
		mbTriedPullOut = false;
	}

	CXsWidget* mpWidget;
	ULONG muTickReached;
	bool mbTriedPullOut;

	void operator=(const CLsWgtcNode& src){
		mpWidget = src.mpWidget;
		muTickReached = src.muTickReached;
		mbTriedPullOut = src.mbTriedPullOut;
	}
	void SetTickCount(void){
		muTickReached = GetTickCount();
	}
};





class CXsWgtContext{
protected:
	cmmStack<CLsWgtcNode,64,64> moStack;
	bool mbDetectionTick;
#ifdef _DEBUG
	int miMaxDepth;
#endif//_DEBUG
	CExclusiveAccess moLock;
public:
	CXsWgtContext(){
#ifdef _DEBUG
		miMaxDepth = 0;
#endif//
		mbDetectionTick = false;
	}
	~CXsWgtContext(){}

	void PushWidget(IXsWidget* npWidget);
	IXsWidget* GetTopWidget(void);
	void PopWidget(void);
	int GetStackDepth(void);

	ULONG CheckElapsedTick(void);
	void EnableTickDetection(bool nbEnalbe=true);

	bool HasTriedPulling(void);

	void SetTriedPulling(void);

};