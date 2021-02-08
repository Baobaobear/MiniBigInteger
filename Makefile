# Copyright (c) 2021 Baobaobear. All Rights Reserved.

CXX ?= g++
CFLAGS03 ?= -O3 -std=c++03 -Wall -pedantic -Wno-format
CFLAGS03O1 ?= -O1 -std=c++03 -Wall -pedantic -Wno-format
BENCHMARKFILE ?= test.cpp

default: clean test

test: test0
	./test0

clean:
	rm -f test0

test0: test.cpp bigint.h
	$(CXX) $(CFLAGS03) $(BENCHMARKFILE) -o test0
