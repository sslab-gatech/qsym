#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  read(0, &a, sizeof(a));

  __m128i b = _mm_slli_epi32(a, 1);
  __m128i c = _mm_set_epi16(0x0, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80);
  __m128i neq = _mm_xor_si128(b, c);
  if (_mm_test_all_zeros(neq, neq))
    good();
  else
    bad();
}
