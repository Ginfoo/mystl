#pragma once
#include <new>
#include <cstdlib>
/**
   ѧϰSGI STL ��׼
   ����SGI STL 
**/

/*
   mystl_alloc.h �����ڴ�ռ�ķ������ͷ�
*/


//����꣺�׳��ڴ�����쳣
#ifndef _THROW_BAD_ALLOC
#  if defined(__STL_NO_BAD_ALLOC)||!defined(__STL_USE_EXCEPTIONS)
#      include<iostream>
#      include<cstdlib>
#      define _THROW_BAD_ALLOC std::cerr<<"out of memory\n";exit(1)
#  else 
#      include<new>
#      define _THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif


class malloc_first_template {
private:
	static void* oom_malloc(const size_t);
	static void* oom_realloc(void*, const size_t);
public:
	//�����ڴ�ռ�
	static void* allocate(const size_t sz)
	{
		void* result = malloc(sz);
		if(result==nullptr)
		{
			result = oom_malloc(sz);
		}
		return result;
	}
	//�����ڴ�ռ�
	static void deallocate(void* ptr,size_t sz)
	{
		free(ptr);
	}
	//���·����ڴ�ռ�
	static void* reallocate(void* ptr,size_t old_sz,const size_t new_sz)
	{
		void* result = realloc(ptr, new_sz);
		if(result==nullptr)
		{
			result = oom_realloc(ptr, new_sz);
		}
		return result;
	}
};

//�ڴ����
inline void* malloc_first_template::oom_malloc(const size_t)
{
	void(*my_malloc_handler)();
	void *result;
	while (true)
	{
		my_malloc_handler=_;
		if(nullptr==my_malloc_handler)
		{
			_THROW_BAD_ALLOC;
		}
	}
}

