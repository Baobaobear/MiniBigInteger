#define _CRT_SECURE_NO_WARNINGS

#include "bigint_mini.h"
#include "bigint_tiny.h"

#include "bigint_dec.h"
#include "bigint_hex.h"

#include <iostream>

using namespace std;

typedef double time_point;

#ifdef _WIN32
#include <time.h>
#include <windows.h>
#undef min
#undef max
static LARGE_INTEGER freq, beg;
struct init_timer {
    init_timer() {
        ::QueryPerformanceFrequency(&freq);
        ::QueryPerformanceCounter(&beg);
    }
} _;
#else
#include <sys/time.h>
#endif

time_point get_time() {
#ifdef _WIN32
    LARGE_INTEGER cur;
    ::QueryPerformanceCounter(&cur);
    return (double)(cur.QuadPart - beg.QuadPart) / freq.QuadPart * 1000000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

double get_time_diff(time_point from, time_point to) { return (double)(to - from); }

template <typename _res_type, uint64_t ret_max = ((uint64_t)0 - 1)> struct RNG_base {
    typedef _res_type result_type;
    static result_type min() { return 0; }
    static result_type max() { return (result_type)ret_max; }
    static const uint64_t seed_mul = ((uint64_t)0x5851F42D << 32) | 0x4C957F2D;
    static const int init_iter = 16;
    uint64_t new_seed() { return time(0); }
    uint64_t def_seed() {
        static uint64_t seed = new_seed();
        seed *= ((uint64_t)0x369DEA0F << 32) | 0x31A53F85;
        return ++seed;
    }
    static inline uint64_t rotl64(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
    static inline uint64_t rotr64(const uint64_t x, int k) { return (x >> k) | (x << (64 - k)); }
    static inline uint32_t rotl32(const uint32_t x, int k) { return (x << k) | (x >> (32 - k)); }
    static inline uint32_t rotr32(const uint32_t x, int k) { return (x >> k) | (x << (32 - k)); }
    static inline uint16_t rotl16(const uint16_t x, int k) { return (x << k) | (x >> (16 - k)); }
    static inline uint16_t rotr16(const uint16_t x, int k) { return (x >> k) | (x << (16 - k)); }
};

struct PCG32 : public RNG_base<uint32_t> // XSH RR
{
    static const int init_iter = 1; // min 1
    uint64_t s;
    PCG32() { seed(); }
    PCG32(uint64_t seed1) { seed(seed1); }
    void seed() { seed(def_seed()); }
    void seed(uint64_t seed1) {
        s = seed1;
        for (int i = 0; i < init_iter; ++i)
            (*this)();
    }
    result_type operator()() {
        uint64_t x = s;
        s = s * (((uint64_t)0x5851F42D << 32) | 0x4C957F2D) + (((uint64_t)0x14057B7E << 32) | 0xF767814F);
        return rotr32((uint32_t)((x ^ (x >> 18)) >> 27), x >> 59);
    }
};

template <class _Eg> static uint64_t i64_distribution(_Eg &eg, uint64_t n) {
    ++n;
    if ((n & (uint64_t) - (int64_t)n) == n) return (int64_t)eg() & (n - 1);
    uint64_t m = n;
    while (m & (m - 1))
        m += m & (uint64_t) - (int64_t)m;
    --m;
    for (;;) {
        uint64_t r = (uint64_t)eg() & m;
        if (r < n) return r;
    }
}

int randint(int a, int b) {
    static PCG32 engine;
    return (int)i64_distribution(engine, b - a) + a;
    // return rand() % (b - a + 1) + a;
}

bool test1_parse() {
    BigIntHex ha;
    BigIntDec hb;
    BigIntMini hc;
    BigIntTiny hd;
    struct {
        const char *p;
        int base;
    } in[] = {{"0", 10},
              {"-1", 2},
              {"1234567890123456789", 10},
              {"-12345678901234567890", 10},
              {"123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 36},
              {"-123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 36},
              {"", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha.from_str(in[i].p, in[i].base);
        if (ha.to_str(in[i].base) != in[i].p) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb.from_str(in[i].p, in[i].base);
        if (hb.to_str(in[i].base) != in[i].p) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc.from_str(in[i].p);
        if (hc.to_str() != in[i].p) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hd = in[i].p;
        if (hd.to_str() != in[i].p) {
            return false;
        }
    }
    return true;
}

bool test2_add() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    BigIntTiny hd1, hd2;
    struct {
        const char *p1;
        const char *p2;
        const char *pa;
        int base;
    } in[] = {{"0", "0", "0", 2},
              {"10101", "1011", "100000", 2},
              {"0", "123456789", "123456789", 10},
              {"123456789", "123456789", "246913578", 10},
              {"-123456789", "-123456789", "-246913578", 10},
              {"123456789", "-123456789", "0", 10},
              {"-123456789", "123456789", "0", 10},
              {"9999999999999999", "1111111111111111", "11111111111111110", 10},
              {"9999999999999999", "-1111111111111111", "8888888888888888", 10},
              {"-9999999999999999", "1111111111111111", "-8888888888888888", 10},
              {"-9999999999999999", "-1111111111111111", "-11111111111111110", 10},
              {"-9999999999999999", "-1111111111111111", "-AAAAAAAAAAAAAAAA", 16},
              {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 += ha2;
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 += hb2;
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 + hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hd1 = in[i].p1;
        hd2 = in[i].p2;
        hd1 = hd1 + hd2;
        if (hd1.to_str() != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test3_sub() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    BigIntTiny hd1, hd2;
    struct {
        const char *p1;
        const char *p2;
        const char *pa;
        int base;
    } in[] = {{"0", "0", "0", 2},
              {"10101", "1011", "1010", 2},
              {"0", "123456789", "-123456789", 10},
              {"123456789", "123456789", "0", 10},
              {"100000000000000000000", "1", "99999999999999999999", 10},
              {"-123456789", "-123456789", "0", 10},
              {"123456789", "-123456789", "246913578", 10},
              {"-123456789", "123456789", "-246913578", 10},
              {"9999999999999999", "1111111111111111", "8888888888888888", 10},
              {"9999999999999999", "-1111111111111111", "11111111111111110", 10},
              {"-9999999999999999", "1111111111111111", "-11111111111111110", 10},
              {"-9999999999999999", "-1111111111111111", "-8888888888888888", 10},
              {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 -= ha2;
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 -= hb2;
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 - hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hd1 = in[i].p1;
        hd2 = in[i].p2;
        hd1 = hd1 - hd2;
        if (hd1.to_str() != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test4_mul() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    BigIntTiny hd1, hd2;
    struct {
        const char *p1;
        const char *p2;
        const char *pa;
        int base;
    } in[] = {
        {"0", "0", "0", 2},
        {"10101", "1011", "11100111", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "15241578750190521", 10},
        {"100001", "123456789", "12345802356789", 10},
        {"100001", "123456789123456789", "12345802369134802356789", 10},
        {"1616161616161161616", "7897879879879797897897979879", "12764250310913255140835516279358765904435124464", 10},
        {"1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624",
         "2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", 10},
        {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 *= ha2;
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 *= hb2;
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 * hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hd1 = in[i].p1;
        hd2 = in[i].p2;
        hd1 = hd1 * hd2;
        if (hd1.to_str() != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test5_div() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    BigIntTiny hd1, hd2;
    struct {
        const char *p1;
        const char *p2;
        const char *pa;
        int base;
    } in[] = {
        {"10101", "1011", "1", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "1", 10},
        {"12764250310913255140835516279358765904435124464", "7897879879879797897897979879", "1616161616161161616", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376",
         "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376",
         "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746623", 10},
        {"99999999999999999999999999999999999999999999999999999999", "999999999999999900000000",
         "100000000000000010000000000000001", 10},
        {"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", "3FFFFFFFFFFFFFFC0000000",
         "4000000000000004000000000000004000000", 16},
        {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 /= ha2;
        string s = ha1.to_str(in[i].base);
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 /= hb2;
        string s = hb1.to_str(in[i].base);
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 / hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hd1 = in[i].p1;
        hd2 = in[i].p2;
        hd1 = hd1 / hd2;
        if (hd1.to_str() != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test6_mod() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    struct {
        const char *p1;
        const char *p2;
        const char *pa;
        int base;
    } in[] = {{"10101", "1011", "1010", 2},
              {"-0", "123456789", "0", 10},
              {"123456789", "123456789", "0", 10},
              {"12764250310913255140835516279358765904435124464", "7897879879879797897897979879", "0", 10},
              {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376",
               "1427247692705959881058285969449495136382746624", "0", 10},
              {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376",
               "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136381746624", 10},
              {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 %= ha2;
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 %= hb2;
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10) continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 % hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test7_sqrt() {
    BigIntHex ha, hal, har, ham;
    BigIntDec hb, hbl, hbr, hbm;
    struct {
        const char *p1;
        const char *pa;
        int base;
    } in[] = {{"1111001", "1011", 2},
              {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376",
               "1427247692705959881058285969449495136382746624", 10},
              {"", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha.from_str(in[i].p1, in[i].base);
        hal = 1;
        har = ha;
        while (hal < har) {
            ham = (hal + har + BigIntHex().set(1)) / BigIntHex().set(2);
            if (ham * ham > ha) {
                har = ham - BigIntHex().set(1);
            } else {
                hal = ham;
            }
        }
        if (hal.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb.from_str(in[i].p1, in[i].base);
        hbl = 1;
        hbr = hb;
        while (hbl < hbr) {
            hbm = (hbl + hbr + BigIntDec().set(1)) / BigIntDec().set(2);
            if (hbm * hbm > hb) {
                hbr = hbm - BigIntDec().set(1);
            } else {
                hbl = hbm;
            }
        }
        if (hbl.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    return true;
}

bool test8_rnd_div(int ncase, int len) {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    string sa, sb;
    char chars[] = "0123456789ABCDEF";
    for (int i = 0; i < ncase; ++i) {
        sa = chars[randint(1, 15)];
        for (int j = 0; j < len; ++j)
            sa += chars[randint(0, 15)];
        sb = chars[randint(1, 15)];
        for (int j = 0; j < len; ++j)
            sb += chars[randint(0, 15)];
        ha1.from_str(sa, 16);
        ha2.from_str(sb, 16);
        string s1 = (ha1 * ha2).to_str(), s2 = ha1.to_str(), s3 = ha2.to_str();

        ha1.from_str(s1.c_str());
        ha2.from_str(s2.c_str());
        ha1 /= ha2;
        if (ha1.to_str() != s3) {
            cout << "HEX: " << s1 << " / " << s2 << " = " << s3 << " out: " << ha1.to_str() << endl;
            return false;
        }

        hb1.from_str(s1.c_str());
        hb2.from_str(s2.c_str());
        hb1 /= hb2;
        if (hb1.to_str() != s3) {
            cout << "DEC: " << s1 << " / " << s2 << " = " << s3 << " out: " << hb1.to_str() << endl;
            return false;
        }

        hc1.from_str(s1.c_str());
        hc2.from_str(s2.c_str());
        hc1 = hc1 / hc2;
        if (hc1.to_str() != s3) {
            cout << "mini: " << s1 << " / " << s2 << " = " << s3 << " out: " << hc1.to_str() << endl;
            return false;
        }
    }
    return true;
}

template <typename BigIntT> BigIntT fast_pow(int base, int exp) {
    BigIntT r(1), b(base);
    for (; exp; exp >>= 1) {
        if (exp & 1) {
            r = r * b;
            if (exp == 1) break;
        }
        b = b * b;
    }
    return r;
}

template <typename BigIntT> BigIntT Product(int n, BigIntT &currentN) {
    int m = n / 2;
    if (m == 0) return currentN = currentN + BigIntT(2);
    if (n == 2) {
        BigIntT t = currentN + BigIntT(2);
        currentN = currentN + BigIntT(4);
        return t * currentN;
    }
    return Product(n - m, currentN) * Product(m, currentN);
}

template <typename BigIntT> BigIntT split_factorial(int n) {
    if (n < 2) return BigIntT(1);
    BigIntT p(1), r(1), s(1), currentN(1);

    int h = 0, shift = 0, high = 1, log2n = 0;

    for (int i = 0; i < 31; ++i) {
        if ((1 << i) > n) {
            log2n = i - 1;
            break;
        }
    }

    while (h != n) {
        shift += h;
        h = n >> log2n--;
        int len = high;
        high = (h - 1) | 1;
        len = (high - len) / 2;
        if (len > 0) {
            p = p * Product(len, currentN);
            r = r * p;
        }
    }
    return r * fast_pow<BigIntT>(2, shift);
}

template <typename BigIntT> BigIntT calc_factorial(int fac) { return split_factorial<BigIntT>(fac); }

bool test_factorial() {
    BigIntHex ha;
    BigIntDec hb;
    BigIntMini hc;
    BigIntTiny hd;
    string s, sa, sb;
    int fac = 50000;

    time_point t_beg, t_end, t_out, t_load;

    t_beg = get_time();
    ha = calc_factorial<BigIntHex>(fac);
    t_end = get_time();
    sa = ha.to_str();
    t_out = get_time();
    ha.from_str(sa);
    t_load = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by hex: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        trans to dec: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "        read from dec: " << (int32_t)(get_time_diff(t_out, t_load) / 1000) << " ms" << endl;
    cout << "        total " << sa.size() << " dec digits" << endl;
    cout << "        total " << ha.to_str(16).size() << " hex digits" << endl;

    t_beg = get_time();
    hb = calc_factorial<BigIntDec>(fac);
    t_end = get_time();
    sb = hb.to_str(16);
    t_out = get_time();
    hb.from_str(sb, 16);
    t_load = get_time();
    // cout << "calc " << fac << "!" << endl;
    cout << "    by dec: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        trans to hex: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "        read from hex: " << (int32_t)(get_time_diff(t_out, t_load) / 1000) << " ms" << endl;
    cout << "        total " << sb.size() << " hex digits" << endl;
    cout << "        total " << hb.to_str().size() << " dec digits" << endl;

    if (ha.to_str(16) != sb || hb.to_str() != sa) {
        return false;
    }

    fac = 20000;
    t_beg = get_time();
    hc = calc_factorial<BigIntMini>(fac);
    t_end = get_time();
    s = hc.to_str();
    t_out = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by mini: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    t_beg = get_time();
    hd = calc_factorial<BigIntTiny>(fac);
    t_end = get_time();
    s = hd.to_str();
    t_out = get_time();
    // cout << "calc " << fac << "!" << endl;
    cout << "    by tiny: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;
    return true;
}

bool test_bigdivrnd(int len1, int len2 = 0) {
    time_point t_pre, t_beg, t_end;
    string sa, sb;
    char chars[] = "0123456789ABCDEF";
    if (len2 == 0) {
        len2 = len1;
    }
    for (int i = 0; i < 1; ++i) {
        {
            BigIntHex ha1, ha2, ha3;
            sa = chars[randint(1, 7)];
            for (int j = 1; j < len1 * 5; ++j)
                sa += chars[randint(0, 7)];
            sb = chars[randint(1, 7)];
            for (int j = 1; j < len2 * 5; ++j)
                sb += chars[randint(0, 7)];
            ha2.from_str(sa, 8);
            ha3.from_str(sb, 8);
            string s1 = (ha2 * ha3).to_str(8);

            t_pre = get_time();
            ha1 = ha2 * ha3;
            t_beg = get_time();
            ha1 /= ha2;
            t_end = get_time();
            if (ha1 != ha3) {
                return false;
            }
            cout << "calc " << sa.size() << "digits * " << sb.size() << "digits , " << s1.size() << "digits / "
                 << sa.size() << "digits (OCT)" << endl;
            cout << "    by hex : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000)
                 << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
        }
        {
            BigIntDec hb1, hb2, hb3;
            sa = chars[randint(1, 9)];
            for (int j = 1; j < len1 * 4; ++j)
                sa += chars[randint(0, 9)];
            sb = chars[randint(1, 9)];
            for (int j = 1; j < len2 * 4; ++j)
                sb += chars[randint(0, 9)];
            hb2.from_str(sa, 10);
            hb3.from_str(sb, 10);
            string s1 = (hb2 * hb3).to_str(10);
            t_pre = get_time();
            hb1 = hb2 * hb3;
            t_beg = get_time();
            hb1 /= hb2;
            t_end = get_time();
            if (hb1 != hb3) {
                return false;
            }
            cout << "calc " << sa.size() << "digits * " << sb.size() << "digits , " << s1.size() << "digits / "
                 << sa.size() << "digits (DEC)" << endl;
            cout << "    by dec : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000)
                 << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
        }
    }
    return true;
}

template <class BIG> void test_efficiency(string classname) {
    char chars[] = "0123456789ABCDEF";
    string sa, sb;
    int firststep = 256, step = firststep / 2;
    double timelimit = 2e5;
    cout << classname << ":" << endl;
    for (int i = firststep;; i += step, (i & (i - 1)) == 0 ? step *= 2 : 0) {
        double diff;
        BIG a1, a2, a3;
        sa = chars[randint(1, 9)];
        for (int j = 1; j < i; ++j)
            sa += chars[randint(0, 9)];
        sb = chars[randint(1, 9)];
        for (int j = 1; j < i; ++j)
            sb += chars[randint(0, 9)];
        a2 = sa.c_str();
        a3 = sb.c_str();
        time_point t_beg, t_end;
        t_beg = get_time();
        int cnt = 1;
        for (;; ++cnt) {
            a1 = a2 * a3;
            t_end = get_time();
            if ((diff = get_time_diff(t_beg, t_end)) > timelimit) break;
        }
        cout << "    mul " << i << " digits: " << diff / cnt / sa.size() << " us/digits" << endl;
        if (cnt <= 4) break;
    }
    step = firststep / 2;
    for (int i = firststep;; i += step, (i & (i - 1)) == 0 ? step *= 2 : 0) {
        double diff;
        BIG a1, a2, a3;
        sa = chars[randint(1, 9)];
        for (int j = 1; j < i; ++j)
            sa += chars[randint(0, 9)];
        sb = chars[randint(1, 9)];
        for (int j = 1; j < i; ++j)
            sb += chars[randint(0, 9)];
        a2 = sa.c_str();
        a3 = sb.c_str();
        a1 = a2 * a3;
        time_point t_beg, t_end;
        t_beg = get_time();
        int cnt = 1;
        for (;; ++cnt) {
            a3 = a1 / a2;
            t_end = get_time();
            if ((diff = get_time_diff(t_beg, t_end)) > timelimit) break;
        }
        cout << "    div " << i << " digits: " << diff / cnt / sa.size() << " us/digits" << endl;
        if (cnt <= 4) break;
    }
}

int main() {
    bool pass = true;
    cout << "test1_parse : " << ((pass = test1_parse()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test2_add   : " << ((pass = test2_add()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test3_sub   : " << ((pass = test3_sub()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test4_mul   : " << ((pass = test4_mul()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test5_div   : " << ((pass = test5_div()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test6_mod   : " << ((pass = test6_mod()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test7_sqrt  : " << ((pass = test7_sqrt()) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    cout << "test8_rnddiv: " << ((pass = test8_rnd_div(10, 256)) ? "pass" : "FAIL") << endl;
    if (!pass) return -1;
    pass = test_bigdivrnd(1 << 14, 1 << 11);
    if (!pass) {
        cout << "test_bigdivrnd FAIL" << endl;
        return -1;
    }
    pass = test_bigdivrnd(1 << 11, 1 << 14);
    if (!pass) {
        cout << "test_bigdivrnd FAIL" << endl;
        return -1;
    }
#ifndef _DEBUG
    pass = test_bigdivrnd(1 << 17, 1 << 16);
    if (!pass) {
        cout << "test_bigdivrnd FAIL" << endl;
        return -1;
    }
    pass = test_bigdivrnd(1 << 17);
    if (!pass) {
        cout << "test_bigdivrnd FAIL" << endl;
        return -1;
    }
    pass = test_factorial();
    if (!pass) {
        cout << "test_factorial FAIL" << endl;
        return -1;
    }
#endif
    test_efficiency<BigIntHex>("BigIntHex");
    test_efficiency<BigIntDec>("BigIntDec");
    test_efficiency<BigIntMini>("BigIntMini");
    test_efficiency<BigIntTiny>("BigIntTiny");
    return 0;
}
