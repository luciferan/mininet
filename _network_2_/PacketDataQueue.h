#pragma once
#ifndef __PACKETDATAQUEUE_H__
#define __PACKETDATAQUEUE_H__

//
#include <unordered_map>
#include <list>
#include <queue>

#include "Buffer.h"
#include "../_common/SafeLock.h"
#include "../_common/Protocol.h"

//
enum ePacketDataQueue
{
	MAX_PACKET_QUEUE_PHASE = 4,
};

//
class CPacketStruct
	: public CBuffer
{
public:
	CConnector *pSession = nullptr;

public:
	CPacketStruct() {};
	virtual ~CPacketStruct() {};
};

//
class CPacketDataQueue
{
protected:
	CRITICAL_SECTION m_cs[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1];
	HANDLE m_hQueueEvent[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1];

	std::list<CPacketStruct*> lstPacketQueue[ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1];

	//
public:
	CPacketDataQueue()
	{
		for( auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE+1; ++idx )
		{
			//InitializeCriticalSection(&m_cs[idx]);
			InitializeCriticalSectionAndSpinCount(&m_cs[idx], 2000);
			m_hQueueEvent[idx] = CreateEvent(0, 0, 0, 0);
			lstPacketQueue[idx].clear();
		}
	}
	virtual ~CPacketDataQueue() 
	{
		for( auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx )
		{
			DeleteCriticalSection(&m_cs[idx]);
			CloseHandle(m_hQueueEvent[idx]);
			lstPacketQueue[idx].clear();
		}
	};

	void ForceActivateQueueEvent()
	{
		for( auto idx = 0; idx < ePacketDataQueue::MAX_PACKET_QUEUE_PHASE + 1; ++idx )
		{
			SetEvent(m_hQueueEvent[idx]);
		}
	}

	size_t Push(CPacketStruct *pPacketData, int phase)
	{
		if( 0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase )
			return -1;
		
		CSafeLock lock(m_cs[phase]);
		lstPacketQueue[phase].push_back(pPacketData);
		return lstPacketQueue[phase].size();
	}

	CPacketStruct* Pop(int phase)
	{
		if( 0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE < phase )
			return nullptr;
		if( lstPacketQueue[phase].empty() )
			return nullptr;

		CPacketStruct *pData = nullptr;

		CSafeLock lock(m_cs[phase]);
		auto iter = lstPacketQueue[phase].begin();
		pData = *iter;
		lstPacketQueue[phase].pop_front();

		return pData;
	}
};

//
class CRecvPacketQueue
	: public CPacketDataQueue
{
private:
	CRecvPacketQueue() {};
	~CRecvPacketQueue() {};

public:
	static CRecvPacketQueue& GetInstance()
	{
		static CRecvPacketQueue *pInstance = new CRecvPacketQueue();
		return *pInstance;
	}

	size_t Push(CPacketStruct *pPacketData, int phase = 0)
	{
		if( !pPacketData )
			return -1;
		if( 0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase )
			return -1;

		auto nDataSize = CPacketDataQueue::Push(pPacketData, phase);
		SetEvent(m_hQueueEvent[phase]);
		return nDataSize;
	}

	CPacketStruct* Pop(int phase = 0)
	{
		if( 0 > phase || ePacketDataQueue::MAX_PACKET_QUEUE_PHASE <= phase )
			return nullptr;

		CPacketStruct *pPacket = CPacketDataQueue::Pop(phase);
		if( !pPacket )
		{
			WaitForSingleObject(m_hQueueEvent[phase], INFINITE);
			pPacket = CPacketDataQueue::Pop(phase);
		}
		return pPacket;
	}

	CPacketStruct* GetFreePacketStruct()
	{
		CPacketStruct *pPacket = new CPacketStruct;
		if( !pPacket )
			return nullptr;

		return pPacket;
	}

	void ReleasePacketStruct(CPacketStruct *pPacket)
	{
		if( !pPacket )
			return;

		SAFE_DELETE(pPacket);
	}
};

//
class CSendPacketQueue
	: public CPacketDataQueue
{
private:
	CSendPacketQueue() {}
	~CSendPacketQueue() {}

public:
	static CSendPacketQueue& GetInstance()
	{
		static CSendPacketQueue *pInstance = new CSendPacketQueue();
		return *pInstance;
	}

	size_t Push(CPacketStruct *pPacketData)
	{
		if( !pPacketData )
			return -1;
		auto nDataSize = CPacketDataQueue::Push(pPacketData, 0);
		SetEvent(m_hQueueEvent[0]);
		return nDataSize;
	}

	CPacketStruct* Pop()
	{
		CPacketStruct *pPacket = CPacketDataQueue::Pop(0);
		if( !pPacket )
		{
			WaitForSingleObject(m_hQueueEvent[0], INFINITE);
			pPacket = CPacketDataQueue::Pop(0);
		}
		return pPacket;
	}
};

//
#endif //__PACKETDATAQUEUE_H__