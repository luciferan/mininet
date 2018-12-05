#include "Connector.h"

//
CConnector::CConnector()
{
	//m_dwUniqueIndex = CConnectorMgr::GetInstance().GetUniqueIndex();
	m_dwUniqueIndex = CConnectorMgr::GetInstance().RegistConnector(this);
}

CConnector::~CConnector()
{
	Finalize();
	CConnectorMgr::GetInstance().RemoveConnector(this);
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
}

void CConnector::GetSocket2IP(char *pszIP)
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(m_Socket, (sockaddr*)&SockAddr, &iLen);
	strncpy_s(pszIP, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);
}

void CConnector::ConvertSocket2IP()
{
	SOCKADDR_IN SockAddr;
	int iLen = sizeof(SockAddr);

	getpeername(m_Socket, (sockaddr*)&SockAddr, &iLen);
	strncpy_s(m_szDomain, eSession::MAX_LEN_IP4_STRING, inet_ntoa(SockAddr.sin_addr), iLen);

	size_t convert = 0;
	mbstowcs_s(&convert, m_wcsDomain, eSession::MAX_LEN_IP4_STRING, m_szDomain, eSession::MAX_LEN_IP4_STRING);
}

int CConnector::AddRecvData(CNetworkBuffer *pBuffer)
{
	int nDataSize = pBuffer->GetDataSize();
	char *pData = pBuffer->GetBuffer();

	int nEmptySize = m_RecvDataBuffer.GetEmptySize();
	if( nEmptySize < nDataSize )
		return -1;

	m_RecvDataBuffer.Write(pData, nDataSize);
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

int CConnector::SendRequest(char *pSendData, int nSendDataSize)
{
	if( !pSendData )
		return -1;
	if( eBuffer::MAX_PACKET_BUFFER_SIZE < nSendDataSize )
		return -1;

	CNetworkBuffer *pBuffer = new CNetworkBuffer();
	if( !pBuffer )
		return -1;

	m_SendRequest.ResetOverlapped();
	m_SendRequest.pSession = this;
	m_SendRequest.pBuffer = pBuffer;
	return pBuffer->SetSendData(this, pSendData, nSendDataSize);
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
	if( m_SendRequest.pBuffer )
		return &m_SendRequest.pBuffer->m_WSABuffer;
	return nullptr;
}

int CConnector::RecvRequest()
{
	CNetworkBuffer *pBuffer = new CNetworkBuffer();
	if( !pBuffer )
		return 0;

	m_RecvRequest.ResetOverlapped();
	m_RecvRequest.pSession = this;
	m_RecvRequest.pBuffer = pBuffer;
	return pBuffer->SetRecvData(this);
}

WSABUF* CConnector::GetRecvWSABuffer()
{
	if( m_RecvRequest.pBuffer )
		return &m_RecvRequest.pBuffer->m_WSABuffer;
	return nullptr;
}

int CConnector::InnerRequest(char *pSendData, int nSendDataSize)
{
	if( !pSendData )
		return -1;
	if( eBuffer::MAX_PACKET_BUFFER_SIZE < nSendDataSize )
		return -1;

	CNetworkBuffer *pBuffer = new CNetworkBuffer();
	if( !pBuffer )
		return 0;

	m_InnerRequest.ResetOverlapped();
	m_InnerRequest.pSession = this;
	m_InnerRequest.pBuffer = pBuffer;
	return pBuffer->SetInnerData(this, pSendData, nSendDataSize);
}

int CConnector::InnerComplete(DWORD dwInnerSize)
{
	CNetworkBuffer *pBuffer = m_InnerRequest.pBuffer;
	if( !pBuffer )
		return 0;

	SAFE_DELETE(pBuffer);
	m_InnerRequest.pBuffer = nullptr;
	m_InnerRequest.ResetOverlapped();

	return 0;
}

WSABUF* CConnector::GetInnerWSABuffer()
{
	if( m_InnerRequest.pBuffer )
		return &m_InnerRequest.pBuffer->m_WSABuffer;
	return nullptr;
}

void CConnector::DoUpdate()
{
}
