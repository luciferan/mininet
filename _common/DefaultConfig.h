#pragma once

#include "util_common.h"
#include <string>

using namespace std;

//
class CIniFile
{
public:
	wstring m_wstrConfigFileName = {};
	WCHAR m_wcsFileName[1024+1] = {0,};

public:
	CIniFile();
	virtual ~CIniFile();

	bool GetValue(LPCWSTR appName, LPCWSTR keyName, LPINT pValue, INT nDefaultValue = -1);
	bool GetValue(LPCWSTR appName, LPCWSTR keyName, LPWORD pValue, WORD dwDefaultValue = -1);
	bool GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen);
};

//
class CDefaultConfig
	: public CIniFile
{
public:
	CDefaultConfig() {};
	virtual ~CDefaultConfig() {};
};

