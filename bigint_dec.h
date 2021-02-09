#pragma once
#include "bigint.h"

namespace BigIntDecNS {
const int COMPRESS_DECMOD = 10000;

const int BIGINT_MUL_THRESHOLD = 48;
const int BIGINT_OUTPUT_THRESHOLD = 32;

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
        trim();
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

        int32_t add = 0;
        for (size_t i = 0; i < m.size(); ++i) {
            v[i + split] += add + m.v[i];
            add = v[i + split] / COMPRESS_DECMOD;
            v[i + split] %= COMPRESS_DECMOD;
        }
        for (size_t i = m.size(); add; ++i) {
            v[i + split] += add;
            add = v[i + split] / COMPRESS_DECMOD;
            v[i + split] %= COMPRESS_DECMOD;
        }
        if (add) {
            v[m.size() + split] += add;
        }
        add = 0;
        for (size_t i = 0; i < h.size(); ++i) {
            v[i + split2] += add + h.v[i];
            add = v[i + split2] / COMPRESS_DECMOD;
            v[i + split2] %= COMPRESS_DECMOD;
        }
        for (size_t i = h.size(); add; ++i) {
            v[i + split2] += add;
            add = v[i + split2] / COMPRESS_DECMOD;
            v[i + split2] %= COMPRESS_DECMOD;
        }
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntDec &raw_div(const BigIntDec &a, const BigIntDec &b) {
        if (a.size() < b.size()) {
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
    BigIntDec &from_str(const char *s, int base = 10) {
        BigIntDec m;
        m.set(1);
        set(0);
        const char *p = s + strlen(s);
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
        for (hbase /= base, p--; p >= s; p--) {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (base > 10) {
                if (*p >= 'A' && *p <= 'Z')
                    digit = *p - 'A' + 10;
                else if (*p >= 'a' && *p <= 'z')
                    digit = *p - 'A' + 10;
            }
            hdigit += digit * hdigit_mul;
            hdigit_mul *= base;
            if (--d == 0) {
                *this += m * hdigit;
                if (p > s) {
                    m *= hbase;
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
        set(n);
        return *this;
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
        } else {
            BigIntDec r;
            r.raw_fastmul(*this, b);
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
                raw_fastmul(r, c);
                sign = r.sign * c.sign;
                return *this;
            } else {
                BigIntDec r = *this;
                raw_fastmul(r, b);
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
        r.raw_div(*this, b);
        r.sign = sign * b.sign;
        return r;
    }
    BigIntDec &operator/=(const BigIntDec &b) {
        if (this == &b) {
            BigIntDec c = b;
            return *this /= c;
        }
        BigIntDec r = *this;
        raw_div(r, b);
        sign = r.sign * b.sign;
        return *this;
    }

    BigIntDec operator%(const BigIntDec &b) const {
        BigIntDec r;
        r.raw_mod(*this, b);
        return r;
    }
    BigIntDec &operator%=(const BigIntDec &b) {
        if (this == &b) {
            BigIntDec c = b;
            return *this %= c;
        }
        BigIntDec r = *this;
        raw_mod(r, b);
        return *this;
    }

    std::string out_dec(int32_t pack = 0) const {
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
        if (pack == 0)
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
