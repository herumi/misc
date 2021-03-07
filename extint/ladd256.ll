define void @add256(i64* nocapture %0, i64* nocapture readonly %1, i64* nocapture readonly %2) {
  %4 = bitcast i64* %1 to i256*
  %5 = load i256, i256* %4
  %6 = bitcast i64* %2 to i256*
  %7 = load i256, i256* %6
  %8 = add i256 %7, %5
  %9 = bitcast i64* %0 to i256*
  store i256 %8, i256* %9
  ret void
}

