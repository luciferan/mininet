#include "DefaultConfig.h"

//
CIniFile::CIniFile()
{
	//m_wstrConfigFileName = GetFileName();
	m_wstrConfigFileName.append(L"./config/");
	m_wstrConfigFileName.append(GetFileName());
	m_wstrConfigFileName.append(L".ini");
}

CIniFile::~CIniFile()
{
}

void CIniFile::SetConfigFile(wstring wstrFileName)
{
	m_wstrConfigFileName = wstrFileName;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, INT &nValue, INT nDefaultValue /*= -1*/)
{
	nValue = GetPrivateProfileIntW(appName, keyName, nDefaultValue, m_wstrConfigFileName.c_str());

	return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, WORD &wValue, WORD wDefaultValue /*= -1*/)
{
	int tempValue = wDefaultValue;
	tempValue = GetPrivateProfileIntW(appName, keyName, wDefaultValue, m_wstrConfigFileName.c_str());
	if( USHRT_MAX < tempValue )
		wValue = wDefaultValue;
	else
		wValue = (WORD)tempValue;

	return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, INT64 &biValue, INT64 biDefaultValue /*= -1*/)
{
	WCHAR wcsTempString[25 + 1] = {};
	DWORD dwTempStringLen = 25;

	GetPrivateProfileStringW(appName, keyName, L"", wcsTempString, dwTempStringLen, m_wstrConfigFileName.c_str());
	dwTempStringLen = wcslen(wcsTempString);
	if( 20 < dwTempStringLen ) //INT64_MIN "-9223372036854775807"
		biValue = biDefaultValue;
	else
		biValue = _wtoi64(wcsTempString);

	return;
}

void CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen)
{
	GetPrivateProfileStringW(appName, keyName, L"", pwcsResult, dwResultLen, m_wstrConfigFileName.c_str());
	
	return;
}
