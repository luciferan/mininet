#include "Log.h"

CLog::CLog()
{
}

CLog::~CLog()
{
}

void CLog::write(std::wstring wstr)
{
	{
		wsprintf(L"%s\n", wstr.c_str());
	}
}

void CLog::write(WCHAR *pFormat, ...)
{
	WCHAR buffer[8192+1] = {0,};

	{
		va_list args;
		va_start(args, pFormat);
		_vsnwprintf_s(buffer, _countof(buffer), _countof(buffer), pFormat, args);
		va_end(args);

		//
		wprintf(L"%s\n", buffer);
	}
}