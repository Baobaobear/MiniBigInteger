// filename:    bigint_header.h
// author:      baobaobear
// create date: 2021-02-19
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

//{hex_b}{hexm_b}{dec_b}{decm_b}{mini_b}
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
//{hex_e}{hexm_e}{dec_e}{decm_e}{mini_e}

#if __cplusplus >= 201103L || _MSC_VER >= 1600
//{hex_b}{hexm_b}{dec_b}{decm_b}{mini_b}
#include <cstdint>
#include <utility>
#define BIGINT_STD_MOVE std::move
//{hex_e}{hexm_e}{dec_e}{decm_e}{mini_e}
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

//{hex_b}{hexm_b}{dec_b}{decm_b}{mini_b}
#if defined(_WIN64) || defined(_M_X64)
#define BIGINT_X64 1
#else
#define BIGINT_X64 0
#endif
#if !defined(BIGINT_LARGE_BASE) && BIGINT_X64
#define BIGINT_LARGE_BASE 1 // only work with BigIntBase & BigIntDec
#endif

#define LESS_THAN_AND_EQUAL_COMPARABLE(T)                      \
    bool operator>(const T &b) const { return b < *this; }     \
    bool operator<=(const T &b) const { return !(b < *this); } \
    bool operator>=(const T &b) const { return !(*this < b); } \
    bool operator!=(const T &b) const { return !(*this == b); }
