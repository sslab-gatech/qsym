#include "common.h"
#include <string.h>
#include <unistd.h>
#include <immintrin.h>
#include <stdint.h>

int main() {
  // 0, 3, 1, 2 == 54
  __m128i a = _mm_setzero_si128();
  read(0, &a, sizeof(a));

  __m128i b = _mm_shuffle_epi32(a, 54);
  __m128i c = _mm_set_epi8(0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe, 0xfe, 0xed, 0xfa, 0xce, 0xca, 0xfe, 0xbe, 0xef);
  __m128i neq = _mm_xor_si128(b, c);
  if (_mm_test_all_zeros(neq, neq))
    good();
  else
    bad();
}
