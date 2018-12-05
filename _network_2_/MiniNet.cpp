#include "./MiniNet.h"

//
CMiniNet::CMiniNet(void)
{
	InterlockedExchange((LONG*)&m_dwRunning, 0);
}

CMiniNet::~CMiniNet(void)
{
	InterlockedExchange((LONG*)&m_dwRunning, 0);
	Finalize();
}

bool CMiniNet::Initialize()
{
	bool bRet = false;

	//
	WSADATA wsa;
	if( 0 != WSAStartup(MAKEWORD(2, 2), &wsa) )
		return false;

	m_hNetworkHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if( NULL == m_hNetworkHandle )
	{
		WriteMiniNetLog(L"### MiniNet::Init() : NetworkHandle create fail");
		return false;
	}

	// thread 초기화
	unsigned int uiThreadID = 0;
	for( int iIndex = 0; iIndex < eNetwork::MAX_THREAD_COUNT; ++iIndex )
	{
		m_hWorkerThread[iIndex] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if( NULL == m_hWorkerThread[iIndex] )
		{
			WriteMiniNetLog(L"### MiniNet::Init() : Worker thread create fail");
			return false;
		}
	}

	{
		m_hPacketThread = (HANDLE)_beginthreadex(NULL, 0, PacketProcess, NULL, CREATE_SUSPENDED, &uiThreadID);
		if( NULL == m_hPacketThread )
		{
			WriteMiniNetLog(L"### MiniNet::Init() : Packet thread create fail");
			return false;
		}
		m_hSenderThread = (HANDLE)_beginthreadex(NULL, 0, SendThread, NULL, CREATE_SUSPENDED, &uiThreadID);
		if( NULL == m_hSenderThread )
		{
			WriteMiniNetLog(L"### MiniNet::Init() : Sender thread create fail");
			return false;
		}
	}

	// 이벤트 객체 초기화
	m_hRecvQueueEvent = CreateEvent(0, 0, 0, 0);
	if( INVALID_HANDLE_VALUE == m_hRecvQueueEvent )
	{
		WriteMiniNetLog(L"### MiniNet::Init() : QueueEventHandle create fail");
		return false;
	}

	m_hSendQueueEvent = CreateEvent(0, 0, 0, 0);
	if( INVALID_HANDLE_VALUE == m_hSendQueueEvent )
	{
		WriteMiniNetLog(L"### MiniNet::Init() : QueueEventHandle create fail");
		return false;
	}

	//
	return true;
}

bool CMiniNet::Finalize()
{
	if( NULL != m_hRecvQueueEvent )
		CloseHandle(m_hRecvQueueEvent);

	//
	WSACleanup();

	//
	return true;
}

bool CMiniNet::Start()
{
	InterlockedExchange((LONG*)&m_dwRunning, 1);

	for( int iIndex = 0; iIndex < eNetwork::MAX_THREAD_COUNT; ++iIndex )
	{
		if( INVALID_HANDLE_VALUE != m_hWorkerThread[iIndex] )
			ResumeThread(m_hWorkerThread[iIndex]);
	}

	{
		if( INVALID_HANDLE_VALUE != m_hAcceptThread )
			ResumeThread(m_hAcceptThread);
		if( INVALID_HANDLE_VALUE != m_hPacketThread )
			ResumeThread(m_hPacketThread);
		if( INVALID_HANDLE_VALUE != m_hSenderThread )
			ResumeThread(m_hSenderThread);
	}

	//
	return true;
}

bool CMiniNet::Stop()
{
	InterlockedExchange((LONG*)&m_dwRunning, 0);

	//
	for( int nIndex = 0; nIndex < eNetwork::MAX_THREAD_COUNT; ++nIndex )
	{
		PostQueuedCompletionStatus(m_hNetworkHandle, 0, 0, NULL);
	}
	for( int nIndex = 0; nIndex < eNetwork::MAX_THREAD_COUNT; ++nIndex )
	{
		WaitForSingleObject(m_hWorkerThread[nIndex], INFINITE);
		m_hWorkerThread[nIndex] = INVALID_HANDLE_VALUE;
	}

	//
	if( INVALID_SOCKET != m_ListenSock )
	{
		closesocket(m_ListenSock);
		m_ListenSock = INVALID_SOCKET;

		WaitForSingleObject(m_hAcceptThread, INFINITE);
		m_hAcceptThread = INVALID_HANDLE_VALUE;
	}

	m_hNetworkHandle = INVALID_HANDLE_VALUE;

	//
	SetEvent(m_hRecvQueueEvent);
	CRecvPacketQueue::GetInstance().ForceActivateQueueEvent();

	//
	return true;
}

unsigned int WINAPI CMiniNet::AcceptThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CConnectorMgr &ConnectMgr = CConnectorMgr::GetInstance();

	//
	HANDLE &hNetworkHandle = Net.m_hNetworkHandle;

	DWORD dwRunning = Net.m_dwRunning;
	SOCKET &ListenSock = Net.m_ListenSock;

	CConnector *pSession = NULL;

	INT32	iWSARet = 0;
	HANDLE	hRet = INVALID_HANDLE_VALUE;
	BOOL	bRet = FALSE;

	DWORD	dwRecvDataSize = 0;
	DWORD	dwFlag = 0;

	//
	WriteMiniNetLog(L"### MiniNet::AcceptThread() : start");

	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		SOCKADDR_IN AcceptAddr;
		memset(&AcceptAddr, 0, sizeof(AcceptAddr));
		int iAcceptAddrLen = sizeof(AcceptAddr);

		//
		SOCKET AcceptSock = WSAAccept(ListenSock, (SOCKADDR*)&AcceptAddr, &iAcceptAddrLen, NULL, NULL);
		if( INVALID_SOCKET == AcceptSock )
		{
			int nErrorCode = WSAGetLastError();
			WriteMiniNetLog(FormatW(L"### MiniNet::AcceptThread() : accept() fail. error:%d", nErrorCode));
			continue;
		}

		BOOL bSockOpt = TRUE;
		bSockOpt = TRUE;
		setsockopt(AcceptSock, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
		bSockOpt = TRUE;
		setsockopt(AcceptSock, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));

		//
		pSession = ConnectMgr.GetFreeConnector(); // new CConnector;
		if( !pSession )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::AcceptThread() : session create fail. socket:%d", AcceptSock));
			closesocket(AcceptSock);
			continue;
		}

		//
		pSession->SetSocket(AcceptSock);
		memcpy((void*)&pSession->m_SockAddr, (void*)&AcceptAddr, sizeof(SOCKADDR_IN));

		pSession->ConvertSocket2IP();

		//
		hRet = CreateIoCompletionPort((HANDLE)AcceptSock, hNetworkHandle, (ULONG_PTR)pSession, 0);
		if( hNetworkHandle != hRet )
		{
			int nErrorCode = GetLastError();
			WriteMiniNetLog(FormatW(L"### MiniNet::AcceptThread() : <%d> CreateIoCompletionPort() fail. socket:%d. error:%d", pSession->GetIndex(), AcceptSock, nErrorCode));

			closesocket(AcceptSock);
			SAFE_DELETE(pSession);
			continue;
		}

		//
		if( 0 >= pSession->RecvRequest() )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::AcceptThread() : <%d> RecvRequest() fail. socket:%d", pSession->GetIndex(), AcceptSock));
			closesocket(AcceptSock);
			SAFE_DELETE(pSession);
			continue;
		}

		//
		dwFlag = 0;
		iWSARet = WSARecv(pSession->GetSocket(), pSession->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlag, pSession->GetRecvOverlapped(), NULL);
		if( SOCKET_ERROR == iWSARet )
		{
			int nErrorCode = WSAGetLastError();
			if( WSA_IO_PENDING != nErrorCode )
			{
				WriteMiniNetLog(FormatW(L"### MiniNet::AcceptThread() : <%d> WSARecv fail. socket:%d. error:%d", pSession->GetIndex(), AcceptSock, nErrorCode));
				closesocket(AcceptSock);
				SAFE_DELETE(pSession);
			}
		}
	}

	return 1;
}

unsigned int WINAPI CMiniNet::WorkerThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();

	CConnectorMgr &SessionMgr = CConnectorMgr::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;

	OVERLAPPED *lpOverlapped = nullptr;
	OVERLAPPED_EX *lpOverlappedEx = nullptr;
	int nSessionIndex = 0;

	DWORD dwIOSize = 0;
	DWORD dwFlags = 0;

	BOOL bRet = FALSE;
	int iRet = 0;
	int nErrorCode = 0;

	CConnector *pSession = nullptr;
	CNetworkBuffer *pNetworkBuffer = nullptr;

	DWORD dwSendDataSize = 0;
	DWORD dwRecvDataSize = 0;

	HANDLE &hNetowkHandle = Net.m_hNetworkHandle;
	HANDLE &hRecvQueueEvent = Net.m_hRecvQueueEvent;

	//
	WriteMiniNetLog(L"### MiniNet::WorkerThread() : start"); 

	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		dwIOSize = 0;
		bRet = FALSE;

		bRet = GetQueuedCompletionStatus(hNetowkHandle, &dwIOSize, (PULONG_PTR)&pSession, &lpOverlapped, INFINITE);
		if( FALSE == bRet )
		{
			nErrorCode = WSAGetLastError();
			if( WAIT_TIMEOUT == nErrorCode )
			{
				continue;
			}

			//
			WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : GetQueuedCompletionStatus() result false. lpOverlapped %08X. error:%d", lpOverlapped, nErrorCode));

			//
			if( NULL == lpOverlapped )
			{
				continue;
			}

			continue;
		}

		//
		if( NULL == lpOverlapped )
		{
			WriteMiniNetLog(L"### MiniNet::WorkerThread() : lpOverlapped is NULL");
			continue;
		}

		//
		lpOverlappedEx = (OVERLAPPED_EX*)lpOverlapped;
		pSession = lpOverlappedEx->pSession;
		if( !pSession )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : Invalid Session %08X", pSession));
			continue;
		}

		pNetworkBuffer = lpOverlappedEx->pBuffer;
		if( !pNetworkBuffer )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : Invalid NetworkBuffer %08X", pNetworkBuffer));
			continue;
		}

		//
		if( 0 == dwIOSize )
		{
			//
			if( pSession )
			{
				WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : <%d> Disconnect. IP %s, Socket %d", pSession->GetIndex(), pSession->GetDomain(), pSession->GetSocket()));

				//
				if( INVALID_SOCKET != pSession->GetSocket() )
				{
					shutdown(pSession->GetSocket(), SD_BOTH);
					closesocket(pSession->GetSocket());
				}

				SAFE_DELETE(pSession);
			}
			else
			{
				WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : <%d> IoSize %d, pCurrSockEx %08X", dwIOSize, pSession));
			}

			continue;
		}

		//
		switch( pNetworkBuffer->GetOperator() )
		{
		case eNetworkBuffer::OP_SEND:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread() : <%d> SendResult : socket:%d. SendRequestSize %d, SendSize %d", pSession->GetIndex(), pSession->GetSocket(), (int)pNetworkBuffer->m_nDataSize, dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> SendData:", pSession->GetIndex()), pNetworkBuffer->m_pBuffer, dwIOSize);

				//
				DWORD dwRemainData = pSession->SendComplete(dwIOSize);
				if( 0 < dwRemainData )
				{
					iRet = WSASend(pSession->GetSocket(), pSession->GetSendWSABuffer(), 1, &dwSendDataSize, 0, pSession->GetSendOverlapped(), NULL);
					if( SOCKET_ERROR == iRet )
					{
						nErrorCode = WSAGetLastError();
						if( WSA_IO_PENDING != nErrorCode )
						{
							WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : <%d> WSASend() fail. socket:%d. error:%d", pSession->GetIndex(), pSession->GetSocket(), nErrorCode));
							Net.Disconnect(pSession);
						}
					}
				}
			}
			break;
		case eNetworkBuffer::OP_RECV:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread() : <%d> RecvResult : socket:%d. RecvDataSize %d", pSession->GetIndex(), pSession->GetSocket(), dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> RecvData: ", pSession->GetIndex()), pNetworkBuffer->m_pBuffer, dwIOSize);

				//
				pNetworkBuffer->m_nDataSize = dwIOSize;

				pSession->AddRecvData(pNetworkBuffer);
				int nPacketLength = pSession->DataParsing();
				if( 0 < nPacketLength )
				{
					CPacketStruct *pPacketData = new CPacketStruct();

					pPacketData->m_nDataSize = pSession->m_RecvDataBuffer.Read(pPacketData->m_pBuffer, nPacketLength);
					pSession->m_RecvDataBuffer.Erase(nPacketLength);
					pPacketData->pSession = pSession;

					RecvPacketQueue.Push(pPacketData);
				}

				SAFE_DELETE(pNetworkBuffer);

				//
				if( 0 < pSession->RecvRequest() )
				{
					iRet = WSARecv(pSession->GetSocket(), pSession->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pSession->GetRecvOverlapped(), NULL);
					if( SOCKET_ERROR == iRet )
					{
						nErrorCode = WSAGetLastError();
						if( WSA_IO_PENDING != nErrorCode )
						{
							WriteMiniNetLog(FormatW(L"### MiniNet::WorkerThread() : <%d> WSARecv() fail. socket:%d. error:%d", pSession->GetIndex(), pSession->GetSocket(), nErrorCode));

							//
							Net.Disconnect(pSession);
						}
					}
				}
				else
				{
					SAFE_DELETE(pSession->m_RecvRequest.pBuffer);
				}

				//
				SetEvent(hRecvQueueEvent);
			}
			break;
		case eNetworkBuffer::OP_INNER:
			{
				WriteMiniNetLog(FormatW(L"debug: MiniNet::WorkerThread() : <%d> InnerResult : socket:%d. RecvDataSize %d", pSession->GetIndex(), pSession->GetSocket(), dwIOSize));
				WritePacketLog(FormatW(L"debug: MiniNet::WorkerThread(): <%d> InnerData: ", pSession->GetIndex()), pNetworkBuffer->m_pBuffer, dwIOSize);

				//
				if( pNetworkBuffer->m_nDataSize == dwIOSize )
				{
					CPacketStruct *pPacketData = new CPacketStruct();

					pPacketData->m_nDataSize = pNetworkBuffer->m_nDataSize;
					memcpy_s((void*)pPacketData->m_pBuffer, pPacketData->GetBufferSize(), pNetworkBuffer->m_pBuffer, pNetworkBuffer->GetDataSize());
					pPacketData->pSession = pSession;

					RecvPacketQueue.Push(pPacketData);
				}

				pSession->InnerComplete(dwIOSize);

				//
				SetEvent(hRecvQueueEvent);
			}
			break;
		default:
			{
			}
			break;
		}
	}

	//
	return 0;
}

unsigned int WINAPI CMiniNet::SendThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CSendPacketQueue &SendPacketQueue = CSendPacketQueue::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;
	HANDLE &SendEvent = Net.m_hSendQueueEvent;

	CConnector *pSession = nullptr;
	CPacketStruct *pPacket = nullptr;

	DWORD dwPacketSize = 0;

	int nRet = 0;

	//
	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		pPacket = SendPacketQueue.Pop();
		if( !pPacket )
			continue;
		pSession = pPacket->pSession;
		if( !pSession )
		{
			SAFE_DELETE(pPacket);
			continue;
		}

		//
		nRet = Net.Write(pSession, pPacket->GetBuffer(), pPacket->GetDataSize());
		if( eNetwork::RESULT_SUCC != nRet )
		{
			if( eNetwork::RESULT_IO_PENDING == nRet )
			{
				SendPacketQueue.Push(pPacket);
				continue;
			}
		}

		//
		SAFE_DELETE(pPacket);
	}

	//
	return 0;
}

unsigned int WINAPI CMiniNet::DoUpdate(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CConnectorMgr &SessionMgr = CConnectorMgr::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;

	CConnector *pSession = nullptr;
	void *pParam = nullptr;

	//
	while( 1 == InterlockedExchange((long*)&dwRunning, dwRunning) )
	{
		SessionMgr.Lock();
		{
			for( auto connector : SessionMgr.m_mapConnector )
			{
				pSession = (CConnector*)connector.second;

				//
				pSession->DoUpdate();

				//
				pParam = pSession->GetParam();
				if( nullptr != pParam )
				{
				}
			}
		}
		SessionMgr.Unlock();

		//
		Sleep(1);
	}

	//
	return 0;
}

bool CMiniNet::Listen(WCHAR * pwcsListenIP, const WORD wListenPort)
{
	wcsncpy_s(m_wcsListenIP, eNetwork::MAX_LEN_IPV4_STRING, pwcsListenIP, wcslen(pwcsListenIP));
	m_wListenPort = wListenPort;

	m_ListenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( INVALID_SOCKET == m_ListenSock )
	{
		WriteMiniNetLog(L"### MiniNet::Listen() : WSASocket fail.");
		return false;
	}

	memset(&m_ListenAddr, 0, sizeof(m_ListenAddr));
	m_ListenAddr.sin_family = AF_INET;
	m_ListenAddr.sin_addr.s_addr = /*inet_addr(pszListenIP);*/ htonl(INADDR_ANY);
	m_ListenAddr.sin_port = htons(m_wListenPort);

	//
	if( SOCKET_ERROR == ::bind(m_ListenSock, (SOCKADDR*)&m_ListenAddr, sizeof(m_ListenAddr)) )
	{
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"### MiniNet::Listen() : bind fail. error %d", nErrorCode));
		return false;
	}
	if( SOCKET_ERROR == listen(m_ListenSock, 10) )
	{
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"### MiniNet::Listen() : listen fail. error %d", nErrorCode));
		return false;
	}

	unsigned int uiThreadID = 0;
	m_hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, NULL, CREATE_SUSPENDED, &uiThreadID);
	if( NULL == m_hAcceptThread )
	{
		WriteMiniNetLog(L"### MiniNet::Listen() : Accept thread create fail");
		return false;
	}

	//
	return true;
}

CConnector* CMiniNet::Connect(WCHAR *pwcsDomain, const WORD wPort)
{
	CConnector *pSession = new CConnector;
	if( !pSession )
		return nullptr;

	//
	SOCKADDR_IN SockAddr;
	memset((void*)&SockAddr, 0, sizeof(SockAddr));

	BOOL bSockOpt = TRUE;

	pSession->SetDomain(pwcsDomain, wPort);

	//
	SockAddr.sin_family = AF_INET;

	if( iswalpha(pwcsDomain[0]) )
	{
		struct hostent *host = gethostbyname(pSession->GetDomainA());
		if( NULL == host )
		{
			goto ProcFail;
		}
		SockAddr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];
	}
	else
	{
		InetPtonW(AF_INET, pwcsDomain, &SockAddr.sin_addr);
	}

	SockAddr.sin_port = htons(wPort);

	// 소켓 생성
	SOCKET connectSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( INVALID_SOCKET == connectSocket )
	{
		WriteMiniNetLog(L"### MiniNet::Connect() : WSASocket() fail");
		goto ProcFail;
	}

	bSockOpt = TRUE;
	setsockopt(connectSocket, SOL_SOCKET, SO_DONTLINGER, (char*)&bSockOpt, sizeof(BOOL));
	bSockOpt = TRUE;
	setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&bSockOpt, sizeof(BOOL));

	// 연결 시도
	INT32 iRet = connect(connectSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr));
	if( SOCKET_ERROR == iRet )
	{
		int nErrorCode = WSAGetLastError();
		WriteMiniNetLog(FormatW(L"### MiniNet::Connect() : connect() fail. error:%d", nErrorCode));
		goto ProcFail;
	}

	WriteMiniNetLog(FormatW(L"MiniNet::Connect() : <%d> Domain: %s, Port: %d, Socket: %d", pSession->GetIndex(), pwcsDomain, wPort, connectSocket));

	//
	pSession->SetSocket(connectSocket);
	memcpy((void*)&pSession->m_SockAddr, (void*)&SockAddr, sizeof(SockAddr));

	//strncpy_s(pSession->m_szDomain, _countof(pSession->m_szDomain), pszIP, strlen(pszIP));
	wcsncpy_s(pSession->m_wcsDomain, eSession::MAX_LEN_DOMAIN_STRING, pwcsDomain, _countof(pSession->m_wcsDomain));
	pSession->m_wPort = wPort;

	//
	HANDLE hRet = CreateIoCompletionPort((HANDLE)connectSocket, m_hNetworkHandle, (ULONG_PTR)pSession, 0);
	if( m_hNetworkHandle != hRet )
	{
		goto ProcFail;
	}

	// 데이터 수신 대기
	DWORD dwRecvDataSize = 0;

	if( 0 >= pSession->RecvRequest() )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Connect() : <%d> RecvRequest() fail", pSession->GetIndex()));
		goto ProcFail;
	}

	//
	DWORD dwFlags = 0;
	iRet = WSARecv(pSession->GetSocket(), pSession->GetRecvWSABuffer(), 1, &dwRecvDataSize, &dwFlags, pSession->GetRecvOverlapped(), NULL);
	if( SOCKET_ERROR == iRet )
	{
		int nErrorCode = WSAGetLastError();
		if( WSA_IO_PENDING != nErrorCode )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::Connect() : WSARecv() fail. %d, Socket %d. error:() %d", pSession->GetIndex(), pSession->GetSocket(), nErrorCode));
			goto ProcFail;
		}
	}

	//
	return pSession;

	//
ProcFail:

	if( INVALID_SOCKET != connectSocket )
		closesocket(connectSocket);
	SAFE_DELETE(pSession);

	return nullptr;
}

BOOL CMiniNet::Disconnect(CConnector *pSession)
{
	if( !pSession )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Disconnect() : Invalid session object %08X", pSession));
		return FALSE;
	}

	//
	WriteMiniNetLog(FormatW(L"### MiniNet::Disconnect() : <%d> Disconnect. socket %d", pSession->GetIndex(), pSession->GetSocket()));

	//
	if( pSession->GetSocket() )
	{
		shutdown(pSession->GetSocket(), SD_BOTH);
		closesocket(pSession->GetSocket());
	}

	PostQueuedCompletionStatus(m_hNetworkHandle, 0, (ULONG_PTR)pSession, pSession->GetRecvOverlapped());

	//
	pSession->SetSocket(INVALID_SOCKET);

	//
	return TRUE;
}

BOOL CMiniNet::Disconnect(SOCKET socket)
{
	if( INVALID_SOCKET != socket )
		return FALSE;

	//
	shutdown(socket, SD_BOTH);
	closesocket(socket);

	//
	return TRUE;
}

int CMiniNet::Write(CConnector *pSession, char *pSendData, int iSendDataSize)
{
	int nResult = eNetwork::RESULT_FAIL;

	//
	if( !pSession )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : Invalid session object %08X %d", pSession, (pSession ? pSession->GetIndex() : -1)));
		goto ProcFail;
	}
	if( INVALID_SOCKET == pSession->GetSocket() )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> Invalid socket %d", pSession->GetIndex(), pSession->GetSocket()));
		nResult = eNetwork::RESULT_SOCKET_DISCONNECTED;
		goto ProcFail;
	}
	if( pSession->m_SendRequest.pBuffer )
		Sleep(1);
	if( pSession->m_SendRequest.pBuffer )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> cannot data send yet. socket %d", pSession->GetIndex(), pSession->GetSocket()));
		nResult = eNetwork::RESULT_IO_PENDING;
		goto ProcFail;
	}

	//
	int iWSARet = 0;
	int nErrorCode = 0;
	DWORD dwSendDataSize = 0;

	DWORD dwFlags = 0;

	//
	if( 0 > pSession->SendRequest(pSendData, iSendDataSize) )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> SendRequest() fail", pSession->GetIndex()));
		goto ProcFail;
	}

	//
	iWSARet = WSASend(pSession->GetSocket(), pSession->GetSendWSABuffer(), 1, &dwSendDataSize, dwFlags, pSession->GetSendOverlapped(), NULL);
	if( SOCKET_ERROR == iWSARet )
	{
		nErrorCode = WSAGetLastError();
		if( WSA_IO_PENDING != nErrorCode )
		{
			WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> WSASend() fail. socket:%d. error:%d", pSession->GetIndex(), pSession->GetSocket(), nErrorCode));
			goto ProcFail;
		}
	}

	//
	return eNetwork::RESULT_SUCC;

	//
ProcFail:

	//if( pSession && INVALID_SOCKET != pSession->m_Socket )
	//	Disconnect(pSession);

	return nResult;
}

int CMiniNet::InnerWrite(CConnector *pSession, char *pSendData, int nSendDataSize)
{
	int nRet = eNetwork::RESULT_FAIL;

	//
	if( !pSession )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : Invalid session object %08X %d", pSession, (pSession ? pSession->GetIndex() : -1)));
		goto ProcFail;
	}
	if( INVALID_SOCKET == pSession->GetSocket() )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> Invalid socket %d", pSession->GetIndex(), pSession->GetSocket()));
		goto ProcFail;
	}
	if( pSession->m_InnerRequest.pBuffer )
		Sleep(1);
	if( pSession->m_InnerRequest.pBuffer )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::Write() : <%d> cannot data send yet. socket %d", pSession->GetIndex(), pSession->GetSocket()));
		nRet = eNetwork::RESULT_IO_PENDING;
		goto ProcFail;
	}

	//
	if( 0 > pSession->InnerRequest(pSendData, nSendDataSize) )
	{
		WriteMiniNetLog(FormatW(L"### MiniNet::InnerWrite() : <%d> InnerRequest() fail", pSession->GetIndex()));
		goto ProcFail;
	}

	BOOL bRet = FALSE;
	int nErrorCode = 0;

	bRet = PostQueuedCompletionStatus(m_hNetworkHandle, nSendDataSize, (ULONG_PTR)pSession, pSession->GetInnerOverlapped());
	if( 0 == bRet )
	{
		nErrorCode = GetLastError();
		WriteMiniNetLog(FormatW(L"### MiniNet::InnerWrite() : <%d> PostQueuedCompletionStatus() fail. socket:%d. error:%d", pSession->GetIndex(), pSession->GetSocket(), nErrorCode));
		goto ProcFail;
	}

	//
	return eNetwork::RESULT_SUCC;

	//
ProcFail:

	return nRet;
}

//
void WritePacketLog(std::wstring str, const char *pPacketData, int nPacketDataSize)
{
	WCHAR wcsLogBuffer[eBuffer::MAX_PACKET_BUFFER_SIZE + 1024 + 1] = {0,};

	int nLen = 0;
	nLen += _snwprintf_s(wcsLogBuffer+nLen, eBuffer::MAX_PACKET_BUFFER_SIZE, eBuffer::MAX_PACKET_BUFFER_SIZE -nLen, L"%s", str.c_str());

	if( 1024 < nPacketDataSize )
	{
		for( int idx = 0; idx < 6; ++idx )
			nLen += _snwprintf_s(wcsLogBuffer + nLen, eBuffer::MAX_PACKET_BUFFER_SIZE, eBuffer::MAX_PACKET_BUFFER_SIZE - nLen, L"%02X", pPacketData[idx]);
		nLen += _snwprintf_s(wcsLogBuffer + nLen, eBuffer::MAX_PACKET_BUFFER_SIZE, eBuffer::MAX_PACKET_BUFFER_SIZE - nLen, L". connot write packetlog. too long.");
	}
	else
	{
		for( int idx = 0; idx < nPacketDataSize; ++idx )
			nLen += _snwprintf_s(wcsLogBuffer + nLen, eBuffer::MAX_PACKET_BUFFER_SIZE, eBuffer::MAX_PACKET_BUFFER_SIZE - nLen, L"%02X", pPacketData[idx]);
		nLen += _snwprintf_s(wcsLogBuffer + nLen, eBuffer::MAX_PACKET_BUFFER_SIZE, eBuffer::MAX_PACKET_BUFFER_SIZE - nLen, L"\0");
	}

	WriteMiniNetLog(wcsLogBuffer);
}