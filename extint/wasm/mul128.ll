define void @mul128(i128* nocapture %pz, i64* nocapture readonly %px, i64* nocapture readonly %py)  {
  %x = load i64, i64* %px, align 8
  %y = load i64, i64* %py, align 8
  %xx = zext i64 %x to i128
  %yy = zext i64 %y to i128
  %zz = mul nuw i128 %xx, %yy
  store i128 %zz, i128* %pz, align 8
  ret void
}

