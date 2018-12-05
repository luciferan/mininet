#ifndef __OLINK_H__
#define __OLINK_H__

#pragma once

#include <windows.h>

/////////////////////////////////////////////////////////////////////////////////
template <class T>
class OLink
{
public:
	int		m_iIndex;

	T*		m_pPrev;
	T*		m_pNext;

public:
	OLink(void) : m_pPrev(NULL), m_pNext(NULL) {};
	virtual ~OLink(void) {};
};

/////////////////////////////////////////////////////////////////////////////////
#endif //__OLINK_H__