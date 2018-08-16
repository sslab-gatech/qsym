#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128d a = _mm_setzero_pd();
  uint64_t b = 0;
  double c = 0;

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  _mm_storeh_pd(&c, a);

  if (b == *(uint64_t*)&c)
    good();
  else
    bad();
}
