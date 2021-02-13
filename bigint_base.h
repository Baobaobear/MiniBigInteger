// filename:    bigint_base.h
// author:      baobaobear
// create date: 2021-02-13
// This library is compatible with C++11
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace NTT_NS {

const int32_t NTT_N = 1 << 18;
const int32_t NTT_POW = 32;
const int32_t NTT_P = (479 << 21) + 1;
const int32_t NTT_G = 3;

int32_t ntt_wn[2][NTT_POW];
int64_t ntt_a[NTT_N], ntt_b[NTT_N];
size_t ntt_r[NTT_N];

int64_t quick_pow_mod(int64_t a, int64_t b) {
    int64_t ans = 1;
    a %= NTT_P;
    while (b) {
        if (b & 1) {
            ans = ans * a % NTT_P;
            b--;
        }
        b >>= 1;
        a = a * a % NTT_P;
    }
    return ans;
}

void GetWn() {
    if (ntt_wn[1][0] == 0) {
        for (int i = 0; i < NTT_POW; i++) {
            ntt_wn[1][i] = (int32_t)quick_pow_mod(NTT_G, (NTT_P - 1) / ((int64_t)1 << i));
            ntt_wn[0][i] = (int32_t)quick_pow_mod(ntt_wn[1][i], NTT_P - 2);
        }
    }
}

void Prepare(const int32_t A[], size_t size_a, const int32_t B[], size_t size_b, size_t &len) {
    len = 1;
    size_t L1 = size_a, L2 = size_b;
    while (len < L1 + L2)
        len <<= 1;
    for (size_t i = 0; i < len; i++) {
        ntt_a[i] = i < L1 ? A[i] : 0;
        ntt_b[i] = i < L2 ? B[i] : 0;
    }
    for (size_t i = 0; i < len; i++) {
        ntt_r[i] = (ntt_r[i >> 1] >> 1) | ((i & 1) * (len >> 1));
    }
}

void NTT(int64_t a[], size_t len, int on) {
    for (size_t i = 0; i < len; i++) {
        if (i < ntt_r[i])
            std::swap(a[i], a[ntt_r[i]]);
    }
    for (size_t h = 1, id = 1; h < len; h <<= 1, ++id) {
        int32_t wn = ntt_wn[on][id];
        for (size_t j = 0; j < len; j += h << 1) {
            int64_t w = 1;
            for (size_t k = j; k < j + h; k++) {
                int64_t u = a[k], t = w * a[k + h] % NTT_P;
                a[k] = (u + t) % NTT_P;
                a[k + h] = (u - t + NTT_P) % NTT_P;
                w = w * wn % NTT_P;
            }
        }
    }
}

void Conv(size_t n) {
    NTT(ntt_a, n, 1);
    NTT(ntt_b, n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * ntt_b[i] % NTT_P;
    NTT(ntt_a, n, 0);
    int64_t inv = quick_pow_mod(n, NTT_P - 2);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * inv % NTT_P;
}
} // namespace NTT_NS

const int BIGINT_MAXBASE = 1 << 15;

struct BigIntBase {
    int base;
    int digits;
    std::vector<int32_t> v;

    BigIntBase(int b) { // b > 1
        base = b;
        for (digits = 1; base <= BIGINT_MAXBASE; base *= b, ++digits)
            ;
        base /= b;
        --digits;
        set(0);
    }
    BigIntBase &set(intmax_t n) {
        v.resize(1);
        uintmax_t s;
        if (n < 0) {
            s = -n;
        } else {
            s = n;
        }
        for (int i = 0; s; i++) {
            v.resize(i + 1);
            v[i] = s % base;
            s /= base;
        }
        return *this;
    }
    size_t size() const {
        return v.size();
    }
    BigIntBase &raw_add(const BigIntBase &b) {
        if (v.size() < b.size()) {
            v.resize(b.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += add + b.v[i];
            add = v[i] / base;
            v[i] %= base;
        }
        if (add) {
            v.push_back(add);
        }
        return *this;
    }
    BigIntBase &raw_mul_int(uint32_t m) {
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = add + v[i] * m;
            add = v[i] / base;
            v[i] %= base;
        }
        while (add) {
            v.push_back(add);
            add = v.back() / base;
            v.back() %= base;
        }
        return *this;
    }
};
