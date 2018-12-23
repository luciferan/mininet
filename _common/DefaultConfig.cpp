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

bool CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, LPINT pValue, INT nDefaultValue /*= -1*/)
{
	*pValue = GetPrivateProfileIntW(appName, keyName, nDefaultValue, m_wstrConfigFileName.c_str());
	return true;
}

bool CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, LPWORD pValue, WORD dwDefaultValue /*= -1*/)
{
	int nValue = dwDefaultValue;
	nValue = GetPrivateProfileIntW(appName, keyName, dwDefaultValue, m_wstrConfigFileName.c_str());
	if( 0xffff < nValue )
		*pValue = dwDefaultValue;
	*pValue = (WORD)nValue;
	return true;
}

bool CIniFile::GetValue(LPCWSTR appName, LPCWSTR keyName, LPWSTR pwcsResult, DWORD dwResultLen)
{
	GetPrivateProfileStringW(appName, keyName, L"", pwcsResult, dwResultLen, m_wstrConfigFileName.c_str());
	return true;
}
