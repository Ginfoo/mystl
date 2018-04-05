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
//��� free_list
inline void* malloc_ii::refill(size_t _Size_up)
{
	//Ĭ����� 20 ������
	int n_objs = 20;
	//���� chunk_alloc ���� ���ڴ��л�ȡ��Ӧ�ռ�
	char* chunk = chunk_alloc(_Size_up, n_objs);
	obj* volatile * cur_free_list;
	obj* result = nullptr;
	obj* cur_obj;
	obj* next_obj;
	int i;
	//�����ȡ��������Ϊ 1 ��ֱ�ӽ���ַ���س�ȥ
	//����Ϊ chunk_alloc ���� ���� ���� n_objs �����ô���
	//�����ô��ݵ�Ŀ������֪�� ʵ�ʻ�ȡ�˼����飬��Ϊ�п����޷���� n_objs ��ô��Ŀ�
	if (1 == n_objs) return chunk;
	//����õĿ� ���� 1
	//���� _Size �ҵ���ǰ�� obj**
	cur_free_list = free_list + free_list_index(_Size_up);
	//�ѵ�ǰ���ڴ��л�ȡ�Ŀռ� �׵�ַ char*  ��Ϊ obj* 
	//:����һ�鷵�ظ� ������
	result = reinterpret_cast<obj*>(chunk);
	//��ǰ�Ŀ����� 
	*cur_free_list = next_obj = reinterpret_cast<obj*>(chunk + _Size_up);
	for (i = 1;; i++)
	{
		cur_obj = next_obj;
		next_obj = reinterpret_cast<obj*>(reinterpret_cast<char*>(next_obj + _Size_up));
		//��i==��������� ��1 ʱ ��ֹͣ
		//��Ϊ�Ѿ���һ�鷵�ظ��ͻ���
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
 * �����ڴ�
 * _Size:�����ڴ�Ĵ�С
 */
inline void* malloc_ii::allocate(const size_t _Size)
{
	void* _res = nullptr;
	//����������ڴ��� ���� ���ֵ ��ֱ�ӽ��� һ��������
	if (_Size > static_cast<size_t>(_MAX_BYTES))
	{
		_res = malloc_i::allocate(_Size);
	}
		//���� �Լ�����
	else
	{
		//���� _Size �ڿ��������ҵ� �������������ڵ�ַ
		//��free_list �Ǹ����顾������Ҳ��һ��ָ�롿����16�� obj* ����
		obj* volatile * cur_free_list = free_list + free_list_index(_Size);
		obj* result = *cur_free_list;
		//��� �鵽��obj*��ָ�� Ϊnullptr,�����װ��
		if (result == nullptr)_res = refill(round_up(_Size));
			//���� ����ǰ��obj*�滻Ϊ ��һ��
			//�������ڵ�����ı���
		else
		{
			*cur_free_list = result->free_list_link;
			_res = result;
		}
	}
	return _res;
}
