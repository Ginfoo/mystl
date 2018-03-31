#pragma once
#include <cstdlib>
/**
   学习SGI STL 标准
   基于SGI STL 
**/

/*
   mystl_alloc.h 负责内存空间的分配与释放
*/

class malloc_first_template {
private:
	static void* out_m_malloc(const size_t);
	static void* out_m_realloc(void*, const size_t);
public:
	//分配内存空间
	static void* allocate(const size_t sz)
	{
		void* result = malloc(sz);
		if(result==nullptr)
		{
			result = out_m_malloc(sz);
		}
		return result;
	}
	//销毁内存空间
	static void deallocate(void* ptr,size_t sz)
	{
		free(ptr);
	}
	//重新分配内存空间
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

