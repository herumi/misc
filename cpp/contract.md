# 契約プログラミング

* [P0542R5](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0542r5.html)
* [n4820.pdf](https://github.com/cplusplus/draft/raw/master/papers/n4820.pdf)

## 属性の文法

```
[[契約の属性: 条件式]]
[[契約の属性 契約レベル: 条件式]]
[[ensures 契約レベル identifier: 条件式]]
```
- 契約の属性 = expects, ensures, assert
  - expects ; 事前条件 ; 関数本体の評価が始まる直前に満たすべき条件
  - ensures ; 事後条件 ; 関数が抜ける直前の戻り値が満たすべき条件 / 例外発生 or longjmp時には評価されない
  - assert ; 関数本体の中で満たされるべき条件

- 契約レベル = axiom, default, audit
  - axiom ; 形式的なコメント / 実行時には評価されない
  - default ; default or auditレベルでbuildされたときに評価される
  - audit ; auditレベルでbuildされたときに評価される
  - buildレベルはoff/default/audit
- identifier = 関数の戻り値

### 例

```
int f(int x)
  [[expects audit: x>0]]
  [[ensures axiom res: res>1]];

void g() {
  int x = f(5);
  int y = f(12);
  //...
  [[assert: x+y>0]]
```

## 評価順序
expectsが先でensuresが最後

```
void f(int * p)
  [[expects: p != nullptr]] // #1
  [[ensures: *p == 1]] // #3
  [[expects: *p == 0]] // #2
{
  *p = 1;
}
```


## 関数の型

```
void f() ***;
```
の***に任意個のattributeを記述する。

### 複数回関数宣言

複数回関数宣言をする場合はattributeが完全に同じか完全に省略するかどちらかのみ

```
int f(int x)
  [[expects: x>0]]
  [[ensures r: r>0]];

int f(int x); // 完全省略なのでOK

int f(int x)
  [[expects: x>=0]]; // 最初の契約と異なるのでエラー

int f(int x)
  [[expects: x>0]]
  [[ensures r: r>0]]; // 完全に同じなのでOK

int f(int y)
  [[expects: y>0]]    // 変数名が違うだけなのはOK
  [[ensures z: z>0]]; // 変数名が違うだけなのはOK
```

```
int f(int x, int y)
  [[expects: x>0]]
  [[expects: y>0]];

int f(int x, int y)
  [[expects: y>0]]
  [[expects: x>0]]; // 順序が違うのはNG
```

### 契約は関数の型の一部ではない

- 関数ポインタに代入できる
- その関数ポインタを実行するときは契約条件は検査される

```
int f(x)
  [[expects:x > 0]]
{
  return x;
}

int (*pf)(int) = &f; // OK
```

- ラムダは未サポート

```
void f() {
  // Not currently supported
  auto increment = [](int x) [[expects: x>0]] { return x+1; };
  // ...
}
```

- 構造化束縛は未サポート

```
std::tuple f()
  [[ensures [x,y]: x>0 && y.size()>0]];
```

今は次のように記述する

```
std::tuple f()
  [[ensures r: get<0>(r)>0 && get<1>(r).size()>0]];
```

## 未定義と例外

- ensuresの変数にodr(one-definition rule)があると未定義

```
int f(int x)
[[ensures r: r == x]]
{
return ++x; // undefined behavior
}
```

- 評価時に副作用がある or ある可能性は未定義

```
int min = -42;
constexpr int max = 42;

constexpr int g(int x)
  [[expects: min <= x]] // error
  [[expects: x < max]] // OK
{
  /* ... */
  [[assert: 2*x < max]];
  [[assert: ++min > 0]]; // undefined behavior
  /* ... */
}
```

```
bool might_increment(int & x);

void f(int n) [[expects: might_increment(n)]]; // Undefined behavior

bool is_valid(int x) {
  std::cerr << "checking x\n";
  return x>0;
}

void g(int n) [[expects: is_valid(n)]]; // Undefined behavior
```

- 評価中の中にライフタイムが含まれているnon-volatile変数の変更は可能

```
bool is_valid(int x) {
  int a=1;
  while (a<x) {
    if (x % a == 0) return true;
    a++;
  }
  return false;
}

void f(int n) [[expects: is_valid(x)]]
```
- 評価時に例外が発生するとstd::terminateが呼ばれる

## violation_hanlder

- 例外評価がfalseだと`void violation_hanlder(const std::contract_violation&) noexcept`の形の関数が呼ばれる。
- この関数を設定したり変更する方法は提供されない
- その後std::terminateが呼ばれる
  - 設定により(ログをとるなどのために)プログラム継続も可能

```
class contract_violation {
public:
  uint_least32_t line_number() const noexcept;
  string_view file_name() const noexcept;
  string_view function_name() const noexcept;
  string_view comment() const noexcept;
  string_view assertion_level() const noexcept;
};
```
- contract_violationの詳細は処理系依存
  - 事前条件違反のときのソース位置は処理系依存
    - 呼び出した場所の報告を奨励する
  - 事後条件違反のときのソース位置は呼び出した場所の一つ
  - assertionはそれ自身の場所

## 識別子としての契約属性

```
X f(X & audit) [[ensures audit: audit.valid()]];
```
これはvalid. ensures auditのauditは関数の引数として正しい
