// filename:    bigint_dec.h
// author:      baobaobear
// create date: 2021-02-08
// This library is compatible with C++03
// https://github.com/Baobaobear/MiniBigInteger
#pragma once
#include "bigint_base.h"

namespace BigIntDecNS {
#if BIGINT_LARGE_BASE
const int32_t COMPRESS_MOD = 100000000;
const uint32_t COMPRESS_DIGITS = 8;
const int32_t COMPRESS_HALF_MOD = 10000;
const uint32_t COMPRESS_HALF_DIGITS = 4;
#else
const int32_t COMPRESS_MOD = 10000;
const uint32_t COMPRESS_DIGITS = 4;
#endif

const uint32_t BIGINT_NTT_THRESHOLD = 256;
const uint32_t BIGINT_MUL_THRESHOLD = 40;
const uint32_t BIGINT_DIV_THRESHOLD = 2000;
const uint32_t BIGINT_DIVIDEDIV_THRESHOLD = 1000;

#ifdef NTT_DOUBLE_MOD
const uint32_t NTT_MAX_SIZE = 1 << 24;
#else
const uint32_t NTT_MAX_SIZE = 1 << 21;
#endif

template <typename T>
inline T high_digit(T digit) {
    return digit / COMPRESS_MOD;
}

template <typename T>
inline uint32_t low_digit(T digit) {
    return (uint32_t)(digit % COMPRESS_MOD);
}

class BigIntDec {
protected:
    typedef uint32_t base_t;
#if BIGINT_LARGE_BASE
    typedef int64_t carry_t;
#else
    typedef int32_t carry_t;
#endif
    int sign;
    std::vector<base_t> v;
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
            v[i] = low_digit(add);
            if (v[i] < COMPRESS_MOD) { // v[i] >= 0
                add = high_digit(add);
            } else {
                v[i] += (base_t)COMPRESS_MOD;
                add = high_digit(add) - 1;
            }
        }
        for (size_t i = b.v.size(); add && i < v.size(); i++) {
            add += v[i];
            v[i] = low_digit(add);
            if (v[i] < COMPRESS_MOD) { // v[i] >= 0
                add = high_digit(add);
            } else {
                v[i] += (base_t)COMPRESS_MOD;
                add = high_digit(add) - 1;
            }
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
#if BIGINT_INT64_OPTIMIZE
        int64_t add = 0;
        size_t i = 0, s = v.size() & ~1;
        for (; i < s; i += 2) {
            add += (int64_t)((v[i + 1] * COMPRESS_MOD) + v[i]) * m;
            v[i] = low_digit(add);
            v[i + 1] = low_digit(high_digit(add));
            add = high_digit(high_digit(add));
        }
        for (; i < v.size(); i++) {
            add += v[i] * (carry_t)m;
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        while (add) {
            v.push_back(low_digit(add));
            add = high_digit(add);
        }
#else
        carry_t add = 0;
        for (size_t i = 0; i < v.size(); i++) {
            add += v[i] * (carry_t)m;
            v[i] = low_digit(add);
            add = high_digit(add);
        }
        while (add) {
            v.push_back(low_digit(add));
            add = high_digit(add);
        }
#endif
        return *this;
    }
    BigInt_t &raw_mul(const BigInt_t &a, const BigInt_t &b) {
        v.clear();
        v.resize(a.size() + b.size());
#if BIGINT_INT64_OPTIMIZE
        size_t i = 0, as = a.size() & ~1;
        for (; i < a.size(); i += 1) {
            int64_t add = 0;
            int64_t av = a.v[i];
            size_t j = 0, bs = b.size() & ~1;
            for (; j < bs; j += 2) {
                add += v[i + j] + av * (int64_t)((b.v[j + 1] * COMPRESS_MOD) + b.v[j]);
                v[i + j] = low_digit(add);
                add = high_digit(add) + v[i + j + 1];
                v[i + j + 1] = low_digit(add);
                add = high_digit(add);
            }
            for (; j < b.size(); j++) {
                add += av * b.v[j];
                v[i + j] += low_digit(add);
                add = high_digit(add);
            }
            v[i + b.size()] += (base_t)add;
        }
#else
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
#endif
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
#if BIGINT_LARGE_BASE
        for (size_t i = 0; i < a.size(); ++i) {
            NTT_NS::ntt_a.push_back(a.v[i] % COMPRESS_HALF_MOD);
            NTT_NS::ntt_a.push_back(a.v[i] / COMPRESS_HALF_MOD);
        }
        for (size_t i = 0; i < b.size(); ++i) {
            NTT_NS::ntt_b.push_back(b.v[i] % COMPRESS_HALF_MOD);
            NTT_NS::ntt_b.push_back(b.v[i] / COMPRESS_HALF_MOD);
        }
        NTT_NS::ntt_prepare(a.size() * 2, b.size() * 2, len, 7);
        lenmul = 2;
#else
        for (size_t i = 0; i < a.size(); ++i) {
            NTT_NS::ntt_a.push_back(a.v[i]);
        }
        for (size_t i = 0; i < b.size(); ++i) {
            NTT_NS::ntt_b.push_back(b.v[i]);
        }
        NTT_NS::ntt_prepare(a.size(), b.size(), len, 7);
#endif
        NTT_NS::mul_conv2(len);
        len = (a.size() + b.size()) * lenmul;
        while (len > 0 && NTT_NS::ntt_a[--len] == 0)
            ;
        v.clear();
        int64_t add = 0;
#if BIGINT_LARGE_BASE
        for (size_t i = 0; i <= len; i += 2) {
            add += NTT_NS::ntt_a[i] + (NTT_NS::ntt_a[i + 1] * COMPRESS_HALF_MOD);
            v.push_back(low_digit(add));
            add = high_digit(add);
        }
#else
        for (size_t i = 0; i <= len; i++) {
            add += NTT_NS::ntt_a[i];
            v.push_back(low_digit(add));
            add = high_digit(add);
        }
#endif
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
        carry_t mul = ((carry_t)COMPRESS_MOD * COMPRESS_MOD - 1) / (*(b.v.begin() + b.v.size() - 1) * (carry_t)COMPRESS_MOD + *(b.v.begin() + b.v.size() - 2) + 1);
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
            return BIGINT_STD_MOVE(sum);
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
            for (; s < size() / 3; s *= 2, ++id) {
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
            return BIGINT_STD_MOVE(sum);
        }
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
            out.push_back('-');
        std::reverse(out.begin(), out.end());
        return out;
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
    BigInt_t &from_str_base10(const char *s) {
        v.clear();
        int32_t base = 10, sign = 1, digits = COMPRESS_DIGITS;
        const char *p = s + strlen(s) - 1;
        while (*s == '-')
            sign *= -1, ++s;
        while (*s == '0')
            ++s;

        int64_t d = digits, hdigit = 0, hdigit_mul = 1;
        for (; p >= s; p--) {
            hdigit += (*p - '0') * hdigit_mul;
            hdigit_mul *= base;
            if (--d == 0) {
                v.push_back((base_t)hdigit);
                d = digits;
                hdigit = 0;
                hdigit_mul = 1;
            }
        }
        if (hdigit || v.empty()) {
            v.push_back((base_t)hdigit);
        }
        this->sign = sign;
        return *this;
    }
    BigInt_t &_from_str(const std::string &s, int base) {
        if (s.size() <= 12) {
            int64_t v = 0;
            for (size_t i = 0; i < s.size(); ++i) {
                int digit = -1;
                if (s[i] >= '0' && s[i] <= '9')
                    digit = s[i] - '0';
                else if (s[i] >= 'A' && s[i] <= 'Z')
                    digit = s[i] - 'A' + 10;
                else if (s[i] >= 'a' && s[i] <= 'z')
                    digit = s[i] - 'a' + 10;
                v = v * base + digit;
            }
            return set(v);
        }
        BigInt_t m(base), h;
        size_t len = 1;
        for (; len * 3 < s.size(); len *= 2) {
            m *= m;
        }
        h._from_str(s.substr(0, s.size() - len), base);
        _from_str(s.substr(s.size() - len), base);
        *this += m * h;
        return *this;
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
    BigInt_t &from_str(const char *s, int base = 10) {
        if (base == 10) {
            return from_str_base10(s);
        }
        int vsign = 1, i = 0;
        while (s[i] == '-') {
            ++i;
            vsign = -vsign;
        }
        _from_str(std::string(s + i), base);
        sign = vsign;
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
        return BIGINT_STD_MOVE(r);
    }
    BigInt_t &operator+=(const BigInt_t &b) {
        if (this == &b) {
            BigInt_t c = b;
            return *this += c;
        }
        if (sign * b.sign > 0) {
            raw_add(b);
        } else {
            raw_sub(b);
        }
        return *this;
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
    BigInt_t &operator-=(const BigInt_t &b) {
        if (this == &b) {
            set(0);
            return *this;
        }
        if (sign * b.sign < 0) {
            raw_add(b);
        } else {
            raw_sub(b);
        }
        return *this;
    }
    BigInt_t operator-() const {
        BigInt_t r = *this;
        r.sign = -r.sign;
        return BIGINT_STD_MOVE(r);
    }

    BigInt_t operator*(const BigInt_t &b) const {
        if (b.size() == 1) {
            BigInt_t r = *this;
            r.raw_mul_int((uint32_t)b.v[0]);
            r.sign *= b.sign;
            return BIGINT_STD_MOVE(r);
        } else if (v.size() == 1) {
            BigInt_t r = b;
            r.raw_mul_int((uint32_t)v[0]);
            r.sign *= sign;
            return BIGINT_STD_MOVE(r);
        } else {
            BigInt_t r;
            r.raw_nttmul(*this, b);
            r.sign = sign * b.sign;
            return BIGINT_STD_MOVE(r);
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
    BigInt_t operator*(intmax_t b) const {
        return BIGINT_STD_MOVE(*this * BigInt_t().set(b));
    }
    BigInt_t &operator*=(intmax_t b) {
        if (b < COMPRESS_MOD && -(intmax_t)COMPRESS_MOD < b) {
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
        return BIGINT_STD_MOVE(d);
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
        return BIGINT_STD_MOVE(*this - *this / b * b);
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
        return BIGINT_STD_MOVE(d);
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
