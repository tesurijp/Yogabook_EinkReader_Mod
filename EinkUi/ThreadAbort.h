/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
/*
	FileName: ThradAbort.h
	Comment: to pull out a stalled thread from a non-cooperating function by throw an exception.
			refered to http://www.codeproject.com/KB/exception/ExcInject.aspx
	History:
			Created by Ax Nov.20,2011

*/

class CThreadAbort
{
protected:
	__declspec (noreturn) static void Throw();
public:
	static bool PullOut(HANDLE nhThread);
	static void Dummy() throw (...);

};

