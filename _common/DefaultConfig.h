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
	void SetConfigFile(wstring wstrFileName);

	void GetValue(LPCWSTR appName, LPCWSTR keyName, WORD &wValue, WORD wDefaultValue = -1);
	void GetValue(LPCWSTR appName, LPCWSTR keyName, INT &nValue, INT nDefaultValue = -1);
	void GetValue(LPCWSTR appName, LPCWSTR keyName, INT64 &biValue, INT64 biDefaultValue = -1);
	void GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen);
};

//
class CDefaultConfig
	: public CIniFile
{
public:
	CDefaultConfig() {};
	virtual ~CDefaultConfig() {};
};

