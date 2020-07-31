# レビュー

## Strings library

### `char8_t`の導入
- `unsigned char`と同じ大きさだが型としては異なる
- `std::basic_string<char8_t>`である`std::u8string`が導入される
- 関連する`hash`, `string_view`

## 破壊的変更
- `u8`プレフィクスのついた文字列リテラルの型が`const char8_t[]`になる
  - `u8`文字列を`const char*`や`std::string`で受けられない
- `std::filesystem::path`の戻り値で`std::string`だったものが`std::u8string`になる

## traits
- `comparison_category`に`strong_ordering`追加
- move, copy, assignがconstexprに
- `operator+`, swapなどもconstexprに
  - `string`のcstrやdstr, 各種iteratorメソッドなどもconstexprに
- `operator<=`などの代わりに`operator<=>`
- erase, erase_ifがstring対応

## 範囲の明記

- `[data(), data() + size()]`が有効な範囲
- `data() + size()`は`charT()`へのポインタを指す
- `size() <= capacity()`は常にtrue

## メソッド
- `reserve`のデフォルト引数(0)が無くなった
- `bool empty()`の戻り値に`[[nodiscard]]`がついた
- メソッドが例外を投げたときに元のobjectは何も変わらないことが明記された
- `starts_with`と`ends_with`の追加

## UTF-8とchar文字列の相互変換

```
size_t mbrtoc8(char8_t* pc8, const char* s, size_t n, mbstate_t* ps);
size_t c8rtomb(char* s, char8_t c8, mbstate_t* ps);
```

## Numerics library

- `<bit>`と`<numbers>`が追加
- `<numeric>`がAlgorithmsのセクションに移動
- `<complex>` ; いろいろな関数が`constpr`に

## bit manipulation
- `bit_cast` ;ビットパターンを保ったまま別の型にキャスト
- `has_single_bit(x)` ; xが2のべきのときのみtrue
- `bit_ceil(x)` ; x以上の最小の2のべき(cf. 0xffffffffのbit_ceilは未定義)
- `bit_floor(x)` ; x == 0なら0。そうでなければxを超えない最大の2べき
- `bit_width(x)` ; x == 0なら0。そうでなければ`1 + floor(log_2(x))`
- `rotl(x, s)` ; xの左(s % N)回転 ; `N=numeric_limits<T>::digits` ; `[[nodiscard]]`つき(xを変化されると勘違いさせないため?)
- `rotr(x, s)` ; xの右(s % N)回転
- `countl_zero` ; MSBから0の続く数(x == 0ならN)
- `countl_one` ; MSBから1の続く数
- `countr_zero` ; 0bit目から0の続く数
- `countr_one` ; 0bit目から1の続く数
- `popcount` ; xを2進数表記したときの1の数

```
enum class endian {
  little,
  big,
  native
};
```

- 全てのスカラー型が1byteなら`little == big && big == native`
- そうでなければ`little != big`
  - 全てのスカラー型がbig endianなら`native == big`
  - 全てのスカラー型がlittle endianなら`native == little`
  - そうでなければ`native != big && native != little`

## random number generation
- `uniform_random_bit_generator`の追加
  - 整数の範囲で一様ランダムな分布の乱数を生成することを示すコンセプト

## numbers
いくつかの関数の定数追加
-  e_v ; e
- log2e_v ; log_2(e)
- log10e_v ; log_10(e)
- pi_v ; π
- inv_pi_v ; 1/π
- inv_sqrtpi_v ; 1/√π
- ln2_v ; log(2)
- ln10_v ; log(10)
- sqrt2_v ; √2
- sqrt3_v ; √3
- inv_sqrt3_v ; 1/√3
- egamma_v ; オイラー定数γ
- phi_v ; (1+√5)/2
