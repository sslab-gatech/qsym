#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  uint64_t a[2];
  char b = 0;
  read(0, &a, sizeof(a));

  __asm__(
    "bt $65, %1\n"
    "setb %0\n"
    : "=r"(b)
    : "m"(a)
    :);

  if (b)
    good();
  else
    bad();
}
