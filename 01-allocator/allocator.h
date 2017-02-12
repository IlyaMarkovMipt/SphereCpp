#ifndef ALLOCATOR
#define ALLOCATOR

#include <set>
#include <utility> // std::pair
#include <map>


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
    char const* base;


    /*
     * -------------------------
     *  Fully internal variables
     * ------------------------
     */

    /**
     * Number of free bytes in memory
     */
    size_t free_bytes;

    /**
     * map of pointer and relative positions in our memory.
     * Contains returned our Pointer-s and their positions of start and end of piece of memory
     */
    std::map<Pointer, std::pair<unsigned int, unsigned int>> pointers;

    /*
     * Compare struct for set of pairs
     */
    struct compare {
        bool operator()(const std::pair<unsigned int, unsigned int>& p1, const std::pair<unsigned int, unsigned int>& p2){
            return p1.first < p2.first;
        } 
    };

    /**
     * vector of free spaces
     */
    std::set<std::pair<unsigned int, unsigned int>, compare> spaces;

public:

    Allocator(void* base_, size_t size_):base((char*) base_), size(size_){
        free_bytes = size;
        spaces.insert(std::make_pair(0, size - 1));
    }

    /**
     * TODO: semantics
     * @param N size_t
     */

    // TODO:
    //TODO: ask whether return reference in sake of defrag. 
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
