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

#include "Connector.h"
#include "Buffer.h"
#include "PacketDataQueue.h"
//#include "../_common/util_String.h"
#include "../_common/util.h"
#include "../_common/ExceptionReport.h"

//
enum eNetwork
{
	RESULT_SUCC = 0,
	RESULT_FAIL = 1,
	RESULT_TIME_OUT,
	RESULT_IO_PENDING,
	RESULT_SOCKET_DISCONNECTED,

	MAX_THREAD_COUNT = 1,

	MAX_LEN_IPV4_STRING = 16,
	MAX_LEN_HOST_DOMAIN = 1024,

	MAX_LOG_BUFFER_SIZE = 1024 * 10,
};

//
class CMiniNet
{
public:
	HANDLE m_hNetworkHandle = INVALID_HANDLE_VALUE;

	HANDLE m_hAcceptThread = INVALID_HANDLE_VALUE;
	HANDLE m_hWorkerThread[eNetwork::MAX_THREAD_COUNT] = {INVALID_HANDLE_VALUE,};
	HANDLE m_hSenderThread = INVALID_HANDLE_VALUE;
	HANDLE m_hPacketThread = INVALID_HANDLE_VALUE;

	//
	//char m_szListenIP[eNetwork::MAX_LEN_IPV4_STRING + 1] = {0,};
	WCHAR m_wcsListenIP[eNetwork::MAX_LEN_IPV4_STRING + 1] = {0,};
	WORD m_wListenPort = 0;

	SOCKET m_ListenSock = INVALID_SOCKET;
	SOCKADDR_IN m_ListenAddr = {0,};

	//
	DWORD m_dwRunning = 0;

	HANDLE m_hRecvQueueEvent = INVALID_HANDLE_VALUE;
	HANDLE m_hSendQueueEvent = INVALID_HANDLE_VALUE;

	//
private:
	CMiniNet(void);
	~CMiniNet(void);

public:
	static CMiniNet& GetInstance()
	{
		static CMiniNet *pInstance = new CMiniNet();
		return *pInstance;
	}

	bool Initialize();
	bool Finalize();
		 
	bool Start();
	bool Stop();

	static unsigned int WINAPI AcceptThread(void *p);
	static unsigned int WINAPI WorkerThread(void *p);

	static unsigned int WINAPI PacketProcess(void *p);
	static unsigned int WINAPI SendThread(void *p);
	static unsigned int WINAPI DoUpdate(void *p);

	bool Listen(WCHAR *pwcsListenIP, const WORD wListenPort);
	CConnector* Connect(WCHAR * pwcszIP, const WORD wPort);

	BOOL Disconnect(CConnector *pSession);
	BOOL Disconnect(SOCKET socket);
	
	int Write(CConnector *pSession, char *pSendData, int iSendDataSize);
	int InnerWrite(CConnector *pSession, char *pSendData, int nSendDataSize);
};

//
extern void WriteMiniNetLog(std::wstring wstr);
extern void WritePacketLog(std::wstring str, const char *pPacketData, int nPacketDataSize);

//
#endif //__MININET_H__