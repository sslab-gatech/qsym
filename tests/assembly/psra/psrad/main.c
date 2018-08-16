#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  read(0, &a, sizeof(a));
  __m128i b = _mm_srai_epi32(a, 1);
  if (*(uint64_t*)&b == 0xc0c1c2c3c4c5c6c7)
    good();
  else
    bad();
}

