// filename:    bigint_decmini.h
// author:      baobaobear
// create date: 2021-02-09
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include "bigint_header.h"

namespace BigIntMiniNS {
const int32_t COMPRESS_MOD = 10000;
const uint32_t COMPRESS_DIGITS = 4;

const uint32_t BIGINT_MUL_THRESHOLD = 512;

template <typename T>
inline T high_digit(T digit) {
    return digit / COMPRESS_MOD;
}

template <typename T>
inline uint32_t low_digit(T digit) {
    return (uint32_t)(digit % COMPRESS_MOD);
}

class BigIntMini {
protected:
    typedef uint32_t base_t;
    typedef int32_t carry_t;
    int sign;
    std::vector<base_t> v;
    typedef BigIntMini BigInt_t;

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
        carry_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            add += v[i];
            add += b.v[i];
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            add += v[i];
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        if (add) {
            v.push_back((base_t)add);
        } else {
            trim();
        }
        return *this;
    }
    BigInt_t &raw_offset_add(const BigInt_t &b, size_t offset) {
        carry_t add = 0;
        for (size_t i = 0; i < b.size(); ++i) {
            add += b.v[i];
            add += v[i + offset];
            v[i + offset] = low_digit(add);
            add = high_digit(add);
        }
        for (size_t i = b.size() + offset; add; ++i) {
            add += v[i];
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        return *this;
    }
    BigInt_t &raw_sub(const BigInt_t &b) {
        if (v.size() < b.v.size()) {
            v.resize(b.v.size());
        }
        carry_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            add += v[i];
            add -= b.v[i];
            if (add >= 0) {
                v[i] = low_digit(add);
                add = high_digit(add);
            } else {
                v[i] = low_digit(++add) + COMPRESS_MOD - 1;
                add = high_digit(add) - 1;
            }
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            add += v[i];
            if (add >= 0) {
                v[i] = low_digit(add);
                add = high_digit(add);
            } else {
                v[i] = low_digit(++add) + COMPRESS_MOD - 1;
                add = high_digit(add) - 1;
            }
        }
        if (add) {
            sign = -sign;
            add = 1 + (COMPRESS_MOD - 1 - v[0]);
            v[0] = low_digit(add);
            add = high_digit(add);
            for (size_t i = 1; i < v.size(); i++) {
                add += COMPRESS_MOD - 1 - v[i];
                v[i] = low_digit(add);
                add = high_digit(add);
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
        }
        carry_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            add += v[i] * m;
            v[i] = low_digit(add);
            add = high_digit(add);
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
            carry_t add = 0, av = a.v[i];
            for (size_t j = 0; j < b.size(); j++) {
                add += v[i + j];
                add += av * b.v[j];
                v[i + j] = low_digit(add);
                add = high_digit(add);
            }
            v[i + b.size()] += (base_t)add;
        }
        trim();
        return *this;
    }
    // Karatsuba algorithm
    BigInt_t &raw_fastmul(const BigInt_t &a, const BigInt_t &b) {
        if (std::min(a.size(), b.size()) <= BIGINT_MUL_THRESHOLD) {
            return raw_mul(a, b);
        }
        BigInt_t ah, al, bh, bl, h, m;
        size_t split = std::max(std::min((a.size() + 1) / 2, (b.size() * 2) / 3), std::min((a.size() * 2) / 3, (b.size() + 1) / 2));
        size_t split2 = split * 2;
        al = a.raw_lowdigits_to(split);
        ah = a.raw_shr_to(split);
        bl = b.raw_lowdigits_to(split);
        bh = b.raw_shr_to(split);

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
    BigInt_t &raw_div(const BigInt_t &a, const BigInt_t &b, BigInt_t &r) {
        if (a.raw_less(b)) {
            r = a;
            return set(0);
        }
        v.resize(a.size() - b.size() + 1);
        r = a;
        r.v.resize(a.size() + 1);
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) {
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD + (b.v[b.size() - 3]) / (double)COMPRESS_MOD / COMPRESS_MOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_MOD;
        }
        db = 1 / db;
        for (size_t i = a.size() - offset; i <= a.size(); i--) {
            carry_t rm = (carry_t)r.v[i + offset] * COMPRESS_MOD + r.v[i + offset - 1] - 1, m;
            m = (carry_t)(rm * db);
            v[i] = (base_t)m;
            carry_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                add += r.v[i + j];
                add -= b.v[j] * m;
                r.v[i + j] = low_digit(add);
                if (r.v[i + j] < COMPRESS_MOD) { // r.v[i + j] >= 0
                    add = high_digit(add);
                } else {
                    r.v[i + j] += (base_t)COMPRESS_MOD;
                    add = high_digit(add) - 1;
                }
            }
            for (size_t j = i + b.size(); add && j < r.size(); ++j) {
                add += r.v[j];
                r.v[j] = low_digit(add);
                if (r.v[j] < COMPRESS_MOD) { // r.v[j] >= 0
                    add = high_digit(add);
                } else {
                    r.v[j] += (base_t)COMPRESS_MOD;
                    add = high_digit(add) - 1;
                }
            }
        }
        r.trim();
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }
        r.trim();

        carry_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            add += v[i];
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        trim();
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
    BigInt_t raw_lowdigits_to(size_t n) const {
        BigInt_t r;
        if (n >= size()) {
            return r = *this;
        }
        r.v.resize(n);
        size_t s = 0;
        for (; s < n; ++s)
            r.v[s] = v[s];
        r.trim();
        return r;
    }
    void trim() {
        while (v.back() == 0 && v.size() > 1)
            v.pop_back();
    }
    size_t size() const {
        return v.size();
    }

public:
    BigIntMini() {
        set(0);
    }
    explicit BigIntMini(intmax_t n) {
        set(n);
    }
    explicit BigIntMini(const char *s, int base = 10) {
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
        return from_str_base10(s);
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
    bool operator==(const BigInt_t &b) const {
        if (is_zero() && b.is_zero())
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }

    BigInt_t &operator=(intmax_t n) {
        return set(n);
    }
    BigInt_t &operator=(const char *s) {
        return from_str(s);
    }
    BigInt_t operator+(const BigInt_t &b) const {
        BigInt_t r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return BIGINT_STD_MOVE(r);
    }
    BigInt_t operator-(const BigInt_t &b) const {
        BigInt_t r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return BIGINT_STD_MOVE(r);
    }
    BigInt_t operator-() const {
        BigInt_t r = *this;
        r.sign = -r.sign;
        return BIGINT_STD_MOVE(r);
    }

    BigInt_t operator*(const BigInt_t &b) const {
        BigInt_t r;
        r.raw_fastmul(*this, b);
        r.sign = sign * b.sign;
        return BIGINT_STD_MOVE(r);
    }

    BigInt_t operator/(const BigInt_t &b) const {
        BigInt_t r, d;
        d.raw_div(*this, b, r);
        d.sign = sign * b.sign;
        return BIGINT_STD_MOVE(d);
    }
    BigInt_t operator%(const BigInt_t &b) const {
        return BIGINT_STD_MOVE(*this - *this / b * b);
    }
    BigInt_t div(const BigInt_t &b, BigInt_t &r) {
        if (this == &b) {
            r.set(0);
            return set(1);
        }
        BigInt_t d;
        d.raw_div(*this, b, r);
        d.sign = sign * b.sign;
        return BIGINT_STD_MOVE(d);
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

    std::string to_str(int32_t out_base = 10) const {
        return out_dec();
    }
};
} // namespace BigIntMiniNS

typedef BigIntMiniNS::BigIntMini BigIntMini;
