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

	DWORD m_dwUsed = 0;
	DWORD m_dwActive = 0;

	INT64 m_uiHeartBeat = 0;

	//
public:
	CUserSession(DWORD dwIndex = 0)
	{
		m_dwIndex = dwIndex;
	}
	virtual ~CUserSession()
	{
	}

	int clear();

	DWORD GetIndex() { return m_dwIndex; }
	void SetConnector(CConnector *pConnector) { m_pConnector = pConnector; }

	//
	int MessageProcess(char *pData, int nLen);

	int DoUpdate(INT64 uiCurrTime);
	eResultCode SendPacketData(DWORD dwProtocol, char *pData, DWORD dwDataSize);

	//
	eResultCode ReqAuth();
	eResultCode ReqEcho();

	eResultCode RepHeartBeat();
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