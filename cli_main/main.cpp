#include "../_network/MiniNet.h"

//
void WriteMiniNetLog(std::wstring wstr)
{
	wprintf(wstr.c_str());
	wprintf(L"\n");
}

unsigned int WINAPI CMiniNet::PacketProcess(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();

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
		__try
		{
			//Net.Write(pSession, pPacket->m_pBuffer, pPacket->m_nDataSize);
		}
		__except( CExceptionReport::GetInstance().makeDump(GetExceptionInformation(), MiniDumpNormal) )
		{
		}

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
	if( false == Net.Start() )
		return;

	//
	CConnector *pSession = nullptr;

	char cmd[4] = {0,};

	//
	while( true )
	{
		pSession = Net.Connect(L"localhost", 65010);
		gets_s(cmd);

		//
		struct
		{
			int nLen = 0;
			WCHAR wcsStr[14+1] = {0,};
			int temp = 0;
		} SendPacket;

		SendPacket.nLen = sizeof(SendPacket);
		_snwprintf_s(SendPacket.wcsStr, 14, 14, L"%s", L"hello.");
		SendPacket.temp = 542;

		for( int cnt = 0; cnt < 1; ++cnt )
		{
			Net.Write(pSession, (char*)&SendPacket, sizeof(SendPacket));
			//Sleep(3000);
		}

		//
		gets_s(cmd);
		Net.Disconnect(pSession);
		pSession = nullptr;
	}

	//
	DWORD &dwRunning = Net.m_dwRunning;

	while( 1 == InterlockedExchange((LONG*)&dwRunning, dwRunning) )
	{
		Sleep(500);
	}

	//
	return;
}