#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  uint64_t a, b;
  read(0, &a, sizeof(a));

  if ((a << 8) == 0xdeadbeef00)
    good();
  else
    bad();
}
