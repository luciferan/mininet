#pragma once
#ifndef __EXCEPTIONREPORT_H__
#define __EXCEPTIONREPORT_H__

//
#include <windows.h>
#include <DbgHelp.h>

#pragma comment (lib, "dbghelp.lib")

//
class CExceptionReport
{
private:
	HMODULE m_hDebugHelp;

	//
private:
	CExceptionReport();
	virtual ~CExceptionReport();

public:
	static CExceptionReport& GetInstance()
	{
		static CExceptionReport *pInstance = new CExceptionReport();
		return *pInstance;
	}

	//DWORD ExceptionHandlerBegin(MINIDUMP_TYPE dumpType);
	DWORD ExceptionHandlerBegin();

	static LONG WINAPI makeFullDump(LPEXCEPTION_POINTERS lpExceptionPointer);
	static LONG WINAPI makeDump(LPEXCEPTION_POINTERS lpExceptionPointer, MINIDUMP_TYPE dumpType);
};

//
#endif //__EXCEPTIONREPORT_H__