/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#include "StdAfx.h"
#include "RedoUndoManager.h"
#include "stdio.h"
#include "cmmStrHandle.h"
#include "cmmPath.h"
#include "MsgDefine.h"
#include <Shlobj.h>


CRedoUndoManager::CRedoUndoManager(void)
{

}


CRedoUndoManager::~CRedoUndoManager(void)
{
	//清除Redo数据
	ClearRedo();

	//清除Redo数据
	ClearUndo();

}

//添加redo数据
void CRedoUndoManager::AddRedo(CRedoUndoStrokeList* npdRedo)
{
	mdRedoStack.Push(npdRedo);
}

//添加undo数据
void CRedoUndoManager::AddUndo(CRedoUndoStrokeList* npdUndo)
{
	mdUndoStack.Push(npdUndo);
}

//获取redo数组大小
int CRedoUndoManager::GetRedoCount(void)
{
	return mdRedoStack.Size();
}

//获取uedo数组大小
int CRedoUndoManager::GetUndoCount(void)
{
	return mdUndoStack.Size();
}

//清空redo数据
void CRedoUndoManager::ClearRedo(void)
{
	for (int j = 0; j < mdRedoStack.Size(); j++)
	{
		CRedoUndoStrokeList* lpList = mdRedoStack.GetEntry(j);
		for (int i = 0; i < lpList->Size(); i++)
		{
			if (lpList->GetEntry(i).PenByteData != NULL)
				delete lpList->GetEntry(i).PenByteData;
		}
		lpList->Clear();
	}
	mdRedoStack.Clear();
}

//清空undo数据
void CRedoUndoManager::ClearUndo(void)
{
	for (int j = 0; j < mdUndoStack.Size(); j++)
	{
		CRedoUndoStrokeList* lpList = mdUndoStack.GetEntry(j);
		for (int i = 0; i < lpList->Size(); i++)
		{
			if (lpList->GetEntry(i).PenByteData != NULL)
				delete lpList->GetEntry(i).PenByteData;
		}
		lpList->Clear();
	}
	mdUndoStack.Clear();
}

//获取redo数据
CRedoUndoStrokeList* CRedoUndoManager::GetRedo(int& rSize)
{
	CRedoUndoStrokeList* lpRet = NULL;
	rSize = mdRedoStack.Size();
	if (rSize > 0)
	{
		lpRet = mdRedoStack.Top();
		mdRedoStack.Pop();
	}

	return lpRet;
}

//获取undo数据
CRedoUndoStrokeList* CRedoUndoManager::GetUndo(int& rSize)
{
	CRedoUndoStrokeList* lpRet = NULL;
	rSize = mdUndoStack.Size();
	if (rSize > 0)
	{
		lpRet = mdUndoStack.Top();
		mdUndoStack.Pop();
	}

	return lpRet;
}