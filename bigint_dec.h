// filename:    bigint_dec.h
// author:      baobaobear
// create date: 2021-02-08
// This library is compatible with C++11
// https://github.com/Baobaobear/MiniBigInteger
#pragma once
#include "bigint_base.h"

namespace BigIntDecNS {
const int32_t COMPRESS_DECMOD = 10000;
const int32_t COMPRESS_DIGITS = 4;

const int32_t BIGINT_NTT_THRESHOLD = 300;
const int32_t BIGINT_MUL_THRESHOLD = 48;
const int32_t BIGINT_DIV_THRESHOLD = 2000;
const int32_t BIGINT_OUTPUT_THRESHOLD = 32;

#ifdef NTT_DOUBLE_MOD
const int32_t NTT_MAX_SIZE = 1 << 24;
#else
const int32_t NTT_MAX_SIZE = 1 << 21;
#endif

class BigIntDec {
protected:
    int sign;
    std::vector<int32_t> v;

    bool raw_less(const BigIntDec &b) const {
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
    bool raw_eq(const BigIntDec &b) const {
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
    BigIntDec &raw_add(const BigIntDec &b) {
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
    BigIntDec &raw_offset_add(const BigIntDec &b, size_t offset) {
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
    BigIntDec &raw_sub(const BigIntDec &b) {
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
    BigIntDec &raw_mul_int(uint32_t m) {
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
    BigIntDec &raw_mul(const BigIntDec &a, const BigIntDec &b) {
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
    // Karatsuba algorithm
    BigIntDec &raw_fastmul(const BigIntDec &a, const BigIntDec &b) {
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
        BigIntDec ah, al, bh, bl, h, m;
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
    BigIntDec &raw_nttmul(const BigIntDec &a, const BigIntDec &b) {
        if (a.size() <= BIGINT_MUL_THRESHOLD || b.size() <= BIGINT_MUL_THRESHOLD) {
            return raw_mul(a, b);
        }
        if ((a.size() <= BIGINT_NTT_THRESHOLD && b.size() <= BIGINT_NTT_THRESHOLD) || (a.size() + b.size()) > NTT_MAX_SIZE) {
            return raw_fastmul(a, b);
        }
        size_t len;
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
        for (size_t i = 0; i < a.size(); ++i) {
            NTT_NS::ntt_a.push_back(a.v[i] % 10);
            NTT_NS::ntt_a.push_back(a.v[i] / 10 % 10);
            NTT_NS::ntt_a.push_back(a.v[i] / 100 % 10);
            NTT_NS::ntt_a.push_back(a.v[i] / 1000 % 10);
        }
        for (size_t i = 0; i < b.size(); ++i) {
            NTT_NS::ntt_b.push_back(b.v[i] % 10);
            NTT_NS::ntt_b.push_back(b.v[i] / 10 % 10);
            NTT_NS::ntt_b.push_back(b.v[i] / 100 % 10);
            NTT_NS::ntt_b.push_back(b.v[i] / 1000 % 10);
        }
        NTT_NS::Prepare(a.size() * 4, b.size() * 4, len);
#endif
        NTT_NS::Conv(len);
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
#ifdef NTT_DOUBLE_MOD
        for (size_t i = 0; i <= len; i++) {
            int64_t s = add + NTT_NS::ntt_a[i];
            v.push_back(s % COMPRESS_DECMOD);
            add = s / COMPRESS_DECMOD;
        }
#else
        for (size_t i = 0; i <= len; i += 4) {
            int64_t s = add + NTT_NS::ntt_a[i] + (NTT_NS::ntt_a[i + 1] * 10) + (NTT_NS::ntt_a[i + 2] * 100) + (NTT_NS::ntt_a[i + 3] * 1000);
            v.push_back(s % COMPRESS_DECMOD);
            add = s / COMPRESS_DECMOD;
        }
#endif
        for (; add; add /= COMPRESS_DECMOD)
            v.push_back(add % COMPRESS_DECMOD);
        trim();
        return *this;
    }
    BigIntDec &raw_div(const BigIntDec &a, const BigIntDec &b) {
        if (a.raw_less(b)) {
            set(0);
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntDec r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when COMPRESS_DECMOD^3 << 2^52
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_DECMOD / COMPRESS_DECMOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD;
        }
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) * COMPRESS_DECMOD) + r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm / db);
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

        int32_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            v[i] += add;
            add = v[i] / COMPRESS_DECMOD;
            v[i] %= COMPRESS_DECMOD;
        }
        trim();
        return *this;
    }
    BigIntDec &raw_shr(size_t n) {
        size_t t = 0, s = n;
        for (; s < v.size(); ++t, ++s)
            v[t] = v[s];
        v.resize(t);
        return *this;
    }
    BigIntDec &keep(size_t n) {
        size_t s = n < v.size() ? v.size() - n : (size_t)0;
        return raw_shr(s);
    }
    BigIntDec &raw_fastdiv(const BigIntDec &a, const BigIntDec &b) {
        if (a.raw_less(b)) {
            set(0);
            return *this;
        } else if (b.size() < BIGINT_DIV_THRESHOLD) {
            return raw_div(a, b);
        }
        v.resize(a.size() - b.size() + 1);
        BigIntDec b2, b2r, x0, x1, t;
        b2.v.resize(v.size());
        b2.v.push_back(2);
        x1.v.resize(1);
        x1.v.back() = (int32_t)(COMPRESS_DECMOD / (b.v.back() + (b.v[b.size() - 2] + 1.0) / COMPRESS_DECMOD));
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
            keep_size = std::min(keep_size * 2, v.size());
            x1.keep(keep_size);
        }
        x0 *= a;
        if (x0.v[a.size() - 1] >= COMPRESS_DECMOD >> 1)
            *this = x0.raw_shr(a.size()).raw_add(BigIntDec(1));
        else
            *this = x0.raw_shr(a.size());
        return *this;
    }
    BigIntDec &raw_mod(const BigIntDec &a, const BigIntDec &b) {
        if (a.size() < b.size()) {
            *this = a;
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntDec r = a;
        int32_t offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 2) { // works when COMPRESS_DECMOD^3 << 2^52
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD + (b.v[b.size() - 3] + 1) / (double)COMPRESS_DECMOD / COMPRESS_DECMOD;
        } else if (b.size() > 1) {
            db += b.v[b.size() - 2] / (double)COMPRESS_DECMOD;
        }
        for (size_t i = r.size() - offset; i <= a.size(); i--) {
            int32_t rm = ((i + offset < r.size() ? r.v[i + offset] : 0) * COMPRESS_DECMOD) + r.v[i + offset - 1], m;
            v[i] = m = (int32_t)(rm / db);
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
        *this = r;
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
    BigIntDec &set(intmax_t n) {
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
            v[i] = s % COMPRESS_DECMOD;
            s /= COMPRESS_DECMOD;
        }
        return *this;
    }
    BigIntDec &from_str_base10(const char *s) {
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
    BigIntDec &from_str(const char *s, int base = 10) {
        if (base == 10) {
            return from_str_base10(s);
        }
        BigIntDec m;
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
    BigIntDec &from_str(const std::string s, int base = 10) {
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
    bool operator<(const BigIntDec &b) const {
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
    bool operator>(const BigIntDec &b) const {
        return b < *this;
    }
    bool operator<=(const BigIntDec &b) const {
        return !(*this > b);
    }
    bool operator>=(const BigIntDec &b) const {
        return !(*this < b);
    }
    bool operator==(const BigIntDec &b) const {
        if (is_zero() && b.is_zero())
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }
    bool operator!=(const BigIntDec &b) const {
        return !(*this == b);
    }

    BigIntDec &operator=(intmax_t n) {
        return set(n);
    }
    BigIntDec &operator=(const char *s) {
        return from_str(s);
    }
    BigIntDec &operator=(const std::string s) {
        return from_str(s);
    }
    BigIntDec operator+(const BigIntDec &b) const {
        BigIntDec r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntDec &operator+=(const BigIntDec &b) {
        if (this == &b) {
            BigIntDec c = b;
            return *this += c;
        }
        BigIntDec &r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }

    BigIntDec operator-(const BigIntDec &b) const {
        BigIntDec r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntDec &operator-=(const BigIntDec &b) {
        if (this == &b) {
            set(0);
            return *this;
        }
        BigIntDec &r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntDec operator-() const {
        BigIntDec r = *this;
        r.sign = -r.sign;
        return r;
    }

    BigIntDec operator*(const BigIntDec &b) const {
        if (b.size() == 1) {
            BigIntDec r = *this;
            r.raw_mul_int((uint32_t)b.v[0]);
            r.sign *= b.sign;
            return r;
        } else if (v.size() == 1) {
            BigIntDec r = b;
            r.raw_mul_int((uint32_t)v[0]);
            r.sign *= sign;
            return r;
        } else {
            BigIntDec r;
            r.raw_nttmul(*this, b);
            r.sign = sign * b.sign;
            return r;
        }
    }
    BigIntDec &operator*=(const BigIntDec &b) {
        if (b.size() == 1) {
            raw_mul_int((uint32_t)b.v[0]);
            sign *= b.sign;
            return *this;
        } else {
            if (this == &b) {
                BigIntDec r = *this, c = b;
                raw_nttmul(r, c);
                sign = r.sign * c.sign;
                return *this;
            } else {
                BigIntDec r = *this;
                raw_nttmul(r, b);
                sign = r.sign * b.sign;
                return *this;
            }
        }
    }
    BigIntDec operator*(int32_t b) const {
        return *this * BigIntDec().set(b);
    }
    BigIntDec &operator*=(int32_t b) {
        if (b < 0x7fff && -0x7fff < b) {
            if (b >= 0)
                raw_mul_int((uint32_t)b);
            else {
                raw_mul_int((uint32_t)-b);
                sign = -sign;
            }
            return *this;
        }
        return *this *= BigIntDec().set(b);
    }

    BigIntDec operator/(const BigIntDec &b) const {
        BigIntDec r;
        r.raw_fastdiv(*this, b);
        r.sign = sign * b.sign;
        return r;
    }
    BigIntDec &operator/=(const BigIntDec &b) {
        if (this == &b) {
            BigIntDec c = b;
            return *this /= c;
        }
        BigIntDec r = *this;
        raw_fastdiv(r, b);
        sign = r.sign * b.sign;
        return *this;
    }

    BigIntDec operator%(const BigIntDec &b) const {
        BigIntDec r;
        r = *this - *this / b * b;
        return r;
    }
    BigIntDec &operator%=(const BigIntDec &b) {
        *this = *this - *this / b * b;
        return *this;
    }

    std::string out_dec() const {
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
            base.raw_mul_int(COMPRESS_DECMOD);
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
        if (out_base == 10) {
            return out_dec();
        }
        if (v.size() < BIGINT_OUTPUT_THRESHOLD) {
            return out_mul(out_base, pack);
        }
        if (sign < 0) {
            BigIntDec a = *this;
            a.sign = 1;
            return "-" + a.to_str(out_base);
        }
        BigIntDec b;
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
} // namespace BigIntDecNS

using BigIntDecNS::BigIntDec;
