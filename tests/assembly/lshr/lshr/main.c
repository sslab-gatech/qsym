#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  unsigned int a, b;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (b == 1 && ((a >> b) == 0x5eadbeef))
    good();
  else
    bad();
}
