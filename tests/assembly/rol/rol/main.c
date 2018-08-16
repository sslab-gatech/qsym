#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));

  __asm__("rol $0x8, %0\n"
    : "+r"(a)
    ::);

  if (a == 0xdeadbeef)
    good();
  else
    bad();
}
