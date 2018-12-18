#include "Connector.h"
#include "PacketDataQueue.h"

//
CConnector::CConnector(DWORD dwUniqueIndex)
{
	m_dwUniqueIndex = dwUniqueIndex;
}

CConnector::~CConnector()
{
	Finalize();
}

bool CConnector::Initialize()
{
	return true;
}

bool CConnector::Finalize()
{
	if( INVALID_SOCKET != m_Socket )
		closesocket(m_Socket);

	SAFE_DELETE(m_RecvRequest.pBuffer);
	SAFE_DELETE(m_SendRequest.pBuffer);
	SAFE_DELETE(m_InnerRequest.pBuffer);

	return true;
}

void CConnector::SetDomain(WCHAR *pwcsDomain, WORD wPort)
{
	wcsncpy_s(m_wcsDomain, eSession::MAX_LEN_DOMAIN_STRING, pwcsDomain, wcslen(pwcsDomain));
	m_wPort = wPort;

	size_t converted = 0;
	wcstombs_s(&converted, m_szDomain, eSession::MAX_LEN_DOMAIN_STRING, m_wcsDomain, eSession::MAX_LEN_DOMAIN_STRING);

	return;
}

void CConnector::GetSocket2IP(char *pszIP)
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(m_Socket, (sockaddr*)&SockAddr, &iLen);
	//strncpy_s(pszIP, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
	inet_ntop(AF_INET, &SockAddr.sin_addr, pszIP, eSession::MAX_LEN_IP4_STRING);
}

void CConnector::ConvertSocket2IP()
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(m_Socket, (sockaddr*)&SockAddr, &iLen);
	//strncpy_s(m_szDomain, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
	inet_ntop(AF_INET, &SockAddr.sin_addr, m_szDomain, _countof(m_szDomain));

	size_t convert = 0;
	mbstowcs_s(&convert, m_wcsDomain, eSession::MAX_LEN_IP4_STRING, m_szDomain, eSession::MAX_LEN_IP4_STRING);
}

//
int CConnector::AddSendQueue(char *pSendData, DWORD dwSendDataSize)
{
	CNetworkBuffer *pSendBuffer = new CNetworkBuffer;
	if( !pSendBuffer )
		return -1;

	int nRet = pSendBuffer->SetSendData(this, pSendData, dwSendDataSize);
	if( 0 > nRet )
	{
		SAFE_DELETE(pSendBuffer);
		return -1;
	}

	//
	CScopeLock lock(m_SendQueueLock);
	m_SendQueue.push(pSendBuffer);

	return m_SendQueue.size();
}

int CConnector::SendPrepare()
{
	if( m_SendQueue.empty() )
		return 0;
	if( 0 < m_dwSendRef ) 
		return 0;

	CScopeLock lock(m_SendQueueLock);

	CNetworkBuffer *pSendData = m_SendQueue.front();
	m_SendQueue.pop();

	m_SendRequest.ResetOverlapped();
	m_SendRequest.pBuffer = pSendData;
	
	return pSendData->GetDataSize();
}

int CConnector::SendComplete(DWORD dwSendSize)
{
	CNetworkBuffer *pBuffer = m_SendRequest.pBuffer;
	if( !pBuffer )
		return 0;

	if( dwSendSize < m_SendRequest.pBuffer->GetDataSize() )
	{
		pBuffer->m_nDataSize -= dwSendSize;
		pBuffer->m_WSABuffer.buf = (char*)(pBuffer->m_pBuffer + dwSendSize);
		pBuffer->m_WSABuffer.len = pBuffer->m_nDataSize -= dwSendSize;

		return pBuffer->m_nDataSize -= dwSendSize;
	}
	else
	{
		SAFE_DELETE(pBuffer);
		m_SendRequest.pBuffer = nullptr;
		m_SendRequest.ResetOverlapped();

		return 0;
	}
}

WSABUF* CConnector::GetSendWSABuffer()
{
	//if( m_SendRequest.pBuffer )
	return &m_SendRequest.pBuffer->m_WSABuffer;
	//return nullptr;
}

int CConnector::RecvPrepare()
{
	if( !m_RecvRequest.pBuffer )
	{
		CNetworkBuffer *pBuffer = new CNetworkBuffer;
		if( !pBuffer )
			return 0;

		m_RecvRequest.ResetOverlapped();
		m_RecvRequest.pSession = this;
		m_RecvRequest.pBuffer = pBuffer;
	}
	else
	{
		m_RecvRequest.ResetOverlapped();
		m_RecvRequest.pBuffer->Reset();
	}

	int nRet = m_RecvRequest.pBuffer->SetRecvData(this);
	if( nRet )
		InterlockedIncrement(&m_dwRecvRef);

	return nRet;
}

int CConnector::RecvComplete(DWORD dwRecvSize)
{
	char *pRecvData = m_RecvRequest.pBuffer->m_pBuffer;
	m_RecvRequest.pBuffer->m_nDataSize = dwRecvSize;

	InterlockedDecrement(&m_dwRecvRef);

	//
	int nEmptySize = m_RecvDataBuffer.GetEmptySize();
	if( nEmptySize < dwRecvSize )
		return -1;

	m_RecvDataBuffer.Write(pRecvData, dwRecvSize);

	//
	int nPacketLength = DataParsing();
	if( 0 < nPacketLength )
	{
		CPacketStruct *pPacket = CRecvPacketQueue::GetInstance().GetFreePacketStruct();
		if( !pPacket )
			return -1;

		pPacket->pSession = this;
		pPacket->m_nDataSize = m_RecvDataBuffer.Read(pPacket->m_pBuffer, nPacketLength);
		m_RecvDataBuffer.Erase(nPacketLength);

		//
		CRecvPacketQueue::GetInstance().Push(pPacket);
	}

	return m_RecvDataBuffer.GetDataSize();
}

int CConnector::DataParsing()
{
	int nPacketLength = 0;
	m_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));

	if( m_RecvDataBuffer.GetDataSize() < nPacketLength )
		return 0;

	return nPacketLength;
}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//	m_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//	nPacketLength = htonl(nPacketLength);
//
//	if( m_RecvDataBuffer.GetDataSize() < nPacketLength )
//		return 0;
//
//	return nPacketLength;
//}

WSABUF* CConnector::GetRecvWSABuffer()
{
	//if( m_RecvRequest.pBuffer )
	return &m_RecvRequest.pBuffer->m_WSABuffer;
	//return nullptr;
}

int CConnector::AddInnerQueue(char *pSendData, DWORD dwSendDataSize)
{
	if( !pSendData )
		return -1;
	if( eBuffer::MAX_PACKET_BUFFER_SIZE < dwSendDataSize )
		return -1;

	CNetworkBuffer *pBuffer = new CNetworkBuffer;
	if( !pBuffer )
		return -1;
	int nRet = pBuffer->SetInnerData(this, pSendData, dwSendDataSize);
	if( 0 > nRet )
	{
		SAFE_DELETE(pBuffer);
		return -1;
	}

	//
	CScopeLock lock(m_InnerQueueLock);
	m_InnerQueue.push(pBuffer);

	return m_InnerQueue.size();
}

int CConnector::InnerPrepare()
{
	if( m_InnerQueue.empty() )
		return 0;
	if( 0 < m_dwInnerRef )
		return 0;

	CScopeLock lock(m_InnerQueueLock);

	CNetworkBuffer *pPacket = m_InnerQueue.front();
	m_InnerQueue.pop();

	m_InnerRequest.ResetOverlapped();
	m_InnerRequest.pBuffer = pPacket;

	return pPacket->GetDataSize();
}

int CConnector::InnerComplete(DWORD dwInnerSize)
{
	CNetworkBuffer *pBuffer = m_InnerRequest.pBuffer;
	if( !pBuffer )
		return -1;
	CPacketStruct *pPacket = (CPacketStruct*)pBuffer;

	//
	CRecvPacketQueue::GetInstance().Push(pPacket);

	return 0;
}

WSABUF* CConnector::GetInnerWSABuffer()
{
	//if( m_InnerRequest.pBuffer )
	return &m_InnerRequest.pBuffer->m_WSABuffer;
	//return nullptr;
}

void CConnector::DoUpdate()
{
}
