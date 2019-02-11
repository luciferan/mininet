#pragma once
#ifndef __PACKET_PROTOCOL_H__
#define __PACKET_PROTOCOL_H__

//
#pragma pack(push, 1)

//
const DWORD P_AUTH = 1;
struct sP_AUTH
{
};

const DWORD P_HEARTBEAT = 0xfffffffe;
struct sP_HEARTBEAT
{
};

const DWORD P_ECHO = 0xffffffff;
struct sP_ECHO
{
	char echoData[32] = {};
};

#pragma pack(pop)

//
#endif //__PACKET_PROTOCOL_H__