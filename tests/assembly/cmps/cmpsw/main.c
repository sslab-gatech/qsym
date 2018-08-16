#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  uintptr_t a, b, v = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "cmpsw\n"
      "jnz f%=\n"
#ifdef __i386__
      "movl $1, %0\n"
#else
      "movq $1, %0\n"
#endif
      "f%=:"
      : "=m"(v)
      : "S"(&a), "D"(&b));

  if (v)
    good();
  else
    bad();
}
