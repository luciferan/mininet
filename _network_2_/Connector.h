#pragma once
#ifndef __OSESSIONMGR__
#define __OSESSIONMGR__

//
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "Buffer.h"
#include "../_common/SafeLock.h"

#include <unordered_map>

using namespace std;

//
class CConnector;

//
enum eSession
{
	MAX_LEN_IP4_STRING = 16,
	MAX_LEN_DOMAIN_STRING = 1024,
};

struct OVERLAPPED_EX
{
	OVERLAPPED overlapped = {0,};

	CConnector *pSession = nullptr;
	CNetworkBuffer *pBuffer = nullptr;

	//
	void ResetOverlapped() 
	{
		memset(&overlapped, 0, sizeof(OVERLAPPED));
	}
};

//
class CConnector 
	: public CMemoryPool<CConnector>
{
protected:
	DWORD m_dwUniqueIndex = 0;
	void *m_pParam = nullptr;

	SOCKET m_Socket = INVALID_SOCKET;

public:
	SOCKADDR_IN m_SockAddr = {0,};

	char m_szDomain[eSession::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WCHAR m_wcsDomain[eSession::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WORD m_wPort = 0;

	OVERLAPPED_EX m_RecvRequest;
	OVERLAPPED_EX m_SendRequest;
	OVERLAPPED_EX m_InnerRequest;

	CCircleBuffer m_RecvDataBuffer;

	//
public:
	CConnector();
	virtual ~CConnector();

	DWORD GetUniqueIndex() { return m_dwUniqueIndex; }
	DWORD GetIndex() { return m_dwUniqueIndex; }

	void* SetParam(void *pParam) { return m_pParam = pParam; }
	void* GetParam() { return m_pParam; }

	SOCKET SetSocket(SOCKET socket) { return m_Socket = socket; }
	SOCKET GetSocket() { return m_Socket; }

	//bool Initialize();
	bool Finalize();

	void SetDomain(WCHAR *pwcsDomain, WORD wPort);
	void GetSocket2IP(char *pszIP);
	void ConvertSocket2IP();

	char* GetDomainA() { return m_szDomain; }
	WCHAR* GetDomain() { return m_wcsDomain; }
	WORD GetPort() { return m_wPort; }

	int AddRecvData(CNetworkBuffer *pBuffer);
	virtual int DataParsing();

	int SendRequest(char *pSendData, int nSendDataSize); //ret: send data size
	int SendComplete(DWORD dwSendSize);
	WSABUF* GetSendWSABuffer();
	OVERLAPPED* GetSendOverlapped() { return (OVERLAPPED*)&m_SendRequest; }

	int RecvRequest(); //ret: recv buffer size
	WSABUF* GetRecvWSABuffer();
	OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&m_RecvRequest; }

	int InnerRequest(char *pSendData, int nSendDataSize);
	int InnerComplete(DWORD dwInnerSize);
	WSABUF* GetInnerWSABuffer();
	OVERLAPPED* GetInnerOverlapped() { return (OVERLAPPED*)&m_InnerRequest; }

	virtual void DoUpdate();
};

//
class CConnectorMgr 
{
private:
	CRITICAL_SECTION m_cs;
	DWORD m_dwConnectorIndex = 0;

public:
	unordered_map<DWORD, CConnector*> m_mapConnector = {};

	//
private:
	CConnectorMgr::CConnectorMgr(void)
	{
		InitializeCriticalSectionAndSpinCount(&m_cs, 2000);
		m_mapConnector.clear();
	}

	CConnectorMgr::~CConnectorMgr(void)
	{
		DeleteCriticalSection(&m_cs);
	}

public:
	static CConnectorMgr& GetInstance()
	{
		static CConnectorMgr *pInstance = new CConnectorMgr();
		return *pInstance;
	}
	
	void Lock() { EnterCriticalSection(&m_cs); }
	void Unlock() { LeaveCriticalSection(&m_cs); }

	DWORD GetUniqueIndex() 
	{
		InterlockedIncrement((DWORD*)&m_dwConnectorIndex); 
		return m_dwConnectorIndex; 
	}

	DWORD RegistConnector(CConnector *pConnector) 
	{
		if( !pConnector )
			return -1;

		auto registKey = GetUniqueIndex();

		CSafeLock lock(m_cs);
		m_mapConnector.insert(make_pair(registKey, pConnector));
		return registKey;
	}

	void RemoveConnector(CConnector *pConnector)
	{
		if( pConnector )
			return;

		CSafeLock lock(m_cs);
		auto iter = m_mapConnector.find(pConnector->GetUniqueIndex());
		if( iter == m_mapConnector.end() )
			return;
		m_mapConnector.erase(iter);
	}

	CConnector* GetFreeConnector()
	{
		CConnector *pConnector = new CConnector;
		if( !pConnector )
			return nullptr;

		//RegistConnector(pConnector);
		return pConnector;
	}

	void ReleaseConnector(CConnector *pConnector)
	{
		//RemoveConnector(pConnector);

		SAFE_DELETE(pConnector);
	}
};

//
#endif //__OSESSIONMGR__