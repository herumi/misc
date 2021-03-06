#include <stdint.h>
typedef __attribute__((mode(TI))) unsigned int uint128;

void add128(uint64_t *pz, const uint64_t *px, const uint64_t *py)
{
  uint128 x = *(uint128*)px;
  uint128 y = *(uint128*)py;
  *(uint128*)pz = x + y;
}
