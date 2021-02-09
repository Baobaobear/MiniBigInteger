#include "bigint.h"
#include "bigint_dec.h"
#include "bigint_decmini.h"

#include <iostream>

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
    BigIntM hc;
    struct {
        const char *p;
        int base;
    } in[] = {
        {"0", 10},
        {"-1", 2},
        {"12345678901234567890", 10},
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
    return true;
}

bool test2_add() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
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
    return true;
}

bool test3_sub() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
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
    return true;
}

bool test4_mul() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
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
    return true;
}

bool test5_div() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
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
    return true;
}

bool test6_mod() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
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

bool test8_rnd_div() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    BigIntM hc1, hc2;
    srand((unsigned)time(0));
    for (int i = 0; i < 100; ++i) {
        hb1 = rand();
        for (int j = 0; j < 16; ++j)
            hb1 += hb1 * RAND_MAX + BigIntDec().set(rand());
        hb2 = rand();
        for (int j = 0; j < 16; ++j)
            hb2 += hb2 * RAND_MAX + BigIntDec().set(rand());
        string s1 = (hb1 * hb2).to_str(), s2 = hb1.to_str(), s3 = hb2.to_str();

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
    string s;
    int fac = 10000;

    time_point t_beg, t_end, t_out;

    t_beg = get_time();
    ha.set(1);
    for (int i = 2; i <= fac; ++i) {
        ha *= i;
    }
    t_end = get_time();
    s = ha.to_str();
    t_out = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by hex: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "    trans to dec: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "    total " << s.size() << " dec digits" << endl;

    t_beg = get_time();
    hb.set(1);
    for (int i = 2; i <= fac; ++i) {
        hb *= i;
    }
    t_end = get_time();
    s = hb.to_str(16);
    t_out = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by dec: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "    trans to hex: " << (int32_t)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "    total " << s.size() << " hex digits" << endl;
    return true;
}

bool test_bigmul() {
    BigIntHex ha;
    BigIntDec hb;
    string s;
    int times = 20;

    time_point t_beg, t_end;

    t_beg = get_time();
    ha.set(2);
    for (int i = 1; i <= times; ++i) {
        ha *= ha;
    }
    t_end = get_time();
    s = ha.to_str(16);
    cout << "calc 2^2^" << times << endl;
    cout << "    by hex: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "    total " << s.size() << " hex digits" << endl;

    t_beg = get_time();
    hb.set(2);
    for (int i = 1; i <= times; ++i) {
        hb *= hb;
    }
    t_end = get_time();
    s = hb.to_str();
    cout << "calc 2^2^" << times << endl;
    cout << "    by dec: " << (int32_t)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "    total " << s.size() << " dec digits" << endl;
    return true;
}

int main() {
    cout << "test1_parse : " << (test1_parse() ? "pass" : "FAIL") << endl;
    cout << "test2_add   : " << (test2_add() ? "pass" : "FAIL") << endl;
    cout << "test3_sub   : " << (test3_sub() ? "pass" : "FAIL") << endl;
    cout << "test4_mul   : " << (test4_mul() ? "pass" : "FAIL") << endl;
    cout << "test5_div   : " << (test5_div() ? "pass" : "FAIL") << endl;
    cout << "test6_mod   : " << (test6_mod() ? "pass" : "FAIL") << endl;
    cout << "test7_sqrt  : " << (test7_sqrt() ? "pass" : "FAIL") << endl;
    cout << "test8_rnddiv: " << (test8_rnd_div() ? "pass" : "FAIL") << endl;
    test_factorial();
    test_bigmul();
    return 0;
}
