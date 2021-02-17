#define _CRT_SECURE_NO_WARNINGS
#define NTT_MODE 0

#include "bigint_dec.h"
#include "bigint_hex.h"
#include "bigint_mini.h"
#include "bigint_tiny.h"

#include <iostream>
#include <random>

using namespace std;

typedef double time_point;

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

time_point get_time() {
#ifdef _WIN32
    return clock() * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

double get_time_diff(time_point from, time_point to) {
    return (double)(to - from);
}

bool test1_parse() {
    BigIntHex ha;
    BigIntDec hb;
    BigIntMini hc;
    BigIntTiny hd;
    struct {
        const char *p;
        int base;
    } in[] = {
        {"0", 10},
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
        if (in[i].base != 10)
            continue;
        hc.from_str(in[i].p);
        if (hc.to_str() != in[i].p) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
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
    } in[] = {
        {"0", "0", "0", 2},
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
        if (in[i].base != 10)
            continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 + hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
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
    } in[] = {
        {"0", "0", "0", 2},
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
        if (in[i].base != 10)
            continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 - hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
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
        {"1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624", "2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", 10},
        {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 *= ha2;
        std::string s = ha1.to_str(in[i].base);
        std::string s16 = ha1.to_str(16);
        std::string sa = ha2.from_str(in[i].pa, in[i].base).to_str(16);
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
        if (in[i].base != 10)
            continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 * hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
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
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746623", 10},
        {"", "", "", 0}};
    for (int i = 0; in[i].base; ++i) {
        ha1.from_str(in[i].p1, in[i].base);
        ha2.from_str(in[i].p2, in[i].base);
        ha1 /= ha2;
        if (ha1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        hb1.from_str(in[i].p1, in[i].base);
        hb2.from_str(in[i].p2, in[i].base);
        hb1 /= hb2;
        if (hb1.to_str(in[i].base) != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
        hc1.from_str(in[i].p1);
        hc2.from_str(in[i].p2);
        hc1 = hc1 / hc2;
        if (hc1.to_str() != in[i].pa) {
            return false;
        }
    }
    for (int i = 0; in[i].base; ++i) {
        if (in[i].base != 10)
            continue;
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
    } in[] = {
        {"10101", "1011", "1010", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "0", 10},
        {"12764250310913255140835516279358765904435124464", "7897879879879797897897979879", "0", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", "0", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136381746624", 10},
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
        if (in[i].base != 10)
            continue;
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
    } in[] = {
        {"1111001", "1011", 2},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", 10},
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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 15);
    std::uniform_int_distribution<> distribf(1, 15);
    char chars[] = "0123456789ABCDEF";
    for (int i = 0; i < ncase; ++i) {
        sa = chars[distribf(gen)];
        for (int j = 0; j < len; ++j)
            sa += chars[distrib(gen)];
        sb = chars[distribf(gen)];
        for (int j = 0; j < len; ++j)
            sb += chars[distrib(gen)];
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

bool test_factorial() {
    BigIntHex ha;
    BigIntDec hb;
    BigIntMini hc;
    BigIntTiny hd;
    string s;
    int fac = 10000;

    time_point t_beg, t_end, t_out;

    t_beg = get_time();
    ha = 1;
    for (int i = 2; i <= fac; ++i) {
        ha *= i;
    }
    t_end = get_time();
    s = ha.to_str();
    t_out = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by hex: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        trans to dec: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;
    cout << "        total " << ha.to_str(16).size() << " hex digits" << endl;

    t_beg = get_time();
    hb = 1;
    for (int i = 2; i <= fac; ++i) {
        hb *= i;
    }
    t_end = get_time();
    s = hb.to_str(16);
    t_out = get_time();
    //cout << "calc " << fac << "!" << endl;
    cout << "    by dec: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        trans to hex: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " hex digits" << endl;
    cout << "        total " << hb.to_str().size() << " dec digits" << endl;

    t_beg = get_time();
    hc = 1;
    for (int i = 2; i <= fac; ++i) {
        hc *= i;
    }
    t_end = get_time();
    s = hc.to_str();
    t_out = get_time();
    //cout << "calc " << fac << "!" << endl;
    cout << "    by mini: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    t_beg = get_time();
    hd = 1;
    for (int i = 2; i <= fac; ++i) {
        hd = hd * i;
    }
    t_end = get_time();
    s = hd.to_str();
    t_out = get_time();
    //cout << "calc " << fac << "!" << endl;
    cout << "    by tiny: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;
    return true;
}

bool test_bigmul() {
    BigIntHex ha;
    BigIntDec hb;
    BigIntMini hc;
    BigIntTiny hd;
    string s;
    int times = 20;

    time_point t_beg, t_end;

    t_beg = get_time();
    ha = 3;
    for (int i = 1; i <= times; ++i) {
        ha *= ha;
    }
    t_end = get_time();
    s = ha.to_str(16);
    cout << "calc 3^2^" << times << endl;
    cout << "    by hex : " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " hex digits" << endl;

    t_beg = get_time();
    hb = 3;
    for (int i = 1; i <= times; ++i) {
        hb *= hb;
    }
    t_end = get_time();
    s = hb.to_str();
    //cout << "calc 3^2^" << times << endl;
    cout << "    by dec : " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    times = 18;
    t_beg = get_time();
    hc = 3;
    for (int i = 1; i <= times; ++i) {
        hc = hc * hc;
    }
    t_end = get_time();
    s = hc.to_str();
    cout << "calc 3^2^" << times << endl;
    cout << "    by mini: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    t_beg = get_time();
    hd = 3;
    for (int i = 1; i <= times; ++i) {
        hd = hd * hd;
    }
    t_end = get_time();
    s = hc.to_str();
    //cout << "calc 3^2^" << times << endl;
    cout << "    by tiny: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;
    return true;
}

bool test_bigdiv() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntMini hc1, hc2;
    BigIntTiny hd1, hd2;
    string s, sa, sb;
    int times = 17;

    time_point t_pre, t_beg, t_end;

    ha2 = 3;
    for (int i = 1; i <= times; ++i) {
        ha2 *= ha2;
    }
    t_pre = get_time();
    ha1 = ha2 * ha2;
    t_beg = get_time();
    ha1 /= ha2;
    t_end = get_time();
    if (ha1 != ha2) {
        return false;
    }
    cout << "calc 3^2^" << times + 1 << " / 3^2^" << times << endl;
    cout << "    by hex : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;

    hb2 = 3;
    for (int i = 1; i <= times; ++i) {
        hb2 *= hb2;
    }
    t_pre = get_time();
    hb1 = hb2 * hb2;
    t_beg = get_time();
    hb1 /= hb2;
    t_end = get_time();
    if (hb1 != hb2) {
        return false;
    }
    //cout << "calc 3^2^" << times + 1 << " / 3^2^" << times << endl;
    cout << "    by dec : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;

    hc2 = 3;
    for (int i = 1; i <= times; ++i) {
        hc2 = hc2 * hc2;
    }
    t_pre = get_time();
    hc1 = hc2 * hc2;
    t_beg = get_time();
    hc1 = hc1 / hc2;
    t_end = get_time();
    if (!(hc1 == hc2)) {
        return false;
    }
    //cout << "calc 3^2^" << times + 1 << " / 3^2^" << times << endl;
    cout << "    by mini: mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;

    hd2 = 3;
    for (int i = 1; i <= times; ++i) {
        hd2 = hd2 * hd2;
    }
    t_pre = get_time();
    hd1 = hd2 * hd2;
    t_beg = get_time();
    hd1 = hd1 / hd2;
    t_end = get_time();
    if (!(hd1 == hd2)) {
        return false;
    }
    //cout << "calc 3^2^" << times + 1 << " / 3^2^" << times << endl;
    cout << "    by tiny: mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;

    return true;
}

bool test_bigdivrnd(int len1, int len2 = 0) {
    time_point t_pre, t_beg, t_end;
    string sa, sb;
    std::random_device rd;
    std::mt19937 gen(rd());
    char chars[] = "0123456789ABCDEF";
    if (len2 == 0) {
        len2 = len1;
    }
    for (int i = 0; i < 1; ++i) {
        {
            BigIntHex ha1, ha2, ha3;
            std::uniform_int_distribution<> distrib(0, 7);
            std::uniform_int_distribution<> distribf(1, 7);
            sa = chars[distribf(gen)];
            for (int j = 1; j < len1 * 5; ++j)
                sa += chars[distrib(gen)];
            sb = chars[distribf(gen)];
            for (int j = 1; j < len2 * 5; ++j)
                sb += chars[distrib(gen)];
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
            cout << "calc " << sa.size() << "digits * " << sb.size() << "digits , " << s1.size() << "digits / " << sa.size() << "digits (OCT)" << endl;
            cout << "    by hex : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
        }
        {
            BigIntDec hb1, hb2, hb3;
            std::uniform_int_distribution<> distrib(0, 9);
            std::uniform_int_distribution<> distribf(1, 9);
            sa = chars[distribf(gen)];
            for (int j = 1; j < len1 * 4; ++j)
                sa += chars[distrib(gen)];
            sb = chars[distribf(gen)];
            for (int j = 1; j < len2 * 4; ++j)
                sb += chars[distrib(gen)];
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
            cout << "calc " << sa.size() << "digits * " << sb.size() << "digits , " << s1.size() << "digits / " << sa.size() << "digits (DEC)" << endl;
            cout << "    by dec : mul: " << (int32_t)(get_time_diff(t_pre, t_beg) / 1000) << " ms , div: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
        }
    }
    return true;
}

int main() {
    bool pass = true;
    cout << "test1_parse : " << ((pass = test1_parse()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test2_add   : " << ((pass = test2_add()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test3_sub   : " << ((pass = test3_sub()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test4_mul   : " << ((pass = test4_mul()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test5_div   : " << ((pass = test5_div()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test6_mod   : " << ((pass = test6_mod()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test7_sqrt  : " << ((pass = test7_sqrt()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    cout << "test8_rnddiv: " << ((pass = test8_rnd_div(10, 128)) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
#ifndef _DEBUG
    test_factorial();
    test_bigmul();
#endif
    pass = test_bigdiv();
    if (!pass) {
        cout << "test_bigdiv FAIL" << endl;
        return -1;
    }
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
    return 0;
}
