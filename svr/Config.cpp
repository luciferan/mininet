#include "Config.h"
#include <stdio.h>

//
CConfig g_Config;

//
bool CConfig::LoadConfig()
{
	// network info
	GetValue(L"network", L"host", wcsHost, _countof(wcsHost));
	GetValue(L"network", L"port", wPort);

	//
	return true;
}