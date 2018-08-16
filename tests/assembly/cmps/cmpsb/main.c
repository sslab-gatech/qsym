#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  int64_t a, b, v = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "cmpsb\n"
      "jnz f%=\n"
#ifdef __x86_64__
      "movq $1, %0\n"
#else
      "movl $1, %0\n"
#endif
      "f%=:"
      : "=m"(v)
      : "S"(&a), "D"(&b));

  if (v)
    good();
  else
    bad();
}
