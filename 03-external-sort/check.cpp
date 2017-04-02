#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <limits>
#include <fstream>
const int BUFFER_SIZE = 65536;

int main()
{
	std::ifstream finalfile;
	finalfile.open("output", std::ios::binary);
	char buffer[BUFFER_SIZE];
	int offset = 0;
	int last_value = std::numeric_limits<int>::min();
	while (finalfile) {
		finalfile.read(buffer + offset, BUFFER_SIZE);
		int i = finalfile.gcount();
		if (i) {
			offset = i % sizeof(int);
			int* tmp = (int*) buffer;
			if (tmp[0] < last_value){
				std::cout << "not sorted " << (int) last_value << ":" << tmp[0]; 
				exit(1);
			}
			for (int j = 1; j < i / sizeof(int); j++) {
				if (tmp[j] < tmp[j - 1]) {
					std::cout << "not sorted" << tmp[j] << ":" <<  tmp[j - 1];
					exit(1);
				}
			}
			last_value = tmp[i / sizeof(int) - 1];
			if (offset > 0) {
				memcpy(buffer, buffer + i - offset - 1, offset);
			}
			
		}
	}
	return 0;
}
