// filename:    bigint_base.h
// author:      baobaobear
// create date: 2021-02-13
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include "bigint_header.h"

#if !defined(NTT_MODE) || NTT_MODE == 0
#define NTT_DOUBLE_MOD
#endif

namespace NTT_NS {

const int32_t NTT_POW = 32;
const int32_t NTT_G = 3;
const int32_t NTT_P = (479 << 21) + 1;
const int32_t NTT_P_INV = 332747959;
const int32_t NTT_P2 = 998244353;
const int32_t NTT_P2_INV = 669690699;

int32_t ntt_wn[2][NTT_POW];
int32_t ntt_wn2[2][NTT_POW];
std::vector<int64_t> ntt_a, ntt_b, ntt_c, ntt_d;
std::vector<size_t> ntt_ra[32];
size_t *ntt_r;

int64_t quick_pow_mod(int64_t a, int64_t b) {
    int64_t ans = 1;
    a %= NTT_P;
    while (b) {
        if (b & 1)
            ans = ans * a % NTT_P;
        b >>= 1;
        a = a * a % NTT_P;
    }
    return ans;
}

int64_t quick_pow_mod2(int64_t a, int64_t b) {
    int64_t ans = 1;
    a %= NTT_P2;
    while (b) {
        if (b & 1)
            ans = ans * a % NTT_P2;
        b >>= 1;
        a = a * a % NTT_P2;
    }
    return ans;
}

void get_wn() {
    if (ntt_wn[1][0] == 0) {
        for (int i = 0; i < NTT_POW; i++) {
            ntt_wn[1][i] = (int32_t)quick_pow_mod(NTT_G, (NTT_P - 1) / ((int64_t)1 << i));
            ntt_wn[0][i] = (int32_t)quick_pow_mod(ntt_wn[1][i], NTT_P - 2);
            ntt_wn2[1][i] = (int32_t)quick_pow_mod2(NTT_G, (NTT_P2 - 1) / ((int64_t)1 << i));
            ntt_wn2[0][i] = (int32_t)quick_pow_mod2(ntt_wn2[1][i], NTT_P2 - 2);
        }
    }
}

void ntt_prepare(size_t size_a, size_t size_b, size_t &len, int flag = 1) {
    len = 1;
    size_t L1 = size_a, L2 = size_b;
    while (len < L1 + L2)
        len <<= 1;
    ntt_a.resize(len);
    if (flag & 1)
        ntt_b.resize(len);
    if (flag & 2)
        ntt_c = ntt_a;
    if (flag & 4)
        ntt_d = ntt_b;
    int32_t id = 0;
    while (((uint64_t)1 << id) < len)
        ++id;
    if (ntt_ra[id].empty()) {
        std::vector<size_t> &r = ntt_ra[id];
        r.resize(len);
        for (size_t i = 0; i < len; i++)
            r[i] = (r[i >> 1] >> 1) | ((i & 1) * (len >> 1));
    }
    ntt_r = &*ntt_ra[id].begin();
    get_wn();
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
                a[k] = ((int32_t)a[k] + t) % NTT_P;
            }
        }
    }
    if (on == 0) {
        int64_t inv = quick_pow_mod(len, NTT_P - 2);
        for (size_t i = 0; i < len; i++)
            a[i] = a[i] * inv % NTT_P;
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
                a[k] = ((int32_t)a[k] + t) % NTT_P2;
            }
        }
    }
    if (on == 0) {
        int64_t inv = quick_pow_mod2(len, NTT_P2 - 2);
        for (size_t i = 0; i < len; i++)
            a[i] = a[i] * inv % NTT_P2;
    }
}

void mul_conv(size_t n) {
    NTT(&*ntt_a.begin(), n, 1);
    NTT(&*ntt_b.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * ntt_b[i] % NTT_P;
    NTT(&*ntt_a.begin(), n, 0);
}

void double_mod_rev(size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (ntt_c[i] != ntt_a[i]) {
            // s * p1 + a1 = val = t * p2 + a2 's solution is t = (a1 - a2) / p2 (mod p1)
            int64_t t = (ntt_a[i] - ntt_c[i]) % NTT_P + NTT_P;
            ntt_a[i] = t * NTT_P2_INV % NTT_P * NTT_P2 + ntt_c[i];
        }
    }
}

void mul_conv2(size_t n) {
    NTT(&*ntt_a.begin(), n, 1);
    NTT(&*ntt_b.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * ntt_b[i] % NTT_P;
    NTT(&*ntt_a.begin(), n, 0);

    NTT2(&*ntt_c.begin(), n, 1);
    NTT2(&*ntt_d.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_c[i] = ntt_c[i] * ntt_d[i] % NTT_P2;
    NTT2(&*ntt_c.begin(), n, 0);
    double_mod_rev(n);
}

void sqr_conv2(size_t n) {
    NTT(&*ntt_a.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_a[i] = ntt_a[i] * ntt_a[i] % NTT_P;
    NTT(&*ntt_a.begin(), n, 0);

    NTT2(&*ntt_c.begin(), n, 1);
    for (size_t i = 0; i < n; i++)
        ntt_c[i] = ntt_c[i] * ntt_c[i] % NTT_P2;
    NTT2(&*ntt_c.begin(), n, 0);
    double_mod_rev(n);
}
} // namespace NTT_NS

namespace BigIntBaseNS {
const int32_t BIGINT_MAXBASE = 1 << 15;

const uint32_t BIGINT_NTT_THRESHOLD = 1024;
const uint32_t BIGINT_MUL_THRESHOLD = 400;
const uint32_t NTT_MAX_SIZE = 1 << 24;

struct BigIntBase {
    typedef uint32_t base_t;
#if BIGINT_LARGE_BASE
    typedef int64_t carry_t;
    typedef uint64_t ucarry_t;
#else
    typedef int32_t carry_t;
    typedef uint32_t ucarry_t;
#endif
    base_t base;
    int32_t digits;
    std::vector<base_t> v;
    typedef BigIntBase BigInt_t;
    template<typename _Tx, typename Ty>
    inline void carry(_Tx& add, Ty& baseval, _Tx newval) {
        add += newval;
        baseval = add % (_Tx)base;
        add /= base;
    }
    template<typename _Tx, typename Ty>
    inline void borrow(_Tx& add, Ty& baseval, _Tx newval) {
        add += newval;
        if (add >= 0) {
            baseval = add % (_Tx)base;
            add /= (_Tx)base;
        } else {
            baseval = ++add % (_Tx)base + (_Tx)base - 1;
            add = add / (_Tx)base - 1;
        }
    }

    BigIntBase() {}
    BigIntBase(int b) { setbase(b); }
    void setbase(int b) { // b > 1
        base = b;
        for (digits = 1; base <= BIGINT_MAXBASE; base *= b, ++digits)
            ;
        base /= b;
        --digits;
        set(0);
    }
    BigIntBase &set(intmax_t n) {
        v.resize(1);
        v[0] = 0;
        uintmax_t s;
        if (n < 0)
            s = -n;
        else
            s = n;
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
    BigInt_t &raw_add(const BigInt_t &b) {
        if (v.size() < b.size())
            v.resize(b.size());
        ucarry_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++)
            carry(add, v[i], (ucarry_t)(v[i] + b.v[i]));
        for (size_t i = b.v.size(); add && i < v.size(); i++)
            carry(add, v[i], (ucarry_t)v[i]);
        add ? v.push_back((base_t)add) : trim();
        return *this;
    }
    BigInt_t &raw_offset_add(const BigInt_t &b, size_t offset) {
        ucarry_t add = 0;
        for (size_t i = 0; i < b.size(); ++i)
            carry(add, v[i + offset], (ucarry_t)(v[i + offset] + b.v[i]));
        for (size_t i = b.size() + offset; add; ++i)
            carry(add, v[i], (ucarry_t)v[i]);
        return *this;
    }
    BigInt_t &raw_sub(const BigInt_t &b) {
        if (v.size() < b.v.size()) {
            v.resize(b.v.size());
        }
        carry_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++)
            borrow(add, v[i], (carry_t)v[i] - (carry_t)b.v[i]);
        for (size_t i = b.v.size(); add && i < v.size(); i++)
            borrow(add, v[i], (carry_t)v[i]);
        if (add) {
            add = 1;
            for (size_t i = 0; i < v.size(); i++)
                carry(add, v[i], (carry_t)(base - v[i] - 1));
        }
        trim();
        return *this;
    }
    BigInt_t &raw_mul_int(uint32_t m) {
        ucarry_t add = 0;
        for (size_t i = 0; i < v.size(); i++)
            carry(add, v[i], v[i] * (ucarry_t)m);
        while (add) {
            v.push_back((base_t)(add % base));
            add /= base;
        }
        return *this;
    }
    BigInt_t &raw_mul(const BigInt_t &a, const BigInt_t &b) {
        v.clear();
        v.resize(a.size() + b.size());
        for (size_t i = 0; i < a.size(); i++) {
            ucarry_t add = 0, av = a.v[i];
            for (size_t j = 0; j < b.size(); j++)
                carry(add, v[i + j], v[i + j] + av * b.v[j]);
            v[i + b.size()] += (base_t)add;
        }
        trim();
        return *this;
    }
    BigInt_t &raw_fastmul(const BigInt_t &a, const BigInt_t &b) {
        if (std::min(a.size(), b.size()) <= BIGINT_MUL_THRESHOLD)
            return raw_mul(a, b);
        if (std::min(a.size(), b.size()) <= BIGINT_NTT_THRESHOLD)
            ;
        else if ((a.size() + b.size()) <= NTT_MAX_SIZE)
            return raw_nttmul(a, b);
        BigInt_t ah(base), al(base), bh(base), bl(base), h(base), m(base);
        size_t split = std::max(std::min(a.size() / 2, b.size() - 1), std::min(a.size() - 1, b.size() / 2));
        al.v.assign(a.v.begin(), a.v.begin() + split);
        ah.v.assign(a.v.begin() + split, a.v.end());
        bl.v.assign(b.v.begin(), b.v.begin() + split);
        bh.v.assign(b.v.begin() + split, b.v.end());

        raw_fastmul(al, bl);
        h.raw_fastmul(ah, bh);
        m.raw_fastmul(al.raw_add(ah), bl.raw_add(bh));
        m.raw_sub(*this);
        m.raw_sub(h);
        v.resize(a.size() + b.size());

        raw_offset_add(m, split);
        raw_offset_add(h, split * 2);
        trim();
        return *this;
    }
    BigInt_t &raw_nttmul(const BigInt_t &a, const BigInt_t &b) {
        if (std::min(a.size(), b.size()) <= BIGINT_MUL_THRESHOLD)
            return raw_mul(a, b);
        if (std::min(a.size(), b.size()) <= BIGINT_NTT_THRESHOLD || (a.size() + b.size()) > NTT_MAX_SIZE)
            return raw_fastmul(a, b);
        size_t len, lenmul = 1;
        NTT_NS::get_wn();
        NTT_NS::ntt_a.clear();
        NTT_NS::ntt_b.clear();
        for (size_t i = 0; i < a.size(); ++i)
            NTT_NS::ntt_a.push_back(a.v[i]);
        for (size_t i = 0; i < b.size(); ++i)
            NTT_NS::ntt_b.push_back(b.v[i]);
        NTT_NS::ntt_prepare(a.size(), b.size(), len, 7);
        NTT_NS::mul_conv2(len);
        len = (a.size() + b.size()) * lenmul;
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
        for (size_t i = 0; i <= len; i++) {
            add += NTT_NS::ntt_a[i];
            v.push_back(add % base);
            add /= base;
        }
        for (; add; add /= base)
            v.push_back(add % base);
        trim();
        return *this;
    }
    BigInt_t &raw_nttsqr(const BigInt_t &a) {
        if (a.size() <= BIGINT_MUL_THRESHOLD)
            return raw_mul(a, a);
        if (a.size() <= BIGINT_NTT_THRESHOLD || (a.size() + a.size()) > NTT_MAX_SIZE)
            return raw_fastmul(a, a);
        size_t len, lenmul = 1;
        NTT_NS::get_wn();
        NTT_NS::ntt_a.clear();
        NTT_NS::ntt_b.clear();
        for (size_t i = 0; i < a.size(); ++i)
            NTT_NS::ntt_a.push_back(a.v[i]);
        NTT_NS::ntt_prepare(a.size() * 2, 0, len, 2);
        NTT_NS::sqr_conv2(len);
        len = (a.size() + a.size()) * lenmul;
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
        for (size_t i = 0; i <= len; i++) {
            add += NTT_NS::ntt_a[i];
            v.push_back(add % base);
            add /= base;
        }
        for (; add; add /= base)
            v.push_back(add % base);
        trim();
        return *this;
    }
    void trim() {
        while (v.back() == 0 && v.size() > 1)
            v.pop_back();
    }
};
} // namespace BigIntBaseNS

using BigIntBaseNS::BigIntBase;
