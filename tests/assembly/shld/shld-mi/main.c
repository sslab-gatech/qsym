#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  __asm__("shldl $0x8, %1, %0\n"
    : "=m"(a)
    : "r"(b));

  if (a == 0xdeadbeef)
    good();
  else
    bad();
}
