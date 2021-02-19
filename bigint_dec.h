// filename:    bigint_dec.h
// author:      baobaobear
// create date: 2021-02-08
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once
#include "bigint_base.h"

namespace BigIntDecNS {
const int32_t COMPRESS_MOD = 10000;
const int32_t COMPRESS_DIGITS = 4;

const int32_t BIGINT_NTT_THRESHOLD = 300;
const int32_t BIGINT_MUL_THRESHOLD = 48;
const int32_t BIGINT_DIV_THRESHOLD = 2000;
const int32_t BIGINT_DIVIDEDIV_THRESHOLD = 128;

#ifdef NTT_DOUBLE_MOD
const int32_t NTT_MAX_SIZE = 1 << 24;
#else
const int32_t NTT_MAX_SIZE = 1 << 21;
#endif

template <typename T>
inline T high_digit(T digit) {
    return digit / COMPRESS_MOD;
}

template <typename T>
inline int32_t low_digit(T digit) {
    return (int32_t)(digit % COMPRESS_MOD);
}

class BigIntDec {
protected:
    int sign;
    std::vector<int32_t> v;
    typedef BigIntDec BigInt_t;

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
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
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
            add = high_digit(v[i + offset]);
            v[i + offset] = low_digit(v[i + offset]);
        }
        for (size_t i = b.size() + offset; add; ++i) {
            v[i] += add;
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
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
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
            if (v[i] < 0)
                v[i] += COMPRESS_MOD, add -= 1;
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
            if (v[i] < 0)
                v[i] += COMPRESS_MOD, add -= 1;
        }
        if (add) {
            sign = -sign;
            v[0] = COMPRESS_MOD - v[0];
            for (size_t i = 1; i < v.size(); i++) {
                v[i] = COMPRESS_MOD - v[i] - 1;
            }
        }
        trim();
        return *this;
    }
    BigInt_t &raw_mul_int(uint32_t m) {
        if (m == 0) {
            set(0);
            return *this;
        } else if (m == 1) {
            return *this;
        } else if (m == COMPRESS_MOD) {
            return raw_shl(1);
        }
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = add + v[i] * m;
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
        }
        while (add) {
            v.push_back(low_digit(add));
            add = high_digit(add);
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
                add = high_digit(v[i + j]);
                v[i + j] = low_digit(v[i + j]);
            }
            v[i + b.size()] += add;
        }
        trim();
        return *this;
    }
    // Karatsuba algorithm
    BigInt_t &raw_fastmul(const BigInt_t &a, const BigInt_t &b) {
        if (a.is_zero() || b.is_zero()) {
            return set(0);
        } else if (a.size() <= 1 || b.size() <= 1) {
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
        NTT_NS::ntt_a.clear();
        NTT_NS::ntt_b.clear();
        for (size_t i = 0; i < a.size(); ++i) {
            NTT_NS::ntt_a.push_back(a.v[i]);
        }
        for (size_t i = 0; i < b.size(); ++i) {
            NTT_NS::ntt_b.push_back(b.v[i]);
        }
        NTT_NS::ntt_prepare(a.size(), b.size(), len, 7);
        NTT_NS::mul_conv2(len);
        len = (a.size() + b.size()) * lenmul;
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
        for (size_t i = 0; i <= len; i++) {
            int64_t s = add + NTT_NS::ntt_a[i];
            v.push_back(low_digit(s));
            add = high_digit(s);
        }
        for (; add; add = high_digit(add))
            v.push_back(low_digit(add));
        trim();
        return *this;
    }
    BigInt_t &raw_div(const BigInt_t &a, const BigInt_t &b, BigInt_t &r) {
        if (a.raw_less(b)) {
            r = a;
            return set(0);
        } else if (b.size() == 2 && b.v[1] == 1 && b.v[0] == 0) {
            r.set(a.v[0]);
            return raw_shr(1);
        }
        v.resize(a.size() - b.size() + 1);
        r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when COMPRESS_MOD^3 << 2^52
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_MOD / COMPRESS_MOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD;
        }
        db = 1 / db;
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) * COMPRESS_MOD) + r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm * db);
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                r.v[i + j] += add - b.v[j] * m;
                add = high_digit(r.v[i + j]);
                r.v[i + j] = low_digit(r.v[i + j]);
                if (r.v[i + j] < 0)
                    r.v[i + j] += COMPRESS_MOD, --add;
            }
            for (size_t j = i + b.size(); add && j < r.size(); ++j) {
                r.v[j] += add;
                add = high_digit(r.v[j]);
                r.v[j] = low_digit(r.v[j]);
                if (r.v[j] < 0)
                    r.v[j] += COMPRESS_MOD, --add;
            }
        }
        r.trim();
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }
        r.trim();

        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += add;
            add = high_digit(v[i]);
            v[i] = low_digit(v[i]);
        }
        trim();
        return *this;
    }
    BigInt_t &raw_shr(size_t n) {
        if (n == 0)
            return *this;
        if (n >= size()) {
            set(0);
            return *this;
        }
        size_t t = 0, s = n;
        for (; s < v.size(); ++t, ++s)
            v[t] = v[s];
        v.resize(t);
        return *this;
    }
    BigInt_t raw_shr_to(size_t n) const {
        BigInt_t r;
        if (n >= size()) {
            return r;
        }
        r.v.clear();
        size_t s = n;
        for (; s < v.size(); ++s)
            r.v.push_back(v[s]);
        return r;
    }
    BigInt_t &raw_shl(size_t n) {
        if (n == 0 || is_zero())
            return *this;
        v.resize(v.size() + n);
        size_t t = v.size() - 1, s = t - n;
        for (; s < v.size(); --t, --s)
            v[t] = v[s];
        for (; t < v.size(); --t)
            v[t] = 0;
        return *this;
    }
    BigInt_t &keep(size_t n) {
        size_t s = n < v.size() ? v.size() - n : (size_t)0;
        if (s && v[s - 1] >= COMPRESS_MOD >> 1) {
            ++v[s];
        }
        return raw_shr(s);
    }
    BigInt_t &raw_fastdiv(const BigInt_t &a, const BigInt_t &b) {
        if (a.raw_less(b)) {
            set(0);
            return *this;
        } else if (b.size() < BIGINT_DIV_THRESHOLD) {
            BigInt_t r;
            return raw_div(a, b, r);
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
        x0.v.push_back(x1.v.back() + 1);
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
    BigInt_t &raw_dividediv_recursion(const BigInt_t &a, const BigInt_t &b, BigInt_t &d) {
        if (a < b) {
            d = a;
            return set(0);
        }
        if (b.size() <= BIGINT_DIVIDEDIV_THRESHOLD) {
            return raw_div(a, b, d);
        }
        BigInt_t ma = a, mb = b, e;
        if (b.size() & 1) {
            ma.raw_shl(1);
            mb.raw_shl(1);
        }
        int32_t base = (int32_t)(mb.size() / 2);
        BigInt_t ha = ma.raw_shr_to(base);
        if ((int32_t)ma.size() <= base * 3) {
            BigInt_t hb = mb.raw_shr_to(base);
            raw_dividediv(ha, hb, d);
            ha = *this * b;
            while (ha > a) {
                ha -= b;
                *this -= BigInt_t(1);
            }
            d = a - ha;
            return *this;
        }
        e.raw_dividediv(ha, mb, d);
        ma.v.resize(base + d.size());
        for (size_t i = 0; i < d.size(); ++i) {
            ma.v[base + i] = d.v[i];
        }
        ma.trim();

        e.raw_shl(base);
        raw_dividediv_recursion(ma, mb, d);
        if (b.size() & 1)
            d.raw_shr(1);
        return *this += e;
    }
    BigInt_t &raw_dividediv(const BigInt_t &a, const BigInt_t &b, BigInt_t &r) {
        if (b.size() <= BIGINT_DIVIDEDIV_THRESHOLD) {
            raw_div(a, b, r);
            return *this;
        }
        int32_t mul = (COMPRESS_MOD * COMPRESS_MOD - 1) / (*(b.v.begin() + b.v.size() - 1) * COMPRESS_MOD + *(b.v.begin() + b.v.size() - 2) + 1);
        BigInt_t ma = a * mul;
        BigInt_t mb = b * mul;
        while (mb.v.back() < COMPRESS_MOD >> 1) {
            int32_t m = 2;
            ma *= m;
            mb *= m;
            mul *= m;
        }
        BigInt_t ha = ma.raw_shr_to(b.size());
        BigInt_t c, d;
        if (ha.size() > mb.size() * 2) {
            raw_dividediv(ha, mb, d);
        } else {
            raw_dividediv_recursion(ha, mb, d);
        }
        raw_shl(b.size());
        ma.v.resize(b.size() + d.size());
        for (size_t i = 0; i < d.size(); ++i) {
            ma.v[b.size() + i] = d.v[i];
        }
        ma.trim();
        c.raw_dividediv_recursion(ma, mb, d);
        r.raw_div(d, BigInt_t(mul), ma);
        raw_add(c);
        d /= BigInt_t(mul);
        return *this;
    }
    void trim() {
        while (v.back() == 0 && v.size() > 1)
            v.pop_back();
    }

public:
    BigIntDec() {
        set(0);
    }
    explicit BigIntDec(intmax_t n) {
        set(n);
    }
    explicit BigIntDec(const char *s, int base = 10) {
        from_str(s, base);
    }
    explicit BigIntDec(const std::string &s, int base = 10) {
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
            v[i] = low_digit(s);
            s = high_digit(s);
        }
        return *this;
    }
    BigInt_t &from_str_base10(const char *s) {
        v.clear();
        int32_t base = 10, sign = 1, digits = COMPRESS_DIGITS;
        const char *p = s + strlen(s) - 1;
        while (*s == '-')
            sign *= -1, ++s;
        while (*s == '0')
            ++s;

        int32_t d = digits, hdigit = 0, hdigit_mul = 1;
        for (; p >= s; p--) {
            hdigit += (*p - '0') * hdigit_mul;
            hdigit_mul *= base;
            if (--d == 0) {
                v.push_back(hdigit);
                d = digits;
                hdigit = 0;
                hdigit_mul = 1;
            }
        }
        if (hdigit || v.empty()) {
            v.push_back(hdigit);
        }
        this->sign = sign;
        return *this;
    }
    BigInt_t &from_str(const char *s, int base = 10) {
        if (base == 10) {
            return from_str_base10(s);
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
        for (; hbase <= 1 << 15; hbase *= base, ++digits)
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
        BigInt_t r, d;
        //d.raw_fastdiv(*this, b);
        d.raw_dividediv(*this, b, r);
        d.sign = sign * b.sign;
        return d;
    }
    BigInt_t &operator/=(const BigInt_t &b) {
        if (this == &b) {
            return set(1);
        }
        BigInt_t a = *this, r;
        //raw_fastdiv(a, b);
        raw_dividediv(a, b, r);
        sign = a.sign * b.sign;
        return *this;
    }
    BigInt_t operator%(const BigInt_t &b) const {
        return *this - *this / b * b;
    }
    BigInt_t &operator%=(const BigInt_t &b) {
        if (this == &b) {
            return set(0);
        }
        *this = *this - *this / b * b;
        return *this;
    }
    BigInt_t div(const BigInt_t &b, BigInt_t &r) {
        if (this == &b) {
            r.set(0);
            return set(1);
        }
        BigInt_t d;
        d.raw_dividediv(*this, b, r);
        d.sign = sign * b.sign;
        return d;
    }

    std::string out_dec() const {
        if (is_zero())
            return "0";
        std::string out;
        int32_t d = 0;
        for (size_t i = 0, j = 0;;) {
            if (j < 1) {
                if (i < size())
                    d += v[i];
                else if (d == 0)
                    break;
                j += 4;
                ++i;
            }
            out.push_back((d % 10) + '0');
            d /= 10;
            j -= 1;
        }
        while (out.size() > 1 && *out.rbegin() == '0')
            out.erase(out.begin() + out.size() - 1);
        if (sign < 0 && !this->is_zero())
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    BigIntBase transbase(int32_t out_base) const {
        if (size() <= 8) {
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
            return sum;
        } else {
            static BigIntBase pow_list[32];
            static int32_t last_base = 0, pow_list_cnt;
            BigIntBase base(out_base);
            if (out_base != last_base) {
                pow_list[0] = base.set(COMPRESS_MOD);
                pow_list_cnt = 0;
                last_base = out_base;
            }
            size_t s = 1, id = 0;
            for (; s < size() / 2; s *= 2, ++id) {
                if (s >= (size_t)1 << pow_list_cnt) {
                    pow_list[pow_list_cnt + 1].setbase(out_base);
                    pow_list[pow_list_cnt + 1].raw_nttsqr(pow_list[pow_list_cnt]);
                    ++pow_list_cnt;
                }
            }
            base = pow_list[id];
            BigInt_t h = raw_shr_to(s), l = *this;
            l.v.resize(s);
            BigIntBase r = h.transbase(out_base);
            BigIntBase sum(out_base);
            sum.raw_nttmul(r, base);
            r = l.transbase(out_base);
            sum.raw_add(r);
            return sum;
        }
    }

    std::string out_mul(int32_t out_base = 10, int32_t pack = 0) const {
        BigIntBase sum = transbase(out_base);
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
            while (out.size() > 1 && *out.rbegin() == '0')
                out.erase(out.begin() + out.size() - 1);
        else
            while ((int32_t)out.size() > pack && *out.rbegin() == '0')
                out.erase(out.begin() + out.size() - 1);
        while ((int32_t)out.size() < pack)
            out.push_back('0');
        if (out.empty())
            out.push_back('0');
        if (sign < 0 && !this->is_zero())
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    std::string to_str(int32_t out_base = 10, int32_t pack = 0) const {
        if (out_base == 10) {
            return out_dec();
        }
        return out_mul(out_base, pack);
    }
};
} // namespace BigIntDecNS

using BigIntDecNS::BigIntDec;
