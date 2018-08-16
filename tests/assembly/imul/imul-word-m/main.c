#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned short a, b, c;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (a != 0 && b != 0) {
    __asm("mov %1, %%ax\n"
        "imulw %2\n"
        "mov %%ax, %0\n"
        : "=r"(c)
        : "r"(a), "m"(b)
        : "eax");
    if (c == 0xdead)
      good();
    else
      bad();
  }
  else
    bad();
}
