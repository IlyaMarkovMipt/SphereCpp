.PHONY: all

CFLAGS := -Wall -Wpedantic -Wextra -Wno-unused-parameter -O3 -std=c++11

COMP := g++

SRC_SORT := ./main.cpp ./external_sort.cpp

CHECK := ./check.cpp

all: 
	$(COMP) $(SRC_SORT) $(CFLAGS) -o sort
	$(COMP) $(CHECK) $(CFLAGS) -o check

