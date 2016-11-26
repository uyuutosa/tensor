# tensor
テンソルの様に演算が行えるC++ライブラリ

まだ工事中です!笑い。
![UnderConstruction](construction.png "Under construction")

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
- テンソルの四則演算：異なる添字同士の演算も可能(例：$$a_i + b_j = c_{ij}$$)
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

処理結果
```c++
-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      1,        4,        7,       10,       13, ]

```



これは、$a_i$の全要素に関数funcをそれぞれ代入し、それぞれの結果を格納するテンソルを得ることを示しています。

今度は$j$の添字を持つ$b_j$との積をとってみましょう。
```c++
// Also b * f, bloadcasting is occur automatically.
std::cout << b * f << std::endl;
```
処理結果
```c++
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
```
$j$のすべての要素に対してfuncを$i$毎に演算しています。その為、出力は$i$と$j$を添え字に持つランク2の行列になります。

また、関数テンソルは何度も使えます、以下のように。念のため。
```c++
// Multiple using is possible.
std::cout << a * f * b * f * a * f << std::endl;
```

処理結果
```c++
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

次は参照テンソルを説明します。
参照テンソルはある一つの軸の中で、各要素が隣(一つ前)の要素を参照しています。
その為、初期値初期値以外は値を持っていません。(別の目的のために値を取らせることもできます)
以下のように、初期値と要素数を定義します。

```c++

    // Reference tensor.
    // Reference tensor enable calculation of like i_n+1 = i_n.
    // Define the reference tensor r_i = (3,<-,<-,<-,<-).
    Tensor<double>& r = Tensor<double>(* new ReferenceTensor<double>(3, s("i", 5, ""), s("i", 0, "")));
```
上の例では要素が5つの参照テンソル$r_i$が定義されました。
以下のように$r_i$に3を掛けてみましょう。

```c++
    std::cout << r * 3 << std::endl;
```

処理結果は以下のようになります。
```c++
-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      9,       27,       81,      243,      729, ]
```
各要素はつまり$(3*3, (3*3)*3, (3*3*3)*3, (3*3*3*3)*3, (3*3*3*3)*3)$のように、
前の値(0番目は初期値)に対して3を掛けるという処理をしています。
これはつまり逐次演算であり、イタレーションを必要とする最適化アルゴリズムを
実装するときに使うことができます。

最後に要素参照とインデクシングを紹介します。メンバ関数refを使うことで要素を参照することができます。
引数には参照する配列番号を添え字のインデックス番号で指定します。

```c++
    // Reference and indexing.
    // The reference of value is possible using a member function 'ref'
    std::cout << "Element of a at i = 0 : " << *a.ref(s("i", 0, "")) << std::endl;
    std::cout << "Element of a at i = 1 : " << *a.ref(s("i", 1, "")) << std::endl;
    std::cout << "Element of a at i = 2 : " << *a.ref(s("i", 2, "")) << std::endl;
    std::cout << "Element of a at i = 3 : " << *a.ref(s("i", 3, "")) << std::endl;
    std::cout << "Element of a at i = 4 : " << *a.ref(s("i", 4, "")) << std::endl;
```

処理結果
```c++
Element of a at i = 0 : 1
Element of a at i = 1 : 2
Element of a at i = 2 : 3
Element of a at i = 3 : 4
Element of a at i = 4 : 5
```

上記はテンソルの要素の参照に添字と番号を指定して単一の要素の値を得ましたが、一度に複数の要素の値を
得ることもできます。その複数のインデックスのアサインにはテンソルを用います。つまりテンソルの要素
参照にテンソルを用います。まず以下のようにテンソルを定義します。
```c++
Tensor<double>& arr  = (Tensor<double>(s("i", 5, ""), s("i", 0, "")) << 0,1,2,3,4);
Tensor<double>& idx1 = (Tensor<double>(s("i", 5, ""), s("i", 0, "")) << 0,1,2,3,4);
Tensor<double>& idx2 = (Tensor<double>(s("i", 5, ""), s("i", 0, "")) << 1,0,2,4,3);
Tensor<double>& idx3 = (Tensor<double>(s("i", 5, ""), s("i", 0, "")) << 1,1,1,1,1);

```

arrというテンソルの要素を参照するためにidx1, idx2, idx3というテンソルを用います
arrの中身は
```c++
fs << "arr:" << std::endl;
fs << arr << std::endl;
```
処理結果
```c++
-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[      0,        1,        2,        3,        4, ]
```
のようになっています。

ではarrをidx1, idx2, idx3を用いて参照します。以下のように書くことができます。
```c++
std::cout << "Indexsing of arr using idx1:" << std::endl;
std::cout << arr[idx1] << std::endl;
std::cout << "Indexsing of arr using idx2:" << std::endl;
std::cout << arr[idx2] << std::endl;
std::cout << "Indexsing of arr using idx3:" << std::endl;
std::cout << arr[idx3] << std::endl;
```

処理結果
```c++
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
```

インデクシングして得られたテンソルは、抜き出された元のテンソルと情報を共有しています。
そのことを利用して、部分的にテンソルの要素の値を書き換えることができます。
例えば、arrの0番目と3番目の要素を100に書き換えてみましょう。それは以下のように書くことができます。
```c++
// Also partial substiution is possible,
Tensor<double>& idx4 = (Tensor<double>(s("i", 2, ""), s("i", 0, "")) << 0, 3);
arr[idx4] = 100;
std::cout << "substituiton by arr using idx1:" << std::endl;
std::cout << arr << std::endl;
```

処理結果は以下のようになります。
```c++
substituiton by arr using idx1:

-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
Total size  : 5
Shape       : (i: 5, )
Num. of Dim.: 1

Up or donw  : (down, )
-*-Values*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
[    100,        1,        2,      100,        4, ]
```
このように、0番目と3番目の要素が100に書き換わりました。


