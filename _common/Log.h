#pragma once
#ifndef __LOG_H__
#define __LOG_H__

//
#include <windows.h>
#include <wchar.h>

#include <string>

//
class CLog
{
private:

	//
public:
	CLog();
	virtual ~CLog();

	void write(std::wstring wstr);
	void write(WCHAR *pwcsFormat, ...);
};

//
#endif //__LOG_H__