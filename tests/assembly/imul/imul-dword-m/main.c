#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b, c;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (a != 0 && b != 0) {
    __asm("mov %1, %%eax\n"
        "imull %2\n"
        "mov %%eax, %0\n"
        : "=r"(c)
        : "r"(a), "m"(b)
        : "eax");
    if (c == 0xdeadbeef)
      good();
    else
      bad();
  }
  else
    bad();
}
