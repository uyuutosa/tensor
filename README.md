# tensor
テンソルの様に演算が行えるC++ライブラリ

数式とプログラムの一対一対応による冗長なコーディング作業からの解放を目指して、
いろんな規則を内包するインテリジェントな配列型を作っています。
また面白いor効率的な演算規則の考案も行っています。
現状はテンソルに似ていますが、今後テンソルじゃなくなる可能性があります。
独立な演算をかんたんに書く事ができます。また[畳込みをテンソル演算で行う方法を(なんとなく)定式化](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74)
したので、numpyのような任意次元のコンボリューションを
テンソルの内積演算で行うことができます。
しかし大量の演算をシングルスレッドで行うととても重いので、
今後GPU対応を検討しています。

## 主な機能
- テンソルの四則演算：異なる添字同士の演算も可能(例：$a_i + b_j = c_{ij}$)
- 関数を要素に持つテンソルの演算
- 逐次的に次の要素の値を参照するテンソル
- ハンドリング性に優れたインデクシング

## 実装例
まず、以下のようにテンソルを定義します。

```c++
// Define tensor a_i = (1,2,3,4,5,6,7,8,9)
Tensor<double>& a = (Tensor<double>(s("i", 5, ""), s("i", 0, "")) << 1,2,3,4,5);
Tensor<double>& b = (Tensor<double>(s("j", 5, ""), s("j", 0, "")) << 1,2,3,4,5);
```
出力はストリームに流して確認できます。
```c++
//view tensor contained values.
std::cout << a << std::endl;
```
処理結果：
```c++
-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        2,        3,        4,        5, ]
```

$a_i$と$b_j$を使って加減乗除の計算をします。
両者は異なる添字を持っており独立なので、計算結果は$i$と$j$の添字を持つランク2のテンソルになります。

```c++
// Tensor supported four arithmetic operation.
std::cout << "Addition:" << std::endl;
std::cout << a + b << std::endl;
std::cout << "Subtraction:" << std::endl;
std:cout << a - b << std::endl;
std::cout << "Multiplication:" << std::endl;
std::cout << a * b << std::endl;
std::cout << "Division:" << std::endl;
std::cout << a / b << std::endl;
```
処理結果
```


Addition:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      2,         3,         4,         5,         6, ]
 [      3,         4,         5,         6,         7, ]
 [      4,         5,         6,         7,         8, ]
 [      5,         6,         7,         8,         9, ]
 [      6,         7,         8,         9,        10, ]]


Subtraction:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      0,        -1,        -2,        -3,        -4, ]
 [      1,         0,        -1,        -2,        -3, ]
 [      2,         1,         0,        -1,        -2, ]
 [      3,         2,         1,         0,        -1, ]
 [      4,         3,         2,         1,         0, ]]


Multiplication:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      1,         2,         3,         4,         5, ]
 [      2,         4,         6,         8,        10, ]
 [      3,         6,         9,        12,        15, ]
 [      4,         8,        12,        16,        20, ]
 [      5,        10,        15,        20,        25, ]]


Division:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      1,       0.5,   0.33333,      0.25,       0.2, ]
 [      2,         1,   0.66667,       0.5,       0.4, ]
 [      3,       1.5,         1,      0.75,       0.6, ]
 [      4,         2,    1.3333,         1,       0.8, ]
 [      5,       2.5,    1.6667,      1.25,         1, ]]



-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        4,        7,       10,       13, ]



-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      1,         2,         3,         4,         5, ]
 [      3,         4,         5,         6,         7, ]
 [      5,         6,         7,         8,         9, ]
 [      7,         8,         9,        10,        11, ]
 [      9,        10,        11,        12,        13, ]]



-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 25
Shape       : (i: 5, j: 5, )
Num. of Dim.: 2

Up or donw  : (down, down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[[      1,         2,         3,         4,         5, ]
 [     14,        22,        30,        38,        46, ]
 [     37,        58,        79,       100,       121, ]
 [     70,       110,       150,       190,       230, ]
 [    113,       178,       243,       308,       373, ]]
```

関数を要素としたテンソルの演算を行うことが可能です。
まず、適当な関数を定義します。

```c++
// For function tensor.
// Simple addition of index number 'i'.
double& func(int i, double& v){
    auto& ret = *new double(v + 2*i);
    return ret;
}
```

そして、この関数を下記のように関数テンソルに設定します。

```c++
// Function tensor. 
// Function tensor is a tensor containes functions.
// Abobe function is substitued to a tensor as follows,
Tensor<double>& f = Tensor<double>(* new FunctionTensor<double>((void*)func, s("i", 5, ""), s("i", 0, "")));
```

これで関数テンソル$f_i=(func, func, func, func, func)$が定義されました。
これを用いてなにか計算してみましょう。先程の$a_i$と$f_i$の積を計算します。

```c++
// Functions inner tensor is executed by usual tensor multiplication,
std::cout << a * f << std::endl;
```

これは、$a_i$の全要素に関数funcをそれぞれ代入し、それぞれの結果を格納するテンソルを得ることを示しています。

```c++
// Also b * f, bloadcasting is occur automatically.
std::cout << b * f << std::endl;
```

// Multiple using is possible.
std::cout << a * f * b * f * a * f << std::endl;
```



```c++


-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      9,       27,       81,      243,      729, ]


Element of a at i = 0 : 1
Element of a at i = 1 : 2
Element of a at i = 2 : 3
Element of a at i = 3 : 4
Element of a at i = 4 : 5
arr:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      0,        1,        2,        3,        4, ]


Indexsing of arr using idx1:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      0,        1,        2,        3,        4, ]


Indexsing of arr using idx2:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        0,        2,        4,        3, ]


Indexsing of arr using idx3:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        1,        1,        1,        1, ]


Indexsing of arr2 using idx1:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      0,        1,        2,        3,        4, ]


Indexsing of arr2 using idx2:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        0,        2,        4,        3, ]


Indexsing of arr2 using idx3:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        1,        1,        1,        1, ]
```


![UnderConstruction](uncon.gif "Under construction")
