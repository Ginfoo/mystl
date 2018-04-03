#pragma once
#include"malloc_i.h"
class malloc_ii {
private:
	static const int _ALIGN ;
	static const int _MAX_BYTES;
	static const int _NFREELISTS=16;
	static size_t round_up(size_t _Bytes);
	union obj {
		union obj* free_list_link;
		char client_data[1];
	};

	static  obj* volatile  free_list[_NFREELISTS];
	static size_t free_list_index(size_t _Bytes);
	static void* refill(size_t _Size);
	static char* chunk_alloc(size_t _Size, int& _nobjs);
	static char* free_satrt;
	static char* free_end;
	static size_t heap_size;
public:
	static void* allocate(const size_t _Size);
	static void deallocate(void* _Block,const size_t _Size);
	static void* reallocate(void* _Block,const size_t _old_Size,const size_t _new_Size);
};
const int malloc_ii::_NFREELISTS;

void* malloc_ii::allocate(const size_t _Size) {
	void* _res=nullptr;
	if (_Size > (size_t)_MAX_BYTES) {
		_res = malloc_i::allocate(_Size);
	}else{
		obj* volatile * cur_free_list = free_list + free_list_index(_Size);
	}
}