#pragma once

#pragma pack(push, 1)
//
struct sPacketHeader
{
	DWORD dwLength;
	DWORD dwProtocol;
};

//
const DWORD P_AUTH = 1;
struct S_AUTH
{
};

const DWORD P_HEAERBEAT = 0xfffffffe;
struct S_HEARTBEAT
{
};

const DWORD P_ECHO = 0xffffffff;
struct S_ECHO
{
	char echoData[32] = {};
};

#pragma pack(pop)