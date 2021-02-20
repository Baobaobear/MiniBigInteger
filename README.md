# MiniBigInteger

[![Build Status]][Travis CI][![Appveyor status]][Appveyor] [![Language]](https://isocpp.org/)[![Standard]][Standard Cpp][![License]][MIT]

[中文版本](README_cn.md)

This is a C++03 port of large integer library. It allows you parsing of numbers as strings in any bases between 2 and 36 and converting them back to string.

## Code Example

Here is an example for calculate 2^100

### BigIntHex or BigIntDec

```c++
#include <iostream>
#include "bigint_hex.h"
#include "bigint_dec.h"

int main() {
    using namespace std;
    BigIntHex s; // the same as BigIntDec
    s = 1;
    for (int i = 1; i <= 100; ++i) {
        s *= 2;
    }
    cout << s.to_str() << endl;
    return 0;
}
```

### BigIntMini

```c++
#include <iostream>
#include "bigint_mini.h"

int main() {
    using namespace std;
    BigIntMini s;
    s = 1;
    for (int i = 1; i <= 100; ++i) {
        s = s * 2; // no operator*= overloading
    }
    cout << s.to_str() << endl;
    return 0;
}
```

### BigIntTiny

```c++
#include <iostream>
#include "bigint_tiny.h"

int main() {
    using namespace std;
    BigIntTiny s;
    s = 1;
    for (int i = 1; i <= 100; ++i) {
        s = s * 2; // no operator*= overloading
    }
    cout << s.to_str() << endl;
    return 0;
}
```

The output is `1267650600228229401496703205376`

## Operators

### Assignment

```c++
BigIntHex a, b;
a = 123;
a = "123";
a = std::string("123");
a.set(123);
a.from_str("ABCDEF", 16);
a.from_str("1011010", 2);
a.from_str("123456789ABCDEFGZXY", 36);
b = a;
```

### Addition

```c++
BigIntHex a, b;
a = a + b;
a += b;
```

### Subtraction

```c++
BigIntHex a, b;
a = a - b;
a -= b;
```

### Multiplication

```c++
BigIntHex a, b;
a = a * b;
a *= b;
a *= 123;
```

### Division

```c++
BigIntHex a, b;
a = a / b;
a /= b;
```

### Comparison

```c++
BigIntHex a, b; // or BigIntDec
a < b;
a <= b;
a >= b;
a > b;
a == b;
a != b;
```

```c++
BigIntMini a, b; // or BigIntTiny
a < b;
a == b;
//not support the belows
//a <= b;
//a >= b;
//a > b;
//a != b;
```

### Output

```c++
BigIntHex a; // or BigIntDec
cout << a.to_str() << endl; // dec by default
cout << a.to_str(16) << endl; // hex output

BigIntMini b; // or BigIntTiny
cout << b.to_str() << endl;
```

## Features preview

|operators|BigIntHex|BigIntDec|BigIntMini|BigIntTiny|
|--------|---------|---------|---------|---------|
| constructor Bigint|✔|✔|✔|✔|
| constructor int|✔|✔|✔|✔|
| constructor char*|✔|✔|❌|❌|
| constructor string|✔|✔|✔|✔|
| =Bigint|✔|✔|✔|✔|
| =int   |✔|✔|✔|✔|
| =string|✔|✔|✔|✔|
| =char* |✔|✔|✔|❌|
| \<, == Bigint |✔|✔|✔|✔|
| \>, <=, >=, != Bigint |✔|✔|❌|❌|
| +, -, /, % int |❌|❌|❌|✔|
| * int |✔|✔|❌|✔|
| *= int |✔|✔|❌|❌|
| +=, -=, /=, %= int |❌|❌|❌|❌|
| +, -, *, /, % Bigint|✔|✔|✔|✔|
| += Bigint|✔|✔|❌|✔|
| -=, *=, /=, %= Bigint|✔|✔|❌|❌|
| Base conversion|✔|✔|❌|❌|
| Efficiency|✬✬✬✬|✬✬✬|✬✬|✬|

# License

This project is licensed under the MIT License.

[Build Status]:     https://travis-ci.com/Baobaobear/MiniBigInteger.svg?branch=main
[Travis CI]:        https://travis-ci.com/Baobaobear/MiniBigInteger
[Appveyor status]:  https://ci.appveyor.com/api/projects/status/yeu4tqao2ri3bc07?svg=true
[Appveyor]:         https://ci.appveyor.com/project/Baobaobear/minibiginteger
[Language]:         https://img.shields.io/badge/language-C++-blue.svg
[Standard]:         https://img.shields.io/badge/C++-11-orange.svg
[Standard Cpp]:     https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[License]:          https://img.shields.io/badge/license-MIT-blue.svg
[MIT]:              https://opensource.org/licenses/MIT
