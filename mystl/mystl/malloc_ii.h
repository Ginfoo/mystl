#pragma once
#include"malloc_i.h"

class malloc_ii
{
private:
	static const int _ALIGN;
	static const int _MAX_BYTES;
	static const int _NFREELISTS = 16;
	static size_t round_up(size_t _Bytes);

	union obj
	{
		union obj* free_list_link;
		char client_data[1];
	};

	static obj* free_list[16];
	static size_t free_list_index(size_t _Bytes);
	static void* refill(size_t _Size_up);
	static char* chunk_alloc(size_t _Size_up, int& _nobjs);
	static char* free_start;
	static char* free_end;
	static size_t heap_size;
public:
	static void* allocate(const size_t _Size);
	static void deallocate(void* _Block, const size_t _Size);
	static void* reallocate(void* _Block, const size_t _old_Size, const size_t _new_Size);
};

const int malloc_ii::_NFREELISTS;
//填充 free_list
inline void* malloc_ii::refill(size_t _Size_up)
{
	//默认填充 20 个区块
	int n_objs = 20;
	//调用 chunk_alloc 函数 从内存中获取相应空间
	char* chunk = chunk_alloc(_Size_up, n_objs);
	obj* volatile * cur_free_list;
	obj* result = nullptr;
	obj* cur_obj;
	obj* next_obj;
	int i;
	//如果获取的区块数为 1 则直接将地址返回出去
	//：因为 chunk_alloc 函数 对于 参数 n_objs 是引用传递
	//：引用传递的目的是想知道 实际获取了几个块，因为有可能无法获得 n_objs 那么多的块
	if (1 == n_objs) return chunk;
	//若获得的块 大于 1
	//根据 _Size 找到当前的 obj**
	cur_free_list = free_list + free_list_index(_Size_up);
	//把当前从内存中获取的空间 首地址 char*  改为 obj* 
	//:将这一块返回给 申请者
	result = reinterpret_cast<obj*>(chunk);
	//当前的空闲链 
	*cur_free_list = next_obj = reinterpret_cast<obj*>(chunk + _Size_up);
	for (i = 1;; i++)
	{
		cur_obj = next_obj;
		next_obj = reinterpret_cast<obj*>(reinterpret_cast<char*>(next_obj + _Size_up));
		//当i==总申请块数 减1 时 则停止
		//因为已经有一块返回给客户端
		if (n_objs - 1 == i)
		{
			cur_obj->free_list_link = nullptr;
			break;
		}
		else
		{
			cur_obj->free_list_link = next_obj;
		}
	}
	return result;
}

inline char* malloc_ii::chunk_alloc(size_t _Size_up, int& _nobjs)
{
	char* result;
	size_t total_bytes = _Size_up * _nobjs;
	size_t left_bytes = free_end - free_start;

	if (left_bytes >= total_bytes)
	{
		result = free_start;
		free_start += total_bytes;
		return result;
	}
	else if (left_bytes >= _Size_up)
	{
		_nobjs = left_bytes / _Size_up;
		total_bytes = _Size_up * _nobjs;
		result = free_start;
		free_start += total_bytes;
		return result;
	}
	else
	{
		size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
		if (left_bytes > 0)
		{
			obj* volatile *des_free_list = free_list + free_list_index(left_bytes);
			(reinterpret_cast<obj*>(free_start))->free_list_link = *des_free_list;
			*des_free_list = reinterpret_cast<obj*>(free_start);
		}

		free_start =static_cast<char*>(malloc(bytes_to_get));
		if(nullptr==free_start)
		{
			
		}
	}
}

/*
 * 分配内存
 * _Size:申请内存的大小
 */
inline void* malloc_ii::allocate(const size_t _Size)
{
	void* _res = nullptr;
	//如果所申请内存量 大于 最大值 则直接交给 一级配置器
	if (_Size > static_cast<size_t>(_MAX_BYTES))
	{
		_res = malloc_i::allocate(_Size);
	}
		//否则 自己处理
	else
	{
		//根据 _Size 在空闲链上找到 待分配区块的入口地址
		//：free_list 是个数组【数组名也是一个指针】，由16个 obj* 构成
		obj* volatile * cur_free_list = free_list + free_list_index(_Size);
		obj* result = *cur_free_list;
		//如果 查到的obj*的指向 为nullptr,则进行装填
		if (result == nullptr)_res = refill(round_up(_Size));
			//否则 将当前的obj*替换为 下一个
			//：类似于单链表的遍历
		else
		{
			*cur_free_list = result->free_list_link;
			_res = result;
		}
	}
	return _res;
}
