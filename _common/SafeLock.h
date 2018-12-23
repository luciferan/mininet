#pragma once
#ifndef __SAFELOCK_H__
#define __SAFELOCK_H__

//
#include <windows.h>

//
class CSafeLock
{
private:
	LPCRITICAL_SECTION m_cs = nullptr;
public:
	CSafeLock(CRITICAL_SECTION &cs) { m_cs = &cs; EnterCriticalSection(m_cs); }
	~CSafeLock() { LeaveCriticalSection(m_cs); m_cs = NULL; }
};

class CLock
{
private:
	CRITICAL_SECTION m_cs;
public:
	CLock() { InitializeCriticalSectionAndSpinCount(&m_cs, 2000); }
	~CLock() { DeleteCriticalSection(&m_cs); }

	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }
};

class CScopeLock
{
private:
	CLock *m_Lock = nullptr;
public:
	CScopeLock(CLock &lock) { m_Lock = &lock; m_Lock->Lock(); }
	~CScopeLock() { m_Lock->Unlock(); m_Lock = nullptr; }
};

//
#endif //__SAFELOCK_H__