# MiniBigInteger

[![Build Status]][Travis CI][![Appveyor status]][Appveyor] [![Language]](https://isocpp.org/)[![Standard]][Standard Cpp][![License]][MIT]

[English version](README.md)

本项目是个兼容C++11的高精度大整数库。它能让你从字符串输入或输出大整数，且支持2到36之间的任意进制

## 代码例子

计算 2^100 的示例

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

输出结果为 `1267650600228229401496703205376`

## 基本操作

### 赋值

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

### 加法

```c++
BigIntHex a, b;
a = a + b;
a += b;
```

### 减法

```c++
BigIntHex a, b;
a = a - b;
a -= b;
```

### 乘法

```c++
BigIntHex a, b;
a = a * b;
a *= b;
a *= 123;
```

### 除法

```c++
BigIntHex a, b;
a = a / b;
a /= b;
```

### 关系运算

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

### 输出

```c++
BigIntHex a; // or BigIntDec
cout << a.to_str() << endl; // dec by default
cout << a.to_str(16) << endl; // hex output

BigIntMini b; // or BigIntTiny
cout << b.to_str() << endl;
```

## 特性预览

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
| *= int |✔|✔|✔|❌|
| +=, -=, /=, %= int |❌|❌|❌|❌|
| +, -, *, /, % Bigint|✔|✔|✔|✔|
| += Bigint|✔|✔|❌|✔|
| -=, *=, /=, %= Bigint|✔|✔|❌|❌|
| Base conversion|✔|✔|❌|❌|
| Efficiency|✬✬✬✬|✬✬✬|✬✬|✬|

# 源码许可

本项目采用 MIT 许可协议.

[Build Status]:     https://travis-ci.com/Baobaobear/MiniBigInteger.svg?branch=main
[Travis CI]:        https://travis-ci.com/Baobaobear/MiniBigInteger
[Appveyor status]:  https://ci.appveyor.com/api/projects/status/yeu4tqao2ri3bc07?svg=true
[Appveyor]:         https://ci.appveyor.com/project/Baobaobear/minibiginteger
[Language]:         https://img.shields.io/badge/language-C++-blue.svg
[Standard]:         https://img.shields.io/badge/C++-11-orange.svg
[Standard Cpp]:     https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[License]:          https://img.shields.io/badge/license-MIT-blue.svg
[MIT]:              https://opensource.org/licenses/MIT
