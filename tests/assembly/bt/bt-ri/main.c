#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a = 0;
  char b = 0;
  read(0, &a, sizeof(a));

  __asm__(
    "bt $0x8, %1\n"
    "setb %0\n"
    : "=r"(b)
    : "r"(a)
    :);

  if (b)
    good();
  else
    bad();
}
