#pragma once
#include <cstdlib>
/**
   ѧϰSGI STL ��׼
   ����SGI STL 
**/

/*
   mystl_alloc.h �����ڴ�ռ�ķ������ͷ�
*/

class malloc_first_template {
private:
	static void* out_m_malloc(const size_t);
	static void* out_m_realloc(void*, const size_t);
public:
	//�����ڴ�ռ�
	static void* allocate(const size_t sz)
	{
		void* result = malloc(sz);
		if(result==nullptr)
		{
			result = out_m_malloc(sz);
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
			result = out_m_realloc(ptr, new_sz);
		}
		return result;
	}
};

inline void* malloc_first_template::out_m_malloc(const size_t)
{
	void(*my_malloc_handler)();
	void *result;
	while (true)
	{
		my_malloc_handler;
		if(nullptr==my_malloc_handler)
		{
			
		}
	}
}

