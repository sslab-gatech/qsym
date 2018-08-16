#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  __m128i b = _mm_setzero_si128();

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));
  __m128i c = _mm_unpacklo_epi32(a, b);
  if (*(uint64_t*)&c == 0xdeadbeefcafebabe)
    good();
  else
    bad();
}

