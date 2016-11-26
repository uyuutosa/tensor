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

![UnderConstruction](uncon.gif "Under construction")
