#include "ExceptionReport.h"
//#include "util_Time.h"
#include "util.h"
#include <wchar.h>

CExceptionReport::CExceptionReport()
{
	m_hDebugHelp = LoadLibraryW(L"dbghelp.dll");
}

CExceptionReport::~CExceptionReport()
{
}

//DWORD CExceptionReport::ExceptionHandlerBegin(MINIDUMP_TYPE dumpType)
DWORD CExceptionReport::ExceptionHandlerBegin()
{
	SetUnhandledExceptionFilter(makeFullDump);
	return 0;
}

LONG WINAPI CExceptionReport::makeFullDump(LPEXCEPTION_POINTERS lpExceptionPointer)
{
	return makeDump(lpExceptionPointer, MiniDumpWithFullMemory);
}

LONG WINAPI CExceptionReport::makeDump(LPEXCEPTION_POINTERS lpExceptionPointer, MINIDUMP_TYPE dumpType)
{
	//WCHAR wcsProgramName[1024 + 1] = {0,};
	//WCHAR wcsDrive[8+1] = {0,}, wcsDir[256+1] = {0,}, wcsFileExt[16+1] = {0,};
	WCHAR wcsFileName[256+1] = {0,};
	CTimeSet currTime;

	//GetModuleFileName(NULL, wcsProgramName, _countof(wcsProgramName));
	//_wsplitpath_s(wcsProgramName, wcsDrive, wcsDir, wcsFileName, wcsFileExt);
	wstring wstrFileName = GetFileName();

	//swprintf(wcsFileName, L"%s_dmp_%04d-%02d-%02d_%02d-%02d-%02d.dmp", wstrFileName.c_str(), currTime.GetYear(), currTime.GetMonth(), currTime.GetDay(), currTime.GetHour(), currTime.GetMin(), currTime.GetSec());
	swprintf_s(wcsFileName, 256, L"%s_dmp_%04d-%02d-%02d_%02d-%02d-%02d.dmp", wstrFileName.c_str(), currTime.GetYear(), currTime.GetMonth(), currTime.GetDay(), currTime.GetHour(), currTime.GetMin(), currTime.GetSec());

	//
	HANDLE hFile = CreateFile(wcsFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if( INVALID_HANDLE_VALUE != hFile )
	{
		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = lpExceptionPointer;
		exceptionInfo.ClientPointers = NULL;

		auto bRet = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, dumpType, &exceptionInfo, NULL, NULL);
		if( !bRet )
		{
			auto err = GetLastError();
		}

		CloseHandle(hFile);
	}
	else
	{
		auto err = GetLastError();
	}

	//
	return EXCEPTION_EXECUTE_HANDLER;
}
