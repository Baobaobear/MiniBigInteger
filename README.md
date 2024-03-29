# MiniBigInteger

![Github Actions] [![Build Status]][Travis CI] [![Appveyor status]][Appveyor] [![Language]](https://isocpp.org/) [![Standard]][Standard Cpp] [![License]][MIT]

[中文版本](README_cn.md)

This is a C++03 port of arbitrary-precision arithmetic of integer library. It allows you input and output large integers from strings in any bases between 2 and 36.

The BigIntMini & BigIntTiny is designed for online contest that enought to use in most of cases although they are not the fastest.

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
        s *= BigIntHex(2);
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
        s = s * BigIntMini(2); // no operator*= overloading
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
        s = s * BigIntTiny(2); // no operator*= overloading
    }
    cout << s.to_str() << endl;
    return 0;
}
```

The output is `1267650600228229401496703205376`

## Usage

### Online contest

Run `build_singlefile.py` to generate the standalone header file, then copy the header file of the class which you need into the front of the source file.

For `single_bigint_hexm.h` and `single_bigint_decm.h`, they streamline the input and output of the base conversion, and since online competitions rarely need to do this step.

### Other cases

Copy all header files to your project's directory then include them.

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
| \<, ==, \>, <=, >=, != Bigint |✔|✔|✔|✔|
| +, -, *, /, % int |❌|❌|❌|✔|
| +=, -=, *=, /=, %= int |❌|❌|❌|❌|
| +, -, *, /, % Bigint|✔|✔|✔|✔|
| +=, -=, *=, /=, %= Bigint|✔|✔|❌|❌|
| Base conversion|✔|✔|❌|❌|
| Efficiency|✬✬✬✬|✬✬✬|✬✬|✬|

# License

This project is licensed under the MIT License.

[Github Actions]:   https://github.com/baobaobear/minibiginteger/actions/workflows/c-cpp.yml/badge.svg
[Build Status]:     https://travis-ci.com/Baobaobear/MiniBigInteger.svg?branch=main
[Travis CI]:        https://travis-ci.org/Baobaobear/MiniBigInteger
[Appveyor status]:  https://ci.appveyor.com/api/projects/status/yeu4tqao2ri3bc07?svg=true
[Appveyor]:         https://ci.appveyor.com/project/Baobaobear/minibiginteger
[Language]:         https://img.shields.io/badge/language-C++-blue.svg
[Standard]:         https://img.shields.io/badge/C++-03-orange.svg
[Standard Cpp]:     https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[License]:          https://img.shields.io/badge/license-MIT-blue.svg
[MIT]:              https://opensource.org/licenses/MIT
