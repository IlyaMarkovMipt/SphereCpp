#include "external_sort.h"
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <exception>

typedef int TYPE;
 
unsigned long get_mem_free() {
    std::string token;
    std::ifstream file("/proc/meminfo");
    while(file >> token) {
        if(token == "MemFree:") {
            unsigned long mem;
            if(file >> mem) {
                return mem * 1024;
            } else {
                return 0;       
            }
        }
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return 0; // nothing found
}

unsigned int get_chunk_size(const std::string& input_filename) {
	std::ifstream in(input_filename, std::ifstream::ate |
			 std::ifstream::binary);
	unsigned long file_size = (unsigned long) in.tellg();
	unsigned long free_memory = (unsigned long) get_mem_free();
	unsigned int chunk_size = 0;	 
	if (free_memory > file_size) {
		chunk_size = file_size / DEFAULT_CHUNK_NUMBER;
		return chunk_size - chunk_size % sizeof(TYPE);
	}
	chunk_size = free_memory / 512;
	return chunk_size - chunk_size % sizeof(TYPE); 
}

void sort(const std::string& input_filename,
		const  std::string& output_filename) {
	unsigned int chunk_size = get_chunk_size(input_filename);	
	std::cout << "chunk size = " << chunk_size << std::endl;
	unsigned int initial_chunks_count = split_into_chunks(input_filename,
								chunk_size);
	std::cout << "Split into " << 
			initial_chunks_count << " chunks of size "
			<< chunk_size
			<< std::endl;

	std::vector<std::string> chunk_filenames;
	for (int i = 0; i < initial_chunks_count; ++i) {
                sort_chunk(get_chunk_filename(i), chunk_size);
                std::cout << "Chunk #" << i << " sorted" << std::endl;
		chunk_filenames.emplace_back(get_chunk_filename(i));
	}

	merge_sorted_chunks(chunk_filenames, 
			get_chunk_filename(initial_chunks_count));
	delete_files(chunk_filenames);
	if (std::rename(get_chunk_filename(initial_chunks_count).c_str(),
				 output_filename.c_str()) != 0) {
		throw std::runtime_error("Can't rename chunk into output file");
	}
}

unsigned int split_into_chunks(const std::string& filename,
		 unsigned int chunk_size) {
	std::ifstream input_file(filename, std::ios::binary);
	int chunks_count = 0;
	char* buffer = new char[chunk_size];

	while (!input_file.eof()) {
		input_file.read(buffer, chunk_size);
		auto read_length = input_file.gcount();
		if (read_length > 0) {
			std::cout << chunks_count << " " << read_length << std::endl;
			auto chunk_filename = get_chunk_filename(chunks_count++);
			std::ofstream chunk_file(chunk_filename,
					 std::ios::binary | std::ios::trunc);
			chunk_file.write(buffer, read_length);
			chunk_file.close();
		}
	}

	delete [] buffer;
	return chunks_count;
}

void sort_chunk(const std::string& chunk_filename, unsigned int chunk_size) {
	std::vector<TYPE> data(chunk_size / sizeof(TYPE));
	std::fstream chunk;

	chunk.open(chunk_filename, std::ios::binary | std::ios::in);
	chunk.read((char*) & data[0], chunk_size);
	chunk.close();

	auto values_count = chunk.gcount() / sizeof(TYPE);
	std::sort(data.begin(), data.begin() + values_count, std::less<TYPE>());

	chunk.open(chunk_filename, std::ios::binary | std::ios::out | std::ios::trunc);
	chunk.write((char*) & data[0], values_count * sizeof(TYPE));
	chunk.close();
}


template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void merge_sorted_chunks(const std::vector<std::string>& chunk_filenames,
			 std::string output_filename) {
    std::vector<std::unique_ptr<std::ifstream>> chunks;
    std::vector<BinaryStreamWalker<TYPE>> heap;
    std::ofstream output_file(output_filename, std::ios::binary | std::ios::trunc);

    for (auto it = chunk_filenames.begin(); it != chunk_filenames.end(); ++it) {
        chunks.emplace_back(make_unique<std::ifstream>(*it, std::ios::binary));
        heap.emplace_back(*(chunks.back()));
        if (!heap.back().move()) {
            heap.pop_back();
        }
    }

    auto heap_cmp = [](const BinaryStreamWalker<TYPE>& a,
			 const BinaryStreamWalker<TYPE>& b) {
        return a.value() > b.value();
    };

    
    std::make_heap(heap.begin(), heap.end(), heap_cmp);
    while (heap.size() > 1) {
        std::pop_heap(heap.begin(), heap.end(), heap_cmp);
        TYPE smallest = heap.back().value();
        output_file.write((char*) & smallest, sizeof(TYPE) / sizeof(char));
        if (heap.back().move()) {
            std::push_heap(heap.begin(), heap.end(), heap_cmp);
        }
        else {
            heap.pop_back();
        }
    }
    // Read up last file
    do {
        TYPE smallest = heap.back().value();
        output_file.write((char*) & smallest, sizeof(TYPE) / sizeof(char));
    } while(heap.back().move());

    output_file.close();
}



std::string get_chunk_filename(unsigned int chunk_no) {
	std::ostringstream oss;
	oss << "chunk" << chunk_no << ".bin";
	return oss.str();
}

void delete_files(const std::vector<std::string>& filenames) {
	std::for_each(filenames.begin(), filenames.end(), [](std::string filename) {
		if (std::remove(filename.c_str()) != 0) {
			throw std::runtime_error("Cannot remove file: " + filename);
		}
	});
}
