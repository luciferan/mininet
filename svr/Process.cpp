#include "process.h"

#include "../_network_2_/Connector.h"
#include "../_network_2_/PacketDataQueue.h"
#include "../_network_2_/Packet.h"
#include "../_common/log.h"

#include "UserSession.h"
#include "Config.h"

//
void WriteMiniNetLog(std::wstring wstr)
{
	//return;

	g_Log.Write(wstr);
}

bool LoadConfig()
{
	g_Config.LoadConfig();
	return true;
}

unsigned int WINAPI ProcessThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CRecvPacketQueue &RecvPacketQueue = CRecvPacketQueue::GetInstance();
	CSendPacketQueue &SendPacketQueue = CSendPacketQueue::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;

	CConnector *pConnector = nullptr;
	CUserSession *pUserSession = nullptr;
	CPacketStruct *pPacket = nullptr;

	DWORD dwPacketSize = 0;

	//
	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		pPacket = RecvPacketQueue.Pop();
		if( !pPacket )
			continue;

		sPacketHead *pHead = (sPacketHead*)pPacket->m_pBuffer;
		sPacketTail *pTail = (sPacketTail*)(pPacket->m_pBuffer + pHead->dwLength - sizeof(sPacketTail));
		if( pTail->dwCheckTail != PACKET_CHECK_TAIL_KEY )
		{
			g_Log.Write(L"Invliad packet");
			Net.Disconnect(pConnector);
		}

		pConnector = pPacket->pSession;
		if( !pConnector )
		{
			RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
			continue;
		}

		//
		pUserSession = (CUserSession*)pConnector->GetParam();
		if( nullptr == pUserSession )
		{
			sPacketHead *pHeader = (sPacketHead*)pPacket->m_pBuffer;
			if( P_AUTH == pHeader->dwProtocol )
			{
				CScopeLock lock(UserSessionMgr.m_Lock);
				pUserSession = UserSessionMgr.GetFreeUserSession();
				if( !pUserSession )
				{
					g_Log.Write(L"CUserSessionMgr::GetFreeUserSession() fail");
				}
				else
				{
					pConnector->SetParam((void*)pUserSession);
					pUserSession->SetConnector(pConnector);
				}
			}
			else
			{
				g_Log.Write(L"Invliad packet");
				Net.Disconnect(pConnector);
			}
		}
		else
		{
			//CPerformanceCheck(L"ProcessThread(): UserSession::MessageProcess()");
			pUserSession->MessageProcess(pPacket->m_pBuffer, pPacket->m_nDataSize);
		}

		//
		RecvPacketQueue.ReleasePacketStruct(pPacket); //SAFE_DELETE(pPacket);
	}

	//
	return 0;
};

//
unsigned int WINAPI UpdateThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;
	INT64 biCurrTime = 0;

	CUserSession *pUserSession = nullptr;
	std::list<CUserSession*> ReleaseSessionList = {};

	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		biCurrTime = GetTimeMilliSec();
		
		//
		{
			CScopeLock lock(UserSessionMgr.m_Lock);
			for( CUserSession *pSession : UserSessionMgr.m_UsedUserSessionList )
			{
				if( pSession->CheckUpdateTime(biCurrTime) )
					continue;

				pSession->SetUpdateTime(biCurrTime + MILLISEC_A_SEC);

				//
				CConnector *pConnector = pSession->GetConnector();
				if( !pConnector || !pConnector->GetActive() )
				{
					ReleaseSessionList.push_back(pSession);
					continue;
				}

				pSession->DoUpdate(biCurrTime);
			}
		}

		{
			for( CUserSession *pSession : ReleaseSessionList )
			{
				pSession->Release();
				UserSessionMgr.ReleaseUserSesssion(pSession);
			}

			ReleaseSessionList.clear();
		}

		//
		Sleep(1);
	}

	//
	return 0;
}

unsigned int WINAPI MonitorThread(void *p)
{
	CMiniNet &Net = CMiniNet::GetInstance();
	CUserSessionMgr &UserSessionMgr = CUserSessionMgr::GetInstance();

	//
	DWORD &dwRunning = Net.m_dwRunning;
	INT64 biCurrTime = 0;
	INT64 biCheckTimer = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);

	//
	while( 1 == InterlockedExchange(&dwRunning, dwRunning) )
	{
		biCurrTime = GetTimeMilliSec();

		//
		if( biCheckTimer < biCurrTime )
		{
			CConnector *pConnector = Net.Connect(g_Config.wcsHost, g_Config.wPort);
			if( !pConnector )
			{
				g_Log.Write(L"error: MonitorThread: Connect fail");
			}
			else
			{
				g_Log.Write(L"log: MonitorThread: <%d> Connected. Socket %d", pConnector->GetIndex(), pConnector->GetSocket());
				Net.Disconnect(pConnector);
			}

			biCheckTimer = GetTimeMilliSec() + (MILLISEC_A_SEC * 15);
		}

		//
		Sleep(1);
	}

	//
	return 0;
}