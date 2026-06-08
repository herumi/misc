# [ContractsforC++: P2900R14](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2900r14.pdf) と CBMCの紹介

## 最初の方の文法例

```cpp
int f(const int x)
  pre (x != 1) // precondition specifier
  post (r : r == x && r != 2) // postcondition specifier; r names the result object of f
{
  contract_assert (x != 3); // assertion statement
  return x;
}
```

```cpp
int f(int i)
  pre (i >= 0);
```

```cpp
void clear()
  post (empty());
```


- コンストラクタの事前条件アサーションやデストラクタの事後条件アサーションの述語が、非静的データメンバを直接参照したらill-formed

```cpp
struct X {
  int i = 0;
  bool f();
  X()
    pre (i == 0) // error
    pre (f()) // error
    pre (check(&this->i)) // OK
    pre (this->f()) // OK
  {}
  ~X()
    pre (i == 0) // OK
    post (i == 0) // error
    post (f()) // error
    post (check(&this->i)) // OK
    post (this->f()); // OK
};
```

## [P4208R0](https://wg21.link/P4208) C++29向けContracts拡張の提案ロードマップ

C++26でcontract assertionsが導入されたが、最小構成のため大規模利用には不十分。C++29で以下を優先すべきと提案。

### 優先基準（A〜Fを3つ以上満たすものを選定）

記号|基準
-|-
A|多様なユーザーに高い価値がある
B|C++26標準化中に繰り返し指摘された懸念への対処
C|提案が設計完了でEWG/LEWGレビュー可能な状態
D|すでにWG21サブグループでレビュー済みでフィードバック反映済み
E|完全なワーディング（仕様文言）が揃っている
F|主要コンパイラに1つ以上の実装がある

### 優先度順の6つの拡張プラン

1. 仮想関数への pre/post サポート（P3097R2）
- C++26では仮想関数に pre/post を書くとill-formed
- virtual qint64 readData(...) pre(data != nullptr) = 0; のような記述を可能にする
- GCCに実装済み、EWGで以前に強いコンセンサスを得ていたが直前で削除された経緯あり

2. contract violation を直接トリガーするライブラリAPI（P3290R4）
- handle_enforced_contract_violation() 等の関数を追加
- 独自のassertionマクロをstandard contract-violation handlerに統合できる
- 「contract assertionを使いたくない」ユーザーのニーズに対応

3. ラベル（Labels）による評価セマンティクスの制御（P3400R3）
- pre<label>(expr) 構文でassertion単位に挙動を制御
- 常に強制するもの（セキュリティチェック）、常に無視するもの（legacy追加分）などを区別可能
- ライブラリ単位でのグループ制御や、audit ラベルで重い検査を専用ビルドのみ有効化

4. 暗黙的contract assertions（UB対策）（P3100R6）
- コンパイラが配列外アクセス・符号付き整数オーバーフロー等のUBをruntimeチェックに変換
- コード変更不要でビルドオプションで有効化（-fbounds-safety 等の標準化版）
- SG21/SG23/EWGで4回審査し全て強いコンセンサスを得ている

5. Postconditionキャプチャ（P3098R2）
- C++26では関数呼び出し時点の状態をpostconditionから参照できない
- post [old_size = size()] (size() == old_size + 1) のような構文を導入
- 値渡しパラメータをconstなしにpostconditionで参照できるようにもなる

6. ユーザー定義メッセージ（P3099R2）
- pre(i < size(), "Out-of-bounds access") のように診断メッセージを追加可能
- static_assert と一貫した構文

### C++29に含めない拡張（設計課題あり・優先度低）
- 関数ポインタへの pre/post
- クラス不変条件（invariants）
- ローカルcontract-violation handler
- コルーチンのpostcondition
- パック展開、前提条件チェック演算子 など

## [cbmc](https://www.cprover.org/cbmc/)
C/C++向けの有界モデル検査ツール

- メモリ安全性（配列境界チェック、ポインタの安全な利用）
- さまざまな未定義動作の検出
- ユーザーが記述したアサーションの検証
  - Verilogなど他言語とのI/O等価性チェックにも対応
- 検証はプログラム中のループを展開し得られた論理式を決定手続き（ソルバ）に渡すことで行う
- Linux、Windows、macOS対応
- ビットベクタ式向けの内蔵ソルバ（MiniSatベース）を備える
  - 外部SMTソルバ（Boolector、CVC5、Z3）も利用可能

### 機能
主な[API]( https://github.com/diffblue/cbmc/blob/develop/doc/cprover-manual/api.md)

- __CPROVER_assume と assert の挙動
  - CBMCはすべての可能な入力に対して検証するモデル検査ツール
  - この2つの関数は「探索空間の制約」と「検証目標」を担う

- __CPROVER_assume(cond);
  - 条件が 偽になるパスを探索空間から除外する
  - 全入力空間 → assume でフィルタ → 残った空間のみ検証
  - 条件が偽のパスは「存在しないもの」として扱われる（単に無視する）
  - __CPROVER_assume(0) を書くと到達不能になり、その後の検証は行われない
  - 通常のCの動作には一切影響しない（CBMC専用の宣言）

- assert(cond)
  - 制約を満たした残りの空間の中で、条件が偽になる例を探す
  - 偽になる例（反例）が見つかれば → VERIFICATION FAILED + --trace でそのパスを出力
  - すべてのパスで真なら → VERIFICATION SUCCESSFUL

- __CPROVER_assert(cond, msg); // メッセージ付き assert
- __CPROVER_cover(cond); // テスト生成用のカバレッジ目標指定

- 算術オーバーフロー検出

```cpp
__CPROVER_overflow_plus(a, b) // a + b がオーバーフローするか
__CPROVER_overflow_minus(a, b) // a - b
__CPROVER_overflow_mult(a, b) // a * b
__CPROVER_overflow_shl(a, b) // a << b
__CPROVER_overflow_unary_minus(a) // -a
```
- 戻り値は bool。オーバーフローするなら true

- 浮動小数点チェック

```cpp
__CPROVER_isnan(x) // NaN か
__CPROVER_isinf(x) // ±∞ か
__CPROVER_isfinite(x) // 有限値か
__CPROVER_isnormal(x) // 正規化数か
__CPROVER_sign(x) // 負か (符号ビット)
```
- x == x によるNaN除外の代わりに !__CPROVER_isnan(x) と書ける

- 絶対値

```cpp
__CPROVER_abs(x) // int
__CPROVER_labs(x) // long
__CPROVER_fabs(x) // doubleなど
```

- ポインタ・メモリ

```cpp
__CPROVER_r_ok(p, size) // 読み取り安全か
__CPROVER_w_ok(p, size) // 書き込み安全か
__CPROVER_same_object(p, q) // 同じオブジェクトを指すか
__CPROVER_POINTER_OFFSET(p) // ベースからのオフセット
__CPROVER_OBJECT_SIZE(p) // オブジェクトのサイズ
__CPROVER_havoc_object(p) // オブジェクト全体を非決定的に
__CPROVER_havoc_slice(p, s) // 指定範囲を非決定的に
```

- 配列

```cpp
__CPROVER_array_equal(a, b) // 配列の内容が等しいか
__CPROVER_array_copy(dst, src)
__CPROVER_array_set(dst, val) // 全要素を val で初期化
```

- 非決定的値の生成

```cpp
int __CPROVER_nondet_int();
bool __CPROVER_nondet_bool();
// 型に応じて nondet_<type>() が存在
// __CPROVER_assume と組み合わせて「条件を満たす任意の値」を作るのに使う
```

- オーバーフロー検出が実用的に便利

```cpp
int a, b;
__CPROVER_assume(!__CPROVER_overflow_plus(a, b));
assert(a + b > a); // オーバーフローなしで成立するか検証
```

## `__CPROVER_assume` と `assert` をつかったパズルの解法

| 関数 | 役割 |
|---|---|
| `__CPROVER_assume(cond)` | 条件が偽になるパスを探索空間から除外する（前提条件） |
| `assert(cond)` | 条件が偽になる入力を探す（検証目標） |
| `assert(0)` | 常に偽なので「assumeを全部満たす入力」を反例として出力させるトリック |

```
全入力空間
    ↓ __CPROVER_assume で制約を絞る
制約を満たす空間
    ↓ assert(0) で反例を要求
解として出力（--trace で確認）
```

- [square root](../cbmc/square.c)
- [数独を解くサンプル](../cbmc/sudoku.cpp)



## 関連文書のリスト

Paper | 日 | 種類 | 主題 | 位置づけ
-----|------|------|------|---------
[P2695](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2695r1.pdf) | 2022 | Plan | ロードマップ | 委員会全体の合意形成と開発計画
[P2680](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2680r1.pdf) | 2022 | Proposal | safety by default | 副作用をどこまで禁止するか
[P2751](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2751r0.pdf) | 2023 | Proposal | 例外送出時における事後条件の評価 | 異常終了したときどうするか
[P2811](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2811r7.pdf) | 2023 | Proposal | 契約違反ハンドラ | ログ出力等を可能にする拡張性の定義
[P2899](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2899r1.pdf) | 2024 | Rationale | 設計思想と根拠 | この仕様になった背景を説明
[P3362](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3362r0.pdf) | 2024 | Position Paper | 厳格/純粋な述語の必須化の要求 | 副作用のある契約は認めるべきでない
[P3285](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3285r1.pdf) | 2024 | Proposal | 契約式における副作用の排除 | 意図しないバグや未定義動作を防ぐため、式の制限ルールを具体化する案
[P3173](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3173r0.pdf) | 2024 | Review | P2900 MVPに対する懸念事項の提示 | コンパイラ最適化との兼ね合いや、仕様の穴を指摘
[P3499](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3499r0.pdf) | 2025 | Discussion | 厳格な契約述語の模索 | 副作用論争を調停するために、より安全な式評価の方向性を探る提案
[P3229R1](https://wg21.link/P3229) | 2025-03 | Semantics | UBと契約の整合性 | 言語仕様の一貫性を確保
[P3099R2](https://wg21.link/P3099) | 2025-12 | UX | カスタム診断メッセージ | デバッグ体験を改善
[P3100R5](https://wg21.link/P3100) | 2025-12 | Feature | 暗黙契約 | 静的解析を支援
[P3400R2](https://wg21.link/P3400) | 2025-12 | Control | 契約評価モード | 実行時の挙動を制御
[P3097R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3097r1.pdf) | 2025-12-13 | Feature | 仮想関数の契約 | OOPとの統合を強化
[P3911R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3911r2.html) | 2026-01-14 | Spec Improvement | 契約を無視不能にする | C++26向け意味論の重要修正
[P4043R0](https://wg21.link/P4043) | 2026-03 | Policy | C++26採用の再検討 | 標準化時期への問題提起
[P3290R4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3290r4.pdf) | 2026-05-02 | Practical | assertとの統合 | 既存コード移行を支援
[P3850R0](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3850r0.pdf) | 2026-05-12 | Strategy | C++29に向けた契約ロードマップ | 今後の方向性を提示
[P4208R0](https://wg21.link/P4208) | 2026-05-12 | Review | P2900への批判的評価 | 設計上の論点を整理
