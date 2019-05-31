/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


typedef bplustree<IEinkuiBrush*> TEBrushTree;

class CXsBrushList{
public:
	CXsBrushList(){};
	~CXsBrushList(){};

	void RegisteBrush(IEinkuiBrush* npBrushIntf);

	void UnregisteBrush(IEinkuiBrush* npBrushIntf);

	void ReleaseDeviceResource(void);

private:
	CExclusiveAccess moLock;
	TEBrushTree moBrushes;

};


