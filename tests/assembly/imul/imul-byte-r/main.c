#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned char a, b, c;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (a != 0 && b != 0) {
    __asm("mov %1, %%al\n"
        "imul %2\n"
        "mov %%al, %0\n"
        : "=r"(c)
        : "r"(a), "r"(b)
        : "eax");
    if (c == 0xde)
      good();
    else
      bad();
  }
  else
    bad();
}
