#pragma once
#include"malloc_i.h"
#include <corecrt_memcpy_s.h>

class malloc_ii
{
private:
	static const int _ALIGN = 8;
	static const int _MAX_BYTES = 128;
	static const int MAX_BLOCKS_GROUP = 16;
	//���������ֽ��� ����Ϊ8�ı���
	static size_t round_up(size_t _Bytes);

	//�ڴ��:block ��Сδ��
	//��ÿ�� δ�õ� block �ж�����һС������tab ָ�� ��һ������ block
	typedef union tab
	{
		union tab* next_block;
		char client_data[1];
	} block;

	//������ ���ÿ�� blocks_group����ʼ��ַ,��16���飬���Ϊ0:15
	static block* blocks_group[MAX_BLOCKS_GROUP];
	//�����ֽ��������㲢��ȡ���ֽ� ���ڵ����� 
	static size_t get_blocks_group_offset(const size_t _Bytes);
	//��chunk���Ϊ����/�ɹ���� blocks
	static void* refill_blocks_from_chunk(const size_t _Size_up);
	//�ӳ��л�ȡ ��� chunk
	static char* get_chunk_from_pool(const size_t _Size_up, int& block_nums);
	//�ڴ�ص���ʼ��ַ
	static char* start_pool;
	//�ڴ�صĽ�����ַ
	static char* end_pool;
	//�Ѵ�С,��¼�ܹ�����˶����ֽ�
	static size_t heap_size;

public:

	static void* allocate(const size_t _Size);
	static void deallocate(void* _Block, const size_t _Size);
	static void* reallocate(void* _Block, const size_t _old_Size, const size_t _new_Size);
};

//����ʽ
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
	//��һ��block ���أ���������ʹ��
	block* first_block = nullptr;
	block* unmounted_block = nullptr;
	//Ĭ��ȡ20��block
	int block_nums = 20;
	char* chunk = get_chunk_from_pool(_Size_up, block_nums);
	first_block = reinterpret_cast<block*>(chunk);
	//�������ȡ��һ��blockֱ�ӷ���
	if (1 == block_nums)return first_block;

	//Ŀ���飺���������blocksӦ�÷������
	block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(_Size_up);
	//�ѵڶ���block ��������
	*target_blocks_group = unmounted_block = first_block + _Size_up;
	block* mounted_block = nullptr;
	//��blocks ���ص���Ӧ������
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
		//�����ȡpoolʧ��
		if (start_pool == nullptr)
		{
			block* volatile* target_blocks_group;
			block* mounted_block;
			//��ѯ���е� ����δʹ�õ�block
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
		//���start_pool !=nullptr
		heap_size += pool_to_get;
		end_pool = start_pool + pool_to_get;
		return get_chunk_from_pool(_Size_up, block_nums);
	}
}

/*
 * �����ڴ�
 * _Size:�����ڴ�Ĵ�С
 */
inline void* malloc_ii::allocate(const size_t _Size)
{
	void* block_ptr = nullptr;
	//����������ڴ��� ���� ���ֵ ��ֱ�ӽ��� һ��������
	if (_Size > static_cast<size_t>(_MAX_BYTES))
	{
		block_ptr = malloc_i::allocate(_Size);
	}
		//���� �Լ�����
	else
	{
		block* volatile * target_blocks_group = blocks_group + get_blocks_group_offset(_Size);
		block* mounted_block = *target_blocks_group;
		//���  Ϊnullptr,�����װ��
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
