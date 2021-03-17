# MiniBigInteger

![Github Actions] [![Build Status]][Travis CI] [![Appveyor status]][Appveyor] [![Language]](https://isocpp.org/) [![Standard]][Standard Cpp] [![License]][MIT]

[English version](README.md)

本项目是个(基本上能)兼容C++03的高精度大整数库。它能让你从字符串输入或输出大整数，且支持2到36之间的任意进制。

其中，BigIntMini和BigIntTiny是为线上比赛定制，虽然不是最快，但大多数场合下足够使用。

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

输出结果为 `1267650600228229401496703205376`

## 用法

### 线上比赛用法

运行`build_singlefile.py`生成独立头文件，然后把你需要用到的类的相应头文件复制进源文件前面即可使用。

对于`single_bigint_hexm.h`和`single_bigint_decm.h`，它们精简了进制转换的输入输出，因为线上比赛很少需要做这步，就添加了这个不含进制转换的版本。

### 非线上比赛用法

把文件复制到与你的cpp同一目录下，然后直接include即可使用

## 性能特性

- BigIntTiny: 代码最短，但性能最差，但仍然比不压位的快不少
- BigIntMini: 代码量中等，性能适合，带上乘法和除法优化，过大部分的题目没有问题
- BigIntDec: 非常多的优化，仅次于BigIntHex，但对于仅用10进制且大量输入输出的情况下会更快
- BigIntHex: 最多的优化，本项目里大部分情况下最快的实现

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
BigIntHex a, b;
a < b;
a <= b;
a >= b;
a > b;
a == b;
a != b;
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
| \<, ==, \>, <=, >=, != Bigint |✔|✔|✔|✔|
| +, -, *, /, % int |❌|❌|❌|✔|
| +=, -=, *=, /=, %= int |❌|❌|❌|❌|
| +, -, *, /, % Bigint|✔|✔|✔|✔|
| +=, -=, *=, /=, %= Bigint|✔|✔|❌|❌|
| Base conversion|✔|✔|❌|❌|
| Efficiency|✬✬✬✬|✬✬✬|✬✬|✬|

# 源码许可

本项目采用 MIT 许可协议.

[Github Actions]:   https://github.com/baobaobear/minibiginteger/actions/workflows/c-cpp.yml/badge.svg
[Build Status]:     https://travis-ci.org/Baobaobear/MiniBigInteger.svg?branch=main
[Travis CI]:        https://travis-ci.org/Baobaobear/MiniBigInteger
[Appveyor status]:  https://ci.appveyor.com/api/projects/status/yeu4tqao2ri3bc07?svg=true
[Appveyor]:         https://ci.appveyor.com/project/Baobaobear/minibiginteger
[Language]:         https://img.shields.io/badge/language-C++-blue.svg
[Standard]:         https://img.shields.io/badge/C++-03-orange.svg
[Standard Cpp]:     https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[License]:          https://img.shields.io/badge/license-MIT-blue.svg
[MIT]:              https://opensource.org/licenses/MIT
