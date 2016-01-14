/*
fccpx -Kfast -S intpair.c
*/
typedef struct IntPair {
    int i, j;
} IntPair;
IntPair add(IntPair x, IntPair y) {
    return (IntPair){x.i + y.i, x.j + y.j};
}
IntPair addp(IntPair* x, IntPair* y) {
    return (IntPair){x->i + y->i, x->j + y->j};
}
void func(int *x, int *y) {
    IntPair a = {1, 2}, b = {3, 4};
    IntPair c = add(a, b);
    *x = c.i;
    *y = c.j;
}
void funcp(int *x, int *y) {
    IntPair a = {1, 2}, b = {3, 4};
    IntPair c = addp(&a, &b);
    *x = c.i;
    *y = c.j;
}

/*
func:
    save    %sp,-240,%sp
    mov     1,%g5
    mov     2,%o0
    stw     %g5,[%fp+2015]
    mov     3,%o1
    mov     4,%o2
    stw     %o0,[%fp+2019]
    mov     3,%o3
    mov     4,%o4
    mov     1,%g1
    mov     2,%g3
    stw     %o1,[%fp+2023]
    add     %g1,3,%g2
    add     %g3,4,%g4
    stw     %o2,[%fp+2027]
    stw     %o3,[%fp+2007]
    stw     %o4,[%fp+2011]
    stw     %g1,[%fp+1999]
    stw     %g3,[%fp+2003]
    stw     %g2,[%i0]
    stw     %g4,[%i1]
    ret
    restore


funcp:
    mov     4,%g1
    mov     6,%g2
    stw     %g1,[%o0]
    stw     %g2,[%o1]
    retl
    nop
*/
