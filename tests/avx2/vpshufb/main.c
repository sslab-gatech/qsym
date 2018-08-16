#include "common.h"
#include <string.h>
#include <unistd.h>
#include <immintrin.h>
#include <stdint.h>

int main() {
  __m256i a = _mm256_setzero_si256();
  __m256i b = _mm256_setzero_si256();
  __m256i c = _mm256_setzero_si256();

  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));
  read(0, &c, sizeof(c));

  __m256i d = _mm256_shuffle_epi8(a, b);
  __m256i neq = _mm256_xor_si256(c, d);
  if (_mm256_testz_si256(neq, neq))
    good();
  else
    bad();
}
