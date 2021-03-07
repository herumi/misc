define void @mul256(i64* nocapture %0, i64* nocapture readonly %1, i64* nocapture readonly %2)  {
  %4 = bitcast i64* %1 to i256*
  %5 = load i256, i256* %4, align 8
  %6 = bitcast i64* %2 to i256*
  %7 = load i256, i256* %6, align 8
  %8 = zext i256 %5 to i512
  %9 = zext i256 %7 to i512
  %10 = mul nuw i512 %9, %8
  %11 = bitcast i64* %0 to i512*
  store i512 %10, i512* %11, align 8
  ret void
}

