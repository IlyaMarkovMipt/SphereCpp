#ifndef ALLOCATOR
#define ALLOCATOR

#include <cstring>
#include <string>

// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Pointer;

/**
 * Wraps given memory area and provides defagmentation allocator interface on
 * the top of it.
 *
 *
 */


class Allocator {
    /**
     * Size of allocated buffer. In our case shouldn't be changed
     */   
    const size_t size;

    /**
     * Base of allocated memory
     */
    char * base;

    /**
     * Number of free bytes in memory
     */
    size_t free_bytes;

public:

    Allocator(void* base_, size_t size_):base((char*) base_), size(size_){
        free_bytes = size;
	std::memset(base, 0, size);
    }

    /**
     * TODO: semantics
     * @param N size_t
     */

    // TODO:
    Pointer alloc(size_t N);

    /**
     * TODO: semantics
     * @param p Pointer
     * @param N size_t
     */
    void realloc(Pointer& p, size_t N); 

    /**
     * TODO: semantics
     * @param p Pointer
     */
    void free(Pointer& p);

    /**
     * TODO: semantics
     */
    void defrag(); 

    /**
     * TODO: semantics
     */
    std::string dump() const { return ""; }
};

#endif // ALLOCATOR
