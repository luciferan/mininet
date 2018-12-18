#include "Log.h"

//
CLog g_Log;

//
CLog::CLog()
{
}

CLog::~CLog()
{
}

void CLog::write(std::wstring wstr)
{
	wprintf(L"%s\n", wstr.c_str());

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
	wprintf(L"%s\n", buffer);

	//
	return;
}