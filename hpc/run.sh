rm sample.sh.o*
#FCCpx -Kfast trismul.cpp
FCCpx trismul.cpp
pjsub ./sample.sh
