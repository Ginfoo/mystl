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
#if defined(__STL_NO_BAD_ALLOC) || !defined(__STL_USE_EXCEPTIONS)
#include <cstdio>
#include <cstdlib>
#define _THROW_BAD_ALLOC              \
	fprintf(stderr, "out of memory"); \
	exit(1)
#else
#include <new>
#define _THROW_BAD_ALLOC throw std::bad_alloc()
#endif
#endif

class malloc_i
{
  private:
	static void (*oom_malloc_handler)();
	static void *oom_malloc(const size_t);
	static void *oom_realloc(void *, const size_t);

  public:
	static void *allocate(const size_t);
	static void deallocate(void *, size_t);
	static void *reallocate(void *, size_t, const size_t);
	static void (*set_oom_malloc_handler(void (*_new_handler)()))();
};
void (*malloc_i::oom_malloc_handler)() = nullptr;
inline void *malloc_i::oom_malloc(const size_t _Size)
{
	void (*cur_oom_malloc_handler)() = nullptr;
	void *result = nullptr;
	for (;;)
	{
		cur_oom_malloc_handler = oom_malloc_handler;
		if (nullptr == cur_oom_malloc_handler)
		{
			_THROW_BAD_ALLOC;
		}
		(*cur_oom_malloc_handler)();
		result = malloc(_Size);
		if (result)
			return result;
	}
}
inline void *malloc_i::oom_realloc(void *_Block, const size_t _Size)
{
	void (*cur_oom_malloc_handler)() = nullptr;
	void *result = nullptr;
	for (;;)
	{
		cur_oom_malloc_handler = oom_malloc_handler;
		if (nullptr == cur_oom_malloc_handler)
		{
			_THROW_BAD_ALLOC;
		}
		(*cur_oom_malloc_handler)();
		result = realloc(_Block, _Size);
	}
}
inline void *malloc_i::allocate(const size_t _Size)
{
	void *result = malloc(_Size);
	if (nullptr == result)
	{
		result = oom_malloc(_Size);
	}
	return result;
}
inline void malloc_i::deallocate(void *_Block, size_t _Size)
{
	free(_Block);
}
inline void *malloc_i::reallocate(void *_Block, size_t _old_Size, size_t _new_Size)
{
	void *result = realloc(_Block, _new_Size);
	if (nullptr == result)
	{
		result = oom_realloc(_Block, _new_Size);
	}
	return result;
}
inline void (*malloc_i::set_oom_malloc_handler(void (*_new_handler)()))()
{
	void (*_old_handler)() = oom_malloc_handler;
	oom_malloc_handler = _new_handler;
	return (_old_handler);
}