#include "common.h"
#include <string.h>
#include <unistd.h>
#include <tmmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  __m128i b = _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
  read(0, &a, sizeof(a));

  __m128i c = _mm_shuffle_epi8(a, b);
  if (*(uint64_t*)&c == 0xdeadbeefcafebabe)
    good();
  else
    bad();
}

