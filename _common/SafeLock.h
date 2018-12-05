#pragma once
#ifndef __SAFELOCK_H__
#define __SAFELOCK_H__

//
#include <windows.h>

//
class CSafeLock
{
	LPCRITICAL_SECTION m_cs;

public:
	CSafeLock(CRITICAL_SECTION &cs) { m_cs = &cs; EnterCriticalSection(m_cs); }
	~CSafeLock() { LeaveCriticalSection(m_cs); m_cs = NULL;}
};

//
#endif //__SAFELOCK_H__