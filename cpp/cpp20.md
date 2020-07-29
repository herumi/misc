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
