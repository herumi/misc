# [SEAL](https://github.com/microsoft/SEAL) memo

## サンプルビルド

```
mkdir build
cd build
cmake .. -DSEAL_BUILD_EXAMPLES=ON -DSEAL_BUILD_BENCH=ON
```

## パラメータ
### 初期化に必要なパラメータ

- 多項式modulus次数(poly_modulus_degree : 2のべきであること)
  - 2のべきの円分多項式を表す
  - 大きくなると
    - 暗号文が大きくなる
    - 演算が遅くなる
    - 複雑な計算ができる
- 係数modulus(coeff_modulus)
  - 最大60bit程度の異なる素数の積
  - 大きいと大きなnoise budgetになるが上限は多項式modulus次数で決まる
  - たとえばpoly_modulus_degree=4096なら36ビットの素数3個の積にできる
- 平文modulus(BFF専用)
  - 任意の正の整数
    - 2のべきにすることもあれば素数にすることもある
  - 平文の型のサイズと乗算時のnoise budgetの消費量を決める
    - noise budget = log_2(coeff_modulus/plain_modulus)
  - 平文の型を小さく保つとパフォーマンスが上がる

これらを設定すると`SEALContext`を生成できる

### noise budget
- 暗号文を作ったときに決定される
- 準同型演算をするとnoise budgetは消費される
  - addはほとんどbudgetを消費しない
  - mulはbudgetを消費する
  - noise budgetが0になると復号できなくなる

### 128ビットセキュリティ

多項式modulusの次数|係数modulusの最大ビット長
-|-
1024 | 27
2048 | 54
4096 | 109
8192 | 218
16384| 438
32768| 881

## BFV方式の手順例
[1_bfv_basics.cpp](https://github.com/microsoft/SEAL/blob/main/native/examples/1_bfv_basics.cpp)

### システム初期化
```cpp
EncryptionParameters parms(scheme_type::bfv); // パラメータを作成する

// paramsの各種設定をする
// ...

SEALContext context(parms); // パラメータからコンテキストを作成する
```

### 鍵生成

```cpp
KeyGenerator keygen(context);
SecretKey secret_key = keygen.secret_key(); // keygenから秘密鍵を作る

PublicKey public_key;
keygen.create_public_key(public_key); // 秘密鍵から公開鍵を作る
```

### 各操作をするもの

```cpp
Encryptor encryptor(context, public_key); // 暗号化するencryptorを公開鍵から作る
Evaluator evaluator(context); // 暗号文を評価するevaluator
Decryptor decryptor(context, secret_key); // 暗号文を復号するdecryptorを秘密鍵から作る
```

### 多項式の評価

- 平文 $x = 6$ を暗号化する
- $4x^4 + 8x^3 + 8x^2 + 8x + 4$ を暗号文のまま評価する
  - 評価する多項式の次数はpoly_modulus_degreeよりも小さい

- 平文生成
```cpp
uint64_t x = 6;
Plaintext x_plain(uint64_to_hex_string(x)); // 平文は多項式の文字列表記
```
- 暗号化
```cpp
Ciphertext x_encrypted;
encryptor.encrypt(x_plain, x_encrypted);
```

- $c^2+1$ を計算する( $c=Enc(x)$ )
```cpp
Ciphertext x_sq_plus_one;
evaluator.square(x_encrypted, x_sq_plus_one); // c^2
Plaintext plain_one("1");
evaluator.add_plain_inplace(x_sq_plus_one, plain_one); // c+1
```
- 最初の暗号文はサイズ2
- サイズMとサイズNの暗号文をmulするとサイズM+N-1の暗号文になる

- 復号
```cpp
Plaintext decrypted_result;
decryptor.decrypt(x_sq_plus_one, decrypted_result);
```

- 再線型化
- サイズ3の暗号文をサイズ2の暗号文にする
```cpp
RelinKeys relin_keys;
keygen.create_relin_keys(relin_keys); // keygenから再線型鍵を作る

evaluator.relinearize_inplace(x_squared, relin_keys); // 暗号文サイズを縮小する
```
