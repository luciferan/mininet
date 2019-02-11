#include "../_network_2_/Connector.h"
#include "../_network_2_/Packet.h"
#include "../_common/util.h"

#include "UserSession.h"

//
int CUserSession::Clear()
{
	//InterlockedExchange(&m_dwUsed, 0);
	//InterlockedExchange(&m_dwActive, 0);
	m_biHeartBeat = 0;
	m_biUpdateTime = 0;

	if( m_pConnector )
		m_pConnector->SetParam(nullptr);

	return 0;
}

int CUserSession::Release()
{
	// todo: 객체 정리 작업
	Clear();

	//InterlockedExchange(&m_dwActive, 0);

	return 0;
}

int CUserSession::MessageProcess(char *pData, int nLen)
{
	if( sizeof(sPacketHead) > nLen )
		return -1;

	//
	sPacketHead *pHeader = (sPacketHead*)pData;
	DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
	DWORD dwProtocol = pHeader->dwProtocol;

	char *pPacketData = (char*)(pHeader + 1);

	//
	switch( dwProtocol )
	{
	case P_ECHO:
		{
			SendPacketData(dwProtocol, pPacketData, dwPackethLength);
		}
		break;
	default:
		break;
	}

	//
	return 0;
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
	if( m_biHeartBeat < uiCurrTime )
	{
		m_biHeartBeat = uiCurrTime + (MILLISEC_A_SEC * 30);
	}

	//
	return 0;
}
//
//int CUserSession::SendPacket(DWORD dwProtocol, char *pPacket, DWORD dwPacketLength)
//{
//	if( !m_pConnector )
//		return -1;
//
//	struct
//	{
//		DWORD dwLength = 0;
//		DWORD dwProtocol = 0;
//		char Data[MAX_PACKET_BUFFER_SIZE] = {};
//	} NetworkData;
//
//	NetworkData.dwLength = sizeof(DWORD) + sizeof(DWORD) + dwPacketLength;
//	NetworkData.dwProtocol = dwProtocol;
//	memcpy(NetworkData.Data, pPacket, dwPacketLength);
//
//	m_pConnector->AddSendData((char*)&NetworkData, NetworkData.dwLength);
//	return 0;
//}


eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char *pData, DWORD dwSendDataSize)
{
	//CNetworkBuffer SendBuffer;
	char SendBuffer[eBuffer::MAX_PACKET_BUFFER_SIZE] = {};
	DWORD dwSendBufferSize = sizeof(SendBuffer);
	MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

	return m_pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}