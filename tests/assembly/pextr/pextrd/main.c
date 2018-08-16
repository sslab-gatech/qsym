#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  int b = 0;

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (b == _mm_extract_epi32(a, 1))
    good();
  else
    bad();
}
