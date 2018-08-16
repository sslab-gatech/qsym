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
    "btc $65, %0\n"
    : "+m"(a)
    :);

  if (a[0] == 0xdeadbeef)
    good();
  else
    bad();
}
