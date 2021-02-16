// filename:    bigint.h
// author:      baobaobear
// create date: 2021-02-08
// This library is compatible with C++11
// https://github.com/Baobaobear/MiniBigInteger
#pragma once
#include "bigint_base.h"

namespace BigIntHexNS {
const int32_t COMPRESS_BIT = 15;
const int32_t COMPRESS_MOD = 1 << COMPRESS_BIT;
const int32_t COMPRESS_MASK = COMPRESS_MOD - 1;

const int32_t BIGINT_NTT_THRESHOLD = 256;
const int32_t BIGINT_MUL_THRESHOLD = 48;
const int32_t BIGINT_DIV_THRESHOLD = 2000;
const int32_t BIGINT_OUTPUT_THRESHOLD = 32;

#ifdef NTT_DOUBLE_MOD
const int32_t NTT_MAX_SIZE = 1 << 24;
#else
const int32_t NTT_MAX_SIZE = 1 << 21;
const int32_t NTT_MID_SIZE = 1 << 18;
#endif

class BigIntHex {
protected:
    int sign;
    std::vector<int32_t> v;
    typedef BigIntHex BigInt_t;

    bool raw_less(const BigInt_t &b) const {
        if (v.size() != b.size()) {
            return v.size() < b.size();
        }
        for (size_t i = v.size() - 1; i < v.size(); i--) {
            if (v[i] != b.v[i]) {
                return v[i] < b.v[i];
            }
        }
        return false; //eq
    }
    bool raw_eq(const BigInt_t &b) const {
        if (v.size() != b.size()) {
            return false;
        }
        for (size_t i = v.size() - 1; i < v.size(); i--) {
            if (v[i] != b.v[i]) {
                return false;
            }
        }
        return true;
    }

    BigInt_t &raw_add(const BigInt_t &b) {
        if (v.size() < b.size()) {
            v.resize(b.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add + b.v[i];
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        if (add) {
            v.push_back(add);
        } else {
            trim();
        }
        return *this;
    }
    BigInt_t &raw_offset_add(const BigInt_t &b, size_t offset) {
        int32_t add = 0;
        for (size_t i = 0; i < b.size(); ++i) {
            v[i + offset] += add + b.v[i];
            add = v[i + offset] >> COMPRESS_BIT;
            v[i + offset] &= COMPRESS_MASK;
        }
        for (size_t i = b.size() + offset; add; ++i) {
            v[i] += add;
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        return *this;
    }
    BigInt_t &raw_sub(const BigInt_t &b) {
        if (v.size() < b.v.size()) {
            v.resize(b.v.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add - b.v[i];
            add = (v[i] >> COMPRESS_BIT);
            v[i] &= COMPRESS_MASK;
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = (v[i] >> COMPRESS_BIT);
            v[i] &= COMPRESS_MASK;
        }
        if (add) {
            sign = -sign;
            v[0] = COMPRESS_MOD - v[0];
            for (size_t i = 1; i < v.size(); i++) {
                v[i] = v[i] ^ COMPRESS_MASK;
            }
        }
        trim();
        return *this;
    }
    BigInt_t &raw_mul_int(uint32_t m) {
        if (m == 0) {
            set(0);
            return *this;
        }
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = add + v[i] * m;
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        while (add) {
            v.push_back(add);
            add = v.back() >> COMPRESS_BIT;
            v.back() &= COMPRESS_MASK;
        }
        return *this;
    }
    BigInt_t &raw_mul(const BigInt_t &a, const BigInt_t &b) {
        v.clear();
        v.resize(a.size() + b.size());
        for (size_t i = 0; i < a.size(); i++) {
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                v[i + j] += add + a.v[i] * b.v[j];
                add = v[i + j] >> COMPRESS_BIT;
                v[i + j] &= COMPRESS_MASK;
            }
            v[i + b.size()] += add;
        }
        trim();
        return *this;
    }
    // Karatsuba algorithm
    BigInt_t &raw_fastmul(const BigInt_t &a, const BigInt_t &b) {
        if (a.size() <= 1 || b.size() <= 1) {
            if (a.size() >= b.size()) {
                *this = a;
                return raw_mul_int(b.v[0]);
            } else {
                *this = b;
                return raw_mul_int(a.v[0]);
            }
        }
        if (a.size() <= BIGINT_MUL_THRESHOLD || b.size() <= BIGINT_MUL_THRESHOLD) {
            return raw_mul(a, b);
        }
        if (a.size() <= BIGINT_NTT_THRESHOLD && b.size() <= BIGINT_NTT_THRESHOLD)
            ;
        else if ((a.size() + b.size()) <= NTT_MAX_SIZE)
            return raw_nttmul(a, b);
        BigInt_t ah, al, bh, bl, h, m;
        size_t split = std::max(std::min(a.size() / 2, b.size() - 1), std::min(a.size() - 1, b.size() / 2)), split2 = split * 2;
        al.v.resize(split);
        std::copy_n(a.v.begin(), al.v.size(), al.v.begin());
        ah.v.resize(a.size() - split);
        std::copy_n(a.v.begin() + split, ah.v.size(), ah.v.begin());
        bl.v.resize(split);
        std::copy_n(b.v.begin(), bl.v.size(), bl.v.begin());
        bh.v.resize(b.size() - split);
        std::copy_n(b.v.begin() + split, bh.v.size(), bh.v.begin());

        raw_fastmul(al, bl);
        h.raw_fastmul(ah, bh);
        m.raw_fastmul(al + ah, bl + bh);
        m.raw_sub(*this);
        m.raw_sub(h);
        v.resize(a.size() + b.size());

        raw_offset_add(m, split);
        raw_offset_add(h, split2);
        trim();
        return *this;
    }
    BigInt_t &raw_nttmul(const BigInt_t &a, const BigInt_t &b) {
        if (a.size() <= BIGINT_MUL_THRESHOLD || b.size() <= BIGINT_MUL_THRESHOLD) {
            return raw_mul(a, b);
        }
        if ((a.size() <= BIGINT_NTT_THRESHOLD && b.size() <= BIGINT_NTT_THRESHOLD) || (a.size() + b.size()) > NTT_MAX_SIZE) {
            return raw_fastmul(a, b);
        }
        size_t len, lenmul = 1;
        NTT_NS::GetWn();
        NTT_NS::ntt_a.clear();
        NTT_NS::ntt_b.clear();
#ifdef NTT_DOUBLE_MOD
        for (size_t i = 0; i < a.size(); ++i) {
            NTT_NS::ntt_a.push_back(a.v[i]);
        }
        for (size_t i = 0; i < b.size(); ++i) {
            NTT_NS::ntt_b.push_back(b.v[i]);
        }
        NTT_NS::Prepare(a.size(), b.size(), len);
#else
        if (a.size() + b.size() <= NTT_MID_SIZE) {
            for (size_t i = 0; i < a.size(); ++i) {
                NTT_NS::ntt_a.push_back(a.v[i] & 0x1f);
                NTT_NS::ntt_a.push_back((a.v[i] >> 5) & 0x1f);
                NTT_NS::ntt_a.push_back((a.v[i] >> 10) & 0x1f);
            }
            for (size_t i = 0; i < b.size(); ++i) {
                NTT_NS::ntt_b.push_back(b.v[i] & 0x1f);
                NTT_NS::ntt_b.push_back((b.v[i] >> 5) & 0x1f);
                NTT_NS::ntt_b.push_back((b.v[i] >> 10) & 0x1f);
            }
            NTT_NS::Prepare(a.size() * 3, b.size() * 3, len);
            lenmul = 3;
        } else {
            for (size_t i = 0; i < a.size(); ++i) {
                NTT_NS::ntt_a.push_back(a.v[i] & 7);
                NTT_NS::ntt_a.push_back((a.v[i] >> 3) & 7);
                NTT_NS::ntt_a.push_back((a.v[i] >> 6) & 7);
                NTT_NS::ntt_a.push_back((a.v[i] >> 9) & 7);
                NTT_NS::ntt_a.push_back((a.v[i] >> 12) & 7);
            }
            for (size_t i = 0; i < b.size(); ++i) {
                NTT_NS::ntt_b.push_back(b.v[i] & 7);
                NTT_NS::ntt_b.push_back((b.v[i] >> 3) & 7);
                NTT_NS::ntt_b.push_back((b.v[i] >> 6) & 7);
                NTT_NS::ntt_b.push_back((b.v[i] >> 9) & 7);
                NTT_NS::ntt_b.push_back((b.v[i] >> 12) & 7);
            }
            lenmul = 5;
            NTT_NS::Prepare(a.size() * 5, b.size() * 5, len);
        }
#endif
        NTT_NS::Conv(len);
        len = (a.size() + b.size()) * lenmul;
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
#ifdef NTT_DOUBLE_MOD
        for (size_t i = 0; i <= len; i++) {
            int64_t s = add + NTT_NS::ntt_a[i];
            v.push_back(s & COMPRESS_MASK);
            add = s >> COMPRESS_BIT;
        }
#else
        if (a.size() + b.size() <= NTT_MID_SIZE) {
            for (size_t i = 0; i <= len; i += 3) {
                int64_t s = add + NTT_NS::ntt_a[i] + (NTT_NS::ntt_a[i + 1] << 5) + (NTT_NS::ntt_a[i + 2] << 10);
                v.push_back(s & COMPRESS_MASK);
                add = s >> COMPRESS_BIT;
            }
        } else {
            for (size_t i = 0; i <= len; i += 5) {
                int64_t s = add + NTT_NS::ntt_a[i] + (NTT_NS::ntt_a[i + 1] << 3) + (NTT_NS::ntt_a[i + 2] << 6) + (NTT_NS::ntt_a[i + 3] << 9) + (NTT_NS::ntt_a[i + 4] << 12);
                v.push_back(s & COMPRESS_MASK);
                add = s >> COMPRESS_BIT;
            }
        }
#endif
        for (; add; add >>= COMPRESS_BIT)
            v.push_back(add & COMPRESS_MASK);
        trim();
        return *this;
    }
    BigInt_t &raw_div(const BigInt_t &a, const BigInt_t &b) {
        if (a.raw_less(b)) {
            set(0);
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigInt_t r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when 3 * COMPRESS_BIT < 52
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_MOD / COMPRESS_MOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD;
        }
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) << COMPRESS_BIT) + r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm / db);
            if (m >= COMPRESS_MOD * 2) {
                ++i;
                m >>= COMPRESS_BIT;
                v[i] += m;
            }
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                r.v[i + j] += add - b.v[j] * m;
                add = r.v[i + j] >> COMPRESS_BIT;
                r.v[i + j] &= COMPRESS_MASK;
            }
            for (size_t j = i + b.size(); add && j < r.size(); ++j) {
                r.v[j] += add;
                add = r.v[j] >> COMPRESS_BIT;
                r.v[j] &= COMPRESS_MASK;
            }
        }
        r.trim();
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }

        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += add;
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        trim();
        return *this;
    }
    BigInt_t &raw_shr(size_t n) {
        if (n == 0)
            return *this;
        size_t t = 0, s = n;
        for (; s < v.size(); ++t, ++s)
            v[t] = v[s];
        v.resize(t);
        return *this;
    }
    BigInt_t &keep(size_t n) {
        size_t s = n < v.size() ? v.size() - n : (size_t)0;
        return raw_shr(s);
    }
    BigInt_t &raw_fastdiv(const BigInt_t &a, const BigInt_t &b) {
        if (a.raw_less(b)) {
            set(0);
            return *this;
        } else if (b.size() < BIGINT_DIV_THRESHOLD) {
            return raw_div(a, b);
        }
        if (b.size() * 2 - 2 > a.size()) {
            BigInt_t ta = a, tb = b;
            size_t ans_len = a.size() - b.size() + 2;
            size_t r = b.size() - ans_len;
            ta.raw_shr(r);
            tb.raw_shr(r);
            return raw_fastdiv(ta, tb);
        }
        size_t ans_len = a.size() - b.size() + 2;
        BigInt_t b2, b2r, x0, x1, t;
        b2.v.resize(ans_len);
        b2.v.push_back(2);
        x1.v.resize(1);
        x1.v.back() = (int32_t)(COMPRESS_MOD / (b.v.back() + (b.v[b.size() - 2] + 1.0) / COMPRESS_MOD));
        size_t keep_size = 1;
        while (x0.v[0] != x1.v[0] || x1 != x0) {
            // x1 = x0(2 - x0 * b)
            x0 = x1;
            b2r = b2;
            t = x0 * b;
            t.keep(keep_size);
            size_t offset = t.size();
            if (t.v.back() != 1)
                offset++;
            x1 *= b2r.keep(offset) - t;
            keep_size = std::min(keep_size * 2, ans_len);
            x1.keep(keep_size);
        }
        x0 *= a;
        if (x0.v[a.size()] >= COMPRESS_MOD >> 1)
            *this = x0.raw_shr(a.size() + 1).raw_add(BigInt_t(1));
        else
            *this = x0.raw_shr(a.size() + 1);
        return *this;
    }
    BigInt_t &raw_mod(const BigInt_t &a, const BigInt_t &b) {
        if (a.size() < b.size()) {
            *this = a;
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigInt_t r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when 3 * COMPRESS_BIT < 52
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_MOD / COMPRESS_MOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD;
        }
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) << COMPRESS_BIT) | r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm / db);
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                r.v[i + j] += add - b.v[j] * m;
                add = r.v[i + j] >> COMPRESS_BIT;
                r.v[i + j] &= COMPRESS_MASK;
            }
            for (size_t j = i + b.size(); add && j < r.size(); ++j) {
                r.v[j] += add;
                add = r.v[j] >> COMPRESS_BIT;
                r.v[j] &= COMPRESS_MASK;
            }
        }
        r.trim();
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }
        *this = r;
        return *this;
    }
    void trim() {
        while (v.back() == 0 && v.size() > 1)
            v.pop_back();
    }

public:
    BigIntHex() {
        set(0);
    }
    explicit BigIntHex(intmax_t n) {
        set(n);
    }
    explicit BigIntHex(const char *s, int base = 10) {
        from_str(s, base);
    }
    explicit BigIntHex(const std::string &s, int base = 10) {
        from_str(s, base);
    }
    BigInt_t &set(intmax_t n) {
        v.resize(1);
        v[0] = 0;
        uintmax_t s;
        if (n < 0) {
            sign = -1;
            s = -n;
        } else {
            sign = 1;
            s = n;
        }
        for (int i = 0; s; i++) {
            v.resize(i + 1);
            v[i] = s & COMPRESS_MASK;
            s >>= COMPRESS_BIT;
        }
        return *this;
    }
    BigInt_t &from_str_base2(const char *s, int bits) {
        v.clear();
        int32_t sign = 1;
        const char *p = s + strlen(s) - 1;
        while (*s == '-')
            sign *= -1, ++s;
        while (*s == '0')
            ++s;

        int32_t d = 0, hdigit = 0;
        for (; p >= s; p--) {
            int32_t digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (*p >= 'A' && *p <= 'Z')
                digit = *p - 'A' + 10;
            else if (*p >= 'a' && *p <= 'z')
                digit = *p - 'a' + 10;
            hdigit += digit << d;
            d += bits;
            if (d >= COMPRESS_BIT) {
                v.push_back(hdigit & COMPRESS_MASK);
                d -= COMPRESS_BIT;
                hdigit >>= COMPRESS_BIT;
            }
        }
        if (hdigit || v.empty()) {
            v.push_back(hdigit);
        }
        this->sign = sign;
        return *this;
    }
    BigInt_t &from_str(const char *s, int base = 10) {
        if (base == 16) {
            return from_str_base2(s, 4);
        } else if (base == 8) {
            return from_str_base2(s, 3);
        } else if (base == 2) {
            return from_str_base2(s, 1);
        }
        BigInt_t m;
        m.set(1);
        set(0);
        const char *p = s + strlen(s) - 1;
        int sign = 1, digits = 1, hbase = base;
        while (*s == '-') {
            sign *= -1;
            ++s;
        }
        while (*s == '0') {
            ++s;
        }
        for (; hbase <= BIGINT_MAXBASE; hbase *= base, ++digits)
            ;

        int d = --digits, hdigit = 0, hdigit_mul = 1;
        for (hbase /= base; p >= s; p--) {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (*p >= 'A' && *p <= 'Z')
                digit = *p - 'A' + 10;
            else if (*p >= 'a' && *p <= 'z')
                digit = *p - 'a' + 10;
            hdigit += digit * hdigit_mul;
            hdigit_mul *= base;
            if (--d == 0) {
                *this += m * hdigit;
                if (p > s) {
                    m.raw_mul_int((uint32_t)hbase);
                }
                d = digits;
                hdigit = 0;
                hdigit_mul = 1;
            }
        }
        if (hdigit) {
            *this += m * hdigit;
        }
        this->sign = sign;
        return *this;
    }
    BigInt_t &from_str(const std::string &s, int base = 10) {
        return this->from_str(s.c_str(), base);
    }
    size_t size() const {
        return v.size();
    }
    bool is_zero() const {
        if (v.size() == 1 && v[0] == 0)
            return true;
        return false;
    }
    bool operator<(const BigInt_t &b) const {
        if (sign * b.sign > 0) {
            if (sign > 0)
                return raw_less(b);
            else
                return b.raw_less(*this);
        } else {
            if (sign > 0)
                return false;
            else
                return true;
        }
    }
    bool operator>(const BigInt_t &b) const {
        return b < *this;
    }
    bool operator<=(const BigInt_t &b) const {
        return !(*this > b);
    }
    bool operator>=(const BigInt_t &b) const {
        return !(*this < b);
    }
    bool operator==(const BigInt_t &b) const {
        if (is_zero() && b.is_zero())
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }
    bool operator!=(const BigInt_t &b) const {
        return !(*this == b);
    }

    BigInt_t &operator=(intmax_t n) {
        return set(n);
    }
    BigInt_t &operator=(const char *s) {
        return from_str(s);
    }
    BigInt_t &operator=(const std::string s) {
        return from_str(s);
    }
    BigInt_t operator+(const BigInt_t &b) const {
        BigInt_t r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigInt_t &operator+=(const BigInt_t &b) {
        if (this == &b) {
            BigInt_t c = b;
            return *this += c;
        }
        BigInt_t &r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }

    BigInt_t operator-(const BigInt_t &b) const {
        BigInt_t r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigInt_t &operator-=(const BigInt_t &b) {
        if (this == &b) {
            set(0);
            return *this;
        }
        BigInt_t &r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigInt_t operator-() const {
        BigInt_t r = *this;
        r.sign = -r.sign;
        return r;
    }

    BigInt_t operator*(const BigInt_t &b) const {
        if (b.size() == 1) {
            BigInt_t r = *this;
            r.raw_mul_int((uint32_t)b.v[0]);
            r.sign *= b.sign;
            return r;
        } else if (v.size() == 1) {
            BigInt_t r = b;
            r.raw_mul_int((uint32_t)v[0]);
            r.sign *= sign;
            return r;
        } else {
            BigInt_t r;
            r.raw_nttmul(*this, b);
            r.sign = sign * b.sign;
            return r;
        }
    }
    BigInt_t &operator*=(const BigInt_t &b) {
        if (b.size() == 1) {
            raw_mul_int((uint32_t)b.v[0]);
            sign *= b.sign;
            return *this;
        } else {
            if (this == &b) {
                BigInt_t r = *this, c = b;
                raw_nttmul(r, c);
                sign = r.sign * c.sign;
                return *this;
            } else {
                BigInt_t r = *this;
                raw_nttmul(r, b);
                sign = r.sign * b.sign;
                return *this;
            }
        }
    }
    BigInt_t operator*(int32_t b) const {
        return *this * BigInt_t().set(b);
    }
    BigInt_t &operator*=(int32_t b) {
        if (b < 0x7fff && -0x7fff < b) {
            if (b >= 0)
                raw_mul_int((uint32_t)b);
            else {
                raw_mul_int((uint32_t)-b);
                sign = -sign;
            }
            return *this;
        }
        return *this *= BigInt_t().set(b);
    }

    BigInt_t operator/(const BigInt_t &b) const {
        BigInt_t r;
        r.raw_fastdiv(*this, b);
        r.sign = sign * b.sign;
        return r;
    }
    BigInt_t &operator/=(const BigInt_t &b) {
        if (this == &b) {
            BigInt_t c = b;
            return *this /= c;
        }
        BigInt_t r = *this;
        raw_fastdiv(r, b);
        sign = r.sign * b.sign;
        return *this;
    }

    BigInt_t operator%(const BigInt_t &b) const {
        return *this - *this / b * b;
    }
    BigInt_t &operator%=(const BigInt_t &b) {
        *this = *this - *this / b * b;
        return *this;
    }

    std::string out_hex() const {
        const char *digits = "0123456789ABCDEF";
        std::string out;
        int32_t d = 0;
        for (size_t i = 0, j = 0;;) {
            if (j < 4) {
                if (i < v.size())
                    d += v[i] << j;
                else if (d == 0)
                    break;
                j += COMPRESS_BIT;
                ++i;
            }
            out.push_back(digits[d & 0xf]);
            d >>= 4;
            j -= 4;
        }
        while (out.size() > 1 && out.back() == '0')
            out.pop_back();
        if (sign < 0 && !this->is_zero())
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    std::string out_mul(int32_t out_base = 10, int32_t pack = 0) const {
        BigIntBase sum(out_base);
        BigIntBase base(out_base);
        {
            base.set(1);
            BigIntBase mul(out_base);
            mul = base;
            mul.raw_mul_int(v[0]);
            sum.raw_add(mul);
        }
        for (size_t i = 1; i < v.size(); i++) {
            base.raw_mul_int(COMPRESS_MOD);
            BigIntBase mul(out_base);
            mul = base;
            mul.raw_mul_int(v[i]);
            sum.raw_add(mul);
        }
        std::string out;
        int32_t d = 0;
        for (size_t i = 0, j = 0;;) {
            if (j < 1) {
                if (i < sum.size())
                    d += sum.v[i];
                else if (d == 0)
                    break;
                j += sum.digits;
                ++i;
            }
            if (out_base <= 10 || d % out_base < 10) {
                out.push_back((d % out_base) + '0');
            } else {
                out.push_back((d % out_base) + 'A' - 10);
            }
            d /= out_base;
            j -= 1;
        }
        if (pack == 0)
            while (out.size() > 1 && out.back() == '0')
                out.pop_back();
        else
            while ((int32_t)out.size() > pack && out.back() == '0')
                out.pop_back();
        while ((int32_t)out.size() < pack)
            out.push_back('0');
        if (sign < 0 && !this->is_zero())
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    std::string to_str(int32_t out_base = 10, int32_t pack = 0) const {
        if (out_base == 16) {
            return out_hex();
        }
        if (v.size() < BIGINT_OUTPUT_THRESHOLD) {
            return out_mul(out_base, pack);
        }
        if (sign < 0) {
            BigInt_t a = *this;
            a.sign = 1;
            return "-" + a.to_str(out_base);
        }
        BigInt_t b;
        b.set(out_base);
        int32_t len = 1;
        for (; b * b < *this; len *= 2, b = b * b)
            ;
        if (pack) {
            std::string s1 = (*this / b).to_str(out_base, pack - len);
            std::string s2 = (*this % b).to_str(out_base, len);
            return s1 + s2;
        } else {
            std::string s1 = (*this / b).to_str(out_base, 0);
            std::string s2 = (*this % b).to_str(out_base, len);
            return s1 + s2;
        }
    }
};
} // namespace BigIntHexNS

using BigIntHexNS::BigIntHex;
