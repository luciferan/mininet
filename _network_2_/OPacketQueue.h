//#ifndef __OPACKETQUEUE_H__
//#define __OPACKETQUEUE_H__
//
//#pragma once
//
///////////////////////////////////////////////////////////////////////////////////
//#include <winsock2.h>
//
//#include "./OLink.h"
//#include "./OObjMgrBase.h"
//
//#include "Buffer.h"
//
//#include <list>
//
////
//enum ePacket
//{
//	MNNET_MAX_PACKET_SIZE = 1024 * 10,
//};
////#ifndef MNNET_MAX_PACKET_SIZE
////#define MNNET_MAX_PACKET_SIZE (1024 * 10)
////#endif
////const INT32 MNNET_MAX_PACKET_SIZE = 1024*10;
//
////
//struct OVERLAPPED_EX;
//class CConnector;
//
///////////////////////////////////////////////////////////////////////////////////
//class OPacket
//	: public OLink<OPacket>
//{
//public:
//	CConnector *m_pSession = nullptr;
//	void *m_pParam = nullptr;
//
//	WSABUF m_WSABuffer = {0,};
//
//	DWORD m_dwSendDataSize = 0;
//	DWORD m_dwRecvDataSize = 0;
//
//	BYTE m_PacketData[ePacket::MNNET_MAX_PACKET_SIZE + 1] = {0,};
//
//	//
//public:
//	OPacket(void);
//	~OPacket(void);
//
//	bool	Initialize();
//	bool	Finalize();
//};
//
///////////////////////////////////////////////////////////////////////////////////
//class OPacketQueue :
//	public OObjMgrBase<OPacket>
//{
//public:
//	std::list<OPacket*> m_RecvPacketList;
//	std::list<OPacket*> m_SendPacketList;
//
//public:
//	OPacketQueue(void);
//	~OPacketQueue(void);
//
//	bool Initialize(const INT32 iQueueSize = 0);
//	bool Finalize();
//
//	OPacket*	GetFree();
//	bool		SetFree(OPacket *pPacket);
//
//	//
//	void		Push(OPacket *pPacket);
//	OPacket*	Pop();
//};
//
//class CPacket
//
///////////////////////////////////////////////////////////////////////////////////
//#endif //__OPACKETQUEUE_H__