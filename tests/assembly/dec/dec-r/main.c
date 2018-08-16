#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));

  __asm("dec %1\n"
      "mov %1, %0\n"
      : "=m"(b)
      : "r"(a));
  if (b == 0xdeadbeef)
    good();
  else
    bad();
}
