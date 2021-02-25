// filename:    bigint_header.h
// author:      baobaobear
// create date: 2021-02-19
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#if __cplusplus >= 201103L || _MSC_VER >= 1600
#include <cstdint>
#include <utility>
#define BIGINT_STD_MOVE std::move
#else
#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
typedef unsigned long long uint64_t;
#if !defined(__linux__)
typedef long long int64_t;
#endif
#endif
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef short int16_t;
#if !defined(__APPLE__)
typedef uint64_t uintmax_t;
typedef int64_t intmax_t;
#endif
#define BIGINT_STD_MOVE
#endif

#if !defined(BIGINT_LARGE_BASE) && (defined(_WIN64) || defined(_M_X64))
#define BIGINT_LARGE_BASE 1
#endif
#if !BIGINT_LARGE_BASE && (defined(_WIN64) || defined(_M_X64)) //&& defined(_MSC_VER)
#define BIGINT_INT64_OPTIMIZE 1
#endif
