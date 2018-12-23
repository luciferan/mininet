#pragma once
#ifndef __BUFFER_H__
#define __BUFFER_H__

//
#include <winsock2.h>
#include <WS2tcpip.h>

#include <string>
#include "../_common/MemoryPool.h"
#include "../_common/SafeLock.h"

//
#ifndef IN
#define IN
#endif //
#ifndef OUT
#define OUT
#endif //

//
class CConnector;

//
enum eBuffer
{
	MAX_PACKET_BUFFER_SIZE = 1024*10,
};

//
class CBuffer
{
public:
	char *m_pBuffer = nullptr;
	size_t m_nDataSize = 0;

protected:
	size_t m_nBufferSize = 0;

	char *m_pHead = nullptr;
	char *m_pTail = nullptr;
	char *m_pRead = nullptr;
	char *m_pWrite = nullptr;

	//
public:
	CBuffer(size_t size = eBuffer::MAX_PACKET_BUFFER_SIZE)
	{
		m_pHead = m_pBuffer = new char[size + 1];
		m_pTail = &m_pBuffer[size];
		m_pRead = m_pWrite = m_pHead;

		m_nBufferSize = size;
		m_nDataSize = 0;
	}

	virtual ~CBuffer()
	{
		m_nBufferSize = m_nDataSize = 0;
		m_pHead = m_pTail = m_pRead = m_pWrite = nullptr;

		if( m_pBuffer )
			delete[] m_pBuffer;
		m_pBuffer = nullptr;
	}

	void Reset() 
	{ 
		m_nDataSize = 0;
		m_pRead = m_pWrite = m_pHead;
		memset(m_pBuffer, 0, m_nBufferSize);
	}

	char* GetBuffer() { return m_pBuffer; }
	size_t GetBufferSize() { return m_nBufferSize; }
	size_t GetDataSize() { return m_nDataSize; }
	size_t GetEmptySize() { return m_nBufferSize - m_nDataSize; }
};

//
class CCircleBuffer
	: public CBuffer
{
private:
	CRITICAL_SECTION m_cs;

public:
	CCircleBuffer() { InitializeCriticalSectionAndSpinCount(&m_cs, 2000); }
	virtual ~CCircleBuffer() { DeleteCriticalSection(&m_cs); }

	size_t Write(char IN *pSrc, size_t IN nLen)
	{
		CSafeLock lock(m_cs);

		size_t empty = m_nBufferSize - m_nDataSize;
		if( empty < nLen )
			return -1;

		size_t gap = m_pTail - m_pWrite;
		if( gap < nLen )
		{
			memcpy(m_pWrite, pSrc, gap);

			m_pWrite = m_pHead;
			memcpy(m_pWrite, pSrc + gap, nLen - gap);

			m_pWrite += (nLen - gap);
			m_nDataSize += nLen;
		}
		else
		{
			memcpy(m_pWrite, pSrc, nLen);

			m_pWrite += nLen;
			m_nDataSize += nLen;
		}

		return m_nDataSize;
	}

	size_t Read(char OUT *pDest, size_t IN nReqSize)
	{
		CSafeLock lock(m_cs);

		if( m_nDataSize < nReqSize )
			return -1;

		char *pMark = m_pRead;

		size_t gap = m_pTail - pMark;
		if( gap < nReqSize )
		{
			memcpy(pDest, pMark, gap);

			pMark = m_pHead;
			memcpy(pDest+gap, pMark, nReqSize - gap);
		}
		else
		{
			memcpy(pDest, m_pRead, nReqSize);
		}

		return nReqSize;
	}

	size_t Erase(size_t IN releaseSize)
	{
		CSafeLock lock(m_cs);

		if( m_nDataSize < releaseSize )
			return -1;

		size_t gap = m_pTail - m_pRead;
		if( gap < releaseSize )
		{
			m_pRead = m_pHead + (releaseSize - gap);
			m_nDataSize -= releaseSize;
		}
		else
		{
			m_pRead += releaseSize;
			m_nDataSize -= releaseSize;
		}

		return m_nDataSize;
	}
};

//
enum eNetworkBuffer
{
	OP_SEND = 1,
	OP_RECV = 2,
	OP_INNER = 3,
};

class CNetworkBuffer
	: public CBuffer
{
public:
	WSABUF m_WSABuffer = {0,};
	eNetworkBuffer m_eOperator;

	CConnector *m_pSession = nullptr;

	//
public:
	CNetworkBuffer() {}
	virtual ~CNetworkBuffer() {}

	WSABUF& GetWSABuffer() { return m_WSABuffer; }
	eNetworkBuffer GetOperator() { return m_eOperator; }

	int SetSendData(CConnector *pSession, char *pData, int nDataSize)
	{
		if( !pData || m_nBufferSize < nDataSize )
			return -1;

		memcpy_s(m_pBuffer, m_nBufferSize, pData, nDataSize);
		m_nDataSize = nDataSize;

		m_WSABuffer.buf = m_pBuffer;
		m_WSABuffer.len = m_nDataSize;
		m_eOperator = eNetworkBuffer::OP_SEND;
		m_pSession = pSession;

		return m_nDataSize;
	}

	int SetRecvData(CConnector *pSession)
	{
		m_nDataSize = 0;

		m_WSABuffer.buf = m_pBuffer;
		m_WSABuffer.len = m_nBufferSize;
		m_eOperator = eNetworkBuffer::OP_RECV;
		m_pSession = pSession;

		return m_nBufferSize;
	}

	int SetInnerData(CConnector *pSession, char *pData, int nDataSize)
	{
		if( !pData || m_nBufferSize < nDataSize )
			return -1;

		m_eOperator = eNetworkBuffer::OP_INNER;

		memcpy_s(m_pBuffer, m_nBufferSize, pData, nDataSize);
		m_nDataSize = nDataSize;

		m_pSession = pSession;
		m_WSABuffer.buf = m_pBuffer;
		m_WSABuffer.len = m_nDataSize;

		return m_nDataSize;
	}
};

//
#endif //__BUFFER_H__