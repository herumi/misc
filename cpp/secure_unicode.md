# [C++ Identifier Security using Unicode Standard Annex 39](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2528r0.html)

[P2528R0 C/C++ Identifier Security using Unicode Standard Annex 39](https://onihusube.hatenablog.com/entry/2022/03/19/224729#P2528R0-CC-Identifier-Security-using-Unicode-Standard-Annex-39)

- [C++ Identifier Syntax using Unicode Standard Annex 31](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1949r7.html)がC++23で入る。
- Unicodeなんでもありではなくzero-withや制御文字を禁止する[Unicode Identifier and Pattern Syntax](https://unicode.org/reports/tr31/)
- ソースコードはNFCとして正規化すべき
- 割り当てられていないコードポイントの仕様は不適格(ill-formed)
- FFFFより小さい絵文字は除外

加えて[Unicode Security Mechanisms Annex 39](https://unicode.org/reports/tr39/)というセキュリティ上の問題を解決する規格を実装すべきという案

- 混合スクリプトの禁止[Optional Detection](https://unicode.org/reports/tr39/#Optional_Detection)

[libu8ident - Check unicode security guidelines for identifiers](https://github.com/rurban/libu8ident/)

この範囲外のもの

- [Trojan Source：Invisible Vulnerabilities](https://www.trojansource.codes/trojan-source.pdf)
  - 文字列の中でテキストの向きを切り換えるbidi-override
  - ホモグリフ砲撃
- [RHSB-2021-007 トロイの木馬ソース攻撃 CVE-2021-42574](https://access.redhat.com/ja/security/vulnerabilities/6466581)
