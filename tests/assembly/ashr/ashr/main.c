#include <stdio.h>
#include <unistd.h>
#include "common.h"

int main(void)
{
  int a, b;
  read(0, &a, sizeof(a));
  read(0, &b, sizeof(b));

  if (b == 1 && ((a >> b) == 0xdeadbeef))
    good();
  else
    bad();
}
