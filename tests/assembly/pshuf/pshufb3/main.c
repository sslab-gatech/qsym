#include "common.h"
#include <string.h>
#include <unistd.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  __m128i b = _mm_set_epi8(0,1,2,3,4,5,6,0x80,0x80,15,14,13,12,11,10,9);
  read(0, &a, sizeof(a));

  __m128i c = _mm_shuffle_epi8(a, b);
  __m128i d = _mm_set_epi8(0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77);
  __m128i neq = _mm_xor_si128(c, d);
  if (_mm_test_all_zeros(neq, neq))
    good();
  else
    bad();
}

