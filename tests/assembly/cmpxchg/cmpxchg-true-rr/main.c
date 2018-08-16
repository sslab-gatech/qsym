#include "common.h"
#include <unistd.h>

int main() {
  int a, v;
  read(0, &a, sizeof(a));

  // eax == dst -> dst = src
  __asm__("mov $0, %%eax\n"
      "mov $0, %%ebx\n"
      "mov %0, %%ecx\n"
      "cmpxchg %%ecx, %%ebx\n"
      "mov %%ebx, %0\n"
      : "=r"(v)
      : "r"(a)
      : "eax", "ebx", "ecx");

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
