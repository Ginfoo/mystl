#pragma once
#include <new>
#include <cstdlib>
/**
   学习SGI STL 标准
   基于SGI STL 
**/

/*
   mystl_alloc.h 负责内存空间的分配与释放
*/


//定义宏：抛出内存分配异常
#ifndef _THROW_BAD_ALLOC
#  if defined(__STL_NO_BAD_ALLOC)||!defined(__STL_USE_EXCEPTIONS)
#      include<stdio.h>
#      include<stdlib.h>
#      define _THROW_BAD_ALLOC fprintf(stderr,"out of memory");exit(1)
#  else 
#      include<new>
#      define _THROW_BAD_ALLOC throw std::bad_alloc()
#  endif
#endif


class malloc_i{
private:
	static void(*oom_malloc_handler)();
	static void* oom_malloc(const size_t);
	static void* oom_realloc(void*, const size_t);
public:
	static void* allocate(const size_t sz);
	static void deallocate(void* ptr, size_t sz);
	static void* reallocate(void* ptr, size_t old_sz, const size_t new_sz);
	static void(*set_oom_malloc_handler(void(*f)()))();
};
void(*malloc_i::oom_malloc_handler)() = nullptr;
void* malloc_i::oom_malloc(const size_t _Size) {
	void(*cur_oom_malloc_handler)();
	void* result;
	for (;;) {
		cur_oom_malloc_handler = oom_malloc_handler;
		if (nullptr == cur_oom_malloc_handler) { _THROW_BAD_ALLOC; }
		(*cur_oom_malloc_handler)();
		result = malloc(_Size);
		if (result)return result;
	}
}
void* malloc_i::oom_realloc(void* _Block, const size_t _Size) {
	void(*cur_oom_malloc_handler)();
	void *result;
	for (;;) {
		cur_oom_malloc_handler = oom_malloc_handler;
		if (nullptr == cur_oom_malloc_handler) { _THROW_BAD_ALLOC; }
		(*cur_oom_malloc_handler)();
		result = realloc(_Block, _Size);
	}
}