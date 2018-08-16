#include "common.h"
#include <string.h>
#include <unistd.h>
#include <xmmintrin.h>
#include <stdint.h>

char TARGET[16] = "\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff";

#define TEST_PCMPEQ(INS) \
int main() { \
  __m128i a = _mm_setzero_si128(); \
  __m128i b = _mm_setzero_si128(); \
  uint64_t c[16]; \
 \
  read(0, &a, sizeof(a)); \
  memcpy(&b, TARGET, sizeof(b)); \
 \
  __asm__( \
      #INS " %2, %1\n" \
      "movdqa %1, %0" \
      : "=xm"(c) \
      : "x"(a), "x"(b)); \
 \
  if (c[0] == 0xffffffffffffffff) \
    good(); \
  else \
    bad(); \
}
