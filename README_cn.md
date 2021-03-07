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

## 用法

### 线上比赛用法

- BigIntTiny: 直接把bigint_tiny.h整个文件内容复制到源文件的前面即可使用
- BigIntMini: 直接把bigint_mini.h整个文件内容复制到源文件的前面，再补充所需的头文件(也可直接复制bigint_header.h)
- BigIntDec: 要用这个则麻烦一些，需要把bigint_dec.h整个文件复制到源文件前面，再复制bigint_base.h到前面，再接着补充头文件，最后视需要删除不要的部分，比如说比赛一般只用到10进制，那么可以把BigIntBaseNS名字空间整个去掉，再在BigIntDec中把编译不通过的函数删除（就是与输出相关的部分）。再然后部分OJ还可能因代码长度限制提交不上去时，把缩进切换为tab。
- BigIntHex: 通过不建议在线上比赛用这个，虽然运算速度最快，但代码最长，且需要输入输出的场合下做进制转换不够快反而可能超时。用法与BigIntDec几乎一样

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
[Standard]:         https://img.shields.io/badge/C++-11-orange.svg
[Standard Cpp]:     https://en.wikipedia.org/wiki/C%2B%2B#Standardization
[License]:          https://img.shields.io/badge/license-MIT-blue.svg
[MIT]:              https://opensource.org/licenses/MIT
