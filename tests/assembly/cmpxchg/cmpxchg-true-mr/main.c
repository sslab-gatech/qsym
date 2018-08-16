#include "common.h"
#include <unistd.h>

int main() {
  int a, v;
  read(0, &a, sizeof(a));
  v = 0;

  // eax != r --> eax = r
  __asm__("mov $0, %%eax\n"
      "mov %1, %%ebx\n"
      "cmpxchg %%ebx, %0\n"
      : "=m"(v)
      : "r"(a)
      : "eax", "ebx");

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
