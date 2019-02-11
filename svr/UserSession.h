#pragma once
#ifndef __USERSESSION_H__
#define __USERSESSION_H__

//
#include <Windows.h>

#include <list>
#include <queue>

#include "../_common/SafeLock.h"

//
class CConnector;

//
class CUserSession
{
private:
	DWORD m_dwIndex = 0;

	CConnector* m_pConnector = nullptr;

	INT64 m_biHeartBeat = 0;
	INT64 m_biUpdateTime = 0;
public:
	//DWORD m_dwUsed = 0;
	//DWORD m_dwActive = 0;

	bool m_bAuthed = false;

	//
public:
	CUserSession(DWORD dwIndex = 0) { m_dwIndex = dwIndex; }
	virtual ~CUserSession() {}

	int Clear();
	int Release();

	//
	DWORD GetIndex() { return m_dwIndex; }
	//DWORD GetActive() { return InterlockedExchange(&m_dwActive, m_dwActive); }
	//DWORD GetUsed() { return InterlockedExchange(&m_dwUsed, m_dwUsed); }
	bool GetActive() { return m_bAuthed; }
	bool GetUsed() { return (nullptr != m_pConnector); }

	void SetConnector(CConnector *pConnector) { m_pConnector = pConnector; /*InterlockedExchange(&m_dwActive, 1);*/ }
	CConnector* GetConnector() { return m_pConnector; }

	void SetUpdateTime(INT64 biCurrTime) { m_biUpdateTime = biCurrTime; }
	INT64 GetUpdateTime() { return m_biUpdateTime; }
	bool CheckUpdateTime(INT64 biCurrTime) { return (m_biUpdateTime > biCurrTime); }

	//
	int MessageProcess(char *pData, int nLen);
	int DoUpdate(INT64 uiCurrTime);

	eResultCode SendPacketData(DWORD dwProtocol, char *pData, DWORD dwDataSize);

	//
	eResultCode RepAuth();
	eResultCode RepEcho();

};

//
class CUserSessionMgr
{
private:
	DWORD m_dwUserSessionIndex;
	int m_nMaxUserSessionCount;
	std::list<CUserSession> m_UserSessionList;
	std::list<CUserSession*> m_FreeUserSessionList;
	
public:
	std::list<CUserSession*> m_UsedUserSessionList;
	CLock m_Lock;

	//
private:
	CUserSessionMgr(int nMaxCount = 2000)
	{
		for( int cnt = 0; cnt < nMaxCount; ++cnt )
		{
			CUserSession *pData = new CUserSession(GetUniqueIndex());

			m_UserSessionList.push_back(*pData);
			m_FreeUserSessionList.push_back(pData);
		}

		m_nMaxUserSessionCount = nMaxCount;
	}

	~CUserSessionMgr()
	{
		m_UserSessionList.clear();
		m_FreeUserSessionList.clear();
		m_UsedUserSessionList.clear();
	}

	DWORD GetUniqueIndex()
	{
		InterlockedIncrement((DWORD*)&m_dwUserSessionIndex);
		return m_dwUserSessionIndex;
	}

public:
	static CUserSessionMgr& GetInstance()
	{
		static CUserSessionMgr *pInstance = new CUserSessionMgr();
		return *pInstance;
	}

	CUserSession* GetFreeUserSession()
	{
		CUserSession *pRet = nullptr;

		{
			CScopeLock lock(m_Lock);

			if( false == m_FreeUserSessionList.empty() )
			{
				pRet = m_FreeUserSessionList.front();
				m_FreeUserSessionList.pop_front();

				m_UsedUserSessionList.push_back(pRet);
			}
		}

		return pRet;
	}

	void ReleaseUserSesssion(CUserSession *pUserSession)
	{
		{
			CScopeLock lock(m_Lock);

			if( std::find(m_UsedUserSessionList.begin(), m_UsedUserSessionList.end(), pUserSession) != m_UsedUserSessionList.end() )
			{
				//InterlockedExchange(&pUserSession->m_dwUsed, 0);
				//InterlockedExchange(&pUserSession->m_dwActive, 0);

				m_UsedUserSessionList.remove(pUserSession);
				m_FreeUserSessionList.push_back(pUserSession);
			}
			else
			{
			}
		}
	}
};

#endif //__USERSESSION_H__