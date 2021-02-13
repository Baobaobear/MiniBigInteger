#define _CRT_SECURE_NO_WARNINGS

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

int main() {
    bool pass = true;
    cout << "test_rnddiv: " << ((pass = test_rnd_div()) ? "pass" : "FAIL") << endl;
    if (!pass)
        return -1;
    return 0;
}
