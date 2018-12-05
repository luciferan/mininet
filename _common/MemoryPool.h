#pragma once
#ifndef __MEMORYPOOL_H__
#define __MEMORYPOOL_H__

//
#include <stdlib.h>

//
#define _BOOST_MEM_POOL_

#if defined _BOOST_MEM_POOL_
#include "boost/pool/pool_alloc.hpp"

template <typename T, typename TMutex = boost::details::pool::default_mutex>
class CMemoryPool
{
public:
	static void* operator new(size_t)
	{
		return boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, TMutex>::allocate(1);
	}
	static void operator delete(void *p)
	{
		boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, TMutex>::deallocate((boost::fast_pool_allocator<T>::pointer)p, 1);
	}
	static void* operator new[](size_t size)
	{
		return boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, TMutex>::allocate(size);
	}
	static void operator delete[](void *p, size_t size)
	{
		boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, TMutex>::deallocate((boost::fast_pool_allocator<T>::pointer)p, size);
	}
};

#else //_BOOST_MEM_POOL_
//
template <typename T>
class CMemoryPool
{
public:
	static void* operator new(size_t)
	{
		size_t size = sizeof(T);
		void *p = malloc(size);
		return p;
	}
	static void operator delete(void *p)
	{
		free(p);
	}
	static void* operator new[](size_t cnt)
	{
		size_t size = sizeof(T);
		void *p = malloc(size * cnt);
		return p;
	}
	static void operator delete[](void *p, size_t size)
	{
		free(p);
	}
};
#endif //_BOOST_MEM_POOL_

#define SAFE_DELETE(p) { if( p ) delete p; p = nullptr; }
#define SAFE_DELETE_ARR(p) { if( p ) delete[] p; p = nullptr; }

//
#endif //__MEMORYPOOL_H__