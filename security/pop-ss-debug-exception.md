# [POS SS/MOV SS Vulnerability](https://everdox.net/popss.pdf) 2018/5/8

## 概要
* [POP SS debug exception](https://access.redhat.com/security/vulnerabilities/pop_ss)
* OSの実装の不具合によりOSがクラッシュしたり秘密情報にアクセスされたりする可能性
* Windowsは任意コード実行の可能性

## 手順
* mov ssやpop ssはその次の命令境界まで割り込みやブレークポイントが禁止される。
* その次の命令がsyscall, sysenterなどの特権レベルのOS処理に制御を移すものだった場合、
デバッグ例外が発生するのはその命令を実行した後になる。
* その状況ではデバッグ例外の中で高い権限のデータにアクセスできる可能性がある。

## デバッグ
### int 3

プログラムのデバッグにおいて、ある処理のところで実行を一時停止したい場合、
デバッガはその場所にint 3命令(0xcc)を埋め込む。

するとプログラムが実行されてその場所に来たときに処理を中断して制御がデバッガに移る。

### デバッグレジスタ
DR0, ..., DR3(特権リソース)に処理を中断したいアドレスを設定すると、同様のことができる。


## Intel Software Developer's Manual Vol. 3

### 2.3 SYSTEM FLAGS AND FIELDS IN THE EFLAGS REGISTER
システムフラグEFLAGSはI/Oやハードウェア割り込みやデバッグなどを管理する。
特権コードしかこれらのフラグを変更できない。


### 6.8.3 Masking Exceptions and Interrupts When Switching Stacks

異なるstack segmentに切り替えるためには

```
mov ss, ax
mov esp, StackTop
```

というコードを実行する。ssが設定されてespが設定される前に割り込みや例外が発生するとスタック空間とアドレスが不整合を起こす。
これを避けるためmov ssやpop ssの次の命令の実行までは割り込みを禁止する仕様になっている。
この仕様はlss命令では起こらない。


## 攻撃コード
```
mov ss, [rax]
int 3
```

mov ssにハードウェアブレークポイントを置くと、mov ssと次のint 3が実行されたところでデバッガに制御が戻る。
Windowsではint 3のハンドラがkernelにあるのでそこは特権レベルにある。


### 17.3.1.1 Instruction-Breakpoint Exception Condition
ブレークポイントアドレスレジスタDR0, ..., DR3で指定されたアドレスを実行しようとするとインストラクションブレークポイントが報告される。
ブレークポイントのターゲット命令を実行する前に#DB(debug exception)があがる。

RF(resume flag)を設定すると#DBが発生せずに処理を継続する。

cf. [Windows Anti-Debug Reference](https://www.symantec.com/connect/articles/windows-anti-debug-reference)

```
push ss
; junk
pop ss
pushf
; junk
pop eax
and eax, 0x100
or eax, eax
jnz @debugged
; carry on normal execution
```

デバッガで動かしているとトラップフラグ設定を解除できないためデバッガを検知されてしまう。

* [Don't use IST entry for #BP stack](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=d8ba61ba58c88d5207c1ba2f7d9a2280e7d03be9)
