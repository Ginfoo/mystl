#pragma once
#include"malloc_i.h"
#include <corecrt_memcpy_s.h>

class malloc_ii
{
private:
	static const int _ALIGN = 8;
	static const int _MAX_BYTES = 128;
	static const int MAX_BLOCKS_GROUP = 16;
	//将给定的字节数 上升为8的倍数
	static size_t round_up(size_t _Bytes);

	//内存块:block 大小未定
	//但每个 未用的 block 中都开辟一小块区域tab 指向 下一个空闲 block
	typedef union tab
	{
		union tab* next_block;
		char client_data[1];
	} block;

	//用数组 存放每个 blocks_group的起始地址,共16个组，编号为0:15
	static block* blocks_group[MAX_BLOCKS_GROUP];
	//根据字节数，计算并获取该字节 所在的组数 
	static size_t get_blocks_group_offset(const size_t _Bytes);
	//将chunk填充为可用/可管理的 blocks
	static void* refill_blocks_from_chunk(const size_t _Size_up);
	//从池中获取 大块 chunk
	static char* get_chunk_from_pool(const size_t _Size_up, int& block_nums);
	//内存池的起始地址
	static char* start_pool;
	//内存池的结束地址
	static char* end_pool;
	//堆大小,记录总共获得了多少字节
	static size_t heap_size;

public:

	static void* allocate(const size_t _Size);
	static void deallocate(void* _Block, const size_t _Size);
	static void* reallocate(void* _Block, const size_t _old_Size, const size_t _new_Size);
};

//定义式
const int malloc_ii::_ALIGN;
const int malloc_ii::_MAX_BYTES;
const int malloc_ii::MAX_BLOCKS_GROUP;
char* malloc_ii::start_pool = nullptr;
char* malloc_ii::end_pool = nullptr;
size_t malloc_ii::heap_size = 0;

inline size_t malloc_ii::round_up(size_t _Bytes)
{
	return (((_Bytes) + static_cast<size_t>(_ALIGN) - 1)
		& ~(static_cast<size_t>(_ALIGN) - 1));
}

inline size_t malloc_ii::get_blocks_group_offset(const size_t _Bytes)
{
	return (_Bytes + static_cast<size_t>(_ALIGN) - 1) / static_cast<size_t>(_ALIGN) - 1;
}

inline void* malloc_ii::refill_blocks_from_chunk(const size_t _Size_up)
{
	//第一个block 返回；给申请者使用
	block* first_block = nullptr;
	block* unmounted_block = nullptr;
	//默认取20个block
	int block_nums = 20;
	char* chunk = get_chunk_from_pool(_Size_up, block_nums);
	first_block = reinterpret_cast<block*>(chunk);
	//如果仅获取了一个block直接返回
	if (1 == block_nums)return first_block;

	//目标组：即所申请的blocks应该放入的组
	block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(_Size_up);
	//把第二个block 放入组中
	*target_blocks_group = unmounted_block = first_block + _Size_up;
	block* mounted_block = nullptr;
	//将blocks 挂载到对应的组中
	for (int i = 1;; i++)
	{
		mounted_block = unmounted_block;
		unmounted_block = mounted_block + _Size_up;
		if (i == block_nums - 1)
		{
			mounted_block->next_block = nullptr;
			break;
		}
		else
		{
			mounted_block->next_block = unmounted_block;
		}
	}
	return first_block;
}

inline char* malloc_ii::get_chunk_from_pool(const size_t _Size_up, int& block_nums)
{
	char* chunk_ptr = nullptr;
	size_t total_bytes = _Size_up * block_nums;
	const size_t bytes_left = end_pool - start_pool;

	if (bytes_left > total_bytes)
	{
		chunk_ptr = start_pool;
		start_pool += total_bytes;
		return chunk_ptr;
	}
	else if (bytes_left > _Size_up)
	{
		block_nums = bytes_left / _Size_up;
		total_bytes = _Size_up * block_nums;
		chunk_ptr = start_pool;
		start_pool += total_bytes;
		return chunk_ptr;
	}
	else
	{
		const size_t pool_to_get = 2 * total_bytes + round_up(heap_size >> 4);
		if (bytes_left > 0)
		{
			block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(bytes_left);
			reinterpret_cast<block*>(start_pool)->next_block = *target_blocks_group;
			*target_blocks_group = reinterpret_cast<block*>(start_pool);
		}
		start_pool = static_cast<char*>(malloc(pool_to_get));
		//如果获取pool失败
		if (start_pool == nullptr)
		{
			block* volatile* target_blocks_group;
			block* mounted_block;
			//查询已有的 但尚未使用的block
			for (int i = _Size_up; i < _MAX_BYTES; i += _ALIGN)
			{
				target_blocks_group = blocks_group + get_blocks_group_offset(i);
				mounted_block = *target_blocks_group;
				if (mounted_block != nullptr)
				{
					*target_blocks_group = mounted_block->next_block;
					start_pool = reinterpret_cast<char*>(mounted_block);
					end_pool = start_pool + i;
					return (get_chunk_from_pool(_Size_up, block_nums));
				}
			}
			end_pool = nullptr;
			start_pool = static_cast<char*>(malloc_i::allocate(pool_to_get));
		}
		//如果start_pool !=nullptr
		heap_size += pool_to_get;
		end_pool = start_pool + pool_to_get;
		return get_chunk_from_pool(_Size_up, block_nums);
	}
}

/*
 * 分配内存
 * _Size:申请内存的大小
 */
inline void* malloc_ii::allocate(const size_t _Size)
{
	void* block_ptr = nullptr;
	//如果所申请内存量 大于 最大值 则直接交给 一级配置器
	if (_Size > static_cast<size_t>(_MAX_BYTES))
	{
		block_ptr = malloc_i::allocate(_Size);
	}
		//否则 自己处理
	else
	{
		block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(_Size);
		block* mounted_block = *target_blocks_group;
		//如果  为nullptr,则进行装填
		if (mounted_block == nullptr)block_ptr = refill_blocks_from_chunk(round_up(_Size));

		else
		{
			*target_blocks_group = mounted_block->next_block;
			block_ptr = mounted_block;
		}
	}
	return block_ptr;
}

inline void malloc_ii::deallocate(void* _Block, const size_t _Size)
{
	if (_Size > static_cast<size_t>(_MAX_BYTES))
		malloc_i::deallocate(_Block, _Size);
	else
	{
		block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(_Size);
		block* unmounted_block = reinterpret_cast<block*>(_Block);
		unmounted_block->next_block = *target_blocks_group;
		*target_blocks_group = unmounted_block;
	}
}

inline void* malloc_ii::reallocate(void* _Block, const size_t _old_Size, const size_t _new_Size)
{
	if (_old_Size > static_cast<size_t>(_MAX_BYTES) && _new_Size > static_cast<size_t>(_MAX_BYTES))
	{
		return realloc(_Block, _new_Size);
	}
	if (round_up(_old_Size) == round_up(_new_Size))return _Block;
	void* block_ptr = malloc_ii::allocate(_new_Size);
	//size_t copy_size = _new_Size > _old_Size ? _old_Size : _new_Size;
	memcpy_s(block_ptr, _new_Size, _Block, _old_Size);
	return block_ptr;
}
