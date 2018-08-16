#include "common.h"
#include <unistd.h>

int main() {
  int a, v;
  read(0, &a, sizeof(a));

  // eax != r --> eax = r
  __asm__("mov %1, %%eax\n"
      "not %%eax\n" // never same
      "cmpxchg %%ebx, %1\n"
      "mov %%eax, %0\n"
      : "=r"(v)
      : "m"(a)
      : "eax", "ebx");

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
