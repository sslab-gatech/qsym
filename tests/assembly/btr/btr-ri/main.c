#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  uint64_t a;
  char b = 0;
  read(0, &a, sizeof(a));

  __asm__(
    "btr $1, %0\n"
    : "+r"(a)
    :);

  if (a == 0xdeadbeed)
    good();
  else
    bad();
}
