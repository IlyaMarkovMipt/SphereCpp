#include "external_sort.h"
#include <cstdlib>
#include <ctime>
#include <limits>
#include <fstream>
#include <iostream>

int main(){
	//std::string input_filename("input");
	//std::string output_filename("output");
	//unsigned long max = std::numeric_limits<std::uint32_t>::max();
	//unsigned long i = 0;
	//std::ofstream myfile(input_filename, std::ios::binary);
	//srand(time(NULL));
	//std::cout << max << std::endl;
	//std::cout << "File of size " << max * sizeof(int) * 1.0 / (1024 * 1024) 
	//			<< "Mb is being generated" << std::endl;
	//int buf_size = 2048;
	//int* buffer = new int[buf_size];
	//
	//for (;i < max; i += buf_size) {
	//	for (int j = 0; j < buf_size; j++)
	//		 buffer[j] = rand(); 
	//	myfile.write((char*) buffer, buf_size * sizeof(int));
	//	if (i % 1073741824 == 0)
	//		std::cout << i << std::endl;
	//}
	//std::cout << "File of size " << max * sizeof(int) 
	//				<< " generated" << std::endl;
	//myfile.flush()
	
	//delete []buffer;
	sort("input", "output");
	return 0;
}
