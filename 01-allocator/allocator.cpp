#include "allocator.h"
#include "allocator_pointer.h"
#include "allocator_error.h"

using namespace std;

/*
 * Util functions
 */

const int extra_size = 3 * sizeof(int) + sizeof(char**);

// offsets inside meta info
const int prev_offset = sizeof(int);
const int next_offset = 2 * sizeof(int);
const int ref_offset = 3 * sizeof(int);

void int_to_bytes(const int num, char* a) {
	memcpy(a, &num, sizeof(int));	
}

int bytes_to_int(const char* arr) {
	int res = 0; 
	memcpy(&res, arr, sizeof(int));
	return res;
}

/*
 * Meta information takes 20 bytes per block
 * 1. size of block
 * 2. offset from start of next block
 * 3. offset from start of prev block. Needed for delete
 * 4. reference to pointer to pointer to block. Needed for defrag
 * next block of last block is zero
 */

// -1 of parameter indicates not to change field of meta info
void set_ints(char* arr, int size, int prev, int next, char** ref) {
	if (size >= 0)
		int_to_bytes(size, arr);
	arr += sizeof(int);
	if (prev >= 0)
		int_to_bytes(prev, arr);
	arr += sizeof(int);
	if (next >= 0)
		int_to_bytes(next, arr);
	arr += sizeof(int);
	if (ref)
		std::memcpy(arr, &ref, sizeof(ref));
}

Pointer Allocator::alloc(size_t N) {
    if (N + extra_size > free_bytes) {
        throw AllocError(AllocErrorType::NoMemory, "No free space");
    } 
	
	int cur_pointer = 0;
	int cur_size = 0;
	int cur_next = 0;

	while (true) {
		cur_size = bytes_to_int(base + cur_pointer);
		cur_next = bytes_to_int(base + cur_pointer + next_offset);
		if (cur_next == 0) {
			//this is the last block
			if (cur_pointer + cur_size + (cur_size > 0) * extra_size
							+ extra_size + N >= size) {
				throw AllocError(AllocErrorType::NoMemory, "No available free block. Try defrag");
			}
			break;
		}
		
		if (cur_size + extra_size * (cur_size > 0) + extra_size + N
						<= cur_next - cur_pointer) {
			// we can alloc our block here
			break;
		}
		cur_pointer = cur_next;
	}
	Pointer p;
	int alloc_pointer = cur_pointer + (cur_size > 0) * extra_size + cur_size;
	// change current block meta
	set_ints(base + cur_pointer, -1, -1, alloc_pointer, NULL);
	// set new block meta
	set_ints(base + alloc_pointer, N, cur_pointer, cur_next, p.getp());
	if (cur_next > 0)
			set_ints(base + cur_next, -1, alloc_pointer, -1, NULL);
	p.set(base + alloc_pointer + extra_size);
    free_bytes -= N;
    return p;
}

void Allocator::free(Pointer& p){
    char *p_todel = (char*)p.get() - extra_size;
	int size_del = bytes_to_int(p_todel);
	int prev_del = bytes_to_int(p_todel + prev_offset);
    int next_del = bytes_to_int(p_todel + next_offset);

	set_ints(base + prev_del, -1, -1, next_del, NULL);
	set_ints(base + next_del, -1, prev_del, -1, NULL);
	set_ints(p_todel, 0, -1, -1, NULL);
	free_bytes += size_del;
	p.set(nullptr);
}

void Allocator::realloc(Pointer& p, size_t N){
	char *p_= (char*)p.get() - extra_size;
    
    if (p_ == nullptr || p_ < base || p_ > base + size) {
		// pointer was not from allocator 
        p = alloc(N);
        return;
    }

	int cur_size = bytes_to_int(p_);
    int cur_next = bytes_to_int(p_ + next_offset);
    if (N == cur_size){
        // do nothing
        return;
    }

    if (N < cur_size){
		set_ints(p_, N, -1, -1, NULL);

        free_bytes += cur_size - N;
        return;
    }
    free(p);
    p = alloc(N);
	memcpy(p.get(), p_ + extra_size, cur_size);
}


void Allocator::defrag(){
	int cur_pointer = 0;
	int cur_size = 0;
	int cur_next = 0;
	int cur_prev = 0;

	while (true) {
		cur_size = bytes_to_int(base + cur_pointer);
		cur_prev = bytes_to_int(base + cur_pointer + prev_offset);
		cur_next = bytes_to_int(base + cur_pointer + next_offset);
		if (cur_next == 0)
				break;
		if (cur_size + (cur_size > 0) * extra_size < cur_next - cur_pointer) {
			// need to move next block
			// first get meta about next block
			int next_size = bytes_to_int(base + cur_next);
			int next_next = bytes_to_int(base + cur_next + next_offset);
			char** next_ref = NULL;
			std::memcpy(&next_ref, base + cur_next + ref_offset, sizeof(char**));

			// get new place to move, refresh pointer of next block and move data
			int new_place = cur_pointer + (cur_size > 0) * extra_size + cur_size;
			*next_ref = base + new_place + extra_size;
			std::memcpy((void*) (base + new_place + extra_size),
						(void*)	(base + cur_next + extra_size),
							next_size
							);
			// refresh meta data of adjacent to next blocks
			set_ints(base + new_place, next_size, cur_pointer, next_next, next_ref);
			set_ints(base + cur_pointer, -1, -1, new_place, NULL);
			set_ints(base + next_next, -1, new_place, -1, NULL);
			cur_next = new_place;
		}
		cur_pointer = cur_next;
	}
}
