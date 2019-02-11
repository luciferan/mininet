#include "MiniNet_Header.h"
#include "Buffer.h"

#include "Packet.h"

//
DWORD MakeNetworkPacket(DWORD dwProtocol, char *pSendData, DWORD dwSendDataSize, char *pSendBuffer, DWORD &dwSendBufferSize)
{
	sPacketHead head;
	sPacketBody	body;
	sPacketTail tail;

	head.dwCheckHead = PACKET_CHECK_HEAD_KEY;
	head.dwLength = sizeof(sPacketHead) + dwSendDataSize + sizeof(sPacketTail);
	head.dwProtocol = dwProtocol;

	body.pData = pSendData;

	tail.dwCheckTail = PACKET_CHECK_TAIL_KEY;

	if( head.dwLength > dwSendBufferSize )
		return eResultCode::RESULT_FAIL;
	
	int nPos = 0;
	memcpy(pSendBuffer+nPos, (char*)&head, sizeof(head)); nPos += sizeof(head);
	memcpy(pSendBuffer+nPos, (char*)&body, dwSendDataSize); nPos += dwSendDataSize;
	memcpy(pSendBuffer+nPos, (char*)&tail, sizeof(tail)); nPos += sizeof(tail);

	dwSendBufferSize = head.dwLength;

	//
	return eResultCode::RESULT_SUCC;
};

DWORD ParseNetworkData(CCircleBuffer PARAM_IN &Buffer, DWORD PARAM_OUT &dwPacketLength)
{
	DWORD dwDataSize = Buffer.GetDataSize();

	if( sizeof(sPacketHead) > dwDataSize )
		return eResultCode::RESULT_PENDING;

	sPacketHead Head;
	Buffer.Read((char*)&Head, sizeof(sPacketHead));

	if( PACKET_CHECK_HEAD_KEY != Head.dwCheckHead )
		return eResultCode::RESULT_INVALID_PACKET;

	if( Head.dwLength > dwDataSize )
		return eResultCode::RESULT_PENDING;

	dwPacketLength = Head.dwLength;
	return eResultCode::RESULT_SUCC;
}