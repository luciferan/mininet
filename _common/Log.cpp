#include "Log.h"
#include "util_Time.h"

//
CLog g_Log;
CLog g_PerformanceLog;

//
CLog::CLog()
{
}

CLog::~CLog()
{
}

void CLog::write(std::wstring wstr)
{
	CTimeSet CurrTime;
	wprintf(L"%d:%d:%d %s\n", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), wstr.c_str());

	//
	return;
}

void CLog::write(WCHAR *pFormat, ...)
{
	WCHAR buffer[8192+1] = {0,};

	va_list args;
	va_start(args, pFormat);
	_vsnwprintf_s(buffer, _countof(buffer), _countof(buffer), pFormat, args);
	va_end(args);

	//
	CTimeSet CurrTime;
	wprintf(L"%d:%d:%d %s\n", CurrTime.GetHour(), CurrTime.GetMin(), CurrTime.GetSec(), buffer);

	//
	return;
}