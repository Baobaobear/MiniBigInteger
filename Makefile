# Copyright (c) 2021 Baobaobear. All Rights Reserved.

CXX ?= g++
CFLAGS03 ?= -O3 -std=c++03 -Wall -pedantic -Wno-format
CFLAGS11 ?= -O3 -std=c++11 -Wall -pedantic -Wno-format
BENCHMARKFILE ?= test.cpp

default: clean test

test: test0 test1
	./test0
	./test1

clean:
	rm -f test0 test1

test0: test.cpp bigint_hex.h bigint_dec.h bigint_mini.h bigint_tiny.h
	$(CXX) $(CFLAGS03) $(BENCHMARKFILE) -o test0

test1: test03.cpp bigint_tiny.h
	$(CXX) $(CFLAGS03) test03.cpp -o test1
