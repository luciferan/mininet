#pragma once
#ifndef __OOBJMGRBASE_H__
#define __OOBJMGRBASE_H__

//
#include <windows.h>

//
template <class T>
class OObjMgrBase
{
public:
	T *m_pData = nullptr;
	T *m_pFree = nullptr;

	T *m_pHead = nullptr;
	T *m_pTail = nullptr;

	int m_iMaxObjCount = 0;
	int m_iUseObjCount = 0;

protected:
	OObjMgrBase(void) {};
	virtual ~OObjMgrBase(void);

	virtual bool ObjMgrInitialize(const int iMaxObjectCount);
	virtual void ObjMgrFinalize();

	T*		GetFreeObj();
	void	SetFreeObj(T *pObj);

	T*		GetObj(const int iIndex);
};

/////////////////////////////////////////////////////////////////////////////////
template <class T>
OObjMgrBase<T>::~OObjMgrBase(void)
{
	ObjMgrFinalize();
}

template <class T>
bool OObjMgrBase<T>::ObjMgrInitialize(const int iMaxObjCount)
{
	if( m_pData )
		delete m_pData;
	
	m_pData = new T[iMaxObjCount+1];
	if( !m_pData )
		return false;

	for( int iIndex = 0; iIndex <= iMaxObjCount; ++iIndex )
	{
		m_pData[iIndex].m_iIndex = iIndex;

		m_pData[iIndex].m_pPrev = NULL;
		m_pData[iIndex].m_pNext = NULL;
	}

	T *pIter = NULL;
	for( int iIndex = 0; iIndex <= iMaxObjCount; ++iIndex )
	{
		pIter = &m_pData[iIndex];

		//
		pIter->m_pPrev = ( 1 < iIndex				? &m_pData[iIndex-1] : NULL );
		pIter->m_pNext = ( iMaxObjCount > iIndex	? &m_pData[iIndex+1] : NULL );
	}

	m_pHead = &m_pData[0];
	m_pTail = &m_pData[iMaxObjCount];

	m_pFree = m_pHead->m_pNext;

	m_iMaxObjCount = iMaxObjCount;
	m_iUseObjCount = 0;

	//
	return true;
}

template <class T>
void OObjMgrBase<T>::ObjMgrFinalize()
{
#if defined DEBUG || defined _DEBUG
	// 이거 왜 디버그모드에서는 에러나지?
#else
	if( m_pData )
		delete m_pData;
#endif //defined DEBUG || defined _DEBUG
}

template <class T>
T* OObjMgrBase<T>::GetFreeObj()
{
	if( m_iUseObjCount >= m_iMaxObjCount )
		return NULL;
	if( NULL == m_pFree )
		return NULL;

	T *pRet = m_pFree;

	//
	m_pHead->m_pNext = m_pFree->m_pNext;

	if( m_pTail != m_pFree )
		m_pFree->m_pNext->m_pPrev = m_pHead;

	m_pFree = m_pHead->m_pNext;

	//
	m_iUseObjCount += 1;

	//
	return pRet;
}

template <class T>
void OObjMgrBase<T>::SetFreeObj(T *pObj)
{
	if( !pObj || 0>=pObj->m_iIndex )
		return;

	//
	m_pTail->m_pNext = pObj;

	pObj->m_pPrev = m_pTail;
	pObj->m_pNext = NULL;

	m_pTail = pObj;

	if( NULL == m_pHead->m_pNext )
		m_pHead->m_pNext = m_pTail;
	m_pFree = m_pHead->m_pNext;

	//
	m_iUseObjCount -= 1;
}

template <class T>
T* OObjMgrBase<T>::GetObj(const int iIndex)
{
	if( 0 >= iIndex || iIndex > m_iMaxObjCount )
		return NULL;

	return &m_pData[iIndex];
}

/////////////////////////////////////////////////////////////////////////////////
#endif //__OOBJMGRBASE_H__