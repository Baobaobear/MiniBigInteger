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

#if !defined(NTT_MODE) || NTT_MODE == 0
#define NTT_DOUBLE_MOD
#endif

namespace NTT_NS {

const int32_t NTT_POW = 32;
const int32_t NTT_G = 3;
const int32_t NTT_P = (479 << 21) + 1;
const int32_t NTT_P2 = 998244353;
const int64_t NTT_LCM = NTT_P * (int64_t)NTT_P2;
const int64_t MOD_M1 = 668514958533372747;
const int64_t MOD_M2 = 334257240187163831;

int32_t ntt_wn[2][NTT_POW];
int32_t ntt_wn2[2][NTT_POW];
std::vector<int64_t> ntt_a, ntt_b, ntt_c, ntt_d;
std::vector<size_t> ntt_ra[32];
size_t *ntt_r;

int64_t quick_mul_mod(int64_t a, int64_t b) {
    a %= NTT_LCM;
    b %= NTT_LCM;
    int64_t c = (int64_t)((long double)a * b / NTT_LCM);
    int64_t ans = a * b - c * NTT_LCM;
    if (ans < 0)
        ans += NTT_LCM;
    else if (ans >= NTT_LCM)
        ans -= NTT_LCM;
    return ans;
}

int64_t quick_pow_mod(int64_t a, int64_t b) {
    int64_t ans = 1;
    a %= NTT_P;
    while (b) {
        if (b & 1) {
            ans = ans * a % NTT_P;
        }
        b >>= 1;
        a = a * a % NTT_P;
    }
    return ans;
}

int64_t quick_pow_mod2(int64_t a, int64_t b) {
    int64_t ans = 1;
    a %= NTT_P2;
    while (b) {
        if (b & 1) {
            ans = ans * a % NTT_P2;
        }
        b >>= 1;
        a = a * a % NTT_P2;
    }
    return ans;
}

void GetWn() {
    if (ntt_wn[1][0] == 0) {
        for (int i = 0; i < NTT_POW; i++) {
            ntt_wn[1][i] = (int32_t)quick_pow_mod(NTT_G, (NTT_P - 1) / ((int64_t)1 << i));
            ntt_wn[0][i] = (int32_t)quick_pow_mod(ntt_wn[1][i], NTT_P - 2);
#ifdef NTT_DOUBLE_MOD
            ntt_wn2[1][i] = (int32_t)quick_pow_mod2(NTT_G, (NTT_P2 - 1) / ((int64_t)1 << i));
            ntt_wn2[0][i] = (int32_t)quick_pow_mod2(ntt_wn2[1][i], NTT_P2 - 2);
#endif
        }
    }
}

void Prepare(size_t size_a, size_t size_b, size_t &len) {
    len = 1;
    size_t L1 = size_a, L2 = size_b;
    while (len < L1 + L2)
        len <<= 1;
    ntt_a.resize(len);
    ntt_b.resize(len);
#ifdef NTT_DOUBLE_MOD
    ntt_c = ntt_a;
    ntt_d = ntt_b;
#endif
    int32_t id = 0;
    while ((1ULL << id) < len)
        ++id;
    if (ntt_ra[id].empty()) {
        std::vector<size_t> &r = ntt_ra[id];
        r.resize(len);
        for (size_t i = 0; i < len; i++)
            r[i] = (r[i >> 1] >> 1) | ((i & 1) * (len >> 1));
    }
    ntt_r = &*ntt_ra[id].begin();
}

void NTT(int64_t a[], size_t len, int on) {
    for (size_t i = 0; i < len; i++) {
        if (i < ntt_r[i])
            std::swap(a[i], a[ntt_r[i]]);
    }
    size_t id = 0;
    for (size_t h = 1; h < len; h <<= 1) {
        int32_t wn = ntt_wn[on][++id];
        for (size_t j = 0; j < len; j += h << 1) {
            int64_t w = 1;
            size_t e = j + h;
            for (size_t k = j; k < e; k++, w = w * wn % NTT_P) {
                int32_t t = (int32_t)(w * a[k + h] % NTT_P);
                a[k + h] = ((int32_t)a[k] - t + NTT_P) % NTT_P;
                a[k] = (int32_t)(a[k] + t) % NTT_P;
            }
        }
    }
}

void NTT2(int64_t a[], size_t len, int on) {
    for (size_t i = 0; i < len; i++) {
        if (i < ntt_r[i])
            std::swap(a[i], a[ntt_r[i]]);
    }
    size_t id = 0;
    for (size_t h = 1; h < len; h <<= 1) {
        int32_t wn = ntt_wn2[on][++id];
        for (size_t j = 0; j < len; j += h << 1) {
            int64_t w = 1;
            size_t e = j + h;
            for (size_t k = j; k < e; k++, w = w * wn % NTT_P2) {
                int32_t t = (int32_t)(w * a[k + h] % NTT_P2);
                a[k + h] = ((int32_t)a[k] - t + NTT_P2) % NTT_P2;
                a[k] = (int32_t)(a[k] + t) % NTT_P2;
            }
        }
    }
}

void Conv(size_t n) {
    NTT(&*ntt_a.begin(), n, 1);
    NTT(&*ntt_b.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * ntt_b[i] % NTT_P;
    NTT(&*ntt_a.begin(), n, 0);

#ifdef NTT_DOUBLE_MOD
    NTT2(&*ntt_c.begin(), n, 1);
    NTT2(&*ntt_d.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_c[i] = ntt_c[i] * ntt_d[i] % NTT_P2;
    NTT2(&*ntt_c.begin(), n, 0);
#endif
    int64_t inv = quick_pow_mod(n, NTT_P - 2);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * inv % NTT_P;
#ifdef NTT_DOUBLE_MOD
    inv = quick_pow_mod2(n, NTT_P2 - 2);
    for (size_t i = 0; i < n; i++)
        ntt_c[i] = ntt_c[i] * inv % NTT_P2;
    for (size_t i = 0; i < n; i++) {
        if (ntt_c[i] != ntt_a[i]) {
            ntt_a[i] = (quick_mul_mod(ntt_a[i], MOD_M1) + quick_mul_mod(ntt_c[i], MOD_M2)) % NTT_LCM;
        }
    }
#endif
}
} // namespace NTT_NS

const int32_t BIGINT_MAXBASE = 1 << 15;

struct BigIntBase {
    int32_t base;
    int32_t digits;
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
