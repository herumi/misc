# Intelの機械学習用命令を使う

Intelは第2世代Xeon SP(Cascade Lake)で機械学習の推論処理に有効な命令群DL Boostをサポート、
そして今年度前半登場予定の次世代Xeon SP(Cooper Lake)でbfloat16命令をサポートします。
ここではJITアセンブラXbyakを用いたそれらの命令の利用方法を紹介します。

## 準備
WindowsやLinux上で動作するC++コンパイラを準備して[Xbyak](https://github.com/herumi/xbyak)をcloneしてください。

```
git clone https://github.com/herumi/xbyak
```

Xbyakはヘッダファイルのみで構成されているのでコンパイラオプションに`-I<xbyak directory>`を指定するだけで使えます。

## 対応環境
サンプルはWindowsのVisual Studio, LinuxやmacOSのgcc/clangでコンパイルできます。

```
// Visual Studio
cl -I<xbyak directory> /EHsc sample.cpp
```

```
// gcc
gcc -I<xbyak directory> sample.cpp
```

## サンプルコード
- [vpdpbusd-test.cpp](https://github.com/herumi/misc/blob/master/avx-512/vpdpbusd-test.cpp)
  - Cascade Lake用8bit積和演算命令`vpdpbusd`のサンプル
- [bfloat16-test.cpp](https://github.com/herumi/misc/blob/master/avx-512/bfloat16-test.cpp)
  - Cooper Lake用bfloat16用積和演算命令`vdpbf16ps`と変換命令`vcvtne2ps2bf16`のサンプル

## エミュレータ
Cooper Lakeは現時点でまだ発売されていないのでエミュレータを使って動作確認します。
[Intel Software Development Emulator Download](https://software.intel.com/content/www/us/en/develop/articles/pre-release-license-agreement-for-intel-software-development-emulator-accept-end-user-license-agreement-and-download.html)から
Intel SDEをダウンロードします。

```
sde -cpx -- サンプルプログラム
```
で実行します。`-cpx`はCooper Lakeエミュレーションを意味します。

## vpdpbusd
vpdpbusdはCascade Lakeでサポートされた8bit整数同士の積和演算命令です。

もう少し正確にいうと8bitの符号無し整数と8bitの符号あり整数の4個同士の積和を求め、
その結果(intに拡張されます)をデスティネーションレジスタに加算します。
AVX-512は8bitが64個あるので、この操作を16回並列に実行します。

Cで記述すると次の動作をします。

```
void vpdpbusdC(int *dst, const uint8_t *u, const int8_t *s)
{
    for (int i = 0; i < 16; i++) {
        int sum = dst[i];
        for (int j = 0; j < 4; j++) {
            sum += u[i * 4 + j] * s[i * 4 + j];
        }
        dst[i] = sum;
    }
}
```

```
dst[ 0] += u[ 0] * s[ 0] + u[ 1] * s[ 1] + u[ 2] * s[ 2] + u[ 3] * s[ 3];
dst[ 1] += u[ 4] * s[ 4] + u[ 5] * s[ 5] + u[ 6] * s[ 6] + u[ 7] * s[ 7];
dst[ 2] += u[ 8] * s[ 8] + u[ 9] * s[ 9] + u[10] * s[10] + u[11] * s[11];
...
dst[15] += u[60] * s[60] + u[61] * s[61] + u[62] * s[62] + u[63] * s[63];
```

## bfloat16
bfloat16は機械学習の計算で従来のfloat(32bit)ほどの精度が必要ないところで利用される16bit浮動小数点数です。

floatが1bitの符号、8bitの指数部、23bitの仮数部なのに対してbfloat16は符号と指数部が同じで仮数部を上位7bitに切り詰めたフォーマットで表現されます。

floatとbfloat16のフォーマット表
型|符号ビット(s)|指数部(e)|仮数部(f)|値|
-|-|-|-|-|
float|1|8|23|(-1)^s 2^(e-127)×(1 + f/2^24)|
bfloat16|1|8|7|(-1)^s 2^(e-127)×(1 + f/2^8)|

### floatとbfloat16の相互変換

準備としてfloatとuint32_tの相互変換関数を用意します。
```
union fi {
    float f;
    uint32_t u;
};

typedef uint16_t bfloat16;
loat u2f(uint32_t u)
{
    fi fi;
    fi.u = u;
    return fi.f;
}

uint32_t f2u(float f)
{
    fi fi;
    fi.f = f;
    return fi.u;
}
```

bfloat16からfloatに変換するには16bit整数として左16bitシフトすればOKです。

```
float bfloat16_to_float(bfloat16 f)
{
    return u2f(f << 16);
}
```

floatからbfloat16も仮数部の下位16bitを切り捨てるだけでもよいのですが、丸め処理を入れると若干精度がよくなります。

```
bfloat16 float_to_bfloat16(float f)
{
    // ignore denormal and infinity
    uint32_t u = f2u(f);
    uint32_t rounding = 0x7fff + ((u >> 16) & 1);
    u += rounding;
    return bfloat16(u >> 16);
}
```
ここではdenormalやinfinityの状態は無視しました。厳密にするには[tensorflowのbfloat16.h](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/lib/bfloat16/bfloat16.h)などを参考にしてください。

### `vcvtne2ps2bf16`

```
vcvtne2ps2bf16 dst, src1, src2
```
はsrc1, src2で指定されたfloat型のSIMDレジスタをbfloat16型に変換しつなげてdstに代入します。
Cでのコードは次の通りです。

```
void vcvtne2ps2bf16C(bfloat16 *dst, const float *src1, const float *src2)
{
    for (int i = 0; i < 16; i++) {
        dst[i] = float_to_bfloat16(src1[i]);
        dst[i+16] = float_to_bfloat16(src2[i]);
    }
}
```

### `vdpbf16ps`
vdpbf16psはbfloat16型のSIMDレジスタを2個とり、floatに変換してからそれらの積和をデスティネーションレジスタに足します。

Cでのコードは次の通りです。

```
void vdpbf16psC(float *dst, const bfloat16 *src1, const bfloat16 *src2)
{
    for (int i = 0; i < 16; i++) {
        dst[i] += bfloat16_to_float(src1[i*2+0]) * bfloat16_to_float(src2[i*2+0]);
        dst[i] += bfloat16_to_float(src1[i*2+1]) * bfloat16_to_float(src2[i*2+1]);
    }
}
```

```
dst[ 0] = float(src1[ 0]) * float(src2[ 0]) + float(src1[ 1]) * float(src2[ 1]);
dst[ 1] = float(src1[ 2]) * float(src2[ 2]) + float(src1[ 3]) * float(src2[ 3]);
dst[ 2] = float(src1[ 4]) * float(src2[ 4]) + float(src1[ 5]) * float(src2[ 5]);
...
dst[15] = float(src1[30]) * float(src2[30]) + float(src1[31]) * float(src2[31]);
```

## CPUの判別
`vpdpbusd`はCascade Lake以降、`vdpbf16ps`などの命令はCooper Lake以降でないと実行できません。
XbyakはCPU判別用のクラス`Xbyak::util::Cpu`を持っています。

CPUの利用フラグ表
命令|利用フラグ|対応CPU|
-|-|-|
`vpdpbusd`|`Xbyak::util::Cpu::tAVX512_VNNI`|Cascade Lake
`vdpbf16ps`|`Xbyak::util::Cpu::tAVX512_BF16`|Cooper Lake

具体的には次のコードで対応できないエラーにするか代替コードを記述します。

```
    Xbyak::util::Cpu cpu;
    if (!cpu.has(Xbyak::util::Cpu::tAVX512_VNNI)) {
        printf("AVX512_VNNI is not supported\n");
        return false;
    }
    if (!cpu.has(Xbyak::util::Cpu::tAVX512_BF16)) {
        printf("AVX512_BF16 is not supported\n");
        return false;
    }
```

## まとめ
第2世代Xeon SP(Cascade Lake)や次世代Xeon SP(Cooper Lake)で利用可能になった機械学習用の命令を紹介しました。
Intelの機械学習ライブラリ[oneDNN](https://github.com/oneapi-src/oneDNN)ではXbyakを用いてこれらの命令を利用しています。
自分でこれらの命令を直接記述することは少ないかもしれませんが、何かの参考になれば幸いです。
