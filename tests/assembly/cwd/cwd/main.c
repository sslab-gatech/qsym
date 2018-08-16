#include "common.h"
#include <unistd.h>
#include <stdint.h>

int main() {
  uintptr_t a = 0, b = 0, c = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

#ifdef __i386__
  __asm__(
      "mov %1, %%eax\n"
      "cwd\n"
      "mov %%edx, %0\n"
      : "=r"(c)
      : "r"(a)
      : "eax", "edx");
#else
  __asm__(
      "mov %1, %%rax\n"
      "cwd\n"
      "mov %%rdx, %0\n"
      : "=r"(c)
      : "r"(a)
      : "rax", "rdx");
#endif

  if (b == c)
    good();
  else
    bad();
}
