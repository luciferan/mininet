#include "../_network_2_/Connector.h"
#include "../_common/util.h"
#include "../_network_2_/Packet.h"

#include "UserSession.h"

//
int CUserSession::clear()
{
	InterlockedExchange(&m_dwUsed, 0);
	InterlockedExchange(&m_dwActive, 0);
	m_uiHeartBeat = 0;

	m_pConnector->SetParam(nullptr);

	return 0;
}

int CUserSession::MessageProcess(char *pData, int nLen)
{
	if( sizeof(sPacketHead) > nLen )
		return -1;

	sPacketHead *pHeader = (sPacketHead*)pData;
	DWORD dwPackethLength = pHeader->dwLength - sizeof(sPacketHead) - sizeof(sPacketTail);
	DWORD dwProtocol = pHeader->dwProtocol;

	char *pPacketData = (char*)(pHeader + 1);

	//
	switch( dwProtocol )
	{
	default:
		break;
	}

	//
	return 0;
}

int CUserSession::DoUpdate(INT64 uiCurrTime)
{
	if( m_uiHeartBeat < uiCurrTime )
	{
		m_uiHeartBeat = uiCurrTime;
	}

	//
	return 0;
}

eResultCode CUserSession::SendPacketData(DWORD dwProtocol, char *pData, DWORD dwSendDataSize)
{
	//CNetworkBuffer SendBuffer;
	char SendBuffer[eBuffer::MAX_PACKET_BUFFER_SIZE] = {};
	DWORD dwSendBufferSize = sizeof(SendBuffer);
	MakeNetworkPacket(dwProtocol, pData, dwSendDataSize, (char*)&SendBuffer, dwSendBufferSize);

	return m_pConnector->AddSendData((char*)&SendBuffer, dwSendBufferSize);
}

//
eResultCode CUserSession::ReqAuth()
{
	sP_AUTH SendPacket;

	//
	return SendPacketData(P_AUTH, (char*)&SendPacket, sizeof(SendPacket));
}

eResultCode CUserSession::ReqEcho()
{
	sP_ECHO SendPacket;

	strncpy_s(SendPacket.echoData, "hello.", _countof(SendPacket.echoData));

	//
	return SendPacketData(P_ECHO, (char*)&SendPacket, sizeof(SendPacket));
}

eResultCode CUserSession::RepHeartBeat()
{
	sP_HEARTBEAT SendPacket;

	//
	return SendPacketData(P_HEARTBEAT, (char*)&SendPacket, sizeof(SendPacket));
}