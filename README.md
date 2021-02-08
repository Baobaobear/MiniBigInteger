# MiniBigInteger

[![Build Status]][Travis CI][![Appveyor status]][Appveyor] [![Language]](https://isocpp.org/)[![Standard]][Standard Cpp][![License]][MIT]

This is a C++11 port of large integer library. It allow you parsing of numbers as strings in any bases between 2 and 36 and converting them back to string.

## Code Example

Here is a example for calculate 2^100

```c++
#include <iostream>
#include "bigint.h"

int main() {
    using namespace std;
    BigIntHex s;
    s.set(1);
    for (int i = 1; i <= 100; ++i) {
        s *= BigIntHex().set(2);
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
BigIntHex a, b;
a < b;
a <= b;
a >= b;
a > b;
a == b;
a != b;
```

### Output

```c++
BigIntHex a;
cout << a.to_str() << endl; // dec by default
cout << a.to_str(16) << endl; // hex output
```


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
