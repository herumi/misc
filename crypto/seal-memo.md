# [Microsoft SEAL](https://github.com/microsoft/SEAL)メモ

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
EncryptionParameters parms(scheme_type::bfv); // BFV用パラメータを作成する

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

- 再線型化(poly_modulus_degree=4096でないとできない?)
- サイズ3の暗号文をサイズ2の暗号文にする
```cpp
RelinKeys relin_keys;
keygen.create_relin_keys(relin_keys); // keygenから再線型鍵を作る

evaluator.relinearize_inplace(x_squared, relin_keys); // 暗号文サイズを縮小する
```

## バッチエンコード
[2_encoders.cpp](https://github.com/microsoft/SEAL/blob/main/native/examples/2_encoders.cpp)
前述の方法では
1. poly_modulus_degree(=N)で割った余りの値しか計算できない
1. 平文多項式の係数のうち1個しか使っていない

それを改善するためにバッチエンコード(BFV or BGV)を使う

- N : poly_modulus_degree
- T : plain_modulus

バッチによってBFV平文多項式は各要素がmod Tの整数の2x(N/2)行列とみなせる
- 要素ごとに操作ができる
- 処理速度が上がる
  - バッチエンコードをそのまま使うだけでは一つ目の問題は解決しない
  - 演算結果がNを超えたかは分からない

バッチのためにplain_modulusをplain_modulus mod 2N=1となる素数に設定する

```cpp
// 20ビットの素数を設定する
parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
```

前回と同じ方法で秘密鍵、公開鍵、再線型化鍵などを作る
```cpp
KeyGenerator keygen(context);
SecretKey secret_key = keygen.secret_key();
PublicKey public_key;
keygen.create_public_key(public_key);
RelinKeys relin_keys;
keygen.create_relin_keys(relin_keys);
Encryptor encryptor(context, public_key);
Evaluator evaluator(context);
Decryptor decryptor(context, secret_key);
```

バッチエンコード用のインスタンスを用意する
```cpp
BatchEncoder batch_encoder(context);
```
- バッチのslot=poly_modulus_degree=N
- N個の整数の要素をN/2個x2の行列成分とみなす

行列を平文に設定する(エンコード)
```cpp
vector<uint64_t> pod_matrix(slot_count);
Plaintext plain_matrix;
batch_encoder.encode(pod_matrix, plain_matrix);
```

暗号化する
```cpp
Ciphertext encrypted_matrix;
encryptor.encrypt(plain_matrix, encrypted_matrix);
```
いろいろ操作する
```cpp
evaluator.add_plain_inplace(encrypted_matrix, plain_matrix2);
evaluator.square_inplace(encrypted_matrix);
evaluator.relinearize_inplace(encrypted_matrix, relin_keys);
```

復号してからデコードする
```cpp
Plaintext plain_result;
decryptor.decrypt(encrypted_matrix, plain_result);
batch_encoder.decode(plain_result, pod_result);
```

## CKKS
- CKKSは準同型の近似計算をする
BFVとの違い
- plain_modulusパラメータを使わない
- coeff_modulusの選択が重要
  - ここで5個の40ビット素数を選ぶ

```cpp
EncryptionParameters parms(scheme_type::ckks);

size_t poly_modulus_degree = 8192;
parms.set_poly_modulus_degree(poly_modulus_degree);
parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 40, 40, 40, 40, 40 }));
```

コンテキストや秘密鍵、公開鍵、再線型化鍵などを作成するのは同じ

CKKSはscaleパラメータを使ってエンコードする

```cpp
vector<double> input{ 0.0, 1.1, 2.2, 3.3 };
Plaintext plain;
double scale = pow(2.0, 30);
encoder.encode(input, scale, plain);
```

暗号化して計算して復号してデコード
```cpp
Ciphertext encrypted;
encryptor.encrypt(plain, encrypted);
evaluator.square_inplace(encrypted);
evaluator.relinearize_inplace(encrypted, relin_keys);
decryptor.decrypt(encrypted, plain);
encoder.decode(plain, output);
```
乗算するとscaleは2倍になる

## レベル
[3_levels.cpp](https://github.com/microsoft/SEAL/blob/main/native/examples/3_levels.cpp)
SEALは暗号化パラメータを256ビットのハッシュ値params_idで管理している。

カスタムcoef_modulus
- N=poly_modulus_degree=8129でカスタムcoef_modulusを50,30,30,50,50とする
  - MaxBitCount(N) = 218で > 50+30+30+50+50=210なのでOK
modulusスイッチイングチェイン
- 最後の素数をspecial primeという
  - special primeは他の素数よりも大きい
  - 最初から順番にlevel 0(lowest level), level 1, level 2, level 3(highest data level), level 4(key level)
- 秘密鍵のような全ての鍵は最上位レベルとして作られる
- 暗号文のようなデータは最下位レベルとして作られる

コンテキストへの参照
```cpp
SEALContext::key_context_data() // key level
SEALContext::first_context_data() // highest level
SEALContext::last_context_data() // lowest level
```
