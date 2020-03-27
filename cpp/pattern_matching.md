# Pattern Matching
[P1371R2](https://github.com/mpark/wg21/blob/master/P1371R2.md)

switch caseに代わる新しいパターンマッチ構文の導入
## サンプル

### 整数マッチ
従来
```
switch (x) {
  case 0: std::cout << "got zero"; break;
  case 1: std::cout << "got one"; break;
  default: std::cout << "don't care";
}
```

提案
```
inspect (x) {
  0: std::cout << "got zero";
  1: std::cout << "got one";
  __: std::cout << "don't care";
}
```
### _の禁止
[p1469r0](https://github.com/mpark/wg21/blob/master/generated/P1469R0.pdf)

```
auto [a, _] = std::make_pair(3, 4);
```
と書きたい。
- _を使いたい
  - __は見づらい
- 構造化束縛で_の使用をdeprecate
- 構造化束縛で_の使用を禁止

### 文字列マッチ

従来
```
if (s == "foo") {
  std::cout << "got foo";
} else if (s == "bar") {
  std::cout << "got bar";
} else {
  std::cout << "don't care";
}
```

提案
```
inspect (s) {
  "foo": std::cout << "got foo";
  "bar": std::cout << "got bar";
  __: std::cout << "don't care";
}
```

### 構造化束縛のマッチ

従来
```
auto&& [x, y] = p;
if (x == 0 && y == 0) {
  std::cout << "on origin";
} else if (x == 0) {
  std::cout << "on y-axis";
} else if (y == 0) {
  std::cout << "on x-axis";
} else {
  std::cout << x << ',' << y;
}
```

提案
```
inspect (p) {
  [0, 0]: std::cout << "on origin";
  [0, y]: std::cout << "on y-axis";
  [x, 0]: std::cout << "on x-axis";
  [x, y]: std::cout << x << ',' << y;
}
```
### 入れ子
従来
```
auto const& [topLeft, unused] = getBoundaryRectangle();
auto const& [topBoundary, leftBoundary] = topLeft;
```

```
auto const& [[topBoundary, leftBoundary], __] = getBoundaryRectangle();
```

### メンバ変数指定
```
struct Player { std::string name; int hitpoints; int coins; };

void get_hint(const Player& p) {
    inspect (p) {
        [.hitpoints: 1]: std::cout << "You're almost destroyed. Give up!\n";
        [.hitpoints: 10, .coins: 10]: std::cout << "I need the hints from you!\n";
        [.coins: 10]: std::cout << "Get more hitpoints!\n";
        [.hitpoints: 10]: std::cout << "Get more ammo!\n";
        [.name: n]: {
            if (n != "The Bruce Dickenson") {
                std::cout << "Get more hitpoints and ammo!\n";
            } else {
                std::cout << "More cowbell!\n";
            }
        }
    }
}
```
構造化の指定子(designator)はメンバ変数の宣言順序と一緒でなくてもよい。

### Variant

従来
```
struct visitor {
  void operator()(int i) const {
    os << "got int: " << i;
  }
  void operator()(float f) const {
    os << "got float: " << f;
  }
  std::ostream& os;
};
std::visit(visitor{strm}, v);
```

提案
```
inspect (v) {
  <int> i: strm << "got int: " << i;
  <float> f: strm << "got float: " << f;
}
```

### 式評価
```
struct Expr;

struct Neg {
  std::shared_ptr<Expr> expr;
};

struct Add {
  std::shared_ptr<Expr> lhs, rhs;
};

struct Mul {
  std::shared_ptr<Expr> lhs, rhs;
};

struct Expr : std::variant<int, Neg, Add, Mul> {
  using variant::variant;
};

namespace std {
  template <>
  struct variant_size<Expr> : variant_size<Expr::variant> {};

  template <std::size_t I>
  struct variant_alternative<I, Expr> : variant_alternative<I, Expr::variant> {};
}
```

従来
```
int eval(const Expr& expr) {
  struct visitor {
    int operator()(int i) const {
      return i;
    }
    int operator()(const Neg& n) const {
      return -eval(*n.expr);
    }
    int operator()(const Add& a) const {
      return eval(*a.lhs) + eval(*a.rhs);
    }
    int operator()(const Mul& m) const {
      // Optimize multiplication by 0.
      if (int* i = std::get_if<int>(m.lhs.get()); i && *i == 0) {
        return 0;
      }
      if (int* i = std::get_if<int>(m.rhs.get()); i && *i == 0) {
        return 0;
      }
      return eval(*m.lhs) * eval(*m.rhs);
    }
  };
  return std::visit(visitor{}, expr);
}
```

提案
```
int eval(const Expr& expr) {
  return inspect (expr) {
    <int> i => i,
    <Neg> [(*?) e] => -eval(e),
    <Add> [(*?) l, (*?) r] => eval(l) + eval(r),
    // Optimize multiplication by 0.
    <Mul> [(*?) <int> 0, __] => 0,
    <Mul> [__, (*?) <int> 0] => 0,
    <Mul> [(*?) l, (*?) r] => eval(l) * eval(r)
  };
}
```

### マッチしないとき

従来
```
enum class Op { Add, Sub, Mul, Div };
Op parseOp(Parser& parser) {
  const auto& token = parser.consumeToken();
  switch (token) {
    case '+': return Op::Add;
    case '-': return Op::Sub;
    case '*': return Op::Mul;
    case '/': return Op::Div;
    default: {
      std::cerr << "Unexpected " << token;
      std::terminate();
    }
  }
}
```

提案
```
enum class Op { Add, Sub, Mul, Div };
Op parseOp(Parser& parser) {
  return inspect(parser.consumeToken()) {
    '+' => Op::Add,
    '-' => Op::Sub,
    '*' => Op::Mul,
    '/' => Op::Div,
    token: {
      std::cerr << "Unexpected: " << token;
      std::terminate();
    }
  }
}
```
# 文法

```
inspect (cond) {
  pattern guard_opt => expression,
  pattern guard_opt: statement
}
```
- `=>` ; 返すための値を生成する式
- `:` ; 値は返さない. 中身を実行する. returnするとinspectを実行してる関数から抜ける
- __ ; wildcard
- `guard:`は `if (expression)`をかける

## 識別子(identifier)パターン

- identifierは任意の値にマッチする
- identifierはvを参照するlvalueとして振る舞う
- スコープは次のパターンラベルの直前のstatementの終わりまで

```
int v = ...;
inspect (v) {
  x: std::cout << x; // identifier pattern
}
```

## 式(expression)マッチ

下記のeはconstant-expression
```
inspect (v) {
  e: ...
}
```
について`e.match(v)`または`match(e, v)`の結果がtrueのときにマッチする。
match(e, v)のデフォルト挙動はe == v

```
inspect (v) {
  0: std::cout << "zero";
  1: std::cout << "one";
}
```

```
enum class Color { Red, Green, Blue };
Color color = ...;
inspect (color) {
  Color::Red: ...
  Color::Green: ...
  Color::Blue: ...
}
```

以下はidentifier patternになる
```
static constexpr int zero = 0, one = 1;
int v = 42;
inspect (v) {
   zero: std::cout << zero; // zeroという名前の一時変数が作られる
}
// prints: 42
```
Q. 42が表示されてうれしいか / 間違えないか?

```
static constexpr int zero = 0, one = 1;
std::pair<int, int> p = /* ... */
inspect (p) {
  case [zero, one]: {
  //    ˆˆˆˆ  ˆˆˆ id-expression
    std::cout << zero << ' ' << one;
  }
}
```
これもidentifier expression


## 代替パターン

以下の順でマッチを試す
- `<auto> pattern`
- `<concept> pattern`
- `<type>` pattern
- `<constant-expression> pattern`

### std::variant
vに対して`V = std::remove_cvref_t<decltype(v)>`とし
`get<v.index()>(v)`が正しく設定されているならそれにマッチする

従来
```
std::visit([&](auto&& v) {
  using V = std::remove_cvref_t<decltype(v)>;
  if constexpr (C1<V>()) {
    strm << "got C1: " << v;
  } else if constexpr (C2<V>()) {
    strm << "got C2: " << v;
  }
}, v);
```

提案
```
inspect (v) {
  <C1> c1: strm << "got C1: " << c1;
  <C2> c2: strm << "got C2: " << c2;
}
```

従来
```
std::visit([&](auto&& v) {
  using V = std::remove_cvref_t<decltype(v)>;
  if constexpr (std::is_same_v<int, V>) {
    strm << "got int: " << v;
  } else if constexpr (
      std::is_same_v<float, V>) {
    strm << "got float: " << v;
  }
}, v);
```

提案
```
inspect (v) {
  <int> i: strm << "got int: " << i;
  <float> f: strm << "got float: " << f;
}
```

### indexの場合
```
std::variant<int, int> v = /* ... */;
```

従来
```
std::visit([&](auto&& x) {
  switch (v.index()) {
    case 0: {
      strm << "got first: " << x; break;
    }
    case 1: {
      strm << "got second: " << x; break;
    }
  }
}, v);
```

提案
```
std::variant<int, int> v = /* ... */;

inspect (v) {
  <0> x: strm << "got first: " << x;
  <1> x: strm << "got second: " << x;
}
```

## Anyの場合
```
std::any a = 42;
```

従来
```
if (int* i = any_cast<int>(&a)) {
  std::cout << "got int: " << *i;
} else if (float* f = any_cast<float>(&a)) {
  std::cout << "got float: " << *f;
}
```

提案
```
inspect (a) {
  <int> i: std::cout << "got int: " << i;
  <float> f: std::cout << "got float: " << f;
}
```

### 多態

クラス
```
struct Shape { virtual ~Shape() = default; };
struct Circle : Shape { int radius; };
struct Rectangle : Shape { int width, height; };
```

従来
```
virtual int Shape::get_area() const = 0;

int Circle::get_area() const override {
  return 3.14 * radius * radius;
}
int Rectangle::get_area() const override {
  return width * height;
}
```

提案
```
int get_area(const Shape& shape) {
  return inspect (shape) {
    <Circle>    [r]    => 3.14 * r * r,
    <Rectangle> [w, h] => w * h
  }
}
```

## Dereferenceパターン
- `(*!)` ; `*v`がマッチするなら
- `(*?)` ; `v`がboolに変換できてtrueになるなら

```
struct Node {
  int value;
  std::unique_ptr<Node> lhs, rhs;
};

void print_leftmost(const Node& node) {
  inspect (node) {
    [.value: v, .lhs: nullptr]: std::cout << v << '\n';
    [.lhs: (*!)l]: print_leftmost(l);
  }
}
```

## extractorパターン
constant-expression cに対して
- `(c!)` ; c.extract(v)なら
- `(c?)` ; c.try_extract(v)なら

```
struct Email {
  std::optional<std::array<std::string_view, 2>>
    try_extract(std::string_view sv) const;
  };

inline constexpr Email email;

struct PhoneNumber {
  std::optional<std::array<std::string_view, 3>>
  try_extract(std::string_view sv) const;
};

inline constexpr PhoneNumber phone_number;
inspect (s) {
  (email?) [address, domain]: std::cout << "got an email";
  (phone_number?) ["415", __, __]: std::cout << "got a San Francisco phone number";
  // ˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆˆ extractor pattern
}
```

## パターンガード

```
inspect (v) {
  pattern1 if (cond1): stmt1
  pattern2: stmt2
  // ...
}
```

は

```
if (MATCHES(pattern1, v) && cond1) stmt1
else if (MATCHES(pattern2, v)) stmt2
// ...
```

の意味

## 網羅性と有用性
`[[strict]]`をつけると網羅性と有用性のチェックをしてくれる
- 網羅性 ; ありえる値の全てが少なくともどれか一つのケースで処理される
  - ; `__`があるそれが残り全部にマッチ
- 有用性 ; 全てのケースが少なくともどれか一つを処理する
  - ; `__`の後ろに書いたケースは絶対にマッチしない

# デザイン

## 反駁

- 反駁性(refutable) ; 失敗する可能性のあるパターン
  - expression pattern
- 反駁不可性(irrefutable) ; 失敗しないパターン
  - identifier pattern

構造化束縛はirrefutableであるべきか / refutableを許容するか

- 構造化束縛の拡張
  - 変数は新しく割り当てるのではなく、参照として扱う
- switchの問題点
  - caseをどこにも書くことができたので扱いづらい
  - breakを書き忘れるとエラーになりやすい
- 効率
  - switchは整数に制限することで効率的だった
  - 従来の書き方と同等の効率のよいコード生成を目指す
- autoではなくlet
  - bindingパターンでletを使う
  - autoだと新しい変数を宣言してstorageに影響がある
- best matchではなくfirst match
  - 単純化のためにfirst match
    - 多重ロードされた関数の解決は複雑
- 副作用を制限しない
- ライブラリではなく言語サポート
  - 識別子の導入、構文オーバーヘッド、最適化の機会のため


## 副作用を制限しない

```
bool f(int &); // defined in a different translation unit.
int x = 1;
inspect (x) {
  0: std::cout << 0;
  1 if (f(x)): std::cout << 1;
  2: std::cout << 2;
}
```

これを次のどちらの方法で処理するか。

副作用を許可しないなら多少最適化可能
```
switch (x) {
  case 0: std::cout << 0; break;
  case 1: if (f(x)) { std::cout << 1; } break;
  case 2: std::cout << 2; break;
}
```

副作用を許可する
```
if (x == 0) std::cout << 0;
else if (x == 1 && f(x)) std::cout << 1;
else if (x == 2) std::cout << 2;
```

後者にする。

## その他
- any_of{1, 2, 3} ; 1|2|3
- within{1, 10} ; 1..10
- (both!) [[x, 0], [0, y]] ; [x, 0] & [0, y]
- (at!) [p, [x, y]] ; p @ [x, y]
- etc.