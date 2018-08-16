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

  __m128i c = _mm_min_epu8(a, b);

  // Use 64bit integer comparison
  // not to rely on xmm register comparison
  if (*(uint64_t*)&c == 0xbadf00ddeadbeef)
    good();
  else
    bad();
}
