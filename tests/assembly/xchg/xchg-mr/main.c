#include "common.h"
#include <unistd.h>

int main() {
  int a, v;
  read(0, &a, sizeof(a));
  v = 0;

  __asm__("mov $0, %%eax\n"
      "xchg %1, %%eax\n"
      "mov %%eax, %0\n"
      : "=r"(v)
      : "m"(a)
      : "eax");

  if (v == 0xdeadbeef)
    good();
  else
    bad();
}
