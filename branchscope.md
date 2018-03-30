# BranchScope論文メモ

[BranchScope: A New Side-ChannelAttack on Directional Branch Predictor](http://www.cs.ucr.edu/~nael/pubs/asplos18.pdf)

# 注意：まだ肝心な部分が理解できてない
* そのうち更新するかも(ただ攻撃可能な条件が厳しすぎて通常は無理だろうと思ったのでもう掘り下げないかも)

# 概要

* BranchScope : 共有された分岐の方向予測(Directional branch predictor)を操作して任意の条件分岐の方向を推測するサイドチャネル攻撃
    * ここで分岐の方向というのは分岐しそう(Taken)・分岐しなさそう(Not taken)という予測のこと
* BTB(branch target buffer)を利用する既存の攻撃とは異なる
    * BTBは前回実行した条件分岐の結果を記録しておくバッファ
* ユーザ空間で実行可能
* エラー率は1%以下
* SGXに対する攻撃が可能?

# 分岐予測ユニットBPU(branch prediction units)
* BPU : 条件分岐を予測する
* 大きく2種類の要素からなる
    * L1(level-1)予測
        * PHT(pattern history table)2ビットの飽和カウンタ
        * プログラムカウンタ(pc)に紐付いた2モード予測
    * gshare形式 L2予測
        * 分岐アドレス以外の過去の情報を用いて予測
* 両方の予測を加味して分岐予測が行われる
    * 今回の攻撃はL1予測のみを利用する

# 攻撃モデル
## 登場人物
* victim(犠牲者)は秘密情報を持つ
* spy(攻撃者)はその秘密情報に直接アクセスせずに情報を盗み取る

## 条件
* victimとspyは同じ物理コアで動いてる
    * BPUを共有するため
* victimの実行速度は遅い
    * 高精度のbranchscope攻撃のためにはvictimのプログラムはゆっくり動かなければならない
    * それをするには別の攻撃(performance degradation攻撃など)で行う
    * スケジューラをいじる
* victimのコード実行のタイミングを制御できる

# 攻撃概要

* ステージ1 : PHTエントリーを初期化する
    * 攻撃者はPHTをある状態に持っていく
    * spyが後述するコードを実行する
* ステージ2 :victimの実行
    * 1個の条件分岐だけ(?)実行する
* ステージ3 : PHTエントリーを探る
    * spyはvictimが利用したPHTエントリーと同じ条件分岐を実行して観察する

# 攻撃可能性
* まず1-level予測しかさせないようにする
* どうやって?
* 分岐予測の実験をすると1回目は50%程度のミス率が3, 4回で10%にまで下がり5～7回でほぼ0%になる
* 初期状態はL2予測可能な情報が無いのでL1予測が使われる

```
randomize_pht:
  cmp %rcx, %rcx
  je .L0
  nop
.L0:
  jne .L1
  nop
.L1:
  je .L2
  ...
.L99999:
  nop
```
* jeとjneはランダムに選ぶ
* それを100000回繰り返す
* 実験の結果L2予測は使われなくなる

# L1予測の2bitFSM(finite state machine)

* T ; taken(分岐する)
* N ; not taken(分岐しない)
* H ; 分岐予測成功(hit)
* M ; 分岐予測失敗(miss)
* SN : strongly not taken ; 強く分岐しない
* WN : weakly not taken ; 弱く分岐しない
* ST : strongly taken ; 強く分岐する
* WT : weakly taken ; 弱く分岐する

```
   T   T   T
SN →WN→WT→ST
   ←  ←  ←
   N   N   N
```

Prime|Primeの後の状態|Target|Targetの後の状態|Probe|観察
-----|---------------|------|----------------|-----|-----
TTT  |ST             |T     |ST              |TT   |HH
TTT  |ST             |T     |ST              |NN   |MM
TTT  |ST             |N     |WT              |TT   |HH
TTT  |ST             |N     |WT              |NN   |MH
NNN  |SN             |T     |WN              |TT   |MH
NNN  |SN             |T     |WN              |NN   |HH
NNN  |SN             |N     |SN              |TT   |MM
NNN  |SN             |N     |SN              |NN   |HH

ProbeがTTのときHH, NNのときMMならTargetの後の状態はSTだったと予測できる

# 分岐予測の状態
* 分布を調べると結構ノイズが多い
* PHTのエントリーを詳しく調べるとアドレスが14ビット離れたものは同じのを利用している模様(下位14ビットをkeyにしている?)

# 実装
* victimのコード

```
int sec_data[] = { 1, 0, 1, ... };
void victim() {
  if (sec_data[i])
    asm("nop;nop");
  i++;
}

```

* spyのコード

```
int probe_array[] = { 1, 1 };
int main()
{
  randomize_pht();
  sleep(); // wait for victim
  spy(probe_arr);
}

void spy(int array[2]) {
  for (int i = 0; i < 2; i++) {
    int a = read_branch_mispred_counter();
    if (array[i])
      asm("nop;nop;nop");
    int b = read_branch_mispred_counter();
    store_branch_mispred_data(b - a);
  }
}
```

* spyはvictimのプロセスをコンテキストスイッチの間に条件分岐命令を一度だけ実行するように強制できるとする
    * SGXのシナリオではOSを制御できるのでその仮定はあり
* victimの条件分岐命令とspyの条件分岐命令の仮想アドレスを同じに設定する
    * victimとspyのPHTを衝突させるため
* spyはrandomize_pht()を実行しsleep()
* victim()が秘密情報に基づき一度だけ条件分岐を実行して(停止)
* spyは条件分岐カウンタを読み出してarray[]に基づいて分岐
* もう一度条件分岐カウンタを読み出し値が変わらないか1増えてるかを確認する

# 実験結果
* SkylakeとHaswellで1%未満のエラー率
* Sandy Bridgeで3～5%のエラー率

## タイムスタンプカウンタ
* 条件分岐カウンタにアクセスするには権限昇格が必要
* rdtsc(p)で代用する
    * 条件分岐ミスすると代替20数サイクル余計にかかるのでそれで判定可能
