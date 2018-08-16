#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128 a = _mm_setzero_ps();
  __m128 b = _mm_setzero_ps();
  __m128 c = _mm_setzero_ps();

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));
  read(0, &c, sizeof(c));

  __m128 d = _mm_move_ss(a, b);
  if (*(uint64_t*)&d == *(uint64_t*)&c)
    good();
  else
    bad();
}
