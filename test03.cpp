
#include "bigint_tiny.h"

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

bool test_rnd_div() {
    BigIntTiny h1, h2;
    srand((unsigned)time(0));
    for (int i = 0; i < 30; ++i) {
        h1 = rand();
        for (int j = 0; j < 16; ++j)
            h1 += h1 * RAND_MAX + rand();
        h2 = rand();
        for (int j = 0; j < 16; ++j)
            h2 += h2 * RAND_MAX + rand();
        string s1 = (h1 * h2).to_str(), s2 = h1.to_str(), s3 = h2.to_str();

        h1 = s1;
        h2 = s2;
        h1 = h1 / h2;
        if (h1.to_str() != s3) {
            cout << "DEC: " << s1 << " / " << s2 << " = " << s3 << " out: " << h1.to_str() << endl;
            return false;
        }
    }
    return true;
}

bool test_factorial() {
    BigIntTiny h;
    string s;
    int fac = 10000;

    time_point t_beg, t_end, t_out;

    t_beg = get_time();
    h = 1;
    for (int i = 2; i <= fac; ++i) {
        h = h * i;
    }
    t_end = get_time();
    s = h.to_str();
    t_out = get_time();
    cout << "calc " << fac << "!" << endl;
    cout << "    by tiny: " << (int)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        to_str: " << (int)(get_time_diff(t_end, t_out) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    return true;
}

bool test_bigmul() {
    BigIntTiny h;
    string s;
    int times = 18;

    time_point t_beg, t_end;

    t_beg = get_time();
    h = 2;
    for (int i = 1; i <= times; ++i) {
        h = h * h;
    }
    t_end = get_time();
    s = h.to_str();
    cout << "calc 2^2^" << times << endl;
    cout << "    by tiny: " << (int)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    return true;
}

bool test_bigdiv() {
    BigIntTiny ha1, ha2, a, b;
    string s, sa, sb;
    int times = 17;

    time_point t_beg, t_end;

    a = 2;
    for (int i = 1; i <= times; ++i) {
        a = a * a;
    }
    b = a * a;
    sa = b.to_str();
    sb = a.to_str();

    ha1 = sa;
    ha2 = sb;
    t_beg = get_time();
    ha1 = ha1 / ha2;
    t_end = get_time();
    s = ha1.to_str();
    cout << "calc 2^2^" << times + 1 << " / 2^2^" << times << endl;
    cout << "    by tiny: " << (int)(get_time_diff(t_beg, t_end) / 1000) << " ms" << endl;
    cout << "        total " << s.size() << " dec digits" << endl;

    return true;
}

int main() {
    cout << "test_rnddiv: " << (test_rnd_div() ? "pass" : "FAIL") << endl;
    test_factorial();
    test_bigmul();
    test_bigdiv();
    return 0;
}
