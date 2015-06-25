rm -rf sample.sh.o*
FCCpx -Kfast expf_test.cpp -I./
#FCCpx trismul.cpp
pjsub ./sample.sh
