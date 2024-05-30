# [P2964R0 : Adding support for user-defined element types in std::simd](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2964r0.html)

カスタマイゼーションポイント(CPO)を利用してstd::simdにユーザ定義の要素型(UDT : User-Defined element Types)をサポートする。

## [std::simd](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1928r8.pdf)
- 2018年 `std::experimental::simd<T>`の提案(TS)
- 2019 TS2のフィードバック(様々な議論)
- 算術型, 複素数型などに対応

## 拡張
- それをUDTでも扱えるようにしたい
- たとえば`fixed_point_16s8`という固定小数点数データ型
  - 必要な演算子のみサポート
  - たとえば不要な`operator/`や`operator%`を削除する

このようなコードを`float`や`double`の組み込み型でも`fixed_point_16s8`でも動作できるようにする。

```cpp
struct fixed_point_16s8 {
  int16_t storage;

  fixed_point_16s8 operator+(fixed_point_16s8 rhs) const;
  fixed_point_16s8& operator+=(fixed_point_16s8 rhs);
};
```

```cpp
template <typename T>
void process_signal(std::span<T> input, std::span<T> output)
{
    using namespace std::experimental;

    const std::size_t N = input.size();
    const std::size_t simd_size = simd<T>::size();

    for (std::size_t i = 0; i < N; i += simd_size) {
        const auto a = simd<T>::copy_from(input.data() + i, element_aligned);
        const auto b = simd<T>::copy_from(input.data() + i + 1, element_aligned);
        const auto c = a + b;
        c.copy_to(output.data() + i, element_aligned);
    }
}
```

`std::simd`が`std::simd<fixed_point_16s8>`をサポートするにはいくつか方法が考えられる。

- コンパイラが型を認識して自動的に`std::simd`に拡張させる
  - これは無理
- `std::simd`が各要素に対して自動ベクトル化を行う
  - `std::simd`の目的の一つが自動ベクトル化に依存しないことなのでそれも避ける
- CPOを利用して必要な場所に特別な動作を挿入する - このアプローチを採用する

## 現状の仕様を把握する
- `std::simd`の各要素は自明にコピー可能
  - 移動しても元の値を表す
### CPOが必要なカテゴリ
- 基本
  - 加算
- カスタム
  - `0-x`を`operator-`としてカスタマイズする
  - 浮動小数点数の符号変換
- コピー
  - 一部の値をコピーする
- アルゴリズム

主な操作
- `simd_mask` : ビットマスクは通常そのまま使える
- コンストラクタ
- `copy_to`, `copy_from`
- `operator[]`
- 単項演算子 : `-`, `--`, `++`, `!`, `~`
- 二項演算子 : `+`, `-`, `*`, `/`, `%`, `<<`, `>>`, `&`, `|`, `^`
- 複合代入演算子 : `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=`
- 比較演算子
- 条件コピー : `simd_select`
- 置換
- 圧縮・拡張
- `gather_from`, `scatter_to`
- リダクション
- `min`, `max`, `clamp`
- `abs`, `sin`, `log`, etc.

### 実装例

### operator CPO
```cpp
constexpr friend basic_simd operator+(const basic_simd& lhs, const basic_simd& rhs)
requires (details::simd_has_custom_binary_plus || details::element_has_plus)
{
    if constexpr (details::simd_has_custom_binary_plus)
        return simd_binary_plus(lhs, rhs);
    else
        /* impl-defined */
}
```

### free-function
```cpp
template<typename Abi>
constexpr auto abs(const basic_simd<fixed_point_16s8, Abi>& v) {
    return /* special-abs-impl */;
}
```

### AVX2による例
```cpp
template<typename Abi>
constexpr auto simd_binary_op(const xvec::basic_simd<saturating_int16, Abi>& lhs,
                              const xvec::basic_simd<saturating_int16, Abi>& rhs,
                              std::plus<>)
{
    auto r = _mm256_adds_epi16(static_cast<__m256i>(lhs), static_cast<__m256i>(rhs));
    return basic_simd<saturating_int16, Abi>(r);
}
```

```cpp
auto add(simd<saturating_int16> lhs,
         simd<saturating_int16> rhs)
{
    return lhs + rhs;
}
```
↓

```asm
add([...]): #
        vpaddsw ymm0, ymm0, ymm1
        ret
```

```cpp
auto compound_add(simd<saturating_int16> lhs,
                  simd<saturating_int16> rhs)
{
    lhs += rhs;
    return lhs;
}
```
↓
```asm
compound_add([...]): #
        vpaddsw ymm0, ymm0, ymm1
        ret
```

```cpp
auto reduce_add(simd<saturating_int16> v)
{
    return reduce(v, std::plus<>{});
}
```
↓
```asm
reduce_add([...]):
        vextracti128    xmm1, ymm0, 1
        vpaddsw xmm0, xmm0, xmm1
        vpshufd xmm1, xmm0, 238
        vpaddsw xmm0, xmm0, xmm1
        vpshufd xmm1, xmm0, 85
        vpaddsw xmm0, xmm0, xmm1
        vpextrw ecx, xmm0, 1
        vmovd   eax, xmm0
        ret
```
