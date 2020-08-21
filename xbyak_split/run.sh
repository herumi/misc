#!/bin/sh
CFLAGS="-I./ -ftime-report"

for CXX in g++-8 clang++-10 ; do
  echo ${CXX}
  for OPT in -O0 -Ofast ; do
    echo ${OPT}
    for NAME in t1 t2 ; do
      echo ${NAME}
      for MODE in 0 1 ; do
        echo time ${CXX} ${CFLAGS} ${OPT} -DXBYAK_SPLIT=${MODE} -c ${NAME}.cpp \> result/${NAME}-${MODE}${OPT}-${CXX}.txt 2>&1
        time ${CXX} ${CFLAGS} ${OPT} -DXBYAK_SPLIT=${MODE} -c ${NAME}.cpp > result/${NAME}-${MODE}${OPT}-${CXX}.txt 2>&1
      done
    done
  done
done
