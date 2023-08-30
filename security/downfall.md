# [Downfall](https://downfall.page/)

## 概要
- メモリ最適化機能の脆弱性により、他のプログラムが保存したデータに信頼できないプログラムがアクセスできる脆弱性
- GDS(Gather Data Sampling)
  - gather命令を悪用して古い(steal)データを盗み取る手法
  - ユーザ・カーネル, プロセス, VM境界を越えて攻撃できる
  - 既存のCPU緩和策はSIMDレジスタバッファをフラッシュしないので有効ではない
  - 最新版のマイクロコードで対策
- 任意のデータを盗む
  - マスクビットがセットされたデータコピー命令は実行されない(no-opと同じ)だけどSIMDレジスタバッファにデータがフェッチされることが分かった
  - これを用いてGSDはアクセスされていないデータを漏洩させる
  - Meltdwn以来のLinux kernelからデータを盗む攻撃
- GVI(Gather Value Injection)
  - GDSとLVI(Hijacking transient execution through microarchitectural load value injection. IEEE(S&P), 2020)を組み合わせる
  - gatherデータ漏洩をマイクロアーキテクチャデータ注入に変えられる
  - 攻撃者がターゲットコードの中からgather命令を探してSpectreガジェットと似たデータ依存の操作をしてデータを盗む
- SGXも攻撃可能
- GDSの緩和策

### 脅威モデル(threat model)
攻撃者と被害者が同じコアで動いてる
- 同じコアの兄弟スレッドで同時実行
- 同じCPUスレッド上でのコンテキストスイッチ

## 過去の攻撃
### Spectre
- 制御フローまたはデータフロー予測の悪用
- 誤予測された命令を投機実行すると被害者の領域内にある境界外データアクセスが発生する場合がある
- 攻撃者からは制限されているけれども、キャッシュを観察することでデータ漏洩させることが可能だった
- 現在はknobつきソフトウェアをサポートすることで投機実行や分割予測を緩和できる
  - [Speculative Load Hardening(https://llvm.org/docs/SpeculativeLoadHardening.html)
### Meltdown
- ユーザプロセスの攻撃者がCPUのアクセス内制御をバイパスしてkernelからデータを漏洩させる
- Spectreと異なり別のアドレス空間のメモリに依存しない
- 脆弱なCPUはkernel dataを後続の命令に渡して一時的に攻撃者に公開してしまう
- [Speculative Execution and Indirect Branch Prediction Side Channel Analysis Method](https://www.intel.com/content/www/us/en/security-center/advisory/intel-sa-00088.html)
- [その他](https://www.intel.com/content/www/us/en/developer/topic-technology/software-security-guidance/processors-affected-consolidated-product-cpu-model.html#tab-blade-1-1)
### [MDS(Microarchitectural data sampling)](https://www.intel.com/content/www/us/en/developer/articles/technical/software-security-guidance/technical-documentation/intel-analysis-microarchitectural-data-sampling.html)
無効なメモリアクセスのあとの一時的な実行が意図せずに内部の一次バッファの内容を漏洩する場合がある
- SMTを止める
- 古いCPUはverw命令を使う
  - 指定された領域が現在の特権レベルで書き込み可能かどうかを調べる
- Intel CPU10世代以降は対策済

##  GDS
gather命令はSIMDに格納された複数のメモリアドレス(rax+zmm2)から読み出してzmm1に集約する命令
k1はマスクレジスタ
```
vgatherdd(zmm1|k1, ptr(rax+zmm2))
```
### マイクロアーキテクチャでの最適化
- マスクされたビットに対応するメモリは読まない
- 同じキャッシュにあるデータは再利用する
- 複数の読み取りを並列に投機実行して、少なくとも一つが失敗すると結果を破棄する
- 複数の読み取り中に中断があると、既に読まれたreadの再実行を避けるために部分実行された状態を保持する

- この中間状態のバッファの(古い)データにアクセスできるのか?
- 古いデータを後続の命令に渡せるのか?
- 古いデータが漏洩するとどんな影響があるのか?

### GDSの活用

攻撃者のコード
[asm.S](https://github.com/flowyroll/downfall/blob/main/POC/gds_aes_ni/asm.S#L92-L112)
```
// Step (i): Increase the transient window
// キャッシュをフラッシュして実行ウィンドウを大きくする
lea rdi, [addresses_normal]
clflush [rdi]
mov rax, [rdi]

// Step (ii): Gather uncacheable memory

vpxord ym1, ym1       # ym1=0
vpcmpeqb ym2, ym2 # ym2 = (-1)
vmovups ym3, [rdi]   # ym3 = *rdi;
vpxord ym4, ym4      # ym4 = 0

char address_buffer[4096];
lea rdi, [address_buffer]
flflush [rdi]
mov rax, [rdi] ; read
xchg [rdi], rax ; write
mov rdi, 0
mov rax, [rdi] ; page fault

// Step (iii): Encode (transient) data to cache
mov r13, 0
vpgatherdq ym4, [r13+xm1], ym2

ym4 -> rax
raxを8bit×8に分割して、それぞれの値vに対して
uint8_t oracle[8*4096*256];
oracle[v*256*4096]の値を読み込む
// Step (iv): Scan the cache
scan_flush_reload //
oracleを調べてraxがどの値だったかを調べる
```

- 犠牲者はStep (ii)と同等の見えるメモリをアクセス
- 攻撃者は兄弟CPUスレッドから見えた
- gather命令は兄弟CPUスレッドで一時バッファを共有しているように見える
- そのデータは後続の命令に転送され、同じコアの別のプロセスに属する

## 評価
影響を受ける命令を調べたところ多くの命令でデータ漏洩が発生することが分かった
- SIMDのread命令全般
- AES, SHA命令
- 高速メモリコピー命令
- メモリ間コピーmovdir64b
- マスクされた読み込まないmaskmov

## 対策
- SMT(ハイパースレッディング)を無効化する
- gatherのあとにlfenceを追加する
  - lfenceはこの命令より前に発行された全てのメモリloadを完了させる
  - Intelのマイクロコードはこの対策が入ったはず

- gather命令は通常コンパイラで勝手に使われることは無い
- HPCなど用の高度な計算ライブラリで使われることはある
