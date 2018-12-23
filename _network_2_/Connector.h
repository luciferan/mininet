#pragma once
#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

//
#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "Buffer.h"
#include "../_common/SafeLock.h"

#include <unordered_map>
#include <queue>
#include <list>

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
	void ResetOverlapped() { memset(&overlapped, 0, sizeof(OVERLAPPED)); }
};

//
class CConnector 
{
protected:
	DWORD m_dwUniqueIndex = 0;
	DWORD m_dwActive;

	void *m_pParam = nullptr;

	SOCKET m_Socket = INVALID_SOCKET;

public:
	SOCKADDR_IN m_SockAddr = {0,};

	char m_szDomain[eSession::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WCHAR m_wcsDomain[eSession::MAX_LEN_DOMAIN_STRING + 1] = {0,};
	WORD m_wPort = 0;

	// send
	OVERLAPPED_EX m_SendRequest; 
	CLock m_SendQueueLock;
	queue<CNetworkBuffer*> m_SendQueue = {}; // 여기있는걸 send 합니다. 최대 갯수를 넘어가면 전송량에 비해 보내야되는 패킷이 많은것
	DWORD m_dwSendRef = 0;

	// recv
	OVERLAPPED_EX m_RecvRequest;
	CCircleBuffer m_RecvDataBuffer; // recv 받으면 여기에 쌓습니다. 오버되면 연결을 터트립니다.
	DWORD m_dwRecvRef = 0;

	// inner
	OVERLAPPED_EX m_InnerRequest;
	CLock m_InnerQueueLock;
	queue<CNetworkBuffer*> m_InnerQueue = {}; //
	DWORD m_dwInnerRef = 0;

	//
	INT64 m_biUpdateTime = 0;
	INT64 m_biHeartBeatTime = 0;

	//
public:
	CConnector(DWORD dwUniqueIndex = 0);
	virtual ~CConnector();

	bool Initialize();
	bool Finalize();

	DWORD GetUniqueIndex() { return m_dwUniqueIndex; }
	DWORD GetIndex() { return m_dwUniqueIndex; }
	
	void SetActive() { InterlockedIncrement(&m_dwActive); }
	void SetDeactive() { InterlockedDecrement(&m_dwActive); }
	DWORD GetActive() { return InterlockedExchange(&m_dwActive, m_dwActive); }

	void* SetParam(void *pParam) { return m_pParam = pParam; }
	void* GetParam() { return m_pParam; }

	SOCKET SetSocket(SOCKET socket) { return m_Socket = socket; }
	SOCKET GetSocket() { return m_Socket; }

	void SetDomain(WCHAR *pwcsDomain, WORD wPort);
	void GetSocket2IP(char *pszIP);
	void ConvertSocket2IP();

	char* GetDomainA() { return m_szDomain; }
	WCHAR* GetDomain() { return m_wcsDomain; }
	WORD GetPort() { return m_wPort; }

	//
	int AddSendData(char *pSendData, DWORD dwSendDataSize);
	int AddSendQueue(char *pSendData, DWORD dwSendDataSize); // ret: sendqueue.size
	int SendPrepare(); //ret: send data size
	int SendComplete(DWORD dwSendSize); // ret: remain send data size

	WSABUF* GetSendWSABuffer();
	OVERLAPPED* GetSendOverlapped() { return (OVERLAPPED*)&m_SendRequest; }
	DWORD IncSendRef() { InterlockedIncrement(&m_dwSendRef); return m_dwSendRef; }
	DWORD DecSendRef() { InterlockedDecrement(&m_dwSendRef); return m_dwSendRef; }
	DWORD GetSendRef() { return m_dwSendRef; }

	// recv
	int RecvPrepare(); //ret: recv buffer size
	int RecvComplete(DWORD dwRecvSize); // ret: recvdatabuffer.getdatasize
	virtual int DataParsing();

	WSABUF* GetRecvWSABuffer();
	OVERLAPPED* GetRecvOverlapped() { return (OVERLAPPED*)&m_RecvRequest; }
	DWORD IncRecvRef() { InterlockedIncrement(&m_dwRecvRef); return m_dwRecvRef; }
	DWORD DecRecvRef() { InterlockedDecrement(&m_dwRecvRef); return m_dwRecvRef; }
	DWORD GetRecvRef() { return m_dwRecvRef; }

	//
	int AddInnerQueue(char *pSendData, DWORD dwSendDataSize);
	int InnerPrepare();
	int InnerComplete(DWORD dwInnerSize);

	WSABUF* GetInnerWSABuffer();
	OVERLAPPED* GetInnerOverlapped() { return (OVERLAPPED*)&m_InnerRequest; }
	DWORD IncInnerRef() { InterlockedIncrement(&m_dwInnerRef); return m_dwInnerRef; }
	DWORD DecInnerRef() { InterlockedDecrement(&m_dwInnerRef); return m_dwInnerRef; }
	DWORD GetInnerRef() { return m_dwInnerRef; }

	//
	virtual void DoUpdate(INT64 biCurrTime);
};

//
class CConnectorMgr 
{
private:
	DWORD m_dwConnectorIndex = 0;
	DWORD m_dwMaxConnectorCount = 0;

	list<CConnector> m_ConnectorList = {}; // 생성된 전체 리스트
	list<CConnector*> m_FreeConnectorList = {}; // 사용할수있는

public:
	list<CConnector*> m_UsedConnectorList = {}; // 사용중인
	CLock m_Lock;

	//
private:
	CConnectorMgr::CConnectorMgr(int nConnectorMax = 2000)
	{
		for( int cnt = 0; cnt < nConnectorMax; ++cnt )
		{
			CConnector *pData = new CConnector(GetUniqueIndex());
			
			m_ConnectorList.push_back(*pData);
			m_FreeConnectorList.push_back(pData);
		}

		m_dwMaxConnectorCount = nConnectorMax;
	}

	CConnectorMgr::~CConnectorMgr(void)
	{
		m_UsedConnectorList.clear();
		m_FreeConnectorList.clear();
		m_ConnectorList.clear();
	}

public:
	static CConnectorMgr& GetInstance()
	{
		static CConnectorMgr *pInstance = new CConnectorMgr();
		return *pInstance;
	}
	
	DWORD GetUniqueIndex() 
	{
		InterlockedIncrement((DWORD*)&m_dwConnectorIndex); 
		return m_dwConnectorIndex; 
	}

	CConnector* GetFreeConnector()
	{
		CConnector *pConnector = nullptr;

		{
			CScopeLock lock(m_Lock);

			if( m_FreeConnectorList.size() )
			{
				pConnector = m_FreeConnectorList.front();
				m_FreeConnectorList.pop_front();

				m_UsedConnectorList.push_back(pConnector);
			}
		}

		return pConnector;
	}

	void ReleaseConnector(CConnector *pConnector)
	{
		{
			CScopeLock lock(m_Lock);

			if( std::find(m_UsedConnectorList.begin(), m_UsedConnectorList.end(), pConnector) != m_UsedConnectorList.end() )
			{
				m_UsedConnectorList.remove(pConnector);
				m_FreeConnectorList.push_back(pConnector);
			}
		}
	}
};

//
#endif //__CONNECTOR_H__