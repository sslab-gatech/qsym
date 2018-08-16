#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));

  if ((a << 1) == 0xdeadbeee)
    good();
  else
    bad();
}
