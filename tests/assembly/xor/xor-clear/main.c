#include <stdio.h>
#include <unistd.h>
#include "common.h"

// TODO: add test for removing xorl

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));

  // b is always zero
  __asm("mov %1, %%eax\n"
      "xorl %1, %%eax\n"
      "mov %%eax, %0\n"
      : "=r"(b)
      : "r"(a)
      : "eax");
  if (b == 0 && a == 0xdeadbeef)
    good();
  else
    bad();
}
