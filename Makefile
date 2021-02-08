# Copyright (c) 2021 Baobaobear. All Rights Reserved.

CXX ?= g++
CFLAGS03 ?= -O3 -std=c++03 -Wall -pedantic -Wno-format
CFLAGS11 ?= -O3 -std=c++11 -Wall -pedantic -Wno-format
BENCHMARKFILE ?= test.cpp

default: clean test

test: test0
	./test0

clean:
	rm -f test0

test0: test.cpp bigint.h bigint_dec.h
	$(CXX) $(CFLAGS11) $(BENCHMARKFILE) -o test0
