# ハイブリッド公開鍵暗号

## [HPKE](https://www.ietf.org/archive/id/draft-irtf-cfrg-hpke-09.html)

受信者の公開鍵に対して、任意サイズの平文の認証付き公開鍵暗号化を行う。

- KEM ; 共通鍵のカプセル化
- HKDF ; HMACベースの鍵導出関数
- AEAD ; 認証付き暗号
- ECDH ; 楕円曲線によるDH鍵共有

を利用する。

従来のハイブリッド暗号は、共通鍵を公開鍵で暗号化する仕組みの定式化。
HPKEは共通鍵を生成し、それを公開鍵で暗号化する仕組み全体を定式化する。

従来の方式は相互運用に難があり、古い暗号プリミティブに基づいていたり、証明がなかったり、テストベクトルがないです。
IND-CCA2安全なものを目指す。

## 記号
- S ; 暗号文の送信者
- R ; 暗号文の受信者
- E ; 一回だけのランダムな役割
- (skX, pkX) ; S∈{S, R, E}に対するKEM
  - skX ; 秘密鍵
  - pkX ; 公開鍵
- pk(skX) ; 秘密鍵skXに対応する公開鍵

### KEM
- (skX, pkX) ← GenerateKeyPair() ; ランダムな鍵生成アルゴリズム
- (skX, pkX) ← DeriveKeyPair(ikm) ; ikmを入力とする決定的鍵生成アルゴリズム
- enc ← Encap(pkR) ; 一時的な共通鍵暗号の秘密鍵を生成してpkRに対応する秘密鍵で復号可能な公開鍵でカプセル化するランダムアルゴリズム
- Decap(enc, skR) ; encをskRで復号して一時的な共通鍵暗号の秘密鍵を復号するアルゴリズム
- enc ← AuthEncap(pkR, skS) ; skSによる認証機能付Encap(pkR)
- AuthDecap(enc, skR, pkS) ; pkSにより検証可能なDecap(enc, skR)

### KDF
- prk ← Extract(salt, ikm) ; ikmとsalt(option)から擬似乱数鍵を取り出すアルゴリズム
- Expand(prk, info, L) ; infoを用いてprkを引き延ばしてLバイトの出力を得る

## AEAD
- ct ← Seal(key, nonce, aad, pt) ; 共通鍵keyとナンスnonceと付加データaad, 平文ptを暗号化
- pt or bot ← Open(key, nonce, aad, ct) ; ctを復号して正しければpt. そうでなければbot

## DHベースのKEM
- DH(skX, pkY) ; 非対話DH鍵共有

### 暗号化の概要

```
def Encap(pkR): ; 受信者の公開鍵pkR
  skE, pkE = GenerateKeyPair() ; 一時的な秘密鍵skEと公開鍵pkEを作る
  dh = DH(skE, pkR)            ; pkRとskEでDH鍵共有してdhを作る
  key = ExtractAndExpand(dh, pkE, pkR) ; dhとpkEとpkRから共有秘密鍵keyを作る
  return key, pkE
```

def Decap(pkE, skR): ; 送信者の公開鍵pkEと受信者の秘密鍵skR
  dh = DH(skR, pkE)  ; DH鍵共有

  key = ExtractAndExpand(dh, pkE, pk(skR))
  return key

def AuthEncap(pkR, skS):
  skE, pkE = GenerateKeyPair()
  dh = concat(DH(skE, pkR), DH(skS, pkR))
  enc = SerializePublicKey(pkE)

  pkRm = SerializePublicKey(pkR)
  pkSm = SerializePublicKey(pk(skS))
  kem_context = concat(enc, pkRm, pkSm)

  shared_secret = ExtractAndExpand(dh, kem_context)
  return shared_secret, enc

def AuthDecap(enc, skR, pkS):
  pkE = DeserializePublicKey(enc)
  dh = concat(DH(skR, pkE), DH(skR, pkS))

  pkRm = SerializePublicKey(pk(skR))
  pkSm = SerializePublicKey(pkS)
  kem_context = concat(enc, pkRm, pkSm)

  shared_secret = ExtractAndExpand(dh, kem_context)
  return shared_secret

```
