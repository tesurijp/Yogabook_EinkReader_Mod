#pragma once


typedef bplustree<IEinkuiBitmap*> TEBitmapTree;

class CXsBmpList {
public:
	CXsBmpList() {
	};
	~CXsBmpList() {};

	void RegisteBitmap(IEinkuiBitmap* npBmpIntf);

	void UnregisteBitmap(IEinkuiBitmap* npBmpIntf);

	void ReleaseDeviceResource(void);

private:
	CExclusiveAccess moLock;
	TEBitmapTree moBitmaps;

};


