# 生成コード
## x64

```
store0:
    movl    $5, (%rdi)
    ret

load0:
    movl    (%rdi), %eax
    ret

store1:
    movl    $3, (%rdi)
    mfence
    ret

load1:
    movl    (%rdi), %eax
    ret

store2:
    movl    $7, (%rdi)
    ret

load2:
    movl    (%rdi), %eax
    ret
```
## aarch64
```
store0:
    mov w1, 5
    str w1, [x0]
    ret

load0:
    ldr w0, [x0]
    ret

store1:
    mov w1, 3
    stlr    w1, [x0]
    ret

load1:
    ldar    w0, [x0]
    ret

store2:
    mov w1, 7
    stlr    w1, [x0]
    ret

load2:
    ldar    w0, [x0]
    ret
```
