#include "../_network_2_/MiniNet.h"

//
CLog g_Log;

void WriteMiniNetLog(std::wstring wstr)
{
	g_Log.write(wstr);
}

unsigned int WINAPI CMiniNet::PacketProcess(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();
	CSendPacketQueue &SendPacketQueue = CSendPacketQueue::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;
	HANDLE &RecvEvent = Net.m_hRecvQueueEvent;

	CConnector *pSession = nullptr;
	CPacketStruct *pPacket = nullptr;

	DWORD dwPacketSize = 0;

	//
	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		pPacket = RecvPacketQueue.Pop();
		if( !pPacket )
			continue;
		pSession = pPacket->pSession;
		if( !pSession )
		{
			SAFE_DELETE(pPacket);
			continue;
		}

		//
		//Net.Write(pSession, pPacket->m_pBuffer, pPacket->m_nDataSize);
		//Net.InnerWrite(pSession, pPacket->m_pBuffer, pPacket->m_nDataSize);

		CPacketStruct *pSendPacket = new CPacketStruct;

		pSendPacket->pSession = pSession;
		memcpy(pSendPacket->m_pBuffer, pPacket->m_pBuffer, pPacket->m_nDataSize);
		pSendPacket->m_nDataSize = pPacket->m_nDataSize;

		SendPacketQueue.Push(pSendPacket);

		//
		SAFE_DELETE(pPacket);
	}

	//
	return 0;
};

void main()
{
	CExceptionReport::GetInstance().ExceptionHandlerBegin();

	//
	CMiniNet &Net = CMiniNet::GetInstance();

	if( false == Net.Initialize() )
		return;
	if( false == Net.Listen(L"127.0.0.1", 65010) )
		return;
	if( false == Net.Start() )
		return;

	//
	DWORD &dwRunning = Net.m_dwRunning;

	while( 1 == InterlockedExchange((LONG*)&dwRunning, dwRunning) )
	{
		Sleep(500);
	}

	//
	return;
}