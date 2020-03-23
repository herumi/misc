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

提案
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

### 入れ子
従来
```
auto const& [topLeft, unused] = getBoundaryRectangle();
auto const& [topBoundary, leftBoundary] = topLeft;
```

```
auto const& [[topBoundary, leftBoundary], __] = getBoundaryRectangle();
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
  pattern => expression,
  pattern: statement
}
```
- `=>` ; 返すための値を生成する式
- `:` ; 値は返さない. 中身を実行する. returnするとinspectを実行してる関数から抜ける
- __ ; wildcard

## マッチ

下記のeはconstant-expression
```
inspect (v) {
  e: ...
}
```
について`e.match(v)`または`match(e, v)`の結果がtrueのときにマッチする。
match(e, v)のデフォルト挙動はe == v

```
static constexpr int zero = 0, one = 1;
int v = 42;
inspect (v) {
   zero: std::cout << zero;
// ^^^^ identifier pattern
}
// prints: 42
```
Q. なぜ42が表示されるの?

## 代替パターン
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