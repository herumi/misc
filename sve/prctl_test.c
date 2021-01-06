#include <stdio.h>
#include <sys/prctl.h>
/*
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./a.out
sve len=00000040
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve2048=on ./a.out
sve len=00000100
*/

int main()
{
  int x = prctl(PR_SVE_GET_VL);
  x = prctl(PR_SVE_GET_VL);
  printf("sve len=%08x\n", x);
  return 0;
}


