#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b = 11, c;
  read(0, &a, sizeof(a));

  if (a != 0) {
    __asm("mov $0, %%edx\n"
        "mov %1, %%eax\n"
        "idivl %2\n"
        "mov %%eax, %0\n"
        : "=r"(c)
        : "r"(a), "m"(b)
        : "eax", "edx");
    if (c == 0x143e572d)
      good();
    else
      bad();
  }
  else
    bad();
}
