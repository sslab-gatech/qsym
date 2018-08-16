#include "common.h"
#include <unistd.h>
#include <stdint.h>

#ifndef __x86_64__
  #error "Only for x86_64"
#endif

int main() {
  uint64_t a = 0, b = 0, c = 0;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__(
      "mov %1, %%rax\n"
      "cdq\n"
      "mov %%rdx, %0\n"
      : "=r"(c)
      : "r"(a)
      : "rax", "rdx");

  if (b == c)
    good();
  else
    bad();
}
