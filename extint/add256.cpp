#include <stdint.h>

void add256(uint64_t *pz, const uint64_t *px, const uint64_t *py)
{
  unsigned _ExtInt(256) x = *(unsigned _ExtInt(256)*)px;
  unsigned _ExtInt(256) y = *(unsigned _ExtInt(256)*)py;
  *(unsigned _ExtInt(256)*)pz = x + y;
}
