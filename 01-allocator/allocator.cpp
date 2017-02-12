#include "allocator.h"
#include "allocator_pointer.h"
#include "allocator_error.h"

#include <cstring>
using namespace std;

Pointer Allocator::alloc(size_t N) {
    if (N > free_bytes){
        throw AllocError(AllocErrorType::NoMemory, "No free space");
    } 

    unsigned int start = 0;
    if (!pointers.empty()){
        //some logic        
        start = -1;
        for (auto it = spaces.begin(); it != spaces.end(); it++){
            if (it->second - it->first + 1 >= N){
                start = it->first;
                unsigned int end = it->second;
                spaces.erase(it);

                if (start + N <= end)
                    spaces.insert(make_pair(start + N, end));
                
                break;
            }
        }
        if (start == -1){
            throw AllocError(AllocErrorType::NoMemory, "No available free chunks. Memory is defragmeted. Try to call defrag()");
        }
    } else{
        spaces.clear();// delete one single element 
        spaces.insert(make_pair(N, size));
    }
    Pointer p((void*)(base + start));
    free_bytes -= N;
    pointers.emplace(p, make_pair(start, start + N - 1));
    return p;
}

void Allocator::free(Pointer& p){
    map<Pointer, pair<unsigned int, unsigned int>>::iterator del_pair_it = pointers.find(p);
    if (del_pair_it == pointers.end()){
        throw AllocError(AllocErrorType::InvalidFree, "Invalid free() usage");
    }
    pair<unsigned int, unsigned int> del_pair = (*del_pair_it).second;

    auto prev_free_space_it = spaces.end(); // free space before deleting chunk
    auto next_free_space_it = spaces.end(); // free space after deleting chunk

    int new_start = del_pair.first; // start of new free space
    int new_end = del_pair.second;  // end of new free space

    for (auto it = spaces.begin(); it != spaces.end(); it++){
        // TODO: Question. Doesn't iterator prev_free.. change while it changes? 
        if (it->second == del_pair.first){
            prev_free_space_it = it;
            new_start = it->first;
            it = spaces.erase(prev_free_space_it);
            continue;
        }
        if (it->first == del_pair.second){
            next_free_space_it = it;
            new_end = it->second;
            it = spaces.erase(next_free_space_it);
        }
    }

    

    spaces.insert(make_pair(new_start, new_end));
    pointers.erase(del_pair_it);
    free_bytes += del_pair.second - del_pair.first + 1;
    p.set(nullptr);
}

void Allocator::realloc(Pointer& p, size_t N){
    map<Pointer, pair<unsigned int, unsigned int>>::iterator pair_it = pointers.find(p);
    
    if (pair_it == pointers.end()){
        p = alloc(N);
        return;
    }

    pair<unsigned int, unsigned int> old_pair = (*pair_it).second;
    size_t current_size = old_pair.second - old_pair.first + 1;
    if (N == current_size){
        // do nothing
        return;
    }

    if (N < current_size){
        for (auto it = spaces.begin(); it != spaces.end(); it++){
            if (it->first == old_pair.second){
                int end = it->second;
                it = spaces.erase(it);
                spaces.insert(make_pair(old_pair.first + N - 1, end));
                break;
            }
        }

        free_bytes += current_size - N;

        //TODO: ask if iterator returns a reference
        old_pair.second = old_pair.first + N - 1;
        return;
    }

    free(p);
    p = alloc(N);
}


void Allocator::defrag(){
    int prev_end = -1;
//    std::cout << "In allocator before" << std::endl;
//    for (auto it = pointers.begin(); it != pointers.end(); it++){
//        std::cout << std::hex << it->first.getp() << " " << it->first.get() << std::dec << " ";
//    }
//
//    std::cout << std::endl;
//
    for (auto it = pointers.begin(); it != pointers.end(); ){ // they are all sorted in order of usual pointers. That is all we need.
        pair<Pointer, pair<unsigned int, unsigned int>> coords = *it;// first - Pointer, second - pair: start: end;
        if (coords.second.first > prev_end + 1){
            int size = coords.second.second - coords.second.first + 1;
            std::memcpy((void*) (base + prev_end + 1), // destination
                    (void*) (base + coords.second.first), // source
                    size * sizeof(char)); // number of bytes 
            coords.first.set((void*)(base + prev_end + 1));
            coords.second = make_pair(prev_end + 1, prev_end + size);
            it = pointers.erase(it);
            pointers.insert(coords);//we insert left always, so it doesn't affect iterator
            prev_end += size;
        } else {
            prev_end = coords.second.second;
            it++;
        }
    }
//    std::cout << "In allocator after: " << pointers.size() << std::endl;
//    for (auto it = pointers.begin(); it != pointers.end(); it++){
//        std::cout << std::hex << it->first.getp() << " " << it->first.get() << std::dec << " ";
//    }
//    std::cout << std::endl;
    spaces.clear();//delete all spaces
    if (prev_end < size - 1)
        spaces.insert(make_pair(prev_end + 1, size - 1));
}
