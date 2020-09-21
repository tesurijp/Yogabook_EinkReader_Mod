#pragma once


typedef bplustree<IEinkuiBrush*> TEBrushTree;

class CXsBrushList {
public:
	CXsBrushList() {};
	~CXsBrushList() {};

	void RegisteBrush(IEinkuiBrush* npBrushIntf);

	void UnregisteBrush(IEinkuiBrush* npBrushIntf);

	void ReleaseDeviceResource(void);

private:
	CExclusiveAccess moLock;
	TEBrushTree moBrushes;

};


