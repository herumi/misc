#include <stdio.h>
#include <stdint.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
/*
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve512=on ./a.out
sve len=00000040
% env QEMU_LD_PREFIX=/usr/aarch64-linux-gnu qemu-aarch64 -cpu max,sve2048=on ./a.out
sve len=00000100
*/
/*
use option -march=armv8.2-a+sve
*/

/*
	FCC-1.0 does not support arm_sve.h
	FCC-1.1 supports it.
*/
#if defined(__ARM_FEATURE_SVE)
	#define USE_INLINE_SVE
#if ((defined(__GNUC__) && __GNUC__ >= 10) || (defined(__clang_major__) && __clang_major__ >= 11) || (defined(__FUJITSU) || defined(__CLANG_FUJITSU)))
	#define USE_INTRINSIC_SVE
#endif
#endif

#ifndef USE_INLINE_SVE
  #warning "use option -march=armv8.2-a+sve and more newer compiler"
#endif

#ifdef USE_INTRINSIC_SVE
#include <arm_sve.h>
#endif

int
#ifndef USE_INLINE_SVE
#ifdef __clang__
__attribute__((optnone))
#else
__attribute__((optimize("O0")))
#endif
#endif
getLen()
{
  uint64_t x = 0;
#if defined(USE_INTRINSIC_SVE)
  x = svcntb();
#elif defined(USE_INLINE_SVE)
  asm __volatile__("cntb %[x]" : [ x ] "=r"(x));
#else
  asm __volatile__(".inst 0x0420e3e0":"=r"(x));
#endif
  return (int)x;
}

int main()
{
#ifdef __ARM_FEATURE_SVE
  puts("__ARM_FEATURE_SVE is defined");
#else
  puts("__ARM_FEATURE_SVE is not defined");
#endif
#ifdef USE_INLINE_SVE
  puts("use inline sve");
#else
  puts("not use inline sve");
#endif
#ifdef __linux__
  int x = prctl(PR_SVE_GET_VL);
  x = prctl(PR_SVE_GET_VL);
  printf("sve len(prctl)     =%08x\n", x);
#endif
  printf("sve len(inline asm)=%08x\n", getLen());
#ifdef USE_INTRINSIC_SVE
  printf("sve len(intrinsic) =%08x\n", (int)svcntb());
#endif
  return 0;
}


