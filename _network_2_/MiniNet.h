#pragma once
#ifndef __MININET_H__
#define __MININET_H__

//
#pragma comment(lib, "ws2_32.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <wchar.h>

#include <queue>

//#include "Connector.h"
//#include "Buffer.h"
//#include "PacketDataQueue.h"
//#include "../_common/util.h"
//#include "../_common/ExceptionReport.h"
#include "MiniNet_Header.h"

//
//enum class eNetworkResult
//{
//	RESULT_SUCC = 0,
//	RESULT_FAIL = 1,
//	RESULT_TIME_OUT,
//	RESULT_IO_PENDING,
//	RESULT_SOCKET_DISCONNECTED,
//};

//enum eNetwork
//{
//	MAX_THREAD_COUNT = 1,
//
//	MAX_LEN_IP4_STRING = 16,
//	MAX_LEN_HOST_DOMAIN = 1024,
//
//	MAX_LOG_BUFFER_SIZE = 1024 * 10,
//};

class CConnector;

//
class CMiniNet
{
public:
	HANDLE m_hNetworkHandle = INVALID_HANDLE_VALUE;

	HANDLE m_hAcceptThread = INVALID_HANDLE_VALUE;
	HANDLE m_hWorkerThread[eNetwork::MAX_THREAD_COUNT] = {INVALID_HANDLE_VALUE,};
	HANDLE m_hUpdateThread = INVALID_HANDLE_VALUE;

	//
	char m_szListenIP[eNetwork::MAX_LEN_IP4_STRING + 1] = {0,};
	WCHAR m_wcsListenIP[eNetwork::MAX_LEN_IP4_STRING + 1] = {0,};
	WORD m_wListenPort = 0;

	SOCKET m_ListenSock = INVALID_SOCKET;
	SOCKADDR_IN m_ListenAddr = {0,};

	//
	DWORD m_dwRunning = 0;
	DWORD m_dwListening = 0;
	INT64 m_biUpdateTime = 0;

	//
private:
	CMiniNet(void);
	~CMiniNet(void);

public:
	static CMiniNet& GetInstance()
	{
		static CMiniNet *pInstance = new CMiniNet;
		return *pInstance;
	}

	bool Initialize();
	bool Finalize();
		 
	bool Start();
	bool ListenStart(WCHAR *pwcsListenIP, const WORD wListenPort);
	bool Stop();

	bool AcceptRestart();
	bool AcceptStop();
	bool AcceptStart();

	static unsigned int WINAPI AcceptThread(void *p);
	static unsigned int WINAPI WorkerThread(void *p);
	static unsigned int WINAPI UpdateThread(void *p);

	eResultCode DoUpdate(INT64 biCurrTime);

	bool lookup_host(const char *hostname, std::string &hostIP);
	bool Listen(WCHAR *pwcsListenIP, const WORD wListenPort);
	CConnector* Connect(WCHAR * pwcszIP, const WORD wPort);

	bool Disconnect(CConnector *pSession);
	bool Disconnect(SOCKET socket);
	
	eResultCode Write(CConnector *pSession, char *pSendData, int iSendDataSize);
	eResultCode InnerWrite(CConnector *pSession, char *pSendData, int nSendDataSize);
};

//
extern void WriteMiniNetLog(std::wstring wstr);
extern void WritePacketLog(std::wstring str, const char *pPacketData, int nPacketDataSize);

//
#endif //__MININET_H__