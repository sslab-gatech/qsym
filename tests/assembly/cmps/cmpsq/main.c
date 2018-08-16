#include "common.h"
#include <unistd.h>
#include <stdint.h>

#ifndef __x86_64__
  #error "Only for x86_64"
#endif

int main() {
  int64_t a, b, v = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "cmpsq\n"
      "jnz f%=\n"
      "movq $1, %0\n"
      "f%=:"
      : "=m"(v)
      : "S"(&a), "D"(&b));

  if (v)
    good();
  else
    bad();
}
