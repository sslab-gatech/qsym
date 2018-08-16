#include "common.h"
#include <unistd.h>

int main() {
  int a, v;
  read(0, &a, sizeof(a));
  v = 0;

  __asm__("mov $0, %%eax\n"
      "mov %0, %%ebx\n"
      "xchg %%eax, %%ebx\n"
      "mov %%eax, %0\n"
      : "=r"(v)
      : "r"(a)
      : "eax", "ebx");

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
