#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));

  if (a != 0) {
    __asm("movl %1, %0\n"
        "imull $11, %0\n"
        : "=r"(b)
        : "r"(a)
        : "eax");
    if (b == 0xdeadbeef)
      good();
    else
      bad();
  }
  else
    bad();
}
