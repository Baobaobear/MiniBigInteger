#include "bigint.h"

#include <iostream>

bool test1_parse() {
    BigIntHex ha;
    BigIntDec hb;
    struct {
        const char* p;
        int base;
    } in[] = {
        {"0", 2},
        {"-1", 2},
        {"12345678901234567890", 10},
        {"-12345678901234567890", 10},
        {"123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 36},
        {"-123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 36},
        {"",0}
    };
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
    return true;
}

bool test2_add() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    struct {
        const char* p1;
        const char* p2;
        const char* pa;
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
        {"", "", "", 0}
    };
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
    return true;
}

bool test3_sub() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    struct {
        const char* p1;
        const char* p2;
        const char* pa;
        int base;
    } in[] = {
        {"0", "0", "0", 2},
        {"10101", "1011", "1010", 2},
        {"0", "123456789", "-123456789", 10},
        {"123456789", "123456789", "0", 10},
        {"-123456789", "-123456789", "0", 10},
        {"123456789", "-123456789", "246913578", 10},
        {"-123456789", "123456789", "-246913578", 10},
        {"9999999999999999", "1111111111111111", "8888888888888888", 10},
        {"9999999999999999", "-1111111111111111", "11111111111111110", 10},
        {"-9999999999999999", "1111111111111111", "-11111111111111110", 10},
        {"-9999999999999999", "-1111111111111111", "-8888888888888888", 10},
        {"", "", "", 0}
    };
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
    return true;
}

bool test4_mul() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    struct {
        const char* p1;
        const char* p2;
        const char* pa;
        int base;
    } in[] = {
        {"0", "0", "0", 2},
        {"10101", "1011", "11100111", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "15241578750190521", 10},
        {"1616161616161161616", "7897879879879797897897979879", "12764250310913255140835516279358765904435124464", 10},
        {"1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624", "2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", 10},
        {"", "", "", 0}
    };
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
    return true;
}

bool test5_div() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    struct {
        const char* p1;
        const char* p2;
        const char* pa;
        int base;
    } in[] = {
        {"10101", "1011", "1", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "1", 10},
        {"12764250310913255140835516279358765904435124464", "7897879879879797897897979879", "1616161616161161616", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746624", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136382746623", 10},
        {"", "", "", 0}
    };
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
    return true;
}

bool test6_mod() {
    BigIntHex ha1, ha2;
    BigIntDec hb1, hb2;
    struct {
        const char* p1;
        const char* p2;
        const char* pa;
        int base;
    } in[] = {
        {"10101", "1011", "1010", 2},
        {"-0", "123456789", "0", 10},
        {"123456789", "123456789", "0", 10},
        {"12764250310913255140835516279358765904435124464", "7897879879879797897897979879", "0", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", "0", 10},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706182397376", "1427247692705959881058285969449495136382746624", "1427247692705959881058285969449495136381746624", 10},
        {"", "", "", 0}
    };
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
    return true;
}

bool test7_sqrt() {
    BigIntHex ha, hal, har, ham;
    BigIntDec hb, hbl, hbr, hbm;
    struct {
        const char* p1;
        const char* pa;
        int base;
    } in[] = {
        {"1111001", "1011", 2},
        {"2037035976334486086268445688409378161051468393665936250636140449354381299763336706183397376", "1427247692705959881058285969449495136382746624", 10},
        {"", "", 0}
    };
    for (int i = 0; in[i].base; ++i) {
        ha.from_str(in[i].p1, in[i].base);
        hal = 1; har = ha;
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
        hbl = 1; hbr = hb;
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

int main() {
    using namespace std;
    cout << "test1_parse: " << (test1_parse() ? "pass" : "FAIL") << endl;
    cout << "test2_add  : " << (test2_add() ? "pass" : "FAIL") << endl;
    cout << "test3_sub  : " << (test3_sub() ? "pass" : "FAIL") << endl;
    cout << "test4_mul  : " << (test4_mul() ? "pass" : "FAIL") << endl;
    cout << "test5_div  : " << (test5_div() ? "pass" : "FAIL") << endl;
    cout << "test6_mod  : " << (test6_mod() ? "pass" : "FAIL") << endl;
    cout << "test7_sqrt : " << (test7_sqrt() ? "pass" : "FAIL") << endl;
    return 0;
}
