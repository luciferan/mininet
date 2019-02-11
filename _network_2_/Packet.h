#pragma once
#ifndef __PACKET_H__
#define __PACKET_H__

//
#include "Packet_Protocol.h"
#include "Buffer.h"

//
const DWORD PACKET_CHECK_HEAD_KEY = 0x00000000;
const DWORD PACKET_CHECK_TAIL_KEY = 0x10000000;

//
struct sPacketHead
{
	DWORD dwCheckHead = 0;
	DWORD dwLength = 0; // = sizeof(Head) + sizeof(Body) + sizeof(Tail)
	DWORD dwProtocol = 0;
};

struct sPacketBody
{
	char *pData = nullptr;
};

struct sPacketTail
{
	DWORD dwCheckTail = 0;
};

//
DWORD MakeNetworkPacket(DWORD PARAM_IN dwProtocol, char PARAM_IN *pSendData, DWORD PARAM_IN dwSendDataSize, char PARAM_OUT *pSendBuffer, DWORD PARAM_INOUT &dwSendBufferSize);
DWORD ParseNetworkData(CCircleBuffer PARAM_IN &Buffer, DWORD PARAM_OUT &dwPacketLength);

//
#endif //__PACKET_H__