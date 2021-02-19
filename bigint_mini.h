// filename:    bigint_decmini.h
// author:      baobaobear
// create date: 2021-02-09
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once

#include "bigint_header.h"

namespace BigIntDecMiniNS {
const int32_t BIGINT_MAXBASE = 1 << 15;
const int32_t COMPRESS_DECMOD = 10000;
const int32_t COMPRESS_DIGITS = 4;

const int BIGINT_MUL_THRESHOLD = 48;

class BigIntDecMini {
protected:
    int sign;
    std::vector<int32_t> v;

    bool raw_less(const BigIntDecMini &b) const {
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
    bool raw_eq(const BigIntDecMini &b) const {
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
    BigIntDecMini &raw_add(const BigIntDecMini &b) {
        if (v.size() < b.size()) {
            v.resize(b.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add + b.v[i];
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        if (add) {
            v.push_back(add);
        } else {
            trim();
        }
        return *this;
    }
    BigIntDecMini &raw_offset_add(const BigIntDecMini &b, size_t offset) {
        int32_t add = 0;
        for (size_t i = 0; i < b.size(); ++i) {
            v[i + offset] += add + b.v[i];
            add = v[i + offset] / COMPRESS_DECMOD;
            v[i + offset] %= COMPRESS_DECMOD;
        }
        for (size_t i = b.size() + offset; add; ++i) {
            v[i] += add;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        return *this;
    }
    BigIntDecMini &raw_sub(const BigIntDecMini &b) {
        if (v.size() < b.v.size()) {
            v.resize(b.v.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add - b.v[i];
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
            if (v[i] < 0)
                v[i] += COMPRESS_DECMOD, add -= 1;
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            v[i] += add;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
            if (v[i] < 0)
                v[i] += COMPRESS_DECMOD, add -= 1;
        }
        if (add) {
            sign = -sign;
            v[0] = COMPRESS_DECMOD - v[0];
            for (size_t i = 1; i < v.size(); i++) {
                v[i] = COMPRESS_DECMOD - v[i] - 1;
            }
        }
        trim();
        return *this;
    }
    BigIntDecMini &raw_mul_int(uint32_t m) {
        if (m == 0) {
            set(0);
            return *this;
        }
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = add + v[i] * m;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        while (add) {
            v.push_back(add);
            add = v.back() / COMPRESS_DECMOD;
            v.back() %= COMPRESS_DECMOD;
        }
        return *this;
    }
    BigIntDecMini &raw_mul(const BigIntDecMini &a, const BigIntDecMini &b) {
        v.clear();
        v.resize(a.size() + b.size());
        for (size_t i = 0; i < a.size(); i++) {
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                v[i + j] += add + a.v[i] * b.v[j];
                add = v[i + j] / COMPRESS_DECMOD;
                v[i + j] %= COMPRESS_DECMOD;
            }
            v[i + b.size()] += add;
        }
        trim();
        return *this;
    }
    BigIntDecMini &raw_div(const BigIntDecMini &a, const BigIntDecMini &b) {
        if (a.raw_less(b)) {
            set(0);
            last_mod() = a;
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntDecMini r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when COMPRESS_DECMOD^3 << 2^52
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_DECMOD / COMPRESS_DECMOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD;
        }
        db = 1 / db;
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) * COMPRESS_DECMOD) + r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm * db);
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                r.v[i + j] += add - b.v[j] * m;
                add = r.v[i + j] / COMPRESS_DECMOD;
                r.v[i + j] %= COMPRESS_DECMOD;
                if (r.v[i + j] < 0)
                    r.v[i + j] += COMPRESS_DECMOD, --add;
            }
            for (size_t j = i + b.size(); add && j < r.size(); ++j) {
                r.v[j] += add;
                add = r.v[j] / COMPRESS_DECMOD;
                r.v[j] %= COMPRESS_DECMOD;
                if (r.v[j] < 0)
                    r.v[j] += COMPRESS_DECMOD, --add;
            }
        }
        r.trim();
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }
        last_mod() = r;
        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += add;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        trim();
        return *this;
    }
    BigIntDecMini &raw_fastmul(const BigIntDecMini &a, const BigIntDecMini &b) {
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
        BigIntDecMini ah, al, bh, bl, h, m;
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
    void trim() {
        while (v.back() == 0 && v.size() > 1)
            v.pop_back();
    }

public:
    explicit BigIntDecMini(intmax_t n = 0) {
        set(n);
    }
    explicit BigIntDecMini(const std::string &s) {
        from_str(s.c_str());
    }
    BigIntDecMini &set(intmax_t n) {
        v.resize(1);
        v[0] = 0;
        uintmax_t s;
        if (n < 0)
            sign = -1, s = -n;
        else
            sign = 1, s = n;
        for (int i = 0; s; i++) {
            v.resize(i + 1);
            v[i] = s % COMPRESS_DECMOD;
            s /= COMPRESS_DECMOD;
        }
        return *this;
    }
    BigIntDecMini &from_str(const char *s) {
        v.clear();
        int base = 10, sign = 1, digits = COMPRESS_DIGITS;
        const char *p = s + strlen(s) - 1;
        while (*s == '-')
            sign *= -1, ++s;
        while (*s == '0')
            ++s;

        int d = digits, hdigit = 0, hdigit_mul = 1;
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
    size_t size() const {
        return v.size();
    }
    bool is_zero() const {
        if (v.size() == 1 && v[0] == 0)
            return true;
        return false;
    }
    static BigIntDecMini &last_mod() {
        static BigIntDecMini m;
        return m;
    }
    bool operator<(const BigIntDecMini &b) const {
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
    bool operator==(const BigIntDecMini &b) const {
        if (is_zero() && b.is_zero())
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }

    BigIntDecMini &operator=(intmax_t n) {
        return set(n);
    }
    BigIntDecMini &operator=(const char *s) {
        return from_str(s);
    }
    BigIntDecMini &operator=(const std::string s) {
        return from_str(s.c_str());
    }

    BigIntDecMini operator+(const BigIntDecMini &b) const {
        BigIntDecMini r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntDecMini operator-(const BigIntDecMini &b) const {
        BigIntDecMini r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntDecMini operator-() const {
        BigIntDecMini r = *this;
        r.sign = -r.sign;
        return r;
    }
    BigIntDecMini operator*(const BigIntDecMini &b) const {
        if (b.size() == 1) {
            BigIntDecMini r = *this;
            r.raw_mul_int((uint32_t)b.v[0]);
            r.sign *= b.sign;
            return r;
        } else if (v.size() == 1) {
            BigIntDecMini r = b;
            r.raw_mul_int((uint32_t)v[0]);
            r.sign *= sign;
            return r;
        } else {
            BigIntDecMini r;
            r.raw_fastmul(*this, b);
            r.sign = sign * b.sign;
            return r;
        }
    }
    BigIntDecMini &operator*=(int32_t b) {
        if (b < BIGINT_MAXBASE && -BIGINT_MAXBASE < b) {
            if (b >= 0)
                raw_mul_int((uint32_t)b);
            else {
                raw_mul_int((uint32_t)-b);
                sign = -sign;
            }
            return *this;
        }
        return *this = *this * BigIntDecMini().set(b);
    }
    BigIntDecMini operator/(const BigIntDecMini &b) const {
        BigIntDecMini r;
        r.raw_div(*this, b);
        r.sign = sign * b.sign;
        return r;
    }
    BigIntDecMini operator%(const BigIntDecMini &b) const {
        return *this - *this / b * b;
    }

    std::string to_str() const {
        std::string out;
        int32_t d = 0;
        for (size_t i = 0, j = 0;;) {
            if (j < 1) {
                if (i < size())
                    d += v[i];
                else if (d == 0)
                    break;
                j += COMPRESS_DIGITS;
                ++i;
            }
            out.push_back((d % 10) + '0');
            d /= 10;
            j -= 1;
        }
        while (out.size() > 1 && *out.rbegin() == '0')
            out.erase(out.begin() + out.size() - 1);
        if (sign < 0 && !this->is_zero())
            out += '-';
        std::reverse(out.begin(), out.end());
        return out;
    }
};
} // namespace BigIntDecMiniNS

typedef BigIntDecMiniNS::BigIntDecMini BigIntMini;
