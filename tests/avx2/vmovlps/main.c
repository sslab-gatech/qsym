#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() { 
  __m128i a = _mm_setzero_si128();
  uint64_t b = 0;
  __m128i c = _mm_setzero_si128();
 
  read(0, &a, sizeof(a)); 
  read(0, &b, sizeof(b));
  read(0, &c, sizeof(c));

  __asm__( 
      "vmovlps %1, %0, %0" 
      : "+x"(a) 
      : "m"(b)
      :);

  __m128i neq = _mm_xor_si128(a, c);
  if (_mm_test_all_zeros(neq, neq))
    good();
  else
    bad();
}
