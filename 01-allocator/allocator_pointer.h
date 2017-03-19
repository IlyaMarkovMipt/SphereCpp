#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER

#include <stdlib.h>
#include <iostream>
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Allocator;

class Pointer {
    char** pointer;
	int *pcounter;
public:
    Pointer() {
        pointer = (char**) malloc (sizeof(char*));
        *pointer = nullptr;
		pcounter = (int*) malloc (sizeof(int));
		*pcounter = 1;
    }
    Pointer(void* p){
        pointer = (char**) malloc (sizeof(char*));
        *pointer = (char*) p;
		pcounter = (int*) malloc (sizeof(int));
		*pcounter = 1;
    }
    void* get() const { 
        return (void*) *pointer;
    }
    void  set(void* new_p) {
        *pointer = (char*) new_p;
    }

    char** getp()const{ return pointer;}

    /*
     * Copy contructor
     */
    Pointer(const Pointer& p){
        pointer = p.getp();
		pcounter = p.pcounter;
		(*pcounter)++;
    }
    
    void operator=(const Pointer& p){
        //std::cout << "Pointer::operator=(const Pointer& )" << std::endl;
        pointer = p.getp();
		pcounter = p.pcounter;
		(*pcounter)++;
    }
    ~Pointer(){
        //TODO: I can refactor it with unique_ptr or shared_ptr, but no time for it
		(*pcounter)--;
		if (!*pcounter && pointer) {
            free(pointer);
			free(pcounter);
		}
    }
};

#endif //ALLOCATOR_POINTER
