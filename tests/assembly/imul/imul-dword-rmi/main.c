#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));

  if (a != 0) {
    __asm("imull $11, %1, %%eax\n"
        "mov %%eax, %0"
        : "=r"(b)
        : "m"(a)
        : "eax");
    if (b == 0xdeadbeef)
      good();
    else
      bad();
  }
  else
    bad();
}
