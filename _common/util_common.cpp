#include "util_common.h"

//
std::wstring GetFileName()
{
	WCHAR wcsProgramName[512 + 1] = {0,};
	WCHAR wcsDrive[8 + 1] = {0,}, wcsDir[512 + 1] = {0,}, wcsFileName[128 + 1] = {0,}, wcsFileExt[32 + 1] = {0,};

	GetModuleFileNameW(NULL, wcsProgramName, _countof(wcsProgramName));
	_wsplitpath_s(wcsProgramName, wcsDrive, wcsDir, wcsFileName, wcsFileExt);

	return wcsFileName;
}