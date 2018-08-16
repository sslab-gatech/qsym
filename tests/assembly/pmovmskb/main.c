#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <stdint.h>

int main() {
  __m128i a = _mm_setzero_si128();
  read(0, &a, sizeof(a));
  int b = _mm_movemask_epi8(a);
  if (b == 0xabcd)
    good();
  else
    bad();
}
