#include <stdio.h>
#include <stdint.h>

template<size_t WidthA, size_t WidthB>
  unsigned _ExtInt(WidthA + WidthB) lossless_mul(unsigned _ExtInt(WidthA) a, unsigned _ExtInt(WidthB) b) {
  return static_cast<unsigned _ExtInt(WidthA + WidthB)>(a)
       * static_cast<unsigned _ExtInt(WidthA + WidthB)>(b);
}


void mul256(uint64_t *pz, const uint64_t *px, const uint64_t *py)
{
  unsigned _ExtInt(256) x = *(unsigned _ExtInt(256)*)px;
  unsigned _ExtInt(256) y = *(unsigned _ExtInt(256)*)py;
  unsigned _ExtInt(512) z = lossless_mul<256, 256>(x, y);
  *(unsigned _ExtInt(512)*)pz = z;
}

void dump(const uint64_t *x, size_t n)
{
    printf("0x");
    for (size_t i = 0; i < n; i++) {
        printf("%016lx", x[n - 1 - i]);
    }
    printf("\n");
}

int main()
{
  uint64_t x[4] = { 1, 2, 3, 4 };
  uint64_t y[4] = { uint64_t(-1), uint64_t(-2), uint64_t(-3), uint64_t(-4) };
  uint64_t z[8];
  mul256(z, x, y);
  dump(x, 4);
  dump(y, 4);
  dump(z, 8);
}

