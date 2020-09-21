/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once

/*
	audo redo管理模块
*/

#include "EdDoc.h"
#include "cmmstruct.h"


#define RUM_TYPE_INK 1
#define RUM_TYPE_HIGHLIGHT 2
#define RUM_TYPE_DELETE_LINE 3
#define RUM_TYPE_UNDER_LINE 4


class CRedoUndoItem {

public:
	bool IsDelete; //true表示本步是添加，反之是删除
	int PageNumber; //操作所在页码
	ULONGLONG PenItemSignature; //笔迹对象签名
	buf_ptr PenByteData; //序列后的笔迹
	uint32Eink PenByteDataSize; //序列后的笔迹大小
	int AnnotType; //类型

	CRedoUndoItem() {
		PageNumber = -1;
		PenItemSignature = 0;
		PenByteData = NULL;
		PenByteDataSize = 0;
		AnnotType = 0;
	}

	CRedoUndoItem(const CRedoUndoItem& src) {
		*this = src;
	}

	void operator = (const CRedoUndoItem& src) {
		IsDelete = src.IsDelete;
		PageNumber = src.PageNumber;
		PenItemSignature = src.PenItemSignature;
		PenByteData = src.PenByteData;
		PenByteDataSize = src.PenByteDataSize;
		AnnotType = src.AnnotType;
	}

};

typedef cmmVector<CRedoUndoItem, 0, 128> CRedoUndoStrokeList;

class CRedoUndoManager
{

public:

	CRedoUndoManager(void);
	~CRedoUndoManager(void);

	//添加redo数据
	void AddRedo(CRedoUndoStrokeList* npdRedo);
	//添加undo数据
	void AddUndo(CRedoUndoStrokeList* npdUndo);
	//获取redo数据
	CRedoUndoStrokeList* GetRedo(int& rSize);
	//获取undo数据
	CRedoUndoStrokeList* GetUndo(int& rSize);
	//清空redo数据
	void ClearRedo(void);
	//清空undo数据
	void ClearUndo(void);
	//获取redo数组大小
	int GetRedoCount(void);
	//获取uedo数组大小
	int GetUndoCount(void);

private:
	cmmStack<CRedoUndoStrokeList*> mdRedoStack;
	cmmStack<CRedoUndoStrokeList*> mdUndoStack;


};

