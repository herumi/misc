# 検証可能なランダム関数(VRF : Verifiable Random Functions)

[draft-irtf-cfrg-vrf](https://datatracker.ietf.org/doc/draft-irtf-cfrg-vrf/)

## 概要
- VRFはハッシュ関数の公開鍵バージョン
- 秘密鍵でハッシュの値を計算できる
- 検証鍵(公開鍵)でハッシュの値の正しさを検証できる

## 用途
- ハッシュベースのデータ構造に対する辞書攻撃に対するプライバシーを守る
- 証明者はVRFの秘密鍵を持ち入力データに対して(ハッシュ値の)データを構築する
  - 証明者のみがデータに対してクエリがあるか無いかを答えられる
  - 検証鍵で証明者が正しいことを確認できる
  - データに対するオフライン攻撃はできない

## アルゴリズム

- KeyGen
  - 公開鍵PKと秘密鍵SKを生成する
- ハッシュ値と証明の計算
  - 証明者は入力データmに対してSKでハッシュ値を計算する h = VRF_hash(SK, m)
    - VRF_hashは決定的アルゴリズム
  - 証明者はmとSKで証明を計算する pi = VRF_prove(SK, m)
- 検証
  - 検証者は証明piからhを計算できる h = VRF_proof_to_hash(pi)
    - VRF_hash(SK, m) = VRF_proof_to_hash(VRF_prove(SK, m))
  - VRF_verify(PK, m, pi)はpiが正しいときのみvalidを返す

## 必要要件
- 一意性
  - 任意の固定されたPKと入力値mに対してvalidになるhは一意でなければならない
    - たとえSKが知られていたとしても
- full uniqueness
  - (制限された計算能力を持つ)攻撃者はPKとmとpi1, pi2でVRF_verify(PK, m, pi1), verify(PK, m, pi2)がともにvalidでVRF_proof_to_hash(pi1) != VRF_proof_to_hash(pi2)であるものを作れない。

- full collision resistance
  - m != m'でVRF_hash(SK, m) = VRF_hash(SK, m')となるm, m'を見つけられない
- trusted collision resistance
  - PKとSKが正規の方法で作られたときにfull collision resistanceを満たす

- pseudorandomness
  - (PK, SK)が正規の方法で作られていて
  - 攻撃者がSKを知らないときに
  - 出力hが乱数と区別できない

- SKを知っていればVRF_hash(SK, m)と比べることで容易に識別できる
- piを知っていればVRF_proof_to_hash(pi)と比べることで容易に識別できる

## ECVRF
- n = 128
- F ; 位数が約2nの有限体
- E/F ; F上定義された楕円曲線
- G ; Eの位数qの部分群(qは素数) ; q<2^(8qLen)でqLenが2nに近いもの
- cofactor ; #(E) / q
- B ; Gの生成元
- Hash ; 2nビット出力ハッシュ関数
- ECVRF_hash_to_curve ; Gへのハッシュ関数
- x = SK ; 秘密鍵
- Y = xB ; 公開鍵

### ECVRF_prove(SK, m)
- x = SK ; 秘密鍵
- m ; 文字列
- H = ECVRF_hash_to_curve(Y, m)
- γ = xH
- k = ECVRF_nonce_generation(SK, H)
- c = ECVRF_hash_points(H, γ, kB, kH)
- s = (k + cx) mod q
- pi = (γ, c, s)

### ECVRF_proof_to_hash(pi)
- pi = (γ, c, s)
- h = Hash(cofactor * γ)

### ECVRF_verify(Y, pi, m)
- pi = (γ, c, s)
- H = ECVRF_hash_to_curve(Y, m)
- U = sB -cY
- V = sH - cγ
- c' = ECVRF_hash_points(H, γ, U, V)
- cとc'が等しければvalid

## 正当性
正しく作られたpi = (γ, c, s) = (xH, c, s)に対して
- U = sB - cY = (k+cx)B - c(xB) = kB
- V = sH - cγ = sH - cxH = (s - cx)H = kH
よってc = c'
