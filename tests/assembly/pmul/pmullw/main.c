#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  __m128i b = _mm_setzero_si128();
  __m128i c = _mm_setzero_si128();

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));
  read(0, &c, sizeof(c));

  __m128i d = _mm_mullo_epi16(a, b);
  __m128i neq = _mm_xor_si128(c, d);
  if (_mm_test_all_zeros(neq, neq))
    good();
  else
    bad();
}
