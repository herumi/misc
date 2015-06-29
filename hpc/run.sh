rm -rf sample.sh.o*
FCCpx -Kfast -Krestp=all expf_test.cpp -I./
#FCCpx trismul.cpp
pjsub ./sample.sh
