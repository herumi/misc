define private i64 @mul32x32L(i32 %r2, i32 %r3)
{
%r4 = zext i32 %r2 to i64
%r5 = zext i32 %r3 to i64
%r6 = mul i64 %r4, %r5
ret i64 %r6
}
define i128 @__multi3(i64 %r2, i64 %r3)
{
%r4 = lshr i64 %r2, 32
%r5 = trunc i64 %r4 to i32
%r6 = trunc i64 %r2 to i32
%r7 = lshr i64 %r3, 32
%r8 = trunc i64 %r7 to i32
%r9 = trunc i64 %r3 to i32
%r10 = call i64 @mul32x32L(i32 %r5, i32 %r9)
%r11 = call i64 @mul32x32L(i32 %r6, i32 %r9)
%r12 = zext i64 %r11 to i96
%r13 = zext i64 %r10 to i96
%r14 = shl i96 %r13, 32
%r15 = add i96 %r14, %r12
%r16 = call i64 @mul32x32L(i32 %r5, i32 %r8)
%r17 = call i64 @mul32x32L(i32 %r6, i32 %r8)
%r18 = zext i64 %r17 to i96
%r19 = zext i64 %r16 to i96
%r20 = shl i96 %r19, 32
%r21 = add i96 %r20, %r18
%r22 = zext i96 %r15 to i128
%r23 = zext i96 %r21 to i128
%r24 = shl i128 %r23, 32
%r25 = add i128 %r24, %r22
ret i128 %r25
}
