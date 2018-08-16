#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  uint64_t a = 0, b = 0, c = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

#ifdef __i386__
  __asm__(
      "cbw\n"
      "mov %%eax, %0\n"
      : "=r"(c)
      : "A"(a));
#else
  __asm__(
      "cbw\n"
      "mov %%rax, %0\n"
      : "=r"(c)
      : "A"(a));
#endif
  if (b == c)
    good();
  else
    bad();
}
