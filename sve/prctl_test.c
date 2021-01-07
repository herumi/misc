#include <stdio.h>
#include <stdint.h>
#include <sys/prctl.h>
/*
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./a.out
sve len=00000040
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve2048=on ./a.out
sve len=00000100
*/
/*
use option -march=armv8.2-a+sve
*/

int
#if 0
#ifdef __clang__
__attribute__((optnone))
#else
__attribute__((optimize("O0")))
#endif
#endif
getLen()
{
  uint64_t x = 0;
  asm __volatile__("cntb %[x]" : [ x ] "=r"(x));
//  asm __volatile__(".inst 0x0420e3e0":"=r"(x));
  return (int)x;
}

int main()
{
  int x = prctl(PR_SVE_GET_VL);
  x = prctl(PR_SVE_GET_VL);
  printf("sve len=%08x\n", x);
  printf("sve len=%08x\n", getLen());
  return 0;
}


