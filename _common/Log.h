#pragma once
#ifndef __LOG_H__
#define __LOG_H__

//
#include <windows.h>
#include <wchar.h>
#include <string>

#include "SafeLock.h"

//
using namespace std;

enum class eLogType
{
	
};

enum class eLogDivideType
{
	Day,
	Hour,
	Min,
};

//
class CLog
{
private:
	wstring m_wstrFileName = {};
	eLogDivideType m_eDivideType = eLogDivideType::Day;

	CLock m_FileLock;
	FILE *m_pFile = nullptr;


	//
private:
	void ConsoleWrite(const WCHAR *pwcsLogText);
	void FileWrite(const WCHAR *pwcsLogText);

public:
	CLog();
	virtual ~CLog();

	bool Set(wstring wstrFileName);

	void Write(std::wstring wstr);
	void Write(WCHAR *pwcsFormat, ...);

};

//
extern CLog g_Log;
extern CLog g_PerformanceLog;

//
#endif //__LOG_H__