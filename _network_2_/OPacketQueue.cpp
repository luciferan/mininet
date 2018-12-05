//#include "./OPacketQueue.h"
//
///////////////////////////////////////////////////////////////////////////////////
//OPacket::OPacket(void)
//{
//	memset(&m_WSABuffer, 0, sizeof(WSABUF));
//}
//
//OPacket::~OPacket(void)
//{
//	Finalize();
//}
//
//bool OPacket::Initialize()
//{
//	m_pSession = nullptr;
//	m_pParam = nullptr;
//	memset((void*)&m_WSABuffer, 0, sizeof(m_WSABuffer));
//
//	m_dwSendDataSize = 0;
//	m_dwRecvDataSize = 0;
//	memset((void*)m_PacketData, 0, sizeof(m_PacketData));
//
//	//
//	return true;
//}
//
//bool OPacket::Finalize()
//{
//	m_pSession = nullptr;
//	m_pParam = nullptr;
//
//	return true;
//}
//
///////////////////////////////////////////////////////////////////////////////////
//OPacketQueue::OPacketQueue(void)
//{
//}
//
//OPacketQueue::~OPacketQueue(void)
//{
//	Finalize();
//}
//
//bool OPacketQueue::Initialize(const INT32 iQueueSize)
//{
//	ObjMgrInitialize(iQueueSize);
//
//	m_RecvPacketList.clear();
//	m_SendPacketList.clear();
//
//	//
//	return true;
//}
//
//bool OPacketQueue::Finalize()
//{
//	m_RecvPacketList.clear();
//	m_SendPacketList.clear();
//
//	ObjMgrFinalize();
//
//	//
//	return true;
//}
//
//OPacket* OPacketQueue::GetFree()
//{
//	return GetFreeObj();
//}
//
//bool OPacketQueue::SetFree(OPacket *pPacket)
//{
//	pPacket->Initialize();
//
//	SetFreeObj(pPacket);
//
//	//
//	return true;
//}
//
//void OPacketQueue::Push(OPacket *pPacket)
//{
//	m_RecvPacketList.push_back(pPacket);
//}
//
//OPacket* OPacketQueue::Pop()
//{
//	OPacket *pRet = NULL;
//
//	for( std::list<OPacket*>::iterator iter = m_RecvPacketList.begin(); iter != m_RecvPacketList.end(); ++iter )
//	{
//		pRet = *iter;
//		m_RecvPacketList.remove(*iter);
//
//		break;
//	}
//
//	return pRet;
//}
//
///////////////////////////////////////////////////////////////////////////////////