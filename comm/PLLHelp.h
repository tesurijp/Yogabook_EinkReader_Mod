/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once


template<int TaskCount>
class tpPllHelper
{
public:
	tpPllHelper(int MaxItem) {
		int CurrentItem = -1;

		for (int i = 0; i < TaskCount; i++)
		{
			TaskBeginArr[i] = ++CurrentItem;
			CurrentItem = TaskBeginArr[i] + (MaxItem / TaskCount) - 1;
			TaskEndArr[i] = CurrentItem;
		}
		if (TaskEndArr[TaskCount - 1] + 1 < MaxItem)
			TaskEndArr[TaskCount - 1] = MaxItem - 1;
	}
	~tpPllHelper() {}
	int Next(int Task,int PreItem) {
		if (PreItem < TaskBeginArr[Task])
			return TaskBeginArr[Task];
		if (PreItem >= TaskEndArr[Task])
			return -1;
		return PreItem+1;
	}

protected:
	int TaskBeginArr[TaskCount];
	int TaskEndArr[TaskCount];
};
