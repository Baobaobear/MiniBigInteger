#pragma once

#include <cstdint>
#include <vector>

const int COMPRESS_BIT = 15;
const int COMPRESS_MOD = 1 << COMPRESS_BIT;
const int COMPRESS_MASK = COMPRESS_MOD - 1;

const int COMPRESS_DECMOD = 10000;

struct BigIntBase {
    int base;
    int digits;
    std::vector<int32_t> v;

    BigIntBase(int b) { // b > 1
        base = b;
        for (digits = 1; base <= 32768; base *= b, ++digits)
            ;
        base /= b;
        --digits;
        set(0);
    }
    BigIntBase &set(int64_t n) {
        v.resize(1);
        uint64_t s;
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

struct BigIntDec {
    int sign;
    std::vector<int32_t> v;

    BigIntDec() {
        set(0);
    }
    BigIntDec &set(int64_t n) {
        v.resize(1);
        v[0] = 0;
        uint64_t s;
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
        const char *p = s;
        int sign = 1;
        while (*p)
            ++p;
        while (*s == '-') {
            sign *= -1;
            ++s;
        }
        while (*s == '0') {
            ++s;
        }
        for (p--; p >= s; p--) {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (base > 10) {
                if (*p >= 'A' && *p <= 'Z')
                    digit = *p - 'A' + 10;
                else if (*p >= 'a' && *p <= 'z')
                    digit = *p - 'A' + 10;
            }
            *this += m * digit;
            if (p > s) {
                m *= base;
            }
        }
        this->sign = sign;
        return *this;
    }
    size_t size() const {
        return v.size();
    }
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
        if (add) {
            v.push_back(add);
        } else {
            while (v.back() == 0 && v.size() > 1)
                v.pop_back();
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
        if (add) {
            sign = -sign;
            v[0] = COMPRESS_DECMOD - v[0];
            for (size_t i = 1; i < v.size(); i++) {
                v[i] = COMPRESS_DECMOD - v[i] - 1;
            }
        }
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntDec &raw_mul_int(uint32_t m) {
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
            if (add) {
                v[i + b.size()] += add;
            }
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
        int32_t remain = 0, offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 1) {
            db += (b.v[b.size() - 2] + 1) / (double)COMPRESS_DECMOD;
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
            if (add) {
                r.v[i + b.size()] += add;
            }
        }
        while (r.v.back() == 0 && r.v.size() > 1) {
            r.v.pop_back();
        }
        for (int32_t mul = COMPRESS_DECMOD >> 1; mul > 1; mul >>= 1) {
            while (!r.raw_less(b * mul)) {
                r.raw_sub(b * mul);
                v[0] += mul;
            }
        }
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
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntDec &raw_mod(const BigIntDec &a, const BigIntDec &b) {
        if (a.size() < b.size()) {
            *this = a;
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntDec r = a;
        int32_t remain = 0, offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 1) {
            db += (b.v[b.size() - 2] + 1) / (double)COMPRESS_DECMOD;
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
            if (add) {
                r.v[i + b.size()] += add;
            }
            while (r.v.back() == 0 && r.v.size() > 1) {
                r.v.pop_back();
            }
        }
        for (int32_t mul = COMPRESS_DECMOD >> 1; mul > 1; mul >>= 1) {
            while (!r.raw_less(b * mul)) {
                r.raw_sub(b * mul);
                v[0] += mul;
            }
        }
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }

        *this = r;
        return *this;
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
        if (size() == 1 && b.size() == 1 && v[0] == 0 && b.v[0] == 0)
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }
    bool operator!=(const BigIntDec &b) const {
        return !(*this == b);
    }

    BigIntDec &operator=(int64_t n) {
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
        r.sign *= -1;
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
            r.raw_mul(*this, b);
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
            BigIntDec r = *this;
            raw_mul(r, b);
            sign = r.sign * b.sign;
            return *this;
        }
    }

    BigIntDec operator*(int32_t b) const {
        return *this * BigIntDec().set(b);
    }

    BigIntDec &operator*=(int16_t b) {
        if (b >= 0)
            raw_mul_int((uint32_t)b);
        else {
            raw_mul_int((uint32_t)-b);
            sign = -sign;
        }
        return *this;
    }

    BigIntDec &operator*=(int32_t b) {
        if (b < 0x7fff && -0x7fff < b)
            return *this *= (int16_t)b;
        return *this *= BigIntDec().set(b);
    }

    BigIntDec operator/(const BigIntDec &b) const {
        BigIntDec r;
        r.raw_div(*this, b);
        r.sign = sign * b.sign;
        return r;
    }

    BigIntDec &operator/=(const BigIntDec &b) {
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
        if (sign < 0 && *this != BigIntDec().set(0))
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
        if (sign < 0 && *this != BigIntDec().set(0))
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    std::string to_str(int32_t out_base = 10, int32_t pack = 0) const {
        if (out_base == 10) {
            return out_dec();
        }
        if (v.size() < 64) {
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

struct BigIntHex {
    int sign;
    std::vector<int32_t> v;

    BigIntHex() {
        set(0);
    }
    BigIntHex &set(int64_t n) {
        v.resize(1);
        v[0] = 0;
        uint64_t s;
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
    BigIntHex &from_str(const char *s, int base = 10) {
        BigIntHex m;
        m.set(1);
        set(0);
        const char *p = s;
        int sign = 1;
        while (*p)
            ++p;
        while (*s == '-') {
            sign *= -1;
            ++s;
        }
        while (*s == '0') {
            ++s;
        }
        for (p--; p >= s; p--) {
            int digit = -1;
            if (*p >= '0' && *p <= '9')
                digit = *p - '0';
            else if (base > 10) {
                if (*p >= 'A' && *p <= 'Z')
                    digit = *p - 'A' + 10;
                else if (*p >= 'a' && *p <= 'z')
                    digit = *p - 'A' + 10;
            }
            *this += m * digit;
            if (p > s) {
                m *= base;
            }
        }
        this->sign = sign;
        return *this;
    }
    size_t size() const {
        return v.size();
    }
    bool raw_less(const BigIntHex &b) const {
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
    bool raw_eq(const BigIntHex &b) const {
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

    BigIntHex &raw_add(const BigIntHex &b) {
        if (v.size() < b.size()) {
            v.resize(b.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add + b.v[i];
            add = v[i] >> COMPRESS_BIT;
            v[i] &= COMPRESS_MASK;
        }
        if (add) {
            v.push_back(add);
        } else {
            while (v.back() == 0 && v.size() > 1)
                v.pop_back();
        }
        return *this;
    }
    BigIntHex &raw_sub(const BigIntHex &b) {
        if (v.size() < b.v.size()) {
            v.resize(b.v.size());
        }
        int32_t add = 0;
        for (size_t i = 0; i < b.v.size(); i++) {
            v[i] += add - b.v[i];
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
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntHex &raw_mul_int(uint32_t m) {
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
    BigIntHex &raw_mul(const BigIntHex &a, const BigIntHex &b) {
        v.clear();
        v.resize(a.size() + b.size());
        for (size_t i = 0; i < a.size(); i++) {
            int32_t add = 0;
            for (size_t j = 0; j < b.size(); j++) {
                v[i + j] += add + a.v[i] * b.v[j];
                add = v[i + j] >> COMPRESS_BIT;
                v[i + j] &= COMPRESS_MASK;
            }
            if (add) {
                v[i + b.size()] += add;
            }
        }
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntHex &raw_div(const BigIntHex &a, const BigIntHex &b) {
        if (a.size() < b.size()) {
            set(0);
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntHex r = a;
        int32_t remain = 0, offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 1) {
            db += (b.v[b.size() - 2] + 1) / (double)COMPRESS_MOD;
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
            if (add) {
                r.v[i + b.size()] += add;
            }
        }
        while (r.v.back() == 0 && r.v.size() > 1) {
            r.v.pop_back();
        }
        for (int32_t mul = COMPRESS_MOD >> 1; mul > 1; mul >>= 1) {
            while (!r.raw_less(b * mul)) {
                r.raw_sub(b * mul);
                v[0] += mul;
            }
        }
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
        while (v.back() == 0 && v.size() > 1) {
            v.pop_back();
        }
        return *this;
    }
    BigIntHex &raw_mod(const BigIntHex &a, const BigIntHex &b) {
        if (a.size() < b.size()) {
            *this = a;
            return *this;
        }
        v.resize(a.size() - b.size() + 1);
        BigIntHex r = a;
        int32_t remain = 0, offset = (int32_t)b.size();
        double db = b.v.back();
        if (b.size() > 1) {
            db += (b.v[b.size() - 2] + 1) / (double)COMPRESS_MOD;
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
            if (add) {
                r.v[i + b.size()] += add;
            }
            while (r.v.back() == 0 && r.v.size() > 1) {
                r.v.pop_back();
            }
        }
        for (int32_t mul = COMPRESS_MOD >> 1; mul > 1; mul >>= 1) {
            while (!r.raw_less(b * mul)) {
                r.raw_sub(b * mul);
                v[0] += mul;
            }
        }
        while (!r.raw_less(b)) {
            r.raw_sub(b);
            v[0]++;
        }

        *this = r;
        return *this;
    }

    bool operator<(const BigIntHex &b) const {
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
    bool operator>(const BigIntHex &b) const {
        return b < *this;
    }
    bool operator<=(const BigIntHex &b) const {
        return !(*this > b);
    }
    bool operator>=(const BigIntHex &b) const {
        return !(*this < b);
    }
    bool operator==(const BigIntHex &b) const {
        if (size() == 1 && b.size() == 1 && v[0] == 0 && b.v[0] == 0)
            return true;
        if (sign != b.sign)
            return false;
        return raw_eq(b);
    }
    bool operator!=(const BigIntHex &b) const {
        return !(*this == b);
    }

    BigIntHex &operator=(int64_t n) {
        set(n);
        return *this;
    }
    BigIntHex operator+(const BigIntHex &b) const {
        BigIntHex r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntHex &operator+=(const BigIntHex &b) {
        BigIntHex &r = *this;
        if (sign * b.sign > 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }

    BigIntHex operator-(const BigIntHex &b) const {
        BigIntHex r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }
    BigIntHex &operator-=(const BigIntHex &b) {
        BigIntHex &r = *this;
        if (sign * b.sign < 0) {
            r.raw_add(b);
        } else {
            r.raw_sub(b);
        }
        return r;
    }

    BigIntHex operator-() const {
        BigIntHex r = *this;
        r.sign *= -1;
        return r;
    }

    BigIntHex operator*(const BigIntHex &b) const {
        if (b.size() == 1) {
            BigIntHex r = *this;
            r.raw_mul_int((uint32_t)b.v[0]);
            r.sign *= b.sign;
            return r;
        } else {
            BigIntHex r;
            r.raw_mul(*this, b);
            r.sign = sign * b.sign;
            return r;
        }
    }

    BigIntHex &operator*=(const BigIntHex &b) {
        if (b.size() == 1) {
            raw_mul_int((uint32_t)b.v[0]);
            sign *= b.sign;
            return *this;
        } else {
            BigIntHex r = *this;
            raw_mul(r, b);
            sign = r.sign * b.sign;
            return *this;
        }
    }

    BigIntHex operator*(int32_t b) const {
        return *this * BigIntHex().set(b);
    }

    BigIntHex &operator*=(int16_t b) {
        if (b >= 0)
            raw_mul_int((uint32_t)b);
        else {
            raw_mul_int((uint32_t)-b);
            sign = -sign;
        }
        return *this;
    }

    BigIntHex &operator*=(int32_t b) {
        if (b < 0x7fff && -0x7fff < b)
            return *this *= (int16_t)b;
        return *this *= BigIntHex().set(b);
    }

    BigIntHex operator/(const BigIntHex &b) const {
        BigIntHex r;
        r.raw_div(*this, b);
        r.sign = sign * b.sign;
        return r;
    }

    BigIntHex &operator/=(const BigIntHex &b) {
        BigIntHex r = *this;
        raw_div(r, b);
        sign = r.sign * b.sign;
        return *this;
    }

    BigIntHex operator%(const BigIntHex &b) const {
        BigIntHex r;
        r.raw_mod(*this, b);
        return r;
    }

    BigIntHex &operator%=(const BigIntHex &b) {
        BigIntHex r = *this;
        raw_mod(r, b);
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
        if (sign < 0 && *this != BigIntHex().set(0))
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
        if (sign < 0 && *this != BigIntHex().set(0))
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
    }

    std::string to_str(int32_t out_base = 10, int32_t pack = 0) const {
        if (out_base == 16) {
            return out_hex();
        }
        if (v.size() < 64) {
            return out_mul(out_base, pack);
        }
        if (sign < 0) {
            BigIntHex a = *this;
            a.sign = 1;
            return "-" + a.to_str(out_base);
        }
        BigIntHex b;
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
