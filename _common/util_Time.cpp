#include "util_Time.h"
#include <stdio.h>

//
int GetTimeSec()
{
	struct __timeb32 tTime;
	_ftime32_s(&tTime);

	return (int)(tTime.time);
}

INT64 GetTimeMilliSec()
{
	struct __timeb64 tTime;
	_ftime64_s(&tTime);

	return (INT64)(tTime.time * 1000 + tTime.millitm);
}

//
CTimeSet::CTimeSet()
{
	SetTime();
}

CTimeSet::CTimeSet(__time64_t tTime, bool bGMT)
{
	SetTime(tTime, bGMT);
}

CTimeSet::CTimeSet(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, bool bGMT)
{
	SetTime(nYear, nMonth, nDay, nHour, nMin, nSec, bGMT);
}

CTimeSet::CTimeSet(TIMESTAMP_STRUCT dbTimeStamp)
{
	SetTime(dbTimeStamp.year, dbTimeStamp.month, dbTimeStamp.day, dbTimeStamp.hour, dbTimeStamp.minute, dbTimeStamp.second, false);
}

void CTimeSet::SetTime()
{
	_time64(&m_tGMT);
	_gmtime64_s(&m_tmGMT, &m_tGMT);
	_localtime64_s(&m_tmLocalT, &m_tGMT);
	m_tLocalT = _mkgmtime64(&m_tmLocalT);
}

void CTimeSet::SetTime(__time64_t tTime, bool bGMT)
{
	struct tm *ptmTime = nullptr;

	if( bGMT )
		m_tGMT = tTime;
	else
		m_tGMT = ConvertLC2GM(tTime);

	_gmtime64_s(&m_tmGMT, &m_tGMT);

	_localtime64_s(&m_tmLocalT, &m_tGMT);
	m_tLocalT = _mkgmtime64(&m_tmLocalT);
}

void CTimeSet::SetTime(int nYear, int nMon, int nDay, int nHour, int nMin, int nSec, bool bGMT)
{
	struct tm tmTime = {0,};

	tmTime.tm_year = nYear - 1900;
	tmTime.tm_mon = nMon - 1;
	tmTime.tm_mday = nDay;
	tmTime.tm_hour = nHour;
	tmTime.tm_min = nMin;
	tmTime.tm_sec = nSec;
	tmTime.tm_isdst = -1; 

	struct tm *ptmTime = nullptr;

	if( bGMT )
		m_tGMT = _mkgmtime64(&tmTime);
	else
		m_tGMT = ConvertLC2GM(tmTime);

	_gmtime64_s(&m_tmGMT, &m_tGMT);

	localtime_s(&m_tmLocalT, &m_tGMT);
	m_tLocalT = _mkgmtime64(&m_tmLocalT);
}

void CTimeSet::SetTime(TIMESTAMP_STRUCT dbTimeStamp)
{
	SetTime(dbTimeStamp.year, dbTimeStamp.month, dbTimeStamp.day, dbTimeStamp.hour, dbTimeStamp.minute, dbTimeStamp.second, false);
}

__time64_t CTimeSet::ConvertLC2GM(__time64_t tTime)
{
	__time64_t tGMT = 0;
	__time64_t tLocal = 0;

	struct tm *ptmLocal = NULL;

	_time64(&tGMT);
	localtime_s(ptmLocal, &tGMT);
	tLocal = _mkgmtime64(ptmLocal);

	__time64_t tTimeGap = tLocal - tGMT;

	//
	return tTime - tTimeGap;
}

__time64_t CTimeSet::ConvertLC2GM(struct tm tmLocal)
{
	__time64_t tGMT = 0;
	__time64_t tLocal = 0;

	struct tm *ptmLocal = NULL;

	_time64(&tGMT);
	localtime_s(ptmLocal, &tGMT);
	tLocal = _mkgmtime64(ptmLocal);

	__time64_t tTimeGap = tLocal - tGMT;

	//
	__time64_t tTime = _mkgmtime64(&tmLocal);
	return tTime - tTimeGap;
}

void CTimeSet::_test()
{
	// time 함수 테스트
	{
		struct tm tm_gmt, tm_local;
		__time64_t time_gmt = 0, time_local = 0, time_gap = 0;

		time_gmt = _time64(NULL);
		_gmtime64_s(&tm_gmt, &time_gmt);
		_localtime64_s(&tm_local, &time_gmt);
		time_local = _mkgmtime64(&tm_local);
		time_gap = time_local - time_gmt;

		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_gmt.tm_year + 1900, tm_gmt.tm_mon + 1, tm_gmt.tm_mday, tm_gmt.tm_hour, tm_gmt.tm_min, tm_gmt.tm_sec);
		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday, tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
		wprintf(L"gmt:%I64d, local:%I64d, gap:%I64d(%I64dh)\n", time_gmt, time_local, time_gap, time_gap / 60 / 60);
	}

	// ctimeset 테스트
	{
		CTimeSet currTime;

		__time64_t time_gmt = currTime.GetTime_GM();
		__time64_t time_local = currTime.GetTime_LC();
		__time64_t time_gap = time_local - time_gmt;
		struct tm &tm_gmt = currTime.GetTimeST_GM();
		struct tm &tm_local = currTime.GetTimeST_LC();

		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_gmt.tm_year + 1900, tm_gmt.tm_mon + 1, tm_gmt.tm_mday, tm_gmt.tm_hour, tm_gmt.tm_min, tm_gmt.tm_sec);
		wprintf(L"%d-%d-%d %d:%d:%d\n", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday, tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
		wprintf(L"gmt:%I64d, local:%I64d, gap:%I64d(%I64dh)\n", time_gmt, time_local, time_gap, time_gap / 60 / 60);
	}
}