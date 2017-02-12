#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER

#include <stdlib.h>
#include <iostream>
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Allocator;

class Pointer {
    char** pointer;
public:
    Pointer() {
        //std::cout << "BEGIN Pointer::Pointer();" << std::endl;
        pointer = (char**) malloc (sizeof(char*));
        *pointer = nullptr;
        //std::cout << "pointer = " << std::hex << pointer << std::dec << std::endl;
        //std::cout << "*pointer = " << std::hex << *pointer << std::dec << std::endl;
    }
    Pointer(void* p){
        //std::cout << "BEGIN Pointer::Pointer(void*);" << std::endl;
        pointer = (char**) malloc (sizeof(char*));
        *pointer = (char*) p;
        //std::cout << "Pointer::Pointer(" << std::hex << p << std::dec << ");" << std::endl;
    }
    void* get() const { 
        //std::cout << "Pointer::get();" << std::endl;
        return (void*) *pointer;
    }
    void  set(void* new_p) {
       // std::cout << "Pointer::set(" << std::hex << new_p << std::dec << ");" << std::endl;
        *pointer = (char*) new_p;
    }

    char** getp()const{ return pointer;}

    bool operator<(const Pointer& p) const {
        //std::cout << "Pointer::operator<()" << std::endl;
        return *pointer < p.get();
    }

    /*
     * Copy contructor
     */
    Pointer(const Pointer& p){
        //std::cout << "BEGIN Pointer::Pointer(const Pointer& p);" << std::endl;
        pointer = p.getp();
    }
    
    void operator=(const Pointer& p){
        //std::cout << "Pointer::operator=(const Pointer& )" << std::endl;
        pointer = p.getp();
    }
    ~Pointer(){
        //std::cout << "BEGIN Pointer::~Pointer() pointer=:" <<std::hex << pointer << ", *pointer = "<< (void*)*pointer << std::dec << std::endl;
        //if (pointer && *pointer)
            //free(pointer);
    }
};

#endif //ALLOCATOR_POINTER
