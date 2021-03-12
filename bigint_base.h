// filename:    bigint_base.h
// author:      baobaobear
// create date: 2021-02-13
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include "bigint_header.h"

namespace NTT_NS
{

const int32_t NTT_POW = 24;
const int32_t NTT_G = 3;
#if BIGINT_X64
typedef int64_t ntt_base_t;
const int32_t NTT_P1 = 469762049;
const int32_t NTT_P1_INV = 130489458;
const int32_t NTT_P2 = 167772161;
const int32_t NTT_P2_INV = 104391568;
#else
typedef int32_t ntt_base_t;
const int32_t NTT_P1 = 111149057;
const int32_t NTT_P1_INV = 34952517;
const int32_t NTT_P2 = 104857601;
const int32_t NTT_P2_INV = 74099389;
#endif

static std::vector<size_t> ntt_ra[NTT_POW];
static size_t *ntt_r;

template <int32_t NTT_MOD> struct NTT {
    ntt_base_t ntt_wn[2][NTT_POW];
    std::vector<ntt_base_t> ntt_a, ntt_b;
    std::vector<int64_t> ntt_c;

#if BIGINT_X64
    inline ntt_base_t mul_mod(int64_t a, int64_t b) {
        return a * b % NTT_MOD;
    }
#else
    inline ntt_base_t mul_mod(int32_t a, int32_t b) {
        //return ntt_base_t((int64_t)a * b % NTT_MOD);
        int32_t c = (int32_t)((double)a * b / NTT_MOD);
        return a * b - c * NTT_MOD;
    }
#endif
    ntt_base_t pow_mod(int64_t a, int64_t b) {
        int64_t ans = 1;
        a %= NTT_MOD;
        while (b) {
            if (b & 1) ans = ans * a % NTT_MOD;
            b >>= 1;
            a = a * a % NTT_MOD;
        }
        return (ntt_base_t)ans;
    }
    NTT() {
        if (ntt_wn[1][0] == 0) {
            for (int i = 0; i < NTT_POW; i++) {
                ntt_wn[1][i] = pow_mod(NTT_G, (NTT_MOD - 1) / ((int64_t)1 << i));
                ntt_wn[0][i] = pow_mod(ntt_wn[1][i], NTT_MOD - 2);
            }
        }
    }
    void transform(ntt_base_t a[], size_t len, int on) {
        for (size_t i = 0; i < len; i++) {
            if (i < ntt_r[i]) std::swap(a[i], a[ntt_r[i]]);
        }
        size_t id = 0;
        for (size_t h = 1; h < len; h <<= 1) {
            ntt_base_t wn = ntt_wn[on][++id];
            for (size_t j = 0; j < len; j += h << 1) {
                ntt_base_t w = 1;
                size_t e = j + h;
                for (size_t k = j; k < e; k++, w = mul_mod(w, wn)) {
                    ntt_base_t t = mul_mod(w, a[k + h]);
                    a[k + h] = (a[k] - t + NTT_MOD) % NTT_MOD;
                    a[k] = (a[k] + t) % NTT_MOD;
                }
            }
        }
        if (on == 0) {
            ntt_base_t inv = pow_mod(len, NTT_MOD - 2);
            for (size_t i = 0; i < len; i++)
                a[i] = mul_mod(a[i], inv);
        }
    }
    void mul_conv(size_t n) {
        transform(&*ntt_a.begin(), n, 1);
        transform(&*ntt_b.begin(), n, 1);
        for (size_t i = 0; i < n; i++)
            ntt_a[i] = mul_mod(ntt_a[i], ntt_b[i]);
        transform(&*ntt_a.begin(), n, 0);
    }
    void sqr_conv(size_t n) {
        transform(&*ntt_a.begin(), n, 1);
        for (size_t i = 0; i < n; i++)
            ntt_a[i] = mul_mod(ntt_a[i], ntt_a[i]);
        transform(&*ntt_a.begin(), n, 0);
    }
};

static NTT<NTT_P1> ntt1;
static NTT<NTT_P2> ntt2;

void ntt_prepare(size_t size_a, size_t size_b, size_t &len, int flag = 1) {
    len = 1;
    size_t L1 = size_a, L2 = size_b;
    while (len < L1 + L2)
        len <<= 1;
    ntt1.ntt_a.resize(len);
    if (flag & 1) ntt1.ntt_b.resize(len);
    if (flag & 2) ntt2.ntt_a = ntt1.ntt_a;
    if (flag & 4) ntt2.ntt_b = ntt1.ntt_b;
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
}

static void double_mod_rev(size_t n) {
    ntt1.ntt_c.resize(n);
    for (size_t i = 0; i < n; i++) {
        // z = x * p1 + c1 = y * p2 + c2 's solution is
        // y = (c1 - c2) * inv(p2) (mod p1)
        int64_t t = (ntt1.ntt_a[i] - ntt2.ntt_a[i]) % NTT_P1 + NTT_P1;
        ntt1.ntt_c[i] = t * NTT_P2_INV % NTT_P1 * NTT_P2 + ntt2.ntt_a[i];
    }
}

void mul_conv2() {
    size_t n = ntt1.ntt_a.size();
    ntt1.mul_conv(n);
    ntt2.mul_conv(n);
    double_mod_rev(n);
}

void sqr_conv2() {
    size_t n = ntt1.ntt_a.size();
    ntt1.sqr_conv(n);
    ntt2.sqr_conv(n);
    double_mod_rev(n);
}
} // namespace NTT_NS

namespace BigIntBaseNS
{
const int32_t BIGINT_MAXBASE = 1 << 15;

const uint32_t BIGINT_NTT_THRESHOLD = 1000;
const uint32_t BIGINT_MUL_THRESHOLD = 70;
const uint32_t NTT_MAX_SIZE = 1 << 21;

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
    template <typename _Tx, typename Ty> inline void carry(_Tx &add, Ty &baseval, _Tx newval) {
        add += newval;
        baseval = add % (_Tx)base;
        add /= (_Tx)base;
    }
    template <typename _Tx, typename Ty> inline void borrow(_Tx &add, Ty &baseval, _Tx newval) {
        add += newval - (_Tx)base + 1;
        baseval = add % (_Tx)base + (_Tx)base - 1;
        add /= (_Tx)base;
    }

    explicit BigIntBase(int b) {
        setbase(b);
    }
    explicit BigIntBase(int b, int d) {
        base = b, digits = d;
    }
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
        if (v.size() < b.size()) v.resize(b.size());
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
        trim();
        return *this;
    }
    BigInt_t &raw_mul_int(uint32_t m) {
        ucarry_t add = 0;
        for (size_t i = 0; i < v.size(); i++)
            carry(add, v[i], (ucarry_t)v[i] * m);
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
    BigInt_t raw_shr_to(size_t n) const {
        BigInt_t r(base, digits);
        if (n >= size()) return r;
        r.v.assign(v.begin() + n, v.end());
        return BIGINT_STD_MOVE(r);
    }
    BigInt_t raw_lowdigits_to(size_t n) const {
        if (n >= size()) return *this;
        BigInt_t r(base, digits);
        r.v.assign(v.begin(), v.begin() + n);
        r.trim();
        return BIGINT_STD_MOVE(r);
    }
    BigInt_t &raw_shl(size_t n) {
        if (n == 0 || (size() == 1 && v[0] == 0)) return *this;
        v.insert(v.begin(), n, 0);
        return *this;
    }
    BigInt_t &raw_mul_karatsuba(const BigInt_t &a, const BigInt_t &b) {
        if (std::min(a.size(), b.size()) <= BIGINT_MUL_THRESHOLD) return raw_mul(a, b);
        if (a.size() * 2 < b.size() || b.size() * 2 < a.size()) { // split
            BigInt_t t(base, digits);
            if (a.size() * 2 < b.size()) {
                size_t split = b.size() / 2;
                t.raw_mul_karatsuba(a, b.raw_shr_to(split));
                t.raw_shl(split);
                raw_mul_karatsuba(a, b.raw_lowdigits_to(split));
                raw_add(t);
            } else {
                size_t split = a.size() / 2;
                t.raw_mul_karatsuba(b, a.raw_shr_to(split));
                t.raw_shl(split);
                raw_mul_karatsuba(b, a.raw_lowdigits_to(split));
                raw_add(t);
            }
            return *this;
        }
        if (std::min(a.size(), b.size()) <= BIGINT_NTT_THRESHOLD)
            ;
        else if ((a.size() + b.size()) <= NTT_MAX_SIZE)
            return raw_nttmul(a, b);
        BigInt_t ah(base, digits), al(base, digits), bh(base, digits), bl(base, digits), h(base, digits), m(base, digits);
        size_t split = std::max(std::min(a.size() / 2, b.size() - 1), std::min(a.size() - 1, b.size() / 2));
        al.v.assign(a.v.begin(), a.v.begin() + split);
        ah.v.assign(a.v.begin() + split, a.v.end());
        bl.v.assign(b.v.begin(), b.v.begin() + split);
        bh.v.assign(b.v.begin() + split, b.v.end());

        raw_mul_karatsuba(al, bl);
        h.raw_mul_karatsuba(ah, bh);
        m.raw_mul_karatsuba(al.raw_add(ah), bl.raw_add(bh));
        m.raw_sub(*this);
        m.raw_sub(h);
        v.resize(a.size() + b.size());

        raw_offset_add(m, split);
        raw_offset_add(h, split * 2);
        trim();
        return *this;
    }
    BigInt_t &raw_nttmul(const BigInt_t &a, const BigInt_t &b) {
        if (std::min(a.size(), b.size()) <= BIGINT_MUL_THRESHOLD) return raw_mul(a, b);
        if (std::min(a.size(), b.size()) <= BIGINT_NTT_THRESHOLD || (a.size() + b.size()) > NTT_MAX_SIZE)
            return raw_mul_karatsuba(a, b);
        size_t len, lenmul = 1;
        std::vector<NTT_NS::ntt_base_t> &ntt_a = NTT_NS::ntt1.ntt_a, &ntt_b = NTT_NS::ntt1.ntt_b;
        std::vector<int64_t> &ntt_c = NTT_NS::ntt1.ntt_c;
        ntt_a.resize(a.size());
        ntt_b.resize(b.size());
        for (size_t i = 0; i < a.size(); ++i)
            ntt_a[i] = a.v[i];
        for (size_t i = 0; i < b.size(); ++i)
            ntt_b[i] = b.v[i];
        NTT_NS::ntt_prepare(a.size(), b.size(), len, 7);
        NTT_NS::mul_conv2();
        len = (a.size() + b.size()) * lenmul;
        while (len > 0 && ntt_c[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
        for (size_t i = 0; i <= len; i++) {
            add += ntt_c[i];
            v.push_back(add % base);
            add /= base;
        }
        for (; add; add /= base)
            v.push_back(add % base);
        trim();
        return *this;
    }
    BigInt_t &raw_nttsqr(const BigInt_t &a) {
        if (a.size() <= BIGINT_MUL_THRESHOLD) return raw_mul(a, a);
        if (a.size() <= BIGINT_NTT_THRESHOLD || (a.size() + a.size()) > NTT_MAX_SIZE) return raw_mul_karatsuba(a, a);
        size_t len, lenmul = 1;
        std::vector<NTT_NS::ntt_base_t> &ntt_a = NTT_NS::ntt1.ntt_a;
        std::vector<int64_t> &ntt_c = NTT_NS::ntt1.ntt_c;
        ntt_a.resize(a.size());
        for (size_t i = 0; i < a.size(); ++i)
            ntt_a[i] = a.v[i];
        NTT_NS::ntt_prepare(a.size() * 2, 0, len, 2);
        NTT_NS::sqr_conv2();
        len = (a.size() + a.size()) * lenmul;
        while (len > 0 && ntt_c[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
        for (size_t i = 0; i <= len; i++) {
            add += ntt_c[i];
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
