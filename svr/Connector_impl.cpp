#include "../_network_2_/Connector.h"
#include "../_network_2_/Packet.h"
#include "../_common/util.h"

//
int CConnector::DataParsing()
{
	int nPacketLength = 0;

	DWORD dwRet = ParseNetworkData(m_RecvDataBuffer, (DWORD&)nPacketLength);
	switch( dwRet )
	{
	case eResultCode::RESULT_INVALID_PACKET:
		return -1;
		break;
	}

	return nPacketLength;
}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//
//	DWORD dwRet = ParseNetworkData(m_RecvDataBuffer.GetBuffer(), m_RecvDataBuffer.GetDataSize(), (DWORD&)nPacketLength);
//	switch( dwRet )
//	{
//	case eResultCode::RESULT_INVALID_PACKET:
//		return -1;
//		break;
//	}
//
//	return nPacketLength;
//}

//int CConnector::DataParsing()
//{
//	int nPacketLength = 0;
//	m_RecvDataBuffer.Read((char*)&nPacketLength, sizeof(int));
//
//	if( m_RecvDataBuffer.GetDataSize() < nPacketLength )
//		return 0;
//
//	return nPacketLength;
//}

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

bool CConnector::DoUpdate(INT64 biCurrTime)
{
	if( 0 == m_biUpdateTimer )
		m_biUpdateTimer = biCurrTime + MILLISEC_A_MIN;

	//
	if( m_biUpdateTimer < biCurrTime )
	{
		m_biUpdateTimer = biCurrTime + MILLISEC_A_MIN;

		g_PerformanceLog.Write(GetStateReport());

		//
		return true;
	}
	else
	{
		return false;
	}
}

bool CConnector::CheckHeartbeat(INT64 biCurrTime)
{
	if( 0 == m_biHeartbeatTimer )
		m_biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

	//
	if( GetActive() && m_biHeartbeatTimer < biCurrTime )
	{
		m_biHeartbeatTimer = biCurrTime + (MILLISEC_A_SEC * 30);

		//struct 
		//{
		//	DWORD dwLength = 0;
		//	DWORD dwProtocol = 0;
		//	sP_HEARTBEAT Data;
		//} SendData;

		//SendData.dwLength = sizeof(SendData);
		//SendData.dwProtocol = P_HEARTBEAT;

		//AddSendData((char*)&SendData, sizeof(SendData));

		//
		sP_HEARTBEAT SendPacket;
		
		char SendBuffer[eBuffer::MAX_PACKET_BUFFER_SIZE] = {};
		DWORD dwSendBufferSize = sizeof(SendBuffer);
		MakeNetworkPacket(P_HEARTBEAT, (char*)&SendPacket, sizeof(SendPacket), (char*)&SendBuffer, dwSendBufferSize);

		//AddSendData((char*)&SendBuffer, dwSendBufferSize);

		//
		return true;
	}
	else
	{
		return false;
	}
}

wstring CConnector::GetStateReport()
{
	wstring wstrReport = {};

	if( GetActive() )
		wstrReport.append(FormatW(L"[%d] connected: %d,", GetUniqueIndex(), GetSocket()));
	else
		wstrReport.append(FormatW(L"[%d] disconnected: ", GetUniqueIndex()));

	wstrReport.append(FormatW(L"sq:%d,", m_SendQueue.size()));
	wstrReport.append(FormatW(L"rb:(%d/%d),", m_RecvDataBuffer.GetDataSize(), m_RecvDataBuffer.GetBufferSize()));
	wstrReport.append(FormatW(L"iq:%d", m_InnerQueue.size()));

	return wstrReport;
}