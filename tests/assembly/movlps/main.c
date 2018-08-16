#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128 a = _mm_setzero_ps();
  uint64_t b = 0;
  __m64 c = _mm_setzero_si64();

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  _mm_storel_pi(&c, a);

  if (b == *(uint64_t*)&c)
    good();
  else
    bad();
}
